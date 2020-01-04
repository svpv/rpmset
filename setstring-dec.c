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

static inline size_t dec1(const char *s, size_t len, int bpp, int m, uint32_t v[],
	bool (*unpack)(const char *s, uint32_t *v, unsigned *e),
	unsigned kn, unsigned ks, unsigned ke, unsigned ko, unsigned kq)
{
    uint32_t v0 = (uint32_t) -1;
    uint32_t vmax = (bpp == 32) ? UINT32_MAX : (1U << bpp) - 1;
    uint32_t *v_start = v;
    uint64_t b = 0, b2 = 0; // variable-length bitstream
    unsigned bfill = 0, b2fill = 0;
    s += 2, len -= 2;
#define FILL(b, k)				\
    do {					\
	b = base64dec##k(s);			\
	if (unlikely((int32_t) b < 0))		\
	    return 0;				\
	b##fill = 6 * k;			\
	s += k, len -= k;			\
    } while (0)
    // Bulk decoding.
    while (len >= ks + ko && 6 * len > 66 + dec_xblen(m, kn, v0, vmax)) {
	// Decode a block of m-bit integers.
	unsigned e;
	bool ok = unpack(s, v, &e);
	if (unlikely(!ok))
	    return 0;
	if (ke) {
	    b |= e << bfill;
	    bfill += ke;
	}
	s += ks, len -= ks;
	// Read the q-bits from the bitstream.
	uint32_t *vend = v + (kn & ~1);
	do {
	    uint32_t z, q;
	    if (likely(b != 0))
		z = __builtin_ctz(b), q = z;
	    else {
		q = 0;
		do {
		    q += bfill;
		    if (unlikely(len < kq))
			return 0;
		    else
			switch (kq) {
			case 2: FILL(b, 2); break;
			case 3: FILL(b, 3); break;
			case 4: FILL(b, 4); break;
			case 5: FILL(b, 5); break;
			}
		} while (unlikely(b == 0));
		z = __builtin_ctz(b), q += z;
	    }
	    uint32_t z2, q2;
	    if (likely(b2 != 0))
		z2 = __builtin_ctz(b2), q2 = z2;
	    else {
		q2 = 0;
		do {
		    q2 += b2fill;
		    if (unlikely(len < kq))
			return 0;
		    else
			switch (kq) {
			case 2: FILL(b2, 2); break;
			case 3: FILL(b2, 3); break;
			case 4: FILL(b2, 4); break;
			case 5: FILL(b2, 5); break;
			}
		} while (unlikely(b2 == 0));
		z2 = __builtin_ctz(b2), q2 += z2;
	    }
	    z++, z2++;
	    b >>= z, bfill -= z;
	    b2 >>= z2, b2fill -= z2;
	    v0 += v[0] + (q << m) + 1;
	    v[0] = v0;
	    v0 += v[1] + (q2 << m) + 1;
	    v[1] = v0;
	    v += 2;
	} while (v < vend);
	if (kn % 2) {
	    uint32_t z, q;
	    if (likely(b != 0))
		z = __builtin_ctz(b), q = z;
	    else {
		q = 0;
		do {
		    q += bfill;
		    if (unlikely(len < kq))
			return 0;
		    else
			switch (kq) {
			case 2: FILL(b, 2); break;
			case 3: FILL(b, 3); break;
			case 4: FILL(b, 4); break;
			case 5: FILL(b, 5); break;
			}
		} while (unlikely(b == 0));
		z = __builtin_ctz(b), q += z;
	    }
	    z++;
	    b >>= z, bfill -= z;
	    v0 += *v + (q << m) + 1;
	    *v++ = v0;
	}
    }
    b |= b2 << bfill, bfill += b2fill;
    // Read the rest from the bitstream.
    uint32_t rmask = (1U << m) - 1;
    while (len || b) {
	uint32_t q = 0;
	while (b == 0) {
	    q += bfill;
	    if (likely(len > 1))
		FILL(b, 2);
	    else if (likely(len == 1))
		FILL(b, 1);
	    else
		return 0;
	}
	uint32_t z = __builtin_ctz(b);
	q += z++;
	b >>= z, bfill -= z;
	uint32_t r = b;
	int rfill = bfill, left;
	while ((left = rfill - m) < 0) {
	    if (likely(len > 1))
		FILL(b, 2);
	    else if (likely(len == 1))
		FILL(b, 1);
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

#define Routine(unpack, m, kn, ks, ke, ko, kq) \
static size_t decode##m(const char *s, size_t len, int bpp, uint32_t v[]) \
{ return dec1(s, len, bpp, m, v, unpack, kn, ks, ke, ko, kq); }

#define Routines \
    Routine(unpack5x19c16e1,    5, 19, 16, 1, 0, 5) \
    Routine(unpack6x16c16,      6, 16, 16, 0, 0, 5) \
    Routine(unpack7x12c14,      7, 12, 14, 0, 0, 5) \
    Routine(unpack8x12c16,      8, 12, 16, 0, 0, 5) \
    Routine(unpack9x10c15,      9, 10, 15, 0, 0, 5) \
    Routine(unpack10x9c15,     10,  9, 15, 0, 0, 5) \
    Routine(unpack11x8c15e2,   11,  8, 15, 2, 0, 5) \
    Routine(unpack12x8c16,     12,  8, 16, 0, 0, 5) \
    Routine(unpack13x6c13o1,   13,  6, 13, 0, 1, 5) \
    Routine(unpack14x6c14,     14,  6, 14, 0, 0, 5) \
    Routine(unpack15x6c15,     15,  6, 15, 0, 0, 5) \
    Routine(unpack16x6c16,     16,  6, 16, 0, 0, 5) \
    Routine(unpack17x6c17,     17,  6, 17, 0, 0, 5) \
    Routine(unpack18x5c15,     18,  5, 15, 0, 0, 5) \
    Routine(unpack19x5c16e1,   19,  5, 16, 1, 0, 5) \
    Routine(unpack20x6c20,     20,  6, 20, 0, 0, 5) \
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
#define Routine(unpack, m, kn, ks, ke, ko, kq) decode##m,
const setstring_decfunc_t setstring_dectab[26] = { Routines };
