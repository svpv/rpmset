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
 * Initialize encoding.  The caller requests to encode an array v[n] of hash
 * values, each value stripped to bpp bits, the array sorted and deduplicated.
 * This routine serves two purposes: 1) it tells the caller how long the
 * resulting set-string may be, so that the caller can allocate the output
 * buffer; 2) it calculates the Golomb parameter, a tunable parameter which
 * can make the resulting string shorter.
 * @param v[]  hash values, sorted and unique
 * @param n    the number of elements in v[]
 * @param bpp  bits per hash value, 7..32
 * @param[out] m  the Golomb parameter
 * @return     the buffer size, 0 on error
 */
size_t setstring_encinit(const uint32_t v[], size_t n, int bpp, int *m);

/**
 * Proceed with encoding and write a set-string.
 * @param v[]  pass the same array
 * @param n    with the same number of hash values
 * @param bpp  the same bits per hash value
 * @param m    the Golomb parameter obtained with setstring_encinit
 * @param[out] s  the set-string written here, null-terminated on success
 * @return     strlen(s), 0 on error
 */
size_t setstring_encode(const uint32_t v[], size_t n, int bpp, int m, char *s);

/**
 * Initialize decoding.
 * @param s    the set-string, not necessarily null-terminated
 * @param len  the length of s (won't read past the end)
 * @param[out] bpp  the original bits per hash value
 * @return     the maximum number of hash values to appear, 0 on error
 */
size_t setstring_decinit(const char *s, size_t len, int *bpp);

/* Gee, these cranks actually implemented 26 decoding routines! */
typedef size_t (*setstring_decfunc_t)(const char *s, size_t len, int bpp, uint32_t v[]);
extern const setstring_decfunc_t setstring_dectab[26];

/**
 * Proceed with decoding and recover the hash values.
 * @param s    the same set-string
 * @param len  of the same length
 * @param bpp  bits per hash value, as reported by setstring_decinit
 * @param[out] v[]  hash values unpacked here
 * @return     the number of hash values in v[], 0 on error
 */
static inline size_t setstring_decode(const char *s, size_t len, int bpp, uint32_t v[])
{
    unsigned m = s[1] - 'A';
    return setstring_dectab[m](s, len, bpp, v);
}

#ifdef __cplusplus
}
#endif
