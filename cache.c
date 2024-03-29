#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#ifdef __SSE2__
#include <emmintrin.h>
#endif
#include "setstring.h"
#include "rpmsetcmp-common.h"

#define CACHE_SIZE (256 - 1)

struct cache {
    // Hash values, for linear search with a sentinel;
    // hv[CACHE_SIZE] is the number of entries in ev[].
    uint16_t hv[CACHE_SIZE+1];
    // Malloc'd cache entries.
    struct cache_ent *ev[CACHE_SIZE];
};

// The cache object is thread-local: it is created when a thread first calls
// cache_decode, and purged when the thread exits.

static void cache_free(void *arg)
{
    struct cache *C = arg;
    size_t nent = C->hv[CACHE_SIZE];
    struct cache_ent **ep = C->ev;
    struct cache_ent **ev_end = C->ev + nent;
    while (ep < ev_end)
	free(*ep++);
    free(C);
}

static pthread_key_t cache_key;

static __attribute__((constructor)) void cache_init(void)
{
    int rc = pthread_key_create(&cache_key, cache_free);
    assert(rc == 0);
}

static struct cache *cache_tlsobj(void)
{
    struct cache *C = pthread_getspecific(cache_key);
    if (likely(C))
	return C;
    long align = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (align != 32 && align != 128)
	align = 64;
    C = aligned_alloc(align, sizeof *C);
    assert(C);
    memset(C, 0, sizeof *C);
    int rc = pthread_setspecific(cache_key, C);
    assert(rc == 0);
    return C;
}

// To find a cache entry corresponding to a set-string, we hash a few bytes
// near the beginning of the string and run a linear search with a sentinel.

static inline unsigned hash16(const char *s, size_t len)
{
    uint32_t h;
    memcpy(&h, s + 4, 4);
    h *= 668265263U + 2 * len;
    return h >> 16;
}

static uint16_t *find16(uint16_t *hp, unsigned h)
{
#ifdef __SSE2__
    unsigned mask;
    __m128i xmm0 = _mm_set1_epi16(h);
    do {
	__m128i xmm1 = _mm_loadu_si128((void *)(hp + 0));
	__m128i xmm2 = _mm_loadu_si128((void *)(hp + 8));
	__m128i xmm3 = _mm_loadu_si128((void *)(hp + 16));
	__m128i xmm4 = _mm_loadu_si128((void *)(hp + 24));
	hp += 32;
	xmm1 = _mm_cmpeq_epi16(xmm1, xmm0);
	xmm2 = _mm_cmpeq_epi16(xmm2, xmm0);
	xmm3 = _mm_cmpeq_epi16(xmm3, xmm0);
	xmm4 = _mm_cmpeq_epi16(xmm4, xmm0);
	xmm1 = _mm_packs_epi16(xmm1, xmm2);
	xmm3 = _mm_packs_epi16(xmm3, xmm4);
	mask = _mm_movemask_epi8(xmm1);
	mask |= (unsigned) _mm_movemask_epi8(xmm3) << 16;
    } while (mask == 0);
    hp -= 32;
    hp += __builtin_ctz(mask);
    return hp;
#else
    while (1) {
	if (unlikely(hp[0] == h)) return hp + 0;
	if (unlikely(hp[1] == h)) return hp + 1;
	if (unlikely(hp[2] == h)) return hp + 2;
	if (unlikely(hp[3] == h)) return hp + 3;
	hp += 4;
    }
#endif
}

/* Our cache replacement policy is similar to LRU.  When an item x is accessed,
 * it has to be moved to front.  Instead of moving it all the way to the front
 * though, we only promote it by a fixed amount, as shown in (a).  To this end
 * we devise a SIMD-friendly primitive which moves items to the right.
 *
 * This very same primitive is employed when a new items y gets inserted.
 * The arrangement at the end of array is shown in (b): z is the last element
 * and the victim, so it has to be freed before the move.
 *
 *     -+---+---+---+---+---+-           -+---+---+---+---+---+
 *      | a | b | c | d | x |->-,         | a | b | c | d | z |->
 *     -+---+---+---+---+---+-  :        -+---+---+---+---+---+
 *       \               \      :          \               \
 *        \     move      \     :           \     move      \
 *         \               \    v            \               \
 *     -+---+---+---+---+---+-  :        -+---+---+---+---+---+
 *      | x | a | b | c | d |   :         | y | a | b | c | d |
 *     -+---+---+---+---+---+-  :        -+---+---+---+---+---+
 *        ^                     :           ^
 *        `- - - - - - - - - - -'           '
 *            (a) update                       (b) insert
 */

#define MOVE_SIZE 16
#define INSERT_AT (CACHE_SIZE - MOVE_SIZE - 1)

#ifdef __SSE2__
static inline void memmove32(void *dst, const void *src)
{
    __m128 *q = dst;
    const __m128 *p = src;
    // Compared to movdqu, movups is encoded with one fewer byte.
    __m128 xmm0 = _mm_loadu_ps((void *)(p + 0));
    __m128 xmm1 = _mm_loadu_ps((void *)(p + 1));
    _mm_storeu_ps((void *)(q + 0), xmm0);
    _mm_storeu_ps((void *)(q + 1), xmm1);
}

static inline void memmove64(void *dst, const void *src)
{
    __m128 *q = dst;
    const __m128 *p = src;
    __m128 xmm0 = _mm_loadu_ps((void *)(p + 0));
    __m128 xmm1 = _mm_loadu_ps((void *)(p + 1));
    __m128 xmm2 = _mm_loadu_ps((void *)(p + 2));
    __m128 xmm3 = _mm_loadu_ps((void *)(p + 3));
    _mm_storeu_ps((void *)(q + 0), xmm0);
    _mm_storeu_ps((void *)(q + 1), xmm1);
    _mm_storeu_ps((void *)(q + 2), xmm2);
    _mm_storeu_ps((void *)(q + 3), xmm3);
}
#endif

static void cache_move(uint16_t *hp, struct cache_ent **ep)
{
#if defined(__SSE2__) && MOVE_SIZE == 16
    memmove32(hp + 1, hp);
    if (sizeof *ep > 4)
	memmove64(ep + 9, ep + 8);
    memmove64(ep +  1, ep +  0);
#else
    memmove(hp + 1, hp, MOVE_SIZE * sizeof *hp);
    memmove(ep + 1, ep, MOVE_SIZE * sizeof *ep);
#endif
}

// Putin it all together.

struct cache_ent {
    // The number of decoded values.
    unsigned n;
    // The original string, null-terminated.
    char s[];
    // Followed by uint32_t v[n], aligned to a 4-byte boundary.
#define ENT_STRSIZE(len) ((len + 1 + 3) & ~3)
#define ENT_V(e, len) ((uint32_t *)(e->s + ENT_STRSIZE(len)))
    // Followed by a few UINT32_MAX sentinels.
};

size_t cache_decode(const char *s, size_t len, const uint32_t **pv)
{
    struct cache *C = cache_tlsobj();
    unsigned h = hash16(s, len);
    size_t i; // entry index
    size_t nent = C->hv[CACHE_SIZE];
    struct cache_ent *e;
    uint16_t *hp = C->hv;
    while (1) {
	// Install the sentinel (may clobber CACHE_SIZE).
	C->hv[nent] = h;
	// Find by hash.
	hp = find16(hp, h);
	C->hv[CACHE_SIZE] = nent; // restore if clobbered
	i = hp - C->hv;
	// Found the sentinel?
	if (unlikely(i == nent))
	    break;
	// Found an entry.
	e = C->ev[i];
	// Recheck the entry.
	if (unlikely(memcmp(e->s, s, len) || e->s[len])) {
	    hp++;
	    continue;
	}
	// Hit, move to front.
	if (i >= MOVE_SIZE) {
	    i -= MOVE_SIZE;
	    cache_move(C->hv + i, C->ev + i);
	    C->hv[i] = h, C->ev[i] = e;
	}
	*pv = ENT_V(e, len);
	return e->n;
    }
    // Miss, decode.
    int bpp;
    size_t n = setstring_decinit(s, len, &bpp);
    if (unlikely(n == 0))
	return 0;
    e = malloc(sizeof(*e) + ENT_STRSIZE(len) + (n + SENTINELS) * 4);
    assert(e);
    uint32_t *v = ENT_V(e, len);
    n = setstring_decode(s, len, bpp, v);
    if (unlikely(n == 0))
	return free(e), 0;
    e->n = n;
    install_sentinels(v, n);
    memset(v - 1, 0, 4);
    memcpy(e->s, s, len);
    // Insert.
    if (unlikely(nent <= INSERT_AT))
	i = nent, C->hv[CACHE_SIZE] = ++nent;
    else {
	if (unlikely(nent < CACHE_SIZE))
	    C->hv[CACHE_SIZE] = ++nent;
	else
	    free(C->ev[CACHE_SIZE-1]);
	i = INSERT_AT;
	cache_move(C->hv + i, C->ev + i);
    }
    C->hv[i] = h, C->ev[i] = e;
    *pv = v;
    return n;
}
