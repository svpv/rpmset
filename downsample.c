#include "rpmsetcmp-common.h"

size_t downsample(const uint32_t *v, size_t n, uint32_t *w, int k)
{
    const uint32_t *v_last = v + n - 1;
    const uint32_t *w_start = w;
    uint32_t v0, v1 = UINT32_MAX;
    if (unlikely(v == v_last))
	goto last;
    if (likely(k == 1))
	do {
	    v0 = v[0] >> 1;
	    if (likely(v0 != v1))
		*w++ = v0;
	    v1 = v[1] >> 1;
	    if (likely(v1 != v0))
		*w++ = v1;
	    v += 2;
	} while (v < v_last);
    else
	do {
	    v0 = v[0] >> k;
	    if (likely(v0 != v1))
		*w++ = v0;
	    v1 = v[1] >> k;
	    if (likely(v1 != v0))
		*w++ = v1;
	    v += 2;
	} while (v < v_last);
    if (v == v_last) {
last:
	v0 = v[0] >> k;
	if (likely(v0 != v1))
	    *w++ = v0;
    }
    return w - w_start;
}
