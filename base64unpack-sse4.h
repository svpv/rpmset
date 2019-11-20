#include <stdint.h>
#include <stdbool.h>
#include <smmintrin.h>
#include "base64.h"

static inline bool unpack6(const char *s, __m128i *x)
{
    *x = _mm_loadu_si128((const void *) s);
    const __m128i lut_lo = _mm_setr_epi8(
	    0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	    0x11, 0x11, 0x13, 0x1a, 0x1b, 0x1b, 0x1b, 0x1a);
    const __m128i lut_hi = _mm_setr_epi8(
	    0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08,
	    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10);
    const __m128i lut_roll = _mm_setr_epi8(
	    0, 16, 19, 4, -65, -65, -71, -71,
	    0,  0,  0, 0,   0,   0,   0,   0);
    const __m128i mask_2f = _mm_set1_epi8(0x2f);
    __m128i lo_nib = _mm_and_si128(*x, mask_2f);
    __m128i hi_nib = _mm_srli_epi32(*x, 4);
    __m128i lo = _mm_shuffle_epi8(lut_lo, lo_nib);
    __m128i eq_2f = _mm_cmpeq_epi8(*x, mask_2f);
    hi_nib = _mm_and_si128(hi_nib, mask_2f);
    __m128i hi = _mm_shuffle_epi8(lut_hi, hi_nib);
    __m128i roll = _mm_shuffle_epi8(lut_roll, _mm_add_epi8(eq_2f, hi_nib));
    if (!_mm_testz_si128(lo, hi))
	return false;
    *x = _mm_add_epi8(*x, roll);
    return true;
}

static inline __m128i glue12(__m128i x)
{
    return _mm_maddubs_epi16(x, _mm_set1_epi32(0x40014001));
}

static inline __m128i glue24(__m128i x)
{
    return _mm_madd_epi16(x, _mm_set1_epi32(0x10000001));
}

static inline bool unpack24(const char *s, __m128i *x)
{
    if (!unpack6(s, x))
	return false;
    *x = glue12(*x);
    *x = glue24(*x);
    return true;
}

#define Mask(k) ((1U << k) - 1)

static inline bool unpack9x32c48(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x0, x1, x2, out;
    const __m128i mask = _mm_set1_epi32((1 << 9) - 1);
    if (!unpack6(s +  0, &x0)) return false;
    x0 = glue12(x0);
    out = _mm_and_si128(mask, x0);
    x0 = glue24(x0);
    _mm_storeu_si128((void *) &v[0], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x0, 9));
    _mm_storeu_si128((void *) &v[4], out);
    if (!unpack6(s + 16, &x1)) return false;
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x0, 18),
			 _mm_slli_epi32(x1, 6)));
    x1 = glue12(x1);
    x1 = glue24(x1);
    _mm_storeu_si128((void *) &v[8], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x1, 3));
    _mm_storeu_si128((void *) &v[12], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x1, 12));
    _mm_storeu_si128((void *) &v[16], out);
    if (!unpack6(s + 32, &x2)) return false;
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x1, 21),
			 _mm_slli_epi32(x2, 3)));
    x2 = glue12(x2);
    x2 = glue24(x2);
    _mm_storeu_si128((void *) &v[20], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x2, 6));
    _mm_storeu_si128((void *) &v[24], out);
    out = _mm_srli_epi32(x2, 15);
    _mm_storeu_si128((void *) &v[28], out);
    return (void) e, true;
}

static inline bool unpack10x24c40(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x0, x1, x2, out;
    const __m128i mask = _mm_set1_epi32((1 << 10) - 1);
    if (!unpack6(s +  0, &x0)) return false;
    x0 = glue12(x0);
    out = _mm_and_si128(mask, x0);
    x0 = glue24(x0);
    _mm_storeu_si128((void *) &v[0], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x0, 10));
    _mm_storeu_si128((void *) &v[4], out);
    if (!unpack6(s + 16, &x1)) return false;
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x0, 20),
			 _mm_slli_epi32(x1, 4)));
    x1 = glue12(x1);
    x1 = glue24(x1);
    _mm_storeu_si128((void *) &v[8], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x1, 6));
    _mm_storeu_si128((void *) &v[12], out);
    if (!unpack6(s + 24, &x2)) return false;
    const __m128i unpack16 = _mm_setr_epi8(
	     8,  9, -1, -1, 10, 11, -1, -1,
	    12, 13, -1, -1, 14, 15, -1, -1);
    x2 = _mm_shuffle_epi8(x2, unpack16);
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x1, 16),
		         _mm_slli_epi32(x2, 8)));
    x2 = glue12(x2);
    _mm_storeu_si128((void *) &v[16], out);
    out = _mm_srli_epi32(x2, 2);
    _mm_storeu_si128((void *) &v[20], out);
    return (void) e, true;
}

static inline bool unpack19x4c13e2o1(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x;
    if (!unpack6(s - 2, &x)) return false;
    *e = _mm_extract_epi16(x, 1) & Mask(2);
    const __m128i shuf = _mm_setr_epi8(
	     1, 2, 4,  5, -1,  5,  6,  8,
	    -1, 8, 9, 10, 10, 12, 13, 14);
    x = glue24(glue12(x));
    x = _mm_shuffle_epi8(x, shuf);
    x = _mm_mullo_epi32(x, _mm_setr_epi32(128, 16, 2, 64));
    x = _mm_srli_epi32(x, 13);
    _mm_storeu_si128((void *) v, x);
    return true;
}

static inline bool unpack19x5c16e1(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x;
    if (!unpack6(s, &x)) return false;
    *e = _mm_cvtsi128_si32(x) & 1;
    const __m128i shuf = _mm_setr_epi8(
	    -1, 2,  4,  5,  5,  6,  8,  9,
	    -1, 9, 10, 12, -1, 12, 13, 14);
    x = glue24(glue12(x));
    v[0] = (_mm_cvtsi128_si32(x) >> 1) & Mask(19);
    x = _mm_shuffle_epi8(x, shuf);
    x = _mm_mullo_epi32(x, _mm_setr_epi32(2, 64, 8, 1));
    x = _mm_srli_epi32(x, 13);
    _mm_storeu_si128((void *)(v + 1), x);
    return true;
}

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
