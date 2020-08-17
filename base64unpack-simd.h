#include <stdint.h>
#include <stdbool.h>
#include "base64.h"
#define Mask(k) ((1U << k) - 1)

#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>

#define Vto8(x) vreinterpretq_u8_u32(x)
#define Vto16(x) vreinterpretq_u16_u32(x)
#define Vfrom8(x) vreinterpretq_u32_u8(x)
#define Vfrom16(x) vreinterpretq_u32_u16(x)

#define V32x4 uint32x4_t
#define VLOAD(p) Vfrom8(vld1q_u8((const void *)(p)))
#define VSTORE(p, x) vst1q_u32(p, x)
#define VSTORE64(p, x) vst1_u32(p, vget_low_u32(x))

#define VDUP8(k) Vfrom8(vdupq_n_u8(k))
#define VDUP32(k) vdupq_n_u32(k)

#define VADD8(x, y) Vfrom8(vaddq_u8(Vto8(x), Vto8(y)))
#define VCMPEQ8(x, y) Vfrom8(vceqq_u8(Vto8(x), Vto8(y)))

#define V8x16_C(k0, k1, k2, k3, k4, k5, k6, k7, \
		k8, k9, ka, kb, kc, kd, ke, kf) \
	Vfrom8(((uint8x16_t) { \
		k0, k1, k2, k3, k4, k5, k6, k7, \
		k8, k9, ka, kb, kc, kd, ke, kf }))
#define VSHUF8(x, t) Vfrom8(vqtbl1q_u8(Vto8(x), Vto8(t)))

#define VEXTR8(x, k) vreinterpretq_u8_u32(x)[k]
#define VEXTR16(x, k) vreinterpretq_u16_u32(x)[k]
#define VEXTR32(x, k) x[k]

#define VSHLV16(x, k0, k1, k2, k3, k4, k5, k6, k7) \
	Vfrom16(vshlq_u16(Vto16(x), (int16x8_t){ k0, k1, k2, k3, k4, k5, k6, k7 }))
#define VSHLV32(x, k0, k1, k2, k3) \
	vshlq_u32(x, (int32x4_t){ k0, k1, k2, k3 })
#define VSHRV32(x, k0, k1, k2, k3) \
	vshlq_u32(x, (int32x4_t){ -(k0), -(k1), -(k2), -(k3) })
#define VSHRV64(x, k0, k1) \
	vreinterpretq_u32_u64(vshlq_u64(vreinterpretq_u64_u32(x), \
					(int64x2_t){ -(k0), -(k1) }))
#define VSHLV32_COST 1

#define VSHR8(x, k) Vfrom8(vshrq_n_u8(Vto8(x), k))
#define VSHL16(x, k) Vfrom16(vshlq_n_u16(Vto16(x), k))
#define VSHR16(x, k) Vfrom16(vshrq_n_u16(Vto16(x), k))
#define VSHL32(x, k) vshlq_n_u32(x, k)
#define VSHR32(x, k) vshrq_n_u32(x, k)

#define VOR(x, y) vorrq_u32(x, y)
#define VAND(x, y) vandq_u32(x, y)
#define VTESTZ(x, y) (vmaxvq_u32(vandq_u32(x, y)) == 0)

static inline V32x4 glue12(V32x4 x)
{
    uint16x8_t y = vshrq_n_u16(vreinterpretq_u16_u32(x), 8);
    y = vsliq_n_u16(vreinterpretq_u16_u32(x), y, 6);
    return vreinterpretq_u32_u16(y);
}

static inline V32x4 glue24(V32x4 x)
{
    V32x4 y = vshrq_n_u32(x, 16);
    return vsliq_n_u32(x, y, 12);
}

#else // x86
#include <smmintrin.h>

#define V32x4 __m128i
#define VLOAD(p) _mm_loadu_si128((const void *)(p))
#define VSTORE(p, x) _mm_storeu_si128((void *)(p), x)
#define VSTORE64(p, x) _mm_storel_epi64((void *)(p), x)

#define VDUP8(k) _mm_set1_epi8(k)
#define VDUP32(k) _mm_set1_epi32(k)

#define VADD8(x, y) _mm_add_epi8(x, y)
#define VCMPEQ8(x, y) _mm_cmpeq_epi8(x, y)

#define V8x16_C(k0, k1, k2, k3, k4, k5, k6, k7, \
		k8, k9, ka, kb, kc, kd, ke, kf) \
	_mm_setr_epi8( \
		k0, k1, k2, k3, k4, k5, k6, k7, \
		k8, k9, ka, kb, kc, kd, ke, kf)
#define VSHUF8(x, t) _mm_shuffle_epi8(x, t)

#define VEXTR8(x, k) (uint32_t)_mm_extract_epi8(x, k)
#define VEXTR16(x, k) (uint32_t)_mm_extract_epi16(x, k)
#define VEXTR32(x, k) (uint32_t)_mm_extract_epi32(x, k)

#if defined(__AVX512BW__) && defined(__AVX512VL__)
#include <immintrin.h>
#define VSHLV16(x, k0, k1, k2, k3, k4, k5, k6, k7) \
	_mm_sllv_epi16(x, _mm_setr_epi16(k0, k1, k2, k3, k4, k5, k6, k7))
#else
#define VSHLV16(x, k0, k1, k2, k3, k4, k5, k6, k7) \
	_mm_mullo_epi16(x, _mm_setr_epi16(1U<<k0, 1U<<k1, 1U<<k2, 1U<<k3, \
					  1U<<k4, 1U<<k5, 1U<<k6, 1U<<k7))
#endif

#if defined(__znver1__)
#define VSHLV32(x, k0, k1, k2, k3) \
	_mm_mullo_epi32(x, _mm_setr_epi32(1U<<k0, 1U<<k1, 1U<<k2, 1U<<k3))
#define VSHLV32_COST 1
#elif defined(__AVX2__)
#include <immintrin.h>
#define VSHLV32(x, k0, k1, k2, k3) \
	_mm_sllv_epi32(x, _mm_setr_epi32(k0, k1, k2, k3))
#define VSHLV32_COST 2
#else
#define VSHLV32(x, k0, k1, k2, k3) \
	_mm_mullo_epi32(x, _mm_setr_epi32(1U<<k0, 1U<<k1, 1U<<k2, 1U<<k3))
#define VSHLV32_COST 3
#endif

#define VSHL16(x, k) _mm_slli_epi16(x, k)
#define VSHR16(x, k) _mm_srli_epi16(x, k)
#define VSHL32(x, k) _mm_slli_epi32(x, k)
#define VSHR32(x, k) _mm_srli_epi32(x, k)
#define VSHR64(x, k) _mm_srli_epi64(x, k)

#define VBLEND16(x, y, c) _mm_blend_epi16(x, y, c)
#define VMOVSIGN32(x) _mm_movemask_ps(_mm_castsi128_ps(x))

#define VOR(x, y) _mm_or_si128(x, y)
#define VAND(x, y) _mm_and_si128(x, y)
#define VTESTZ(x, y) _mm_testz_si128(x, y)

#define glue12(x) _mm_maddubs_epi16(x, _mm_set1_epi32(0x40014001))
#define glue24(x) _mm_madd_epi16(x, _mm_set1_epi32(0x10000001))

#endif

// Decode 16 base64 characters into 6-bit values.
// The lo and hi registers convey additional information to validate the input.
static inline V32x4 unpack6x(V32x4 x, V32x4 *lo, V32x4 *hi)
{
    V32x4 v2f = VDUP8(0x2f);
    // There are two peculiarities:
    // - on x86, there is no VSHR8, it has to be emulated with VSHR32 and mask;
    // - at the same time, VSHUF8 on x86 isn't sensitive to bits 4..6, so we can
    //   use 0x2f, the code point of '/', as a mask instead of 0x0f.
#ifdef VSHR8
    *hi = VSHR8(x, 4);
    *lo = VAND(x, VDUP8(0x0f));
#else
    *hi = VSHR32(x, 4);
    *lo = VAND(x, v2f);
    *hi = VAND(*hi, v2f);
#endif
    const V32x4 lut_roll = V8x16_C(
	    0, 16, 19, 4, -65, -65, -71, -71,
	    0,  0,  0, 0,   0,   0,   0,   0);
    V32x4 eq2f = VCMPEQ8(x, v2f);
    V32x4 roll = VSHUF8(lut_roll, VADD8(*hi, eq2f));
    return VADD8(x, roll);
}

// Check if the intput was invalid, i.e. contained non-base64 characters.
static inline bool u6err(V32x4 lo, V32x4 hi)
{
    const V32x4 lut_lo = V8x16_C(
	    0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	    0x11, 0x11, 0x13, 0x1a, 0x1b, 0x1b, 0x1b, 0x1a);
    const V32x4 lut_hi = V8x16_C(
	    0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08,
	    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10);
    lo = VSHUF8(lut_lo, lo);
    hi = VSHUF8(lut_hi, hi);
    return !VTESTZ(lo, hi);
}

static inline bool unpack6(const char *s, V32x4 *x)
{
    V32x4 lo, hi;
    *x = unpack6x(VLOAD(s), &lo, &hi);
    return !u6err(lo, hi);
}

static inline bool unpack24(const char *s, V32x4 *x)
{
    V32x4 lo, hi;
    *x = unpack6x(VLOAD(s), &lo, &hi);
    bool err = u6err(lo, hi);
    *x = glue12(*x);
    *x = glue24(*x);
    return !err;
}

static inline bool unpack5x19c16e1(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s, &x)) return false;
    *e = VEXTR32(x, 0) & 1;
    x = glue24(glue12(x));
    y = VSHUF8(x, V8x16_C(
	    -1, -1, -1, 0, -1, -1,  0, 1,
	    -1, -1, -1, 1, -1, -1, -1, 2));
    y = VSHLV32(y, 2, 5, 0, 3);
    VSTORE(v + 0, VSHR32(y, 27));
    y = VSHUF8(x, V8x16_C(
	    -1, -1, 2, 4, -1, -1, -1, 4,
	    -1, -1, 4, 5, -1, -1,  5, 6));
    y = VSHLV32(y, 6, 1, 4, 7);
    VSTORE(v + 4, VSHR32(y, 27));
    y = VSHUF8(x, V8x16_C(
	    -1, -1, -1, 6, -1, -1, 6, 8,
	    -1, -1, -1, 8, -1, -1, 8, 9));
    y = VSHLV32(y, 2, 5, 0, 3);
    VSTORE(v + 8, VSHR32(y, 27));
    y = VSHUF8(x, V8x16_C(
	    -1, -1,  9, 10, -1, -1, -1, 10,
	    -1, -1, 10, 12, -1, -1, 12, 13));
    y = VSHLV32(y, 6, 1, 4, 7);
    VSTORE(v + 12, VSHR32(y, 27));
    y = VSHUF8(x, V8x16_C(
	    -1, -1, -1, 13, -1, -1, 13, 14,
	    -1, -1, -1, 14, -1, -1, -1, -1));
    y = VSHLV32(y, 2, 5, 0, 3);
    VSTORE(v + 16, VSHR32(y, 27));
    return (void) e, true;
}

static inline bool unpack6x16c16(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack6(s, &x)) return false;
    VSTORE(v + 0, VAND(x, VDUP32(Mask(6))));
    VSTORE(v + 4, VSHUF8(x, V8x16_C(
	     1, -1, -1, -1,  5, -1, -1, -1,
	     9, -1, -1, -1, 13, -1, -1, -1)));
    VSTORE(v + 8, VSHUF8(x, V8x16_C(
	     2, -1, -1, -1,  6, -1, -1, -1,
	    10, -1, -1, -1, 14, -1, -1, -1)));
    VSTORE(v + 12, VSHR32(x, 24));
    return (void) e, true;
}

static inline bool unpack7x12c14(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    V32x4 mask = VDUP32(Mask(7));
    if (!unpack6(s - 2, &x)) return false;
    x = VSHUF8(x, V8x16_C(
	    2,  3,  4,  5,  5,  6,  7,  8,
	    9, 10, 11, 12, 12, 13, 14, 15));
    x = glue24(glue12(x));
#ifdef VSHRV32
    x = VSHRV32(x, 0, 3, 0, 3);
#else
    x = VBLEND16(VSHR32(x, 3), x, 0x33);
#endif
    VSTORE(v + 0, VAND(x, mask));
    VSTORE(v + 4, VAND(VSHR32(x, 7), mask));
    VSTORE(v + 8, VAND(VSHR32(x, 14), mask));
    return (void) e, true;
}

static inline bool unpack7x24c28(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y, out;
    V32x4 mask = VDUP32(Mask(7));
    if (!unpack6(s, &x)) return false;
    x = glue12(x);
    VSTORE(v + 0, VAND(x, mask));
    x = glue24(x);
    VSTORE(v + 4, VAND(VSHR32(x, 7), mask));
    VSTORE(v + 8, VAND(VSHR32(x, 14), mask));
    if (!unpack6(s + 12, &y)) return false;
    y = VSHUF8(y, V8x16_C(
	     4,  5,  6, -1,  7,  8,  9, -1,
	    10, 11, 12, -1, 13, 14, 15, -1));
    out = VOR(VSHR32(x, 21), VSHL32(y, 3));
    y = glue12(y);
    VSTORE(v + 12, VAND(out, mask));
    VSTORE(v + 16, VAND(VSHR32(y, 4), mask));
    y = glue24(y);
    VSTORE(v + 20, VSHR32(y, 11));
    return (void) e, true;
}

static inline bool unpack8x12c16(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack6(s, &x)) return false;
    x = glue12(x);
    VSTORE(v + 0, VAND(x, VDUP32(Mask(8))));
    x = glue24(x);
    VSTORE(v + 4, VSHUF8(x, V8x16_C(
	    1, -1, -1, -1,  5, -1, -1, -1,
	    9, -1, -1, -1, 13, -1, -1, -1)));
    VSTORE(v + 8, VSHR32(x, 16));
    return (void) e, true;
}

static inline bool unpack9x10c15(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack24(s - 1, &x)) return false;
    y = VSHUF8(x, V8x16_C(
	    -1, -1, 0, 1, -1, -1, 1, 2,
	    -1, -1, 4, 5, -1, -1, 5, 6));
    y = VSHLV32(y, 1, 0, 7, 6);
    y = VSHR32(y, 23);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1, 6,  8, -1, -1,  8,  9,
	    -1, -1, 9, 10, -1, -1, 10, 12));
    y = VSHLV32(y, 5, 4, 3, 2);
    y = VSHR32(y, 23);
    VSTORE(v + 4, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1, 12, 13, -1, -1, 13, 14,
	    -1, -1, -1, -1, -1, -1, -1, -1));
#if VSHLV32_COST >= 2
    x = VSHL32(y, 1);
    y = VBLEND16(y, x, 0x33);
#else
    y = VSHLV32(y, 1, 0, 0, 0);
#endif
    y = VSHR32(y, 23);
    VSTORE64(v + 8, y);
    return (void) e, true;
}

static inline bool unpack9x32c48(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x0, x1, x2, out;
    V32x4 mask = VDUP32(Mask(9));
    if (!unpack6(s, &x0)) return false;
    x0 = glue12(x0);
    VSTORE(v + 0, VAND(x0, mask));
    x0 = glue24(x0);
    VSTORE(v + 4, VAND(VSHR32(x0, 9), mask));
    if (!unpack6(s + 16, &x1)) return false;
    out = VOR(VSHR32(x0, 18), VSHL32(x1, 6));
    x1 = glue12(x1);
    VSTORE(v + 8, VAND(out, mask));
    VSTORE(v + 12, VAND(VSHR32(x1, 3), mask));
    x1 = glue24(x1);
    VSTORE(v + 16, VAND(VSHR32(x1, 12), mask));
    if (!unpack6(s + 32, &x2)) return false;
    out = VOR(VSHR32(x1, 21), VSHL32(x2, 3));
    x2 = glue12(x2);
    VSTORE(v + 20, VAND(out, mask));
    x2 = glue24(x2);
    VSTORE(v + 24, VAND(VSHR32(x2, 6), mask));
    VSTORE(v + 28, VSHR32(x2, 15));
    return (void) e, true;
}

static inline bool unpack10x8c14e4(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s - 2, &x)) return false;
#if 0
    *e = VEXTR32(x, 0) >> 26;
    x = glue24(glue12(x));
    y = VSHUF8(x, V8x16_C(
	    -1, -1, 1, 4, -1, -1, 4, 5,
	    -1, -1, 6, 8, -1, -1, 8, 9));
    y = VSHLV32(y, 2, 0, 6, 4);
    y = VSHR32(y, 22);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1,  9, 10, -1, -1, 10, 12,
	    -1, -1, 13, 14, -1, -1, 14,  2));
    y = VSHLV32(y, 2, 0, 6, 4);
#else
    *e = VEXTR16(x, 1) & Mask(4);
    x = glue24(glue12(x));
    y = VSHUF8(x, V8x16_C(
	    -1, -1, 2, 4, -1, -1, 4, 5,
	    -1, -1, 5, 6, -1, -1, 6, 8));
    y = VSHLV32(y, 6, 4, 2, 0);
    y = VSHR32(y, 22);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1,  9, 10, -1, -1, 10, 12,
	    -1, -1, 12, 13, -1, -1, 13, 14));
    y = VSHLV32(y, 6, 4, 2, 0);
#endif
    y = VSHR32(y, 22);
    VSTORE(v + 4, y);
    return true;
}

static inline bool unpack10x9c15(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack6(s - 1, &x)) return false;
    x = glue12(x);
    v[8] = VEXTR16(x, 7) >> 2;
    x = glue24(x);
    x = VSHUF8(x, V8x16_C(
	    0, 1,  6,  8, 2, 4,  9, 10,
	    4, 5, 10, 12, 5, 6, 12, 13));
    x = VSHLV16(x, 0, 0, 6, 6, 4, 4, 2, 2);
    x = VSHR16(x, 6);
    VSTORE(v, VAND(x, VDUP32(0xffff)));
    VSTORE(v + 4, VSHR32(x, 16));
    return (void) e, true;
}

static inline bool unpack10x18c30(const char *s, uint32_t *v, unsigned *e)
{
#if 1
    V32x4 x, y, z;
    V32x4 mask = VDUP32(Mask(10));
    if (!unpack6(s, &x)) return false;
    x = glue12(x);
    VSTORE(v, VAND(x, mask));
    x = glue24(x);
    VSTORE(v + 4, VAND(VSHR32(x, 10), mask));
    if (!unpack6(s + 14, &y)) return false;
    y = glue24(glue12(y));
    z = VSHUF8(y, V8x16_C(
	    -1,  5,  6, -1, -1,  8,  9, -1,
	    -1, 10, 12, -1, -1, 13, 14, -1));
    x = VOR(VSHR32(x, 16), z);
    VSTORE(v + 8, VAND(VSHR32(x, 4), mask));
    VSTORE(v + 12, VSHR32(x, 14));
    y = VSHUF8(y, V8x16_C(
	    -1, -1,  1,  2, -1, -1,  2,  4,
	    -1, -1, -1, -1, -1, -1, -1, -1));
#if VSHLV32_COST >= 2
    x = VSHL32(y, 2);
    y = VBLEND16(y, x, 0x33);
#else
    y = VSHLV32(y, 2, 0, 0, 0);
#endif
    y = VSHR32(y, 22);
    VSTORE64(v + 16, y);
#else
    V32x4 x0, y0, x1, y1;
    if (!unpack6(s + 00 - 1, &x0)) return false;
    x0 = glue12(x0);
    if (!unpack6(s + 15 - 1, &x1)) return false;
    x1 = glue12(x1);
    v[0+8] = VEXTR16(x0, 7) >> 2;
    x0 = glue24(x0);
    v[9+8] = VEXTR16(x1, 7) >> 2;
    x1 = glue24(x1);
    y0 = VSHUF8(x0, V8x16_C(
	    -1, -1, 0, 1, -1, -1, 2, 4,
	    -1, -1, 4, 5, -1, -1, 5, 6));
    y0 = VSHLV32(y0, 0, 6, 4, 2);
    y0 = VSHR32(y0, 22);
    VSTORE(v + 0, y0);
    y1 = VSHUF8(x1, V8x16_C(
	    -1, -1, 0, 1, -1, -1, 2, 4,
	    -1, -1, 4, 5, -1, -1, 5, 6));
    y1 = VSHLV32(y1, 0, 6, 4, 2);
    y1 = VSHR32(y1, 22);
    VSTORE(v + 9, y1);
    y0 = VSHUF8(x0, V8x16_C(
	    -1, -1,  6,  8, -1, -1,  9, 10,
	    -1, -1, 10, 12, -1, -1, 12, 13));
    y0 = VSHLV32(y0, 0, 6, 4, 2);
    y0 = VSHR32(y0, 22);
    VSTORE(v + 0 + 4, y0);
    y1 = VSHUF8(x1, V8x16_C(
	    -1, -1,  6,  8, -1, -1,  9, 10,
	    -1, -1, 10, 12, -1, -1, 12, 13));
    y1 = VSHLV32(y1, 0, 6, 4, 2);
    y1 = VSHR32(y1, 22);
    VSTORE(v + 9 + 4, y1);
#endif
    return (void) e, true;
}

static inline bool unpack10x24c40(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x0, x1, x2, out;
    V32x4 mask = VDUP32(Mask(10));
    if (!unpack6(s +  0, &x0)) return false;
    x0 = glue12(x0);
    VSTORE(v, VAND(x0, mask));
    x0 = glue24(x0);
    VSTORE(v + 4, VAND(VSHR32(x0, 10), mask));
    if (!unpack6(s + 16, &x1)) return false;
    out = VOR(VSHR32(x0, 20), VSHL32(x1, 4));
    x1 = glue12(x1);
    VSTORE(v + 8, VAND(out, mask));
    x1 = glue24(x1);
    VSTORE(v + 12, VAND(VSHR32(x1, 6), mask));
    if (!unpack6(s + 24, &x2)) return false;
    x2 = VSHUF8(x2, V8x16_C(
	     8,  9, -1, -1, 10, 11, -1, -1,
	    12, 13, -1, -1, 14, 15, -1, -1));
    out = VOR(VSHR32(x1, 16), VSHL32(x2, 8));
    x2 = glue12(x2);
    VSTORE(v + 16, VAND(out, mask));
    VSTORE(v + 20, VSHR32(x2, 2));
    return (void) e, true;
}

static inline bool unpack11x8c15e2(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s - 1, &x)) return false;
    *e = VEXTR32(x, 0) >> 28;
    x = glue24(glue12(x));
    y = VSHUF8(x, V8x16_C(
	    -1,  0, 1, 4, -1, -1, 4, 5,
	    -1, -1, 5, 6, -1,  6, 8, 9));
    y = VSHLV32(y, 7, 4, 1, 6);
    y = VSHR32(y, 21);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1,  9, 10, -1, -1, 10, 12,
	    -1, -1, 13, 14, -1, -1, 14,  2));
    y = VSHLV32(y, 3, 0, 5, 2);
    y = VSHR32(y, 21);
    VSTORE(v + 4, y);
    return true;
}

static inline bool unpack11x8c16e8(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s, &x)) return false;
    x = glue12(x);
    *e = VEXTR8(x, 0);
    x = glue24(x);
    y = VSHUF8(x, V8x16_C(
	    -1, -1, 1, 2, -1, -1, 2, 4,
	    -1,  4, 5, 6, -1,  5, 6, 8));
    y = VSHLV32(y, 5, 2, 7, 4);
    y = VSHR32(y, 21);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1,  8,  9, -1,  9, 10, 12,
	    -1, -1, 12, 13, -1, -1, 13, 14));
    y = VSHLV32(y, 1, 6, 3, 0);
    y = VSHR32(y, 21);
    VSTORE(v + 4, y);
    return true;
}

static inline bool unpack11x9c18e9(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s + 2, &x)) return false;
    x = glue12(x);
    uint32_t b = base64dec2(s);
    if ((int32_t) b < 0) return false;
    uint32_t c = VEXTR8(x, 0);
    x = glue24(x);
    *e = b & Mask(9);
    v[0] = (b | c << 12) >> 9;
    y = VSHUF8(x, V8x16_C(
	    -1, 0, 1, 2, -1, -1, 2, 4,
	    -1, 4, 5, 6, -1,  5, 6, 8));
    y = VSHLV32(y, 5, 2, 7, 4);
    y = VSHR32(y, 21);
    VSTORE(v + 1, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1,  8,  9, -1,  9, 10, 12,
	    -1, -1, 12, 13, -1, -1, 13, 14));
    y = VSHLV32(y, 1, 6, 3, 0);
    y = VSHR32(y, 21);
    VSTORE(v + 5, y);
    return true;
}

static inline bool unpack11x10c20e10(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s + 4, &x)) return false;
    x = glue12(x);
    uint32_t b = base64dec4(s);
    if ((int32_t) b < 0) return false;
    uint32_t c = VEXTR8(x, 0);
    x = glue24(x);
    *e = b & Mask(10);
    v[0] = (b >> 10) & Mask(11);
    v[1] = (b | c << 24) >> 21;
    y = VSHUF8(x, V8x16_C(
	    -1, 0, 1, 2, -1, -1, 2, 4,
	    -1, 4, 5, 6, -1,  5, 6, 8));
    y = VSHLV32(y, 5, 2, 7, 4);
    y = VSHR32(y, 21);
    VSTORE(v + 2, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1,  8,  9, -1,  9, 10, 12,
	    -1, -1, 12, 13, -1, -1, 13, 14));
    y = VSHLV32(y, 1, 6, 3, 0);
    y = VSHR32(y, 21);
    VSTORE(v + 6, y);
    return true;
}

static inline bool unpack12x8c16(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s, &x)) return false;
    x = glue12(x);
    y = VAND(x, VDUP32(0xffff));
    VSTORE(v, y);
    y = VSHR32(x, 16);
    VSTORE(v + 4, y);
    return (void) e, true;
}

static inline bool unpack13x6c13o1(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack24(s - 2, &x)) return false;
    y = VSHUF8(x, V8x16_C(
	    -1, 1, 2, 4, -1, -1, 4, 5,
	    -1, 5, 6, 8, -1, -1, 8, 9));
    y = VSHLV32(y, 7, 2, 5, 0);
    y = VSHR32(y, 19);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1, 10, 12, -1, 12, 13, 14,
	    -1, -1, -1, -1, -1, -1, -1, -1));
#if VSHLV32_COST >= 3
    x = VSHL32(y, 3);
    y = VSHL32(y, 6);
    y = VBLEND16(y, x, 0x33);
#else
    y = VSHLV32(y, 3, 6, 0, 0);
#endif
    y = VSHR32(y, 19);
    VSTORE64(v + 4, y);
    return (void) e, true;
}

static inline bool unpack13x7c16e5(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s, &x)) return false;
    *e = VEXTR32(x, 0) & Mask(5);
    x = glue24(glue12(x));
    y = VSHUF8(x, V8x16_C(
	    -1, 0, 1, 2, -1, -1, 2, 4,
	    -1, 4, 5, 6, -1,  6, 8, 9));
    y = VSHLV32(y, 6, 1, 4, 7);
    y = VSHR32(y, 19);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1,  9, 10, -1, 10, 12, 13,
	    -1, -1, 13, 14, -1, -1, -1, -1));
    y = VSHLV32(y, 2, 5, 0, 0);
    y = VSHR32(y, 19);
    VSTORE(v + 4, y);
    return (void) e, true;
}

static inline bool unpack14x6c14(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack24(s - 2, &x)) return false;
    y = VSHUF8(x, V8x16_C(
	    -1,  1, 2, 4, -1, -1, 4,  5,
	    -1, -1, 6, 8, -1,  8, 9, 10));
    y = VSHLV32(y, 6, 0, 2, 4);
    y = VSHR32(y, 18);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, 10, 12, 13, -1, -1, 13, 14,
	    -1, -1, -1, -1, -1, -1, -1, -1));
#if VSHLV32_COST >= 2
    x = VSHL32(y, 6);
    y = VBLEND16(y, x, 0x33);
#else
    y = VSHLV32(y, 6, 0, 0, 0);
#endif
    y = VSHR32(y, 18);
    VSTORE64(v + 4, y);
    return (void) e, true;
}

static inline bool unpack15x6c15(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack24(s - 1, &x)) return false;
    y = VSHUF8(x, V8x16_C(
	    -1, 0, 1, 2, -1, 2, 4, 5,
	    -1, 5, 6, 8, -1, 8, 9, 10));
    y = VSHLV32(y, 3, 4, 5, 6);
    y = VSHR32(y, 17);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, 10, 12, 13, -1, -1, 13, 14,
	    -1, -1, -1, -1, -1, -1, -1, -1));
#if VSHLV32_COST >= 2
    x = VSHL32(y, 7);
    y = VBLEND16(y, x, 0x33);
#else
    y = VSHLV32(y, 7, 0, 0, 0);
#endif
    y = VSHR32(y, 17);
    VSTORE64(v + 4, y);
    return (void) e, true;
}

static inline bool unpack16x6c16(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack24(s, &x)) return false;
    y = VSHUF8(x, V8x16_C(
	    0, 1, -1, -1, 2, 4, -1, -1,
	    5, 6, -1, -1, 8, 9, -1, -1));
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    10, 12, -1, -1, 13, 14, -1, -1,
	    -1, -1, -1, -1, -1, -1, -1, -1));
    VSTORE64(v + 4, y);
    return (void) e, true;
}

static inline bool unpack17x5c15e5(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack6(s - 1, &x)) return false;
    *e = VEXTR32(x, 0) >> 25;
    x = glue24(glue12(x));
    v[0] = VEXTR32(x, 1) >> 7;
    x = VSHUF8(x, V8x16_C(
	    -1,  0,  1,  4, -1,  8,  9, 10,
	    -1, 10, 12, 13, -1, 13, 14,  2));
    x = VSHLV32(x, 1, 7, 6, 5);
    x = VSHR32(x, 15);
    VSTORE(v + 1, x);
    return true;
}

static inline bool unpack17x6c17(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack24(s, &x)) return false;
    y = VSHUF8(x, V8x16_C(
	    -1, 0, 1, 2, -1, 2, 4,  5,
	    -1, 5, 6, 8, -1, 8, 9, 10));
    y = VSHLV32(y, 7, 6, 5, 4);
    y = VSHR32(y, 15);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, 10, 12, 13, -1, 13, 14, -1,
	    -1, -1, -1, -1, -1, -1, -1, -1));
    y = VSHLV32(y, 3, 2, 0, 0);
    y = VSHR32(y, 15);
    VSTORE64(v + 4, y);
    int32_t z = base64dec1(s + 16);
    if (z < 0) return false;
    v[5] |= (z << 11);
    return (void) e, true;
}

static inline bool unpack18x5c15(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack24(s - 1, &x)) return false;
    v[0] = VEXTR32(x, 0) >> 6;
    const V32x4 shuf = V8x16_C(
	    -1, 4,  5,  6, -1,  6,  8,  9,
	    -1, 9, 10, 12, -1, 12, 13, 14);
    x = VSHUF8(x, shuf);
    x = VSHLV32(x, 6, 4, 2, 0);
    x = VSHR32(x, 14);
    VSTORE(v + 1, x);
    return (void) e, true;
}

static inline bool unpack19x4c13e2o1(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack6(s - 2, &x)) return false;
    *e = VEXTR16(x, 1) & Mask(2);
    x = glue24(glue12(x));
    x = VSHUF8(x, V8x16_C(
	     1, 2, 4,  5, -1,  5,  6,  8,
	    -1, 8, 9, 10, 10, 12, 13, 14));
    x = VSHLV32(x, 7, 4, 1, 6);
    VSTORE(v, VSHR32(x, 13));
    return true;
}

static inline bool unpack19x5c16e1(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack6(s, &x)) return false;
    *e = VEXTR32(x, 0) & 1;
    x = glue24(glue12(x));
    v[0] = (VEXTR32(x, 0) >> 1) & Mask(19);
    x = VSHUF8(x, V8x16_C(
	    -1, 2,  4,  5,  5,  6,  8,  9,
	    -1, 9, 10, 12, -1, 12, 13, 14));
    x = VSHLV32(x, 1, 6, 3, 0);
    VSTORE(v + 1, VSHR32(x, 13));
    return true;
}

static inline bool unpack20x4c14e4(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack6(s - 2, &x)) return false;
    *e = VEXTR16(x, 1) & Mask(4);
    x = glue24(glue12(x));
    x = VSHUF8(x, V8x16_C(
	    -1, 2,  4,  5, -1,  5,  6,  8,
	    -1, 9, 10, 12, -1, 12, 13, 14));
#if VSHLV32_COST >= 2
    x = VBLEND16(x, VSHL32(x, 4), 0x33);
#else
    x = VSHLV32(x, 4, 0, 4, 0);
#endif
    VSTORE(v, VSHR32(x, 12));
    return true;
}

static inline bool unpack20x6c20(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack24(s, &x)) return false;
    uint32_t d = VEXTR32(x, 3);
    x = VSHUF8(x, V8x16_C(
	    -1, 0, 1, 2, -1, 2,  4,  5,
	    -1, 6, 8, 9, -1, 9, 10, 12));
#if VSHLV32_COST >= 2
    x = VBLEND16(x, VSHL32(x, 4), 0x33);
#else
    x = VSHLV32(x, 4, 0, 4, 0);
#endif
    VSTORE(v, VSHR32(x, 12));
    int32_t b = base64dec4(s + 16);
    if (b < 0) return false;
    v[4] = (d >> 8) | (b & Mask(4)) << 16;
    v[5] = (b >> 4);
    return (void) e, true;
}

static inline bool unpack21x4c14(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
#if VSHLV32_COST >= 2
    if (!unpack6(s - 2, &x)) return false;
    x = VSHUF8(x, V8x16_C(
	    2,  3,  4,  5,  5,  6,  7,  8,
	    9, 10, 11, 12, 12, 13, 14, 15));
    x = glue24(glue12(x));
    x = VBLEND16(VSHR32(x, 3), x, 0x33);
    VSTORE(v, VAND(x, VDUP32(Mask(21))));
#else
    if (!unpack24(s - 2, &x)) return false;
    x = VSHUF8(x, V8x16_C(
	    1, 2,  4,  5,  5,  6,  8, -1,
	    8, 9, 10, 12, 12, 13, 14, 15));
    x = VSHLV32(x, 7, 10, 5, 8);
    VSTORE(v, VSHR32(x, 11));
#endif
    return (void) e, true;
}

static inline bool unpack22x4c15e2(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack6(s - 1, &x)) return false;
    *e = VEXTR32(x, 0) >> 28;
    x = glue24(glue12(x));
    x = VSHUF8(x, V8x16_C(
	    0,  1,  4,  5,  5,  6, 8,  9,
	    9, 10, 12, -1, 13, 14, 2, -1));
    x = VSHLV32(x, 4, 6, 8, 10);
    VSTORE(v, VSHR32(x, 10));
    return true;
}

static inline bool unpack23x4c16e4(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s, &x)) return false;
#ifdef VMOVSIGN32
    y = VSHL32(x, 31);
    x = glue12(x);
    *e = VMOVSIGN32(y);
#else
    y = VSHUF8(x, V8x16_C(
	     0,  4,  8, 12, -1, -1, -1, -1,
	    -1, -1, -1, -1, -1, -1, -1, -1));
    x = glue12(x);
    *e = (y[0] & 0x1010101) * 0x10204080 >> 28;
#endif
    x = glue24(x);
    VSTORE(v, VSHR32(x, 1));
    return true;
}

static inline bool unpack24x4c16(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack24(s, &x)) return false;
    VSTORE(v, x);
    return (void) e, true;
}

static inline bool unpack25x3c13e3o1(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack24(s - 2, &x)) return false;
    x = VSHUF8(x, V8x16_C(
	     1,  2,  4,  5,  5, 6, 8,  9,
	    10, 12, 13, 14, 14, 5, 9, -1));
#ifdef VMOVSIGN32
    x = VSHLV32(x, 2, 0, 6, 0);
    *e = VMOVSIGN32(x);
    x = VSHR32(x, 6);
    VSTORE(v, VAND(x, VDUP32(Mask(25))));
#else
    x = VSHLV32(x, 3, 1, 7, 7);
    VSTORE(v, VSHR32(x, 7));
    *e = (v[3] & 0x802002) * 0x40010080 >> 29;
#endif
    return true;
}

static inline bool unpack25x4c17e2(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack24(s, &x)) return false;
    x = VSHUF8(x, V8x16_C(
	    0, 1,  2,  4,  4,  5,  6,  8,
	    8, 9, 10, 12, 12, 13, 14, -1));
    x = VSHLV32(x, 7, 6, 5, 4);
    VSTORE(v, VSHR32(x, 7));
    int32_t hi = base64dec1(s + 16);
    if (hi < 0) return false;
    v[3] |= (hi & Mask(4)) << 21;
    *e = (hi >> 4);
    return true;
}

static inline bool unpack26x3c13o1(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack24(s - 2, &x)) return false;
    x = VSHUF8(x, V8x16_C(
	     1,  2,  4,  5,  5,  6,  8,  9,
	    10, 12, 13, 14, -1, -1, -1, -1));
    x = VSHLV32(x, 2, 0, 6, 0);
    VSTORE(v, VSHR32(x, 6));
    return (void) e, true;
}

static inline bool unpack26x4c18e4(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack24(s, &x)) return false;
    x = VSHUF8(x, V8x16_C(
	    0, 1,  2,  4,  4,  5,  6,  8,
	    8, 9, 10, 12, 12, 13, 14, -1));
    x = VSHLV32(x, 6, 4, 2, 0);
    VSTORE(v, VSHR32(x, 6));
    int32_t hi = base64dec2(s + 16);
    if (hi < 0) return false;
    v[3] |= (hi & Mask(8)) << 18;
    *e = (hi >> 8);
    return true;
}

static inline bool unpack27x3c14e3(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack24(s - 2, &x)) return false;
    x = VSHUF8(x, V8x16_C(
	     1,  2,  4,  5, 6,  8,  9, 10,
	    10, 12, 13, 14, 5, 10, 14, -1));
#ifdef VMOVSIGN32
    x = VBLEND16(x, VSHL32(x, 4), 0x0c);
    *e = VMOVSIGN32(x);
    x = VSHR32(x, 4);
    VSTORE(v, VAND(x, VDUP32(Mask(27))));
#else
    x = VSHLV32(x, 1, 5, 1, 1);
    VSTORE(v, VSHR32(x, 5));
    *e = (v[3] & 0x80088) * 0x4801000 >> 29;
#endif
    return true;
}

static inline bool unpack27x4c18(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack24(s, &x)) return false;
    x = VSHUF8(x, V8x16_C(
	    0, 1,  2,  4,  4,  5,  6,  8,
	    8, 9, 10, 12, 13, 14, -1, -1));
#ifdef VSHRV64
    x = VSHRV64(x, 0, 1);
#else
    x = VBLEND16(x, VSHR64(x, 1), 0xf0);
#endif
    x = VSHLV32(x, 5, 2, 0, 17);
    VSTORE(v, VSHR32(x, 5));
    int32_t lo = base64dec2(s + 16);
    if (lo < 0) return false;
    v[3] |= lo;
    return (void) e, true;
}

static inline bool unpack28x3c14(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x;
    if (!unpack24(s - 2, &x)) return false;
    x = VSHUF8(x, V8x16_C(
	     1,  2,  4,  5,  6,  8,  9, 10,
	    10, 12, 13, 14, -1, -1, -1, -1));
#if VSHLV32_COST >= 2
    x = VBLEND16(x, VSHL32(x, 4), 0x0c);
#else
    x = VSHLV32(x, 0, 4, 0, 0);
#endif
    VSTORE(v, VSHR32(x, 4));
    return (void) e, true;
}

static inline bool unpack29x3c15e3o1(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s, &x)) return false;
    y = glue12(x);
    x = VSHUF8(x, V8x16_C(
	    -1, -1, -1, 12, -1, -1, -1, 13,
	    -1, -1, -1, 14, -1, -1, -1, -1));
#ifdef VMOVSIGN32
    *e = VMOVSIGN32(VSHL32(x, 2));
    y = glue24(y);
#else
    y = glue24(y);
    *e = (y[3] & 0x20820) * 0x1084000 >> 29;
#endif
    x = VOR(x, y);
    VSTORE(v, VAND(x, VDUP32(Mask(29))));
    return true;
}

static inline bool unpack30x3c15o1(const char *s, uint32_t *v, unsigned *e)
{
    V32x4 x, y;
    if (!unpack6(s, &x)) return false;
    y = glue12(x);
    x = VSHUF8(x, V8x16_C(
	    -1, -1, -1, 12, -1, -1, -1, 13,
	    -1, -1, -1, 14, -1, -1, -1, -1));
    y = glue24(y);
    VSTORE(v, VOR(x, y));
    return (void) e, true;
}
