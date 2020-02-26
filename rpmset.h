#pragma once
#ifndef __cplusplus
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
extern "C" {
#endif

/**
 * Compare two set-versions, Requires against Provides.
 * The "set:" prefix must be stripped, strings not necessarily null-treminated.
 * @return  1 if  P  > R (aka R \subset P)
 *          0 if  P == R
 *         -1 if  P  < R
 *         -2 if  P != R
 *         -3     P decode error
 *         -4     R decode error
 * The comparison is logically symmetric, i.e. cmp(P,R)=1 iff cmp(R,P)=-1.
 * Nevertheless the arguments should not be swapped, this will subvert a few
 * optimizations resulting in a 3x slowdown.
 */
int rpmsetcmp(const char *Ps, size_t Plen, const char *Rs, size_t Rlen);

#ifdef __cplusplus
}
#endif
