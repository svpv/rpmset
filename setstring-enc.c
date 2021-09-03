#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "setstring.h"
#include "base64.h"
#include "base64pack.h"

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

// The same as dec_xblen.
static inline size_t enc_xblen(int m, unsigned kn, uint32_t v0, uint32_t vmax)
{
    unsigned n = kn - 1;
    size_t bits1 = n * (m + 1);
    size_t bits2 = (vmax - v0 - n) >> m;
    return bits1 + bits2;
}

#define popcnt32 __builtin_popcount
#define popcnt64 __builtin_popcountll

static inline size_t enc1(const uint32_t v[], size_t n, int bpp, int m, char *s,
	void (*pack)(const uint32_t *v, char *s, unsigned e),
	unsigned kn, unsigned kc, unsigned ke, unsigned ko, unsigned kq)
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
    unsigned dbfill = 0; // bfill from the decoder's perspective
    char *bput = NULL; // anchor to put kq characters

#define Put(s, b, kq)						\
    do {							\
	switch (kq) {						\
	case 10:s[9] = base64[(b>>54)&63]; /* FALLTHRU */	\
	case 9: s[8] = base64[(b>>48)&63]; /* FALLTHRU */	\
	case 8: s[7] = base64[(b>>42)&63]; /* FALLTHRU */	\
	case 7: s[6] = base64[(b>>36)&63]; /* FALLTHRU */	\
	case 6: s[5] = base64[(b>>30)&63]; /* FALLTHRU */	\
	case 5: s[4] = base64[(b>>24)&63]; /* FALLTHRU */	\
	case 4: s[3] = base64[(b>>18)&63]; /* FALLTHRU */	\
	case 3: s[2] = base64[(b>>12)&63]; /* FALLTHRU */	\
	default:s[1] = base64[(b>>06)&63];			\
		s[0] = base64[(b>>00)&63];			\
	}							\
    } while (0)

#define Flush1(kq)						\
    do {							\
	bput = bput ? bput : s;					\
	Put(bput, b, kq);					\
	if (bput == s)						\
	    s += kq, len -= kq;					\
	bput = NULL;						\
	b >>= 6 * kq, bfill -= 6 * kq;				\
    } while (0)

    while (len >= kc + ko && 6 * len + dbfill > 5 + enc_xblen(m, kn, v0, vmax)) {
	// Make a block of deltas.
	uint32_t dv[kn];
	for (unsigned i = 0; i < kn; i++) {
	    uint32_t v1 = v[i];
	    dv[i] = v1 - v0 - 1;
	    v0 = v1;
	}
	v += kn;
	// May need to collect ke extra bits for the block.
	unsigned i = 0;
	unsigned e = 0;
	for (unsigned efill = 0; efill < ke; i++) {
	    efill += dv[i] >> m;
	    if (efill >= ke) {
		bfill += efill - ke;
		// The stop bit is still pending.  Hence i is not increased,
		// and dv[i] is clobbered to obliterate its q-bits.
		dv[i] &= (1U << m) - 1;
		break;
	    }
	    e |= (1U << efill++);
	}
	// Put the block.
	pack(dv, s, e);
	s += kc, len -= kc;
	// Collect and flush the remaining q-bits into the bitsream.
	for (; i < kn; i++) {
	    bfill += dv[i] >> m;
	    while (bfill >= 6 * kq)
		Flush1(kq);
	    b |= (1ULL << bfill++);
	}
	if (bfill >= 6 * kq)
	    Flush1(kq);
	// If there are pending q-bits for the current block, the right place
	// to put them is here, before the next block.
	if (bfill && bput == NULL) {
	    if (len < kq)
		break;
	    bput = s;
	    s += kq, len -= kq;
	}
	// Update the decoder's view for the next iteration.
	dbfill = bfill ? 6 * kq - bfill : 0;
    }

#define Flush2							\
    do {							\
	if (bput) {						\
	    if (bfill < 6 * kq)					\
		break;						\
	    Flush1(kq);						\
	}							\
	while (bfill >= 6 * 2)					\
	    Flush1(2);						\
    } while (0)

    uint32_t rmask = (1U << m) - 1;
    while (v < v_end) {
	uint32_t v1 = *v++;
	uint32_t dv = v1 - v0 - 1;
	v0 = v1;
	bfill += dv >> m;
	Flush2;
	b |= (1ULL << bfill);
	bfill++;
	dv &= rmask;
	b |= (uint64_t) dv << bfill;
	bfill += m;
	if (bfill > 64) {
	    int left = bfill - 64;
	    bfill = 64;
	    Flush2;
	    dv >>= m - left;
	    b |= (uint64_t) dv << bfill;
	    bfill += left;
	}
    }

    if (bput) {
	assert(bfill > 6 * (kq - 1));
	Flush1(kq); // bfill goes negative
    }
    while ((int) bfill > 6)
	Flush1(2);
    if ((int) bfill > 0)
	Flush1(1);
    assert(len == 0);
    *s = '\0';
    return s - s_start;
}

#define Routine(pack, m, kn, kc, ke, ko, kq) \
static size_t encode##m(const uint32_t v[], size_t n, int bpp, char *s) \
{ return enc1(v, n, bpp, m, s, pack, kn, kc, ke, ko, kq); }

#define Routines \
    Routine(pack5x19c16e1,  5, 19, 16, 1, 0, 5) \
    Routine(pack6x16c16,    6, 16, 16, 0, 0, 5) \
    Routine(pack7x12c14,    7, 12, 14, 0, 0, 5) \
    Routine(pack8x12c16,    8, 12, 16, 0, 0, 5) \
    Routine(pack9x10c15,    9, 10, 15, 0, 0, 5) \
    Routine(pack10x9c15,   10,  9, 15, 0, 0, 5) \
    Routine(pack11x9c18e9, 11,  9, 18, 9, 0, 4) \
    Routine(pack12x16c32,  12, 16, 32, 0, 0,10) \
    Routine(pack13x6c13,   13,  6, 13, 0, 1, 5) \
    Routine(pack14x6c14,   14,  6, 14, 0, 0, 5) \
    Routine(pack15x6c15,   15,  6, 15, 0, 0, 5) \
    Routine(pack16x6c16,   16,  6, 16, 0, 0, 5) \
    Routine(pack17x6c17,   17,  6, 17, 0, 0, 5) \
    Routine(pack18x5c15,   18,  5, 15, 0, 0, 5) \
    Routine(pack19x5c16e1, 19,  5, 16, 1, 0, 5) \
    Routine(pack20x4c14e4, 20,  4, 14, 4, 0, 4) \
    Routine(pack21x4c14,   21,  4, 14, 0, 0, 5) \
    Routine(pack22x4c15e2, 22,  4, 15, 2, 0, 5) \
    Routine(pack23x4c16e4, 23,  4, 16, 4, 0, 4) \
    Routine(pack24x4c16,   24,  4, 16, 0, 0, 5) \
    Routine(pack25x4c17e2, 25,  4, 17, 2, 0, 5) \
    Routine(pack26x3c13,   26,  3, 13, 0, 1, 5) \
    Routine(pack27x3c14e3, 27,  3, 14, 3, 0, 5) \
    Routine(pack28x3c14,   28,  3, 14, 0, 0, 5) \
    Routine(pack29x3c15e3, 29,  3, 15, 3, 1, 5) \
    Routine(pack30x3c15,   30,  3, 15, 0, 1, 5) \

Routines

#undef Routine
#define Routine(pack, m, kn, kc, ke, ko, kq) encode##m,

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
