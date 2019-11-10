#include <stdint.h>
#include <stdbool.h>
#include <smmintrin.h>

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

static inline unsigned unpack9x32(const char *s, uint32_t v[32])
{
    __m128i x0, x1, x2, out;
    const __m128i mask = _mm_set1_epi32((1 << 9) - 1);
    if (!base64unpack6(s +  0, &x0)) return 0;
    x0 = base64glue12(x0);
    out = _mm_and_si128(mask, x0);
    x0 = base64glue24(x0);
    _mm_storeu_si128((void *) &v[0], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x0, 9));
    _mm_storeu_si128((void *) &v[4], out);
    if (!base64unpack6(s + 16, &x1)) return 0;
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
    if (!base64unpack6(s + 32, &x2)) return 0;
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
    return 48;
}

static inline unsigned unpack10x24(const char *s, uint32_t v[24])
{
    __m128i x0, x1, x2, out;
    const __m128i mask = _mm_set1_epi32((1 << 10) - 1);
    if (!base64unpack6(s +  0, &x0)) return 0;
    x0 = base64glue12(x0);
    out = _mm_and_si128(mask, x0);
    x0 = base64glue24(x0);
    _mm_storeu_si128((void *) &v[0], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x0, 10));
    _mm_storeu_si128((void *) &v[4], out);
    if (!base64unpack6(s + 16, &x1)) return 0;
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x0, 20),
			 _mm_slli_epi32(x1, 4)));
    x1 = base64glue12(x1);
    x1 = base64glue24(x1);
    _mm_storeu_si128((void *) &v[8], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x1, 6));
    _mm_storeu_si128((void *) &v[12], out);
    if (!base64unpack6(s + 24, &x2)) return 0;
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
    return 40;
}

static inline unsigned unpack30x3(const char *s, uint32_t v[3])
{
    __m128i x, y;
    if (!base64unpack6(s, &x)) return 0;
    const __m128i hi6 = _mm_setr_epi8(
	    -1, -1, -1, 12, -1, -1, -1, 13,
	    -1, -1, -1, 14, -1, -1, -1, 15);
    y = base64glue12(x);
    x = _mm_shuffle_epi8(x, hi6);
    y = base64glue24(y);
    x = _mm_or_si128(x, y);
    _mm_storeu_si128((void *) v, x);
    return 15;
}
