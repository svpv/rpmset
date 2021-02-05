#include "rpmsetcmp-common.h"

#pragma GCC visibility push(hidden)
int setcmploop_sse2(const uint32_t *Pv, size_t Pn, const uint32_t *Rv, size_t Rn);
int setcmploop_avx2(const uint32_t *Pv, size_t Pn, const uint32_t *Rv, size_t Rn);
int setcmploop_cmov(const uint32_t *Pv, size_t Pn, const uint32_t *Rv, size_t Rn);
#pragma GCC visibility pop

typedef int (*FuncPtr)(const uint32_t *Pv, size_t Pn, const uint32_t *Rv, size_t Rn);

static FuncPtr setcmploop_ifunc()
{
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx2")) {
	// Slow AVX loads on Excavator.
	if (__builtin_cpu_is("bdver4"))
	    return setcmploop_sse2;
	// Slow vzeroupper on KNL.
	if (__builtin_cpu_supports("avx512er"))
	    return setcmploop_cmov; // slow tzcnt/bsf
	return setcmploop_avx2;
    }
    if (__builtin_cpu_is("intel")) {
	// We use "corei7" as a synonym for "modern big core".
	if (__builtin_cpu_is("corei7"))
	    return setcmploop_sse2;
	// On small cores such as Goldmont, tzcnt/bsf is slow.
	return setcmploop_cmov;
    }
    // On older AMD cores, the sse2 version is slower.
    if (__builtin_cpu_supports("sse4.1"))
	return setcmploop_sse2;
    return setcmploop_cmov;
}

int setcmploop(const uint32_t *Pv, size_t Pn, const uint32_t *Rv, size_t Rn)
	__attribute__((ifunc("setcmploop_ifunc")));
