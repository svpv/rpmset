#include "rpmsetcmp-common.h"

size_t downsample(const uint32_t *v, size_t n, uint32_t *w, int bpp)
{
    uint32_t mask = (1U << bpp) - 1;
    // Find the first element with high bit set.
    size_t l = 0;
    size_t u = n;
    do {
	size_t i = (l + u) / 2;
	if (v[i] <= mask)
	    l = i + 1;
	else
	    u = i;
    } while (l < u);
    // Initialize parts.
    const uint32_t *w_start = w;
    const uint32_t *v1 = v + 0, *v1end = v + u;
    const uint32_t *v2 = v + u, *v2end = v + n;
    // Merge v1 and v2 into w.
    if (v1 < v1end && v2 < v2end) {
	uint32_t v1val = *v1;
	uint32_t v2val = *v2 & mask;
	while (1) {
	    if (v1val < v2val) {
		*w++ = v1val;
		v1++;
		v1val = *v1;
		if (v1 == v1end)
		    break;
	    }
	    else if (v1val > v2val) {
		*w++ = v2val;
		v2++;
		if (v2 == v2end)
		    break;
		v2val = *v2 & mask;
	    }
	    else {
		*w++ = v1val;
		v1++, v2++;
		if (v2 == v2end)
		    break;
		v1val = *v1;
		v2val = *v2 & mask;
		if (v1 == v1end)
		    break;
	    }
	}
    }
    // Append what's left.
    while (v1 < v1end)
	*w++ = *v1++;
    while (v2 < v2end)
	*w++ = *v2++ & mask;
    return w - w_start;
}
