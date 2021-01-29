#include <stdlib.h>
#include <assert.h>
#include "rpmset.h"
#include "rpmsetcmp-common.h"
#include "setstring.h"

#define CACHE_MINLEN 128
#define STACK_MAXV1 1024
#define STACK_MAXV2 1280
#define xmalloc malloc
#define vmalloc(n) xmalloc((n) * 4)

int rpmsetcmp(const char *Ps, size_t Plen, const char *Rs, size_t Rlen)
{
    size_t Pn, Rn;
    int Pbpp, Rbpp;
    if (unlikely(Plen < CACHE_MINLEN)) {
	Pn = setstring_decinit(Ps, Plen, &Pbpp);
	if (unlikely(!Pn))
	    return -3;
	uint32_t Pv[Pn+SENTINELS];
	Pn = setstring_decode(Ps, Plen, Pbpp, Pv);
	if (unlikely(!Pn))
	    return -3;
	Rn = setstring_decinit(Rs, Rlen, &Rbpp);
	if (unlikely(!Rn))
	    return -4;
	if (unlikely(Pbpp > Rbpp))
	    Pn = downsample(Pv, Pn, Pv, Pbpp - Rbpp);
	install_sentinels(Pv, Pn);
	if (likely(Rn < STACK_MAXV1)) {
	    uint32_t Rv[Rn];
	    Rn = setstring_decode(Rs, Rlen, Rbpp, Rv);
	    if (unlikely(!Rn))
		return -4;
	    if (unlikely(Rbpp > Pbpp)) {
		Rn = downsample(Rv, Rn, Rv, Rbpp - Pbpp);
		Rbpp = Pbpp;
	    }
	    return setcmploop(Pv, Pn, Rv, Rn, Rbpp);
	}
	uint32_t *Rv = vmalloc(Rn);
	Rn = setstring_decode(Rs, Rlen, Rbpp, Rv);
	int ret;
	if (unlikely(!Rn))
	    ret = -4;
	else {
	    if (unlikely(Rbpp > Pbpp)) {
		Rn = downsample(Rv, Rn, Rv, Rbpp - Pbpp);
		Rbpp = Pbpp;
	    }
	    ret = setcmploop(Pv, Pn, Rv, Rn, Rbpp);
	}
	free(Rv);
	return ret;
    }
    const uint32_t *Pv;
    Pn = cache_decode(Ps, Plen, &Pv);
    if (unlikely(!Pn))
	return -3;
    Pbpp = *Ps - 'a' + 7;
    Rn = setstring_decinit(Rs, Rlen, &Rbpp);
    if (unlikely(!Rn))
	return -4;
    if (likely(Rbpp >= Pbpp)) {
	if (likely(Rn < STACK_MAXV1)) {
	    uint32_t Rv[Rn];
	    Rn = setstring_decode(Rs, Rlen, Rbpp, Rv);
	    if (unlikely(!Rn))
		return -4;
	    if (unlikely(Rbpp > Pbpp))
		Rn = downsample(Rv, Rn, Rv, Rbpp - Pbpp);
	    return setcmploop(Pv, Pn, Rv, Rn, Pbpp);
	}
	uint32_t *Rv = vmalloc(Rn);
	Rn = setstring_decode(Rs, Rlen, Rbpp, Rv);
	int ret;
	if (unlikely(!Rn))
	    ret = -4;
	else {
	    if (unlikely(Rbpp > Pbpp))
		Rn = downsample(Rv, Rn, Rv, Rbpp - Pbpp);
	    ret = setcmploop(Pv, Pn, Rv, Rn, Pbpp);
	}
	free(Rv);
	return ret;
    }
    if (likely(Rn < STACK_MAXV1)) {
	uint32_t Rv[Rn];
	Rn = setstring_decode(Rs, Rlen, Rbpp, Rv);
	if (unlikely(!Rn))
	    return -4;
	if (Pn + Rn < STACK_MAXV2) {
	    uint32_t Pw[Pn+SENTINELS];
	    Pn = downsample(Pv, Pn, Pw, Pbpp - Rbpp);
	    install_sentinels(Pw, Pn);
	    return setcmploop(Pw, Pn, Rv, Rn, Rbpp);
	}
	uint32_t *Pw = vmalloc(Pn + SENTINELS);
	Pn = downsample(Pv, Pn, Pw, Pbpp - Rbpp);
	install_sentinels(Pw, Pn);
	int ret = setcmploop(Pw, Pn, Rv, Rn, Rbpp);
	free(Pw);
	return ret;
    }
    uint32_t *Rv = vmalloc(Rn + Pn + SENTINELS);
    Rn = setstring_decode(Rs, Rlen, Rbpp, Rv);
    int ret;
    if (unlikely(!Rn))
	ret = -4;
    else {
	uint32_t *Pw = Rv + Rn;
	Pn = downsample(Pv, Pn, Pw, Pbpp - Rbpp);
	install_sentinels(Pw, Pn);
	ret = setcmploop(Pw, Pn, Rv, Rn, Rbpp);
    }
    free(Rv);
    return ret;
}
