#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>

#define Vto8(x) vreinterpretq_u8_u32(x)
#define Vfrom8(x) vreinterpretq_u32_u8(x)

#define V32x4 uint32x4_t
#define VLOAD(p) Vfrom8(vld1q_u8((const void *)(p)))
#define VSTORE(p, x) vst1q_u32(p, x)

#define VDUP8(k) Vfrom8(vdupq_n_u8(k))
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

#define FAST_VSHIFT 1
#define VSHLV32(x, k0, k1, k2, k3) \
	vshlq_u32(x, (int32x4_t){ k0, k1, k2, k3 })

#define VSHR8(x, k) Vfrom8(vshrq_n_u8(Vto8(x), k))
#define VSHR32(x, k) vshrq_n_u32(x, k)

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
#define VSTORE(p, x) _mm_storeu_si128((void *)(p), x);

#define VDUP8(k) _mm_set1_epi8(k)
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

#ifdef __AVX2__
#include <immintrin.h>
#define FAST_VSHIFT 1
#define VSHLV32(x, k0, k1, k2, k3) \
	_mm_sllv_epi32(x, _mm_setr_epi32(k0, k1, k2, k3))
#else
#define FAST_VSHIFT 0
#define VSHLV32(x, k0, k1, k2, k3) \
	_mm_mullo_epi32(x, _mm_setr_epi32(1U<<k0, 1U<<k1, 1U<<k2, 1U<<k3))
#endif

#define VSHR32(x, k) _mm_srli_epi32(x, k)

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
