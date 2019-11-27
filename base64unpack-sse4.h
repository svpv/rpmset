#include <stdint.h>
#include <stdbool.h>
#include <smmintrin.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)
#include "base64unpack-simd.h"

static inline bool unpack20x4c14e4(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x, y;
    if (!unpack6(s - 2, &x)) return false;
    *e = _mm_extract_epi16(x, 1) & Mask(4);
    const __m128i shuf = _mm_setr_epi8(
	    -1, 2,  4,  5, -1,  5,  6,  8,
	    -1, 9, 10, 12, -1, 12, 13, 14);
    x = glue24(glue12(x));
    x = _mm_shuffle_epi8(x, shuf);
    y = _mm_slli_epi32(x, 4);
    x = _mm_blend_epi16(x, y, 0x33);
    x = _mm_srli_epi32(x, 12);
    _mm_storeu_si128((void *) v, x);
    return true;
}

static inline bool unpack21x4c14(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x, y;
    if (!unpack6(s - 2, &x)) return false;
    const __m128i mask = _mm_set1_epi32((1 << 21) - 1);
    const __m128i shuf = _mm_setr_epi8(
	    2,  3,  4,  5,  5,  6,  7,  8,
	    9, 10, 11, 12, 12, 13, 14, 15);
    x = _mm_shuffle_epi8(x, shuf);
    x = glue24(glue12(x));
    y = _mm_and_si128(x, mask);
    x = _mm_srli_epi32(x, 3);
    x = _mm_blend_epi16(x, y, 0x33);
    _mm_storeu_si128((void *) v, x);
    return (void) e, true;
}

static inline bool unpack22x4c15e2(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x;
    if (!unpack6(s - 1, &x)) return false;
    *e = _mm_cvtsi128_si32(x) >> 28;
    x = glue24(glue12(x));
    const __m128i shuf = _mm_setr_epi8(
	    0,  1,  4,  5,  5,  6, 8,  9,
	    9, 10, 12, -1, 13, 14, 2, -1);
    x = _mm_shuffle_epi8(x, shuf);
    x = _mm_mullo_epi32(x, _mm_setr_epi32(16, 64, 256, 1024));
    x = _mm_srli_epi32(x, 10);
    _mm_storeu_si128((void *) v, x);
    return true;
}

static inline bool unpack23x4c16e4(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x, y;
    if (!unpack6(s, &x)) return false;
    y = _mm_slli_epi32(x, 31);
    x = glue12(x);
    *e = _mm_movemask_ps(_mm_castsi128_ps(y));
    x = glue24(x);
    x = _mm_srli_epi32(x, 1);
    _mm_storeu_si128((void *) v, x);
    return true;
}

static inline bool unpack24x4c16(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x;
    if (!unpack24(s, &x)) return false;
    _mm_storeu_si128((void *) v, x);
    return (void) e, true;
}

static inline bool unpack25x3c13e3o1(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x;
    if (!unpack24(s - 2, &x)) return false;
    const __m128i mask = _mm_set1_epi32((1 << 25) - 1);
    const __m128i shuf = _mm_setr_epi8(
	     1,  2,  4,  5,  5,  6,  8,  9,
	    10, 12, 13, 14, -1, -1, -1, -1);
    x = _mm_shuffle_epi8(x, shuf);
    x = _mm_mullo_epi32(x, _mm_setr_epi32(4, 1, 64, 0));
    *e = _mm_movemask_ps(_mm_castsi128_ps(x));
    x = _mm_srli_epi32(x, 6);
    x = _mm_and_si128(x, mask);
    _mm_storeu_si128((void *) v, x);
    return true;
}

static inline bool unpack25x4c17e2(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x;
    if (!unpack24(s, &x)) return false;
    const __m128i shuf = _mm_setr_epi8(
	    0, 1,  2,  4,  4,  5,  6,  8,
	    8, 9, 10, 12, 12, 13, 14, -1);
    x = _mm_shuffle_epi8(x, shuf);
    x = _mm_mullo_epi32(x, _mm_setr_epi32(128, 64, 32, 16));
    x = _mm_srli_epi32(x, 7);
    _mm_storeu_si128((void *) v, x);
    int32_t hi = base64dec1(s + 16);
    if (hi < 0) return false;
    v[3] |= (hi & Mask(4)) << 21;
    *e = (hi >> 4);
    return true;
}

static inline bool unpack26x3c13o1(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x;
    if (!unpack24(s - 2, &x)) return false;
    const __m128i shuf = _mm_setr_epi8(
	     1,  2,  4,  5,  5,  6,  8,  9,
	    10, 12, 13, 14, -1, -1, -1, -1);
    x = _mm_shuffle_epi8(x, shuf);
    x = _mm_mullo_epi32(x, _mm_setr_epi32(4, 1, 64, 0));
    x = _mm_srli_epi32(x, 6);
    _mm_storeu_si128((void *) v, x);
    return (void) e, true;
}

static inline bool unpack26x4c18e4(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x;
    if (!unpack24(s, &x)) return false;
    const __m128i shuf = _mm_setr_epi8(
	    0, 1,  2,  4,  4,  5,  6,  8,
	    8, 9, 10, 12, 12, 13, 14, -1);
    x = _mm_shuffle_epi8(x, shuf);
    x = _mm_mullo_epi32(x, _mm_setr_epi32(64, 16, 4, 1));
    x = _mm_srli_epi32(x, 6);
    _mm_storeu_si128((void *) v, x);
    int32_t hi = base64dec2(s + 16);
    if (hi < 0) return false;
    v[3] |= (hi & Mask(8)) << 18;
    *e = (hi >> 8);
    return true;
}

static inline bool unpack27x3c14e3(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x, y;
    if (!unpack24(s - 2, &x)) return false;
    const __m128i mask = _mm_set1_epi32((1 << 27) - 1);
    const __m128i shuf = _mm_setr_epi8(
	     1,  2,  4,  5,  6,  8,  9, 10,
	    10, 12, 13, 14, -1, -1, -1, -1);
    x = _mm_shuffle_epi8(x, shuf);
    y = _mm_slli_epi32(x, 4);
    x = _mm_blend_epi16(x, y, 8 + 4);
    *e = _mm_movemask_ps(_mm_castsi128_ps(x));
    x = _mm_srli_epi32(x, 4);
    x = _mm_and_si128(x, mask);
    _mm_storeu_si128((void *) v, x);
    return true;
}

static inline bool unpack27x4c18(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x, y;
    if (!unpack24(s, &x)) return false;
    const __m128i shuf = _mm_setr_epi8(
	    0, 1,  2,  4,  4,  5,  6,  8,
	    8, 9, 10, 12, 13, 14, -1, -1);
    x = _mm_shuffle_epi8(x, shuf);
    y = _mm_srli_epi64(x, 1);
    x = _mm_blend_epi16(x, y, 0xf0);
    x = _mm_mullo_epi32(x, _mm_setr_epi32(32, 4, 1, 1 << 17));
    x = _mm_srli_epi32(x, 5);
    _mm_storeu_si128((void *) v, x);
    int32_t lo = base64dec2(s + 16);
    if (lo < 0) return false;
    v[3] |= lo;
    return (void) e, true;
}

static inline bool unpack28x3c14(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x, y;
    if (!unpack24(s - 2, &x)) return false;
    const __m128i shuf = _mm_setr_epi8(
	     1,  2,  4,  5,  6,  8,  9, 10,
	    10, 12, 13, 14, -1, -1, -1, -1);
    x = _mm_shuffle_epi8(x, shuf);
    y = _mm_slli_epi32(x, 4);
    x = _mm_blend_epi16(x, y, 8 + 4);
    x = _mm_srli_epi32(x, 4);
    _mm_storeu_si128((void *) v, x);
    return (void) e, true;
}

static inline bool unpack29x3c15e3o1(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x, y, z;
    if (!unpack6(s, &x)) return false;
    const __m128i mask = _mm_set1_epi32((1 << 29) - 1);
    const __m128i hi6 = _mm_setr_epi8(
	    -1, -1, -1, 12, -1, -1, -1, 13,
	    -1, -1, -1, 14, -1, -1, -1, -1);
    y = glue12(x);
    x = _mm_shuffle_epi8(x, hi6);
    z = _mm_slli_epi32(x, 2);
    *e = _mm_movemask_ps(_mm_castsi128_ps(z));
    y = glue24(y);
    x = _mm_or_si128(x, y);
    x = _mm_and_si128(x, mask);
    _mm_storeu_si128((void *) v, x);
    return true;
}

static inline bool unpack30x3c15o1(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x, y;
    if (!unpack6(s, &x)) return false;
    const __m128i hi6 = _mm_setr_epi8(
	    -1, -1, -1, 12, -1, -1, -1, 13,
	    -1, -1, -1, 14, -1, -1, -1, -1);
    y = glue12(x);
    x = _mm_shuffle_epi8(x, hi6);
    y = glue24(y);
    x = _mm_or_si128(x, y);
    _mm_storeu_si128((void *) v, x);
    return (void) e, true;
}
