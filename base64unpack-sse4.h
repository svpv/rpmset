#include <stdint.h>
#include <tmmintrin.h>

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

static inline unsigned pack9x32(const uint32_t v[32], char *s)
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
    return 48;
}

static inline unsigned pack10x24(const uint32_t v[24], char *s)
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
    return 40;
}

static inline unsigned pack30x3(const uint32_t v[3], char *s)
{
    __m128i x, y;
    const __m128i mask = _mm_set1_epi32((1 << 30) - 1);
    const __m128i hi6 = _mm_setr_epi8(
	    -1, -1, -1, -1, -1, -1, -1, -1,
	    -1, -1, -1, -1,  3,  7, 11, -1);
    x = _mm_and_si128(mask, _mm_loadu_si128((const void *) v));
    y = _mm_shuffle_epi8(x, hi6);
    x = base64unglue(x);
#ifdef __SSE4_1__
    x = _mm_blend_epi16(x, y, 128 + 64);
#else
    x = _mm_and_si128(x, _mm_setr_epi32(-1, -1, -1, 0));
    x = _mm_or_si128(x, y);
#endif
    _mm_storeu_si128((void *) s, base64pack6(x));
    return 15;
}
