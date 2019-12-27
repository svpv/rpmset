#include <stdlib.h>
#include <assert.h>
#include "setstring.h"
#include "base64.h"

static inline int log2i(uint32_t x)
{
#ifdef __LZCNT__
    return 31 - __builtin_clz(x);
#endif
    return 31 ^ __builtin_clz(x);
}

// Calculate m from dv, the average delta.
static inline int enc_m(const uint32_t v[], size_t n)
{
    // Each delta implies "+1", except for the first one.
    // Thus deltas add up to v[n-1] + (n - 1), not v[n-1].
    uint32_t dv = (v[n-1] - (n - 1)) / n;
    if (dv < 64)
	return 5;
    int m = log2i(dv);
    if (m > 30)
	return 30;
    // As shown in [Kiely 2004], generally there are 2 or 3 choices for the
    // optimal m, to be tried exhaustively.  However, when the source of deltas
    // is geometric (which is another way of saying that the hash values are
    // distributed uniformly), there exists a threshold for switching from m-1
    // to m, at around 2^m+2^{m-5}+2^{m-6} (this approximation is not in the
    // paper).  Still, there is a considerable variation around the threshold.
    // Therefore, in the vicinity of the threshold, we try m and m-1, whichever
    // yields the shorter string.  This is indicated by returning negative m.
    uint32_t hi = (1 << m) + (7 << (m - 6));
    return (dv < hi) ? -m : m;
}

// The maximum number of bits produced by the Golomb-Rice code.
static inline size_t enc_maxbits(const uint32_t *v, size_t n, int m)
{
    // Each value takes at least m bits + 1 stop bit.
    size_t bits1 = n * (m + 1);
    // In the worst case, m-bit values are small, and we have to expend extra
    // q-bits to cover the range up to v[n-1].  Each q-bit covers 2^m.
    size_t bits2 = (v[n-1] - (n - 1)) >> m;
    return bits1 + bits2;
}

size_t setstring_encinit(const uint32_t v[], size_t n, int bpp, int *m)
{
    if (bpp < 7 || bpp > 32)
	return 0;
    if (n == 0 || v[n-1] < n - 1)
	return 0;
    if (bpp < 32 && v[n-1] >> bpp)
	return 0;
    *m = enc_m(v, n);
    // It can be shown (by straightforward algebraic manipulation) that the
    // condition maxbits(m) >= maxbits(m - 1) is equivalent to m >= log2(dv).
    // Therefore, it suffices to evaluate enc_maxbits only once.
    size_t maxbits = enc_maxbits(v, n, abs(*m));
    return 2 + (maxbits + 5) / 6 + 1;
}

// Simulate the encoding to get the resulting string length (not including
// the leading bM characters).
static size_t enc_dryrun(const uint32_t v[], size_t n, int m)
{
    size_t bits = n * (m + 1);
    const uint32_t *v_end = v + n;
    uint32_t v0, v1, dv;
    v0 = dv = *v++;
    while (1) {
	bits += dv >> m;
	if (v == v_end)
	    break;
	v1 = *v++;
	if (v0 >= v1)
	    return 0;
	dv = v1 - v0 - 1;
	v0 = v1;
    }
    return (bits + 5) / 6;
}

// The same as dec_xlen.
static inline size_t enc_xlen(int m, unsigned kn, uint32_t v0, uint32_t vmax)
{
    unsigned n = kn - 1;
    size_t bits1 = n * (m + 1);
    size_t bits2 = (vmax - v0 - n) >> m;
    return (bits1 + bits2 + 5) / 6;
}

// To interleave SIMD blocks and the q-bitstream properly, we maintain
// a small queue of delta blocks.
struct Q {
    unsigned start, end;
#define QN 256
    uint32_t v[QN];
};

#define Q_init(Q) (Q)->start = (Q)->end = 0
#define Q_empty(Q) ((Q)->start == (Q)->end)

static inline unsigned Q_nblock(struct Q *Q, unsigned n)
{
    unsigned k = Q->end - Q->start;
    k += (Q->end < Q->start) * QN;
    return k / n;
}

static inline uint32_t *Q_push(struct Q *Q, unsigned n)
{
    uint32_t *v = Q->v + Q->end;
    Q->end += n;
    if (Q->end > QN) {
	v = Q->v;
	Q->end = n;
	assert(Q->end <= Q->start);
    }
    return v;
}

static inline uint32_t *Q_pop(struct Q *Q, unsigned n)
{
    uint32_t *v = Q->v + Q->start;
    Q->start += n;
    if (Q->start > QN) {
	v = Q->v;
	Q->start = n;
    }
    return v;
}

static inline size_t enc1(const uint32_t v[], size_t n, int bpp, int m, char *s,
	void (*pack)(const uint32_t *v, char *s, unsigned e),
	unsigned kn, unsigned ks, unsigned ke, unsigned ko)
{
    size_t len = enc_dryrun(v, n, m);
    if (len == 0)
	return 0;
    const uint32_t *v_end = v + n;
    const char *s_start = s;
    *s++ = bpp - 7 + 'a';
    *s++ = m - 5 + 'A';
    uint32_t v0 = (uint32_t) -1;
    uint32_t vmax = (bpp == 32) ? UINT32_MAX : (1U << bpp) - 1;
    uint64_t b = 0;
    unsigned bfill = 0;
    unsigned bcnt = 0; // how many q-parts in b
    struct Q Q; Q_init(&Q);
    int bal = 0; // balance between SIMD blocks and the q-bitstream
    unsigned ctl = 0; // loop control correction, see below

#define FLUSH1							\
    do {							\
	while (bal >= 0 && bfill >= ke && !Q_empty(&Q)) {	\
	    uint32_t *odv = Q_pop(&Q, kn);			\
	    pack(odv, s, b);					\
	    s += ks;						\
	    if (ke) {						\
		b >>= ke, bfill -= ke;				\
		bal += bcnt, bcnt = 0;				\
	    }							\
	    bal -= kn;						\
	}							\
	if (len >= 3 && bfill >= 18) {				\
	    s[0] = base64[(b>>00)&63];				\
	    s[1] = base64[(b>>06)&63];				\
	    s[2] = base64[(b>>12)&63];				\
	    b >>= 18, bfill -= 18;				\
	    s += 3, len -= 3;					\
	    bal += bcnt, bcnt = 0;				\
	    continue;						\
	}							\
	break;							\
    } while (1)

    while (len - ctl > enc_xlen(m, kn, v0, vmax) && len - ctl >= ks + ko) {
	// Make a block of deltas.
	uint32_t *dv = Q_push(&Q, kn);
	for (unsigned i = 0; i < kn; i++) {
	    uint32_t v1 = v[i];
	    dv[i] = v1 - v0 - 1;
	    v0 = v1;
	}
	v += kn;
	len -= ks; // not yet written, but must be accounted for
	// Write the bitstream and flush the blocks along the way.
	for (unsigned i = 0; i < kn; i++) {
	    bfill += dv[i] >> m;
	    FLUSH1;
	    b |= (1U << bfill);
	    bfill++, bcnt++;
	    FLUSH1;
	}
	// The condition in the loop control must be the same as in the
	// decoder.  In the decoder, the condition is checked after all
	// the necessary q-bits from the previous iteration have been read.
	// Therefore, we must account for the pending q-bits.
	ctl = (bfill > ke * Q_nblock(&Q, kn)) ? (len < 3 ? len : 3) : 0;
    }

    assert(bal + bcnt - Q_nblock(&Q, kn) * kn == 0);
    if (Q_empty(&Q))
	bal = 0;

#define FLUSH2 \
    do {							\
	if (bal < 0) {						\
	    if (bfill < 18)					\
		break;						\
	    s[0] = base64[(b>>00)&63];				\
	    s[1] = base64[(b>>06)&63];				\
	    s[2] = base64[(b>>12)&63];				\
	    b >>= 18, bfill -= 18;				\
	    s += 3, len -= 3;					\
	    bal = 0;						\
	}							\
	while (bfill >= ke && !Q_empty(&Q)) {			\
	    uint32_t *odv = Q_pop(&Q, kn);			\
	    pack(odv, s, b);					\
	    s += ks;						\
	    b >>= ke, bfill -= ke;				\
	}							\
	while (bfill >= 18 && Q_empty(&Q)) {			\
	    s[0] = base64[(b>>00)&63];				\
	    s[1] = base64[(b>>06)&63];				\
	    s[2] = base64[(b>>12)&63];				\
	    b >>= 18, bfill -= 18;				\
	    s += 3, len -= 3;					\
	}							\
    } while (0)

    uint32_t rmask = (1U << m) - 1;
    while (v < v_end) {
	uint32_t v1 = *v++;
	uint32_t dv = v1 - v0 - 1;
	v0 = v1;
	bfill += dv >> m;
	FLUSH2;
	b |= (1U << bfill);
	bfill++;
	dv &= rmask;
	b |= (uint64_t) dv << bfill;
	bfill += m;
	FLUSH2;
    }

    if (!Q_empty(&Q)) {
	if (bal < 0) {
	    assert(bfill > 12);
	    s[0] = base64[(b>>00)&63];
	    s[1] = base64[(b>>06)&63];
	    s[2] = base64[(b>>12)&63];
	    s += 3, len -= 3;
	    b = 0, bfill = 0;
	    bal = 0;
	}
	do {
	    uint32_t *odv = Q_pop(&Q, kn);
	    pack(odv, s, b);
	    s += ks;
	    b >>= ke, bfill -= ke;
	    if ((int) bfill < 0) bfill = 0;
	} while (!Q_empty(&Q));
    }
    if (bfill > 00) *s++ = base64[(b>>00)&63], len--;
    if (bfill > 06) *s++ = base64[(b>>06)&63], len--;
    if (bfill > 12) *s++ = base64[(b>>12)&63], len--;
    assert(len == 0);
    *s = '\0';
    return s - s_start;
}

#include "base64pack.h"

#define Routine(pack, m, kn, ks, ke, ko) \
static size_t encode##m(const uint32_t v[], size_t n, int bpp, char *s) \
{ return enc1(v, n, bpp, m, s, pack, kn, ks, ke, ko); }

#define Routines \
    Routine(pack5x19c16e1,  5, 19, 16, 1, 0) \
    Routine(pack6x16c16,    6, 16, 16, 0, 0) \
    Routine(pack7x12c14,    7, 12, 14, 0, 0) \
    Routine(pack8x12c16,    8, 12, 16, 0, 0) \
    Routine(pack9x10c15,    9, 10, 15, 0, 0) \
    Routine(pack10x9c15,   10,  9, 15, 0, 0) \
    Routine(pack11x8c15e2, 11,  8, 15, 2, 0) \
    Routine(pack12x8c16,   12,  8, 16, 0, 0) \
    Routine(pack13x6c13,   13,  6, 13, 0, 1) \
    Routine(pack14x6c14,   14,  6, 14, 0, 0) \
    Routine(pack15x6c15,   15,  6, 15, 0, 0) \
    Routine(pack16x6c16,   16,  6, 16, 0, 0) \
    Routine(pack17x6c17,   17,  6, 17, 0, 0) \
    Routine(pack18x5c15,   18,  5, 15, 0, 0) \
    Routine(pack19x5c16e1, 19,  5, 16, 1, 0) \
    Routine(pack20x4c14e4, 20,  4, 14, 4, 0) \
    Routine(pack21x4c14,   21,  4, 14, 0, 0) \
    Routine(pack22x4c15e2, 22,  4, 15, 2, 0) \
    Routine(pack23x4c16e4, 23,  4, 16, 4, 0) \
    Routine(pack24x4c16,   24,  4, 16, 0, 0) \
    Routine(pack25x4c17e2, 25,  4, 17, 2, 0) \
    Routine(pack26x3c13,   26,  3, 13, 0, 1) \
    Routine(pack27x3c14e3, 27,  3, 14, 3, 0) \
    Routine(pack28x3c14,   28,  3, 14, 0, 0) \
    Routine(pack29x3c15e3, 29,  3, 15, 3, 1) \
    Routine(pack30x3c15,   30,  3, 15, 0, 1) \

Routines

#undef Routine
#define Routine(pack, m, kn, ks, ke, ko) encode##m,

typedef size_t (*encfunc_t)(const uint32_t v[], size_t n, int bpp, char *s);
static const encfunc_t enctab[26] = { Routines };

size_t setstring_encode(const uint32_t v[], size_t n, int bpp, int m, char *s)
{
    if (m >= 0)
	return enctab[m-5](v, n, bpp, s);
    // In case of a tie, prefer higher m.  This will improve the decoder's
    // memory footprint: estimating that each (m + 1) bits can make a value,
    // the decoder will request smaller chunks.
    size_t len1 = enctab[-m-5-1](v, n, bpp, s);
    if (len1 == 0)
	return 0;
    size_t len2 = enctab[-m-5-0](v, n, bpp, s);
    assert(len2);
    if (len2 <= len1)
	return len2;
    return enctab[-m-5-1](v, n, bpp, s);
}
