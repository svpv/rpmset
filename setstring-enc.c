#include <stdlib.h>
#include "setstring.h"

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
