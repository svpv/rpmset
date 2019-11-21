#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>

#define Vto8(x) vreinterpretq_u8_u32(x)
#define Vfrom8(x) vreinterpretq_u32_u8(x)

#define V32x4 uint32x4_t
#define VSTORE(p, x) vst1q_u32(p, x)

#define V8x16_C(k0, k1, k2, k3, k4, k5, k6, k7, \
		k8, k9, ka, kb, kc, kd, ke, kf) \
	Vfrom8((uint8x16_t) { \
		k0, k1, k2, k3, k4, k5, k6, k7, \
		k8, k9, ka, kb, kc, kd, ke, kf })
#define VSHUF8(x, t) Vfrom8(vqtbl1q_u8(Vto8(x), Vto8(t)))

#define VEXTR16(x, k) vreinterpretq_u16_u32(x)[k]
#define VEXTR32(x, k) x[k]

#define FAST_VSHIFT 1
#define VSHLV32(x, k0, k1, k2, k3) \
	vshlq_u32(x, (int32x4_t){ k0, k1, k2, k3 })

#define VSHR32(x, k) vshrq_n_u32(x, k)

#else // x86
#include <smmintrin.h>

#define V32x4 __m128i
#define VSTORE(p, x) _mm_storeu_si128((void *)(p), x);

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

#endif

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
