#include <stdint.h>
#include <stdbool.h>
#include <smmintrin.h>

static inline __m128i base64pack6(__m128i x)
{
    const __m128i lut = _mm_setr_epi8(
	    65, 71, -4, -4,  -4,  -4, -4, -4,
	    -4, -4, -4, -4, -19, -16,  0,  0);
    __m128i y = _mm_subs_epu8(x, _mm_set1_epi8(51));
    __m128i z = _mm_cmpgt_epi8(x, _mm_set1_epi8(25));
    y = _mm_sub_epi8(y, z);
    return _mm_add_epi8(x, _mm_shuffle_epi8(lut, y));
}

static inline bool base64unpack6(const char *s, __m128i *x)
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

static inline __m128i base64unglue(__m128i x)
{
    __m128i x0 =                _mm_and_si128(x, _mm_set1_epi32(63));
    __m128i x1 = _mm_slli_epi32(_mm_and_si128(x, _mm_set1_epi32(63<<6)), 2);
    __m128i x2 = _mm_slli_epi32(_mm_and_si128(x, _mm_set1_epi32(63<<12)), 4);
    __m128i x3 = _mm_slli_epi32(_mm_and_si128(x, _mm_set1_epi32(63<<18)), 6);
    return _mm_or_si128(_mm_or_si128(x0, x1), _mm_or_si128(x2, x3));
}

static inline __m128i base64pack24(__m128i x)
{
    x = base64unglue(x);
    return base64pack6(x);
}

static inline __m128i base64glue12(__m128i x)
{
    return _mm_maddubs_epi16(x, _mm_set1_epi32(0x40014001));
}

static inline __m128i base64glue24(__m128i x)
{
    return _mm_madd_epi16(x, _mm_set1_epi32(0x10000001));
}

static inline bool base64unpack24(const char *s, __m128i *x)
{
    if (!base64unpack6(s, x))
	return false;
    *x = base64glue12(*x);
    *x = base64glue24(*x);
    return true;
}

static inline void pack9x32c48e0(const uint32_t *v, char *s, unsigned e)
{
    // 9+9+6 | 3+9+9+3 | 6+9+9
    __m128i x0, x1, x2, x3, x4, x5, x6, x7;
    const __m128i mask = _mm_set1_epi32((1 << 9) - 1);
    x0 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[0]));
    x1 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[4]));
    x2 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[8]));
    x0 = _mm_or_si128(x0, _mm_slli_epi32(x1, 9));
    x0 = _mm_or_si128(x0, _mm_slli_epi32(x2, 18));
    _mm_storeu_si128((void *) &s[0], base64pack24(x0));
    x3 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[12]));
    x4 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[16]));
    x5 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[20]));
    x2 = _mm_srli_epi32(x2, 6);
    x2 = _mm_or_si128(x2, _mm_slli_epi32(x3, 3));
    x2 = _mm_or_si128(x2, _mm_slli_epi32(x4, 12));
    x2 = _mm_or_si128(x2, _mm_slli_epi32(x5, 21));
    _mm_storeu_si128((void *) &s[16], base64pack24(x2));
    x6 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[24]));
    x7 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[28]));
    x5 = _mm_srli_epi32(x5, 3);
    x5 = _mm_or_si128(x5, _mm_slli_epi32(x6, 6));
    x5 = _mm_or_si128(x5, _mm_slli_epi32(x7, 15));
    _mm_storeu_si128((void *) &s[32], base64pack24(x5));
    (void) e;
}

static inline bool unpack9x32c48e0(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x0, x1, x2, out;
    const __m128i mask = _mm_set1_epi32((1 << 9) - 1);
    if (!base64unpack6(s +  0, &x0)) return false;
    x0 = base64glue12(x0);
    out = _mm_and_si128(mask, x0);
    x0 = base64glue24(x0);
    _mm_storeu_si128((void *) &v[0], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x0, 9));
    _mm_storeu_si128((void *) &v[4], out);
    if (!base64unpack6(s + 16, &x1)) return false;
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x0, 18),
			 _mm_slli_epi32(x1, 6)));
    x1 = base64glue12(x1);
    x1 = base64glue24(x1);
    _mm_storeu_si128((void *) &v[8], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x1, 3));
    _mm_storeu_si128((void *) &v[12], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x1, 12));
    _mm_storeu_si128((void *) &v[16], out);
    if (!base64unpack6(s + 32, &x2)) return false;
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x1, 21),
			 _mm_slli_epi32(x2, 3)));
    x2 = base64glue12(x2);
    x2 = base64glue24(x2);
    _mm_storeu_si128((void *) &v[20], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x2, 6));
    _mm_storeu_si128((void *) &v[24], out);
    out = _mm_srli_epi32(x2, 15);
    _mm_storeu_si128((void *) &v[28], out);
    return (void) e, true;
}

static inline void pack10x24c40e0(const uint32_t *v, char *s, unsigned e)
{
    // 10+10+4 | 6+10+8 | 2+10
    __m128i x0, x1, x2, x3, x4, x5;
    const __m128i mask = _mm_set1_epi32((1 << 10) - 1);
    x0 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[0]));
    x1 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[4]));
    x2 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[8]));
    x0 = _mm_or_si128(x0, _mm_slli_epi32(x1, 10));
    x0 = _mm_or_si128(x0, _mm_slli_epi32(x2, 20));
    _mm_storeu_si128((void *) &s[0], base64pack24(x0));
    x3 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[12]));
    x4 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[16]));
    x2 = _mm_srli_epi32(x2, 4);
    x2 = _mm_or_si128(x2, _mm_slli_epi32(x3, 6));
    x2 = _mm_or_si128(x2, _mm_slli_epi32(x4, 16));
    _mm_storeu_si128((void *) &s[16], base64pack24(x2));
    x5 = _mm_and_si128(mask, _mm_loadu_si128((const void *) &v[20]));
    x4 = _mm_srli_epi32(x4, 8);
    x4 = _mm_or_si128(x4, _mm_slli_epi32(x5, 2));
    x4 = base64pack24(x4);
    const __m128i pack16 = _mm_setr_epi8(
	     0,  1,  4,  5,  8,  9, 12, 13,
	    -1, -1, -1, -1, -1, -1, -1, -1);
    x4 = _mm_shuffle_epi8(x4, pack16);
    _mm_storel_epi64((void *) &s[32], x4);
    (void) e;
}

static inline bool unpack10x24c40e0(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x0, x1, x2, out;
    const __m128i mask = _mm_set1_epi32((1 << 10) - 1);
    if (!base64unpack6(s +  0, &x0)) return false;
    x0 = base64glue12(x0);
    out = _mm_and_si128(mask, x0);
    x0 = base64glue24(x0);
    _mm_storeu_si128((void *) &v[0], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x0, 10));
    _mm_storeu_si128((void *) &v[4], out);
    if (!base64unpack6(s + 16, &x1)) return false;
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x0, 20),
			 _mm_slli_epi32(x1, 4)));
    x1 = base64glue12(x1);
    x1 = base64glue24(x1);
    _mm_storeu_si128((void *) &v[8], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x1, 6));
    _mm_storeu_si128((void *) &v[12], out);
    if (!base64unpack6(s + 24, &x2)) return false;
    const __m128i unpack16 = _mm_setr_epi8(
	     8,  9, -1, -1, 10, 11, -1, -1,
	    12, 13, -1, -1, 14, 15, -1, -1);
    x2 = _mm_shuffle_epi8(x2, unpack16);
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x1, 16),
		         _mm_slli_epi32(x2, 8)));
    x2 = base64glue12(x2);
    _mm_storeu_si128((void *) &v[16], out);
    out = _mm_srli_epi32(x2, 2);
    _mm_storeu_si128((void *) &v[20], out);
    return (void) e, true;
}

#define Mask(k) ((1U << k) - 1)

static inline void pack28x6c28e0(const uint32_t *v, char *s, unsigned e)
{
    uint32_t x[8];
    x[0]  =  v[0] & Mask(24);
    x[1]  = (v[0] & Mask(28)) >> 24;
    x[1] |= (v[1] & Mask(20)) << 4;
    x[2]  = (v[1] & Mask(28)) >> 20;
    x[2] |= (v[2] & Mask(16)) << 8;
    x[3]  = (v[2] & Mask(28)) >> 16;
    x[3] |= (v[3] & Mask(12)) << 12;
    x[4]  = (v[3] & Mask(28)) >> 12;
    x[4] |= (v[4] & Mask( 8)) << 16;
    x[5]  = (v[4] & Mask(28)) >> 8;
    x[5] |= (v[5] & Mask( 4)) << 20;
    x[6]  = (v[5] & Mask(28)) >> 4;
    x[7]  =  0;
    __m128i x0 = _mm_loadu_si128((void *) &x[0]);
    __m128i x1 = _mm_loadu_si128((void *) &x[4]);
    _mm_storeu_si128((void *) &s[0], base64pack24(x0));
    _mm_storeu_si128((void *) &s[16], base64pack24(x1));
    (void) e;
}

static inline bool unpack28x6c28e0(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x0, x1;
    if (!base64unpack24(s +  0, &x0)) return false;
    if (!base64unpack24(s + 12, &x1)) return false;
    const __m128i shuf0 = _mm_setr_epi8(
	    0,  1,  2,  4,  4,  5,  6,  8,
	    9, 10, 12, 13, -1, -1, -1, -1);
    const __m128i shuf1 = _mm_setr_epi8(
	     1,  2,  4,  5,  6,  8,  9, 10,
	    10, 12, 13, 14, -1, -1, -1, -1);
    x0 = _mm_shuffle_epi8(x0, shuf0);
    x1 = _mm_shuffle_epi8(x1, shuf1);
    x0 = _mm_mullo_epi32(x0, _mm_setr_epi32(16, 1, 16, 0));
    x1 = _mm_mullo_epi32(x1, _mm_setr_epi32(1, 16, 1, 0));
    x0 = _mm_srli_epi32(x0, 4);
    x1 = _mm_srli_epi32(x1, 4);
    _mm_storeu_si128((void *) &v[0], x0);
    _mm_storeu_si128((void *) &v[3], x1);
    return (void) e, true;
}

static inline void pack29x3c15e3(const uint32_t *v, char *s, unsigned e)
{
    __m128i x, y;
    const __m128i mask = _mm_set1_epi32((1 << 29) - 1);
    const __m128i hi6 = _mm_setr_epi8(
	    -1, -1, -1, -1, -1, -1, -1, -1,
	    -1, -1, -1, -1,  3,  7, 11, -1);
    x = _mm_and_si128(mask, _mm_loadu_si128((const void *) v));
    y = _mm_setr_epi32((e & 1) << 29, (e & 2) << 28, (e & 4) << 27, 0);
    x = _mm_or_si128(x, y);
    y = _mm_shuffle_epi8(x, hi6);
    x = base64unglue(x);
    x = _mm_blend_epi16(x, y, 128 + 64);
    _mm_storeu_si128((void *) s, base64pack6(x));
}

static inline bool unpack29x3c15e3(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x, y;
    if (!base64unpack6(s, &x)) return false;
    const __m128i mask = _mm_set1_epi32((1 << 29) - 1);
    const __m128i hi6 = _mm_setr_epi8(
	    -1, -1, -1, 12, -1, -1, -1, 13,
	    -1, -1, -1, 14, -1, -1, -1, -1);
    y = base64glue12(x);
    x = _mm_shuffle_epi8(x, hi6);
    y = base64glue24(y);
    x = _mm_or_si128(x, y);
    y = _mm_and_si128(x, mask);
    _mm_storeu_si128((void *) v, y);
    x = _mm_slli_epi32(x, 2);
    *e = _mm_movemask_ps(_mm_castsi128_ps(x));
    return true;
}

static inline void pack30x3c15e0(const uint32_t *v, char *s, unsigned e)
{
    __m128i x, y;
    const __m128i mask = _mm_set1_epi32((1 << 30) - 1);
    const __m128i hi6 = _mm_setr_epi8(
	    -1, -1, -1, -1, -1, -1, -1, -1,
	    -1, -1, -1, -1,  3,  7, 11, -1);
    x = _mm_and_si128(mask, _mm_loadu_si128((const void *) v));
    y = _mm_shuffle_epi8(x, hi6);
    x = base64unglue(x);
    x = _mm_blend_epi16(x, y, 128 + 64);
    _mm_storeu_si128((void *) s, base64pack6(x));
    (void) e;
}

static inline bool unpack30x3c15e0(const char *s, uint32_t *v, unsigned *e)
{
    __m128i x, y;
    if (!base64unpack6(s, &x)) return false;
    const __m128i hi6 = _mm_setr_epi8(
	    -1, -1, -1, 12, -1, -1, -1, 13,
	    -1, -1, -1, 14, -1, -1, -1, -1);
    y = base64glue12(x);
    x = _mm_shuffle_epi8(x, hi6);
    y = base64glue24(y);
    x = _mm_or_si128(x, y);
    _mm_storeu_si128((void *) v, x);
    return (void) e, true;
}
