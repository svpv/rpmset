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

static inline bool base64unpack24(const char *s, __m128i *x)
{
    if (!base64unpack6(s, x))
	return false;
    *x = _mm_maddubs_epi16(*x, _mm_set1_epi32(0x40014001));
    *x = _mm_madd_epi16(*x, _mm_set1_epi32(0x10000001));
    return true;
}

static inline unsigned unpack9x32(const char *s, uint32_t v[32])
{
    __m128i x0, x1, x2, out;
    if (!base64unpack24(s +  0, &x0)) return 0;
    if (!base64unpack24(s + 16, &x1)) return 0;
    if (!base64unpack24(s + 32, &x2)) return 0;
    const __m128i mask = _mm_set1_epi32((1 << 9) - 1);
    out = _mm_and_si128(mask, x0);
    _mm_storeu_si128((void *) &v[0], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x0, 9));
    _mm_storeu_si128((void *) &v[4], out);
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x0, 18),
			 _mm_slli_epi32(x1, 6)));
    _mm_storeu_si128((void *) &v[8], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x1, 3));
    _mm_storeu_si128((void *) &v[12], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x1, 12));
    _mm_storeu_si128((void *) &v[16], out);
    out = _mm_and_si128(mask,
	    _mm_or_si128(_mm_srli_epi32(x1, 21),
			 _mm_slli_epi32(x2, 3)));
    _mm_storeu_si128((void *) &v[20], out);
    out = _mm_and_si128(mask, _mm_srli_epi32(x2, 6));
    _mm_storeu_si128((void *) &v[24], out);
    out = _mm_srli_epi32(x2, 15);
    _mm_storeu_si128((void *) &v[28], out);
    return 48;
}
