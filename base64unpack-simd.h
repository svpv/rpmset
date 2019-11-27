#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>

#define Vto8(x) vreinterpretq_u8_u32(x)
#define Vfrom8(x) vreinterpretq_u32_u8(x)

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

#define VEXTR16(x, k) vreinterpretq_u16_u32(x)[k]
#define VEXTR32(x, k) x[k]

#define VSHLV32(x, k0, k1, k2, k3) \
	vshlq_u32(x, (int32x4_t){ k0, k1, k2, k3 })
#define VSHLV32_COST 1

#define VSHR8(x, k) Vfrom8(vshrq_n_u8(Vto8(x), k))
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

#define VEXTR16(x, k) (uint32_t)_mm_extract_epi16(x, k)
#define VEXTR32(x, k) (uint32_t)_mm_extract_epi32(x, k)

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

#define VSHL32(x, k) _mm_slli_epi32(x, k)
#define VSHR32(x, k) _mm_srli_epi32(x, k)

#define VBLEND16(x, y, c) _mm_blend_epi16(x, y, c)

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
    V32x4 x, y;
    if (!unpack6(s - 1, &x)) return false;
    x = glue12(x);
    v[8] = VEXTR16(x, 7) >> 2; // or better v[4]?
    x = glue24(x);
    y = VSHUF8(x, V8x16_C(
	    -1, -1, 0, 1, -1, -1, 2, 4,
	    -1, -1, 4, 5, -1, -1, 5, 6));
    y = VSHLV32(y, 0, 6, 4, 2);
    y = VSHR32(y, 22);
    VSTORE(v, y);
    y = VSHUF8(x, V8x16_C(
	    -1, -1,  6,  8, -1, -1,  9, 10,
	    -1, -1, 10, 12, -1, -1, 12, 13));
    y = VSHLV32(y, 0, 6, 4, 2);
    y = VSHR32(y, 22);
    VSTORE(v + 4, y);
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
