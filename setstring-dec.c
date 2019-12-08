#include <stdbool.h>
#include "setstring.h"
#include "base64.h"

#define unlikely(x) __builtin_expect(x, 0)

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
static inline size_t dec_xlen(int m, unsigned kn, uint32_t v0, uint32_t vmax)
{
    // We must not process a SIMD block if there is a possibility that
    // only (kn - 1) elements are left.  Therefore we estimate that the
    // remaining (kn - 1) elements take at most dec_xlen characters.
    unsigned n = kn - 1;
    size_t bits1 = n * (m + 1);
    // Ensure the remaining range can be covered by q-bits, cf. enc_maxbits.
    size_t bits2 = (vmax - v0 - n) >> m;
    return (bits1 + bits2 + 5) / 6;
}

static inline size_t dec1(const char *s, size_t len, int bpp, int m, uint32_t v[],
	bool (*unpack)(const char *s, uint32_t *v, unsigned *e),
	unsigned kn, unsigned ks, unsigned ke, unsigned ko)
{
    uint32_t v0 = (uint32_t) -1;
    uint32_t vmax = (bpp == 32) ? UINT32_MAX : (1U << bpp) - 1;
    uint32_t *v_start = v;
    uint32_t b = 0; // variable-length bitstream
    unsigned bfill = 0;
    s += 2, len -= 2;
    // Bulk decoding.
    while (len > dec_xlen(m, kn, v0, vmax)) {
	if (unlikely(len < ks))
	    return 0;
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
	for (unsigned i = 0; i < kn; i++) {
	    uint32_t q = 0;
	    while (b == 0) {
		q += bfill;
		if (unlikely(len <= 1)) {
		    if (unlikely(len < 1))
			return 0;
		    b = base64dec1(s);
		    if (unlikely((int32_t) b < 0))
			return 0;
		    bfill = 6, s++, len--;
		}
		else {
		    b = base64dec2(s);
		    if (unlikely((int32_t) b < 0))
			return 0;
		    bfill = 12, s += 2, len -= 2;
		}
	    }
	    uint32_t z = __builtin_clz(b);
	    q += z++;
	    b >>= z, bfill -= z;
	    v0 += v[i] + (q << m) + 1;
	    v[i] = v0;
	}
	v += kn;
    }
    // Read the rest from the bitstream.
    uint32_t rmask = (1U << m) - 1;
    while (len) {
	uint32_t q = 0;
	while (b == 0) {
	    q += bfill;
	    if (unlikely(len == 0))
		return 0;
	    b = base64dec1(s);
	    if ((int32_t) b < 0)
		return 0;
	    bfill = 6, s++, len--;
	}
	uint32_t z = __builtin_clz(b);
	q += z++;
	b >>= z, bfill -= z;
	uint32_t r = b;
	int rfill = bfill, left;
	while ((left = rfill - m) < 0) {
	    if (unlikely(len == 0))
		return 0;
	    b = base64dec1(s);
	    if ((int32_t) b < 0)
		return 0;
	    bfill = 6, s++, len--;
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
