#include <stdlib.h>
#include <assert.h>
#include "rpmset.h"
#include "rpmsetcmp-common.h"
#include "setstring.h"

// Compare after downsampling Pv into temporary array Pw.  At least two
// downsampling steps are required, so Pw is double-sized.
static inline int setcmp_withPwR(const uint32_t *Pv, size_t Pn, int Pbpp,
				       uint32_t *Pw,
				 const uint32_t *Rv, size_t Rn, int Rbpp)
{
    Pn = downsample(Pv, Pn, Pw, --Pbpp);
    uint32_t *Pw0 = Pw;
    uint32_t *Pw1 = Pw + Pn;
    do {
	Pn = downsample(Pw0, Pn, Pw1, --Pbpp);
	uint32_t *Pw2 = Pw0;
	Pw0 = Pw1, Pw1 = Pw2;
    } while (Pbpp > Rbpp);
    memset(Pw0 + Pn, 0xff, SENTINELS * 4);
    return setcmploop(Pw0, Pn, Rv, Rn);
}

// Compare after downsampling Rv into temporary array Rw.  Rw is signle-sized,
// so Rv will be clobbered if more than one downsampling step is needed.
static inline int setcmp_withPRw(const uint32_t *Pv, size_t Pn, int Pbpp,
				       uint32_t *Rv, size_t Rn, int Rbpp,
				       uint32_t *Rw)
{
    do {
	Rn = downsample(Rv, Rn, Rw, --Rbpp);
	uint32_t *Rx = Rv;
	Rv = Rw, Rw = Rx;
    } while (Rbpp > Pbpp);
    return setcmploop(Pv, Pn, Rv, Rn);
}

// Compare with downsampling (Pbpp != Rbpp).
static int setcmp_withD(const uint32_t *Pv, size_t Pn, int Pbpp,
			      uint32_t *Rv, size_t Rn, int Rbpp)
{
    if (likely(Pbpp == Rbpp + 1)) {
	if (Pn > 1024) {
	    uint32_t *Pw = malloc((Pn + SENTINELS) * 4);
	    assert(Pw);
	    Pn = downsample(Pv, Pn, Pw, --Pbpp);
	    memset(Pw + Pn, 0xff, SENTINELS * 4);
	    int ret = setcmploop(Pw, Pn, Rv, Rn);
	    free(Pw);
	    return ret;
	}
	uint32_t Pw[Pn+SENTINELS];
	Pn = downsample(Pv, Pn, Pw, --Pbpp);
	memset(Pw + Pn, 0xff, SENTINELS * 4);
	return setcmploop(Pw, Pn, Rv, Rn);
    }
    if (Pbpp > Rbpp + 1) {
	if (Pn > 640) {
	    uint32_t *Pw = malloc((2 * Pn + SENTINELS) * 4);
	    assert(Pw);
	    int ret = setcmp_withPwR(Pv, Pn, Pbpp, Pw, Rv, Rn, Rbpp);
	    free(Pw);
	    return ret;
	}
	uint32_t Pw[2*Pn+SENTINELS];
	return setcmp_withPwR(Pv, Pn, Pbpp, Pw, Rv, Rn, Rbpp);
    }
    // Rbpp > Pbpp
    if (Rn > 1024) {
	uint32_t *Rw = malloc(Rn * 4);
	assert(Rw);
	int ret = setcmp_withPRw(Pv, Pn, Pbpp, Rv, Rn, Rbpp, Rw);
	free(Rw);
	return ret;
    }
    uint32_t Rw[Rn];
    return setcmp_withPRw(Pv, Pn, Pbpp, Rv, Rn, Rbpp, Rw);
}

int rpmsetcmp(const char *Ps, size_t Plen, const char *Rs, size_t Rlen)
{
    int Pbpp;
    size_t Pn;
    const uint32_t *Pv;
    uint32_t Pa[128+SENTINELS];
    if (likely(Plen >= 128)) {
	Pn = cache_decode(Ps, Plen, &Pv);
	if (unlikely(!Pn))
	    return -3;
	Pbpp = *Ps - 'a' + 7;
    }
    else {
	Pn = setstring_decinit(Ps, Plen, &Pbpp);
	if (unlikely(!Pn))
	    return -3;
	Pn = setstring_decode(Ps, Plen, Pbpp, Pa);
	if (unlikely(!Pn))
	    return -3;
	memset(Pa + Pn, 0xff, SENTINELS * 4);
	Pv = Pa;
    }
    int Rbpp;
    size_t Rn = setstring_decinit(Rs, Rlen, &Rbpp);
    if (unlikely(!Rn))
	return -4;
    if (Rn > 1024) {
	uint32_t *Rv = malloc(Rn * 4);
	assert(Rv);
	Rn = setstring_decode(Rs, Rlen, Rbpp, Rv);
	int ret;
	if (unlikely(!Rn))
	    ret = -4;
	else if (likely(Pbpp == Rbpp))
	    ret = setcmploop(Pv, Pn, Rv, Rn);
	else
	    ret = setcmp_withD(Pv, Pn, Pbpp, Rv, Rn, Rbpp);
	free(Rv);
	return ret;
    }
    uint32_t Rv[Rn];
    Rn = setstring_decode(Rs, Rlen, Rbpp, Rv);
    if (unlikely(!Rn))
	return -4;
    if (likely(Pbpp == Rbpp))
	return setcmploop(Pv, Pn, Rv, Rn);
    return setcmp_withD(Pv, Pn, Pbpp, Rv, Rn, Rbpp);
}
