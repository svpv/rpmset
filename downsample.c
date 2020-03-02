#include "rpmsetcmp-common.h"

size_t downsample(const uint32_t *v, size_t n, uint32_t *w, int k)
{
    const uint32_t *v_end = v + n;
    const uint32_t *w_start = w;
    uint32_t v0 = UINT32_MAX;
    do {
	uint32_t v1 = *v++ >> k;
	if (likely(v1 != v0))
	    *w++ = v1;
	v0 = v1;
    } while (v < v_end);
    return w - w_start;
}
