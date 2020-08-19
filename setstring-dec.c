#include <stdbool.h>
#include "setstring.h"
#include "base64.h"

#define unlikely(x) __builtin_expect(x, 0)
#define likely(x) __builtin_expect(!!(x), 1)

size_t setstring_decinit(const char *s, size_t len, int *bpp)
{
    if (len < 3)
	return 0;
    *bpp = s[0] - 'a' + 7;
    if (*bpp < 7 || *bpp > 32)
	return 0;
    int m = s[1] - 'A' + 5;
    if (m < 5 || m > 30)
	return 0;
    if (m > *bpp)
	return 0;
    // Each character carries 6 bits.
    size_t bits = (len - 2) * 6;
    // Each (m + 1) bits can make a value.
    return bits / (m + 1);
}

// Decide whether to process a SIMD block (kn elements) or to fall
// back to a slower code path near the end of the string.
static inline size_t dec_xblen(int m, unsigned kn, uint32_t v0, uint32_t vmax)
{
    // We must not process a SIMD block if there is a possibility that
    // only (kn - 1) elements are left.  Therefore we estimate that the
    // remaining (kn - 1) elements take at most dec_xblen bits.
    unsigned n = kn - 1;
    size_t bits1 = n * (m + 1);
    // Ensure the remaining range can be covered by q-bits, cf. enc_maxbits.
    size_t bits2 = (vmax - v0 - n) >> m;
    return bits1 + bits2;
}

// On ARM, clz is cheaper than ctz (ctz compiles to rbit+clz).
#ifdef __aarch64__
#define BitCnt(x) __builtin_clz(x)
#define BitRev(x) asm("rbit %w0,%w0" : "+r" (x))
#define BitShift(x, k) x <<= k
#else
#define BitCnt(x) __builtin_ctz(x)
#define BitRev(x) (void)(x)
#define BitShift(x, k) x >>= k
#endif

#define ADD1 1

#define Fill(k)					\
    do {					\
	b = base64dec##k(s);			\
	if (unlikely((int32_t) b < 0))		\
	    return 0;				\
	bfill = 6 * k;				\
	s += k, len6 -= 6 * k;			\
    } while (0)

#define Refill					\
    do {					\
	if (unlikely(len6 < 6 * kq))		\
	    switch (len6) {			\
	    case 6 * 0: return 0;		\
	    case 6 * 1: Fill(1); break;		\
	    case 6 * 2: Fill(2); break;		\
	    case 6 * 3: Fill(3); break;		\
	    case 6 * 4: Fill(4); break;		\
	    }					\
	else					\
	    switch (kq) {			\
	    case 2: Fill(2); break;		\
	    case 3: Fill(3); break;		\
	    case 4: Fill(4); break;		\
	    case 5: Fill(5); break;		\
	    }					\
    } while (0)

// Q-delta loop iteration, opimized for fewer instructions.
#define IterB(i)				\
    do {					\
	uint32_t z, q;				\
	if (likely(b != 0)) {			\
	    z = BitCnt(b);			\
	    v0 += v[i];				\
	    q = z++;				\
	}					\
	else {					\
	    q = 0;				\
	    do {				\
		q += bfill;			\
		Refill;				\
	    } while (unlikely(b == 0));		\
	    BitRev(b);				\
	    z = BitCnt(b);			\
	    v0 += v[i];				\
	    q += z++;				\
	}					\
	BitShift(b, z);				\
	bfill -= z;				\
	v0 += (q << m) + ADD1;			\
	v[i] = v0;				\
    } while (0)

// On ARM, z = ctz32(b) and b >>= z are cheap instructions (1c latency).
// On x86, they are typically 3c+2c, so we need a special version optimized
// for latency: instead of z++, we issue two instructions b >>= 1 and
// bfill -= 1; the latency is reduced because z = ctz32(b) and b >>= 1
// can execute in parallel.
#ifdef __aarch64__
#define IterA(i) IterB(i)
#else
#define IterA(i)				\
    do {					\
	uint32_t z, q;				\
	if (likely(b != 0)) {			\
	    z = BitCnt(b);			\
	    BitShift(b, 1);			\
	    BitShift(b, z);			\
	    v0 += v[i];				\
	    bfill -= 1;				\
	    bfill -= z;				\
	    v0 += (z << m) + ADD1;		\
	}					\
	else {					\
	    q = 0;				\
	    do {				\
		q += bfill;			\
		Refill;				\
	    } while (unlikely(b == 0));		\
	    BitRev(b);				\
	    z = BitCnt(b);			\
	    BitShift(b, 1);			\
	    BitShift(b, z);			\
	    q += z;				\
	    v0 += v[i];				\
	    bfill -= 1;				\
	    bfill -= z;				\
	    v0 += (q << m) + ADD1;		\
	}					\
	v[i] = v0;				\
    } while (0)
#endif

static inline size_t dec1(const char *s, size_t len6, int bpp, int m, uint32_t v[],
	bool (*unpack)(const char *s, uint32_t *v, unsigned *e),
	unsigned kn, unsigned kc, unsigned ke, unsigned ko, unsigned kq)
{
    uint32_t v0 = (uint32_t) -1;
    uint32_t vmax = (bpp == 32) ? UINT32_MAX : (1U << bpp) - 1;
    uint32_t *v_start = v;
    uint32_t b = 0; // variable-length bitstream
    unsigned bfill = 0;
    len6 *= 6;
    s += 2, len6 -= 2 * 6;
    // Bulk decoding.
    while (len6 >= 6 * (kc + ko) && len6 + bfill > 5 + dec_xblen(m, kn, v0, vmax)) {
	// Decode a block of m-bit integers.
	unsigned e;
	bool ok = unpack(s, v, &e);
	if (unlikely(!ok))
	    return 0;
	if (ke) {
	    BitRev(b);
	    b = (b << ke) | e;
	    bfill += ke;
	    BitRev(b);
	}
	s += kc, len6 -= 6 * kc;
	// Read the q-bits from the bitstream.
	uint32_t *vend = v + (kn & ~1);
	do {
	    IterA(0);
	    IterA(1);
	    v += 2;
	} while (v < vend);
	if (kn & 1) {
	    IterB(0);
	    v++;
	}
    }
    BitRev(b);
    // Read the rest from the bitstream.
    uint32_t rmask = (1U << m) - 1;
    while (len6 || b) {
	uint32_t q = 0;
	while (b == 0) {
	    q += bfill;
	    if (likely(len6 > 6))
		Fill(2);
	    else if (likely(len6 == 6))
		Fill(1);
	    else
		return 0;
	}
	uint32_t z = __builtin_ctz(b);
	q += z++;
	b >>= z, bfill -= z;
	uint32_t r = b;
	int rfill = bfill, left;
	while ((left = rfill - m) < 0) {
	    if (likely(len6 > 6))
		Fill(2);
	    else if (likely(len6 == 6))
		Fill(1);
	    else
		return 0;
	    r |= b << rfill;
	    rfill += bfill;
	}
	b >>= bfill - left;
	bfill = left;
	v0 += (r & rmask) + (q << m) + 1;
	*v++ = v0;
    }
    if (b)
	return 0;
    return v - v_start;
}

#define Routine(unpack, m, kn, kc, ke, ko, kq) \
static size_t decode##m(const char *s, size_t len, int bpp, uint32_t v[]) \
{ return dec1(s, len, bpp, m, v, unpack, kn, kc, ke, ko, kq); }

#define Routines \
    Routine(unpack5x19c16e1,    5, 19, 16, 1, 0, 5) \
    Routine(unpack6x16c16,      6, 16, 16, 0, 0, 5) \
    Routine(unpack7x12c14,      7, 12, 14, 0, 0, 5) \
    Routine(unpack8x12c16,      8, 12, 16, 0, 0, 5) \
    Routine(unpack9x10c15,      9, 10, 15, 0, 0, 5) \
    Routine(unpack10x9c15,     10,  9, 15, 0, 0, 5) \
    Routine(unpack11x9c18e9,   11,  9, 18, 9, 0, 4) \
    Routine(unpack12x8c16,     12,  8, 16, 0, 0, 5) \
    Routine(unpack13x6c13o1,   13,  6, 13, 0, 1, 5) \
    Routine(unpack14x6c14,     14,  6, 14, 0, 0, 5) \
    Routine(unpack15x6c15,     15,  6, 15, 0, 0, 5) \
    Routine(unpack16x6c16,     16,  6, 16, 0, 0, 5) \
    Routine(unpack17x6c17,     17,  6, 17, 0, 0, 5) \
    Routine(unpack18x5c15,     18,  5, 15, 0, 0, 5) \
    Routine(unpack19x5c16e1,   19,  5, 16, 1, 0, 5) \
    Routine(unpack20x4c14e4,   20,  4, 14, 4, 0, 4) \
    Routine(unpack21x4c14,     21,  4, 14, 0, 0, 5) \
    Routine(unpack22x4c15e2,   22,  4, 15, 2, 0, 5) \
    Routine(unpack23x4c16e4,   23,  4, 16, 4, 0, 4) \
    Routine(unpack24x4c16,     24,  4, 16, 0, 0, 5) \
    Routine(unpack25x4c17e2,   25,  4, 17, 2, 0, 5) \
    Routine(unpack26x3c13o1,   26,  3, 13, 0, 1, 5) \
    Routine(unpack27x3c14e3,   27,  3, 14, 3, 0, 5) \
    Routine(unpack28x3c14,     28,  3, 14, 0, 0, 5) \
    Routine(unpack29x3c15e3o1, 29,  3, 15, 3, 1, 5) \
    Routine(unpack30x3c15o1,   30,  3, 15, 0, 1, 5) \

Routines

#undef Routine
#define Routine(unpack, m, kn, kc, ke, ko, kq) decode##m,
const setstring_decfunc_t setstring_dectab[26] = { Routines };
