#include <stdint.h>
#include <stdbool.h>
#include <arm_neon.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)
#include "base64unpack-simd.h"

static inline bool unpack26x3c13o1(const char *s, uint32_t *v, unsigned *e)
{
    uint32x4_t x;
    bool ok = unpack24(s - 2, &x);
    const uint8x16_t shuf = {
	     1,  2,  4,  5,  5,  6,  8,  9,
	    10, 12, 13, 14, -1, -1, -1, -1 };
    x = vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(x), shuf));
#ifdef SLOW_VSHIFT
    const uint32x4_t vmul = { 4, 1, 64, 0 };
    x = vmulq_u32(x, vmul);
#else
    const int32x4_t vshift = { 2, 0, 6, 0 };
    x = vshlq_u32(x, vshift);
#endif
    x = vshrq_n_u32(x, 6);
    vst1q_u32(v, x);
    return (void) e, ok;
}

static inline bool unpack26x4c18e4(const char *s, uint32_t *v, unsigned *e)
{
    uint32x4_t x;
    bool ok = unpack24(s, &x);
    const uint8x16_t shuf = {
	    0, 1,  2,  4,  4,  5,  6,  8,
	    8, 9, 10, 12, 12, 13, 14, -1 };
    x = vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(x), shuf));
#ifdef SLOW_VSHIFT
    const uint32x4_t vmul = { 64, 16, 4, 1 };
    x = vmulq_u32(x, vmul);
#else
    const int32x4_t vshift = { 6, 4, 2, 0 };
    x = vshlq_u32(x, vshift);
#endif
    x = vshrq_n_u32(x, 6);
    vst1q_u32(v, x);
    int32_t hi = base64dec2(s + 16);
    if (hi < 0) return false;
    v[3] |= (hi & Mask(8)) << 18;
    *e = (hi >> 8);
    return ok;
}

static inline bool unpack27x3c14e3(const char *s, uint32_t *v, unsigned *e)
{
    uint32x4_t x;
    bool ok = unpack24(s - 2, &x);
    const uint8x16_t shuf = {
	     1,  2,  4,  5, 6,  8,  9, 10,
	    10, 12, 13, 14, 5, 10, 14, -1 };
    x = vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(x), shuf));
#ifdef SLOW_VSHIFT
    const uint32x4_t vmul = { 2, 32, 2, 2 };
    x = vmulq_u32(x, vmul);
#else
    const int32x4_t vshift = { 1, 5, 1, 1 };
    x = vshlq_u32(x, vshift);
#endif
    x = vshrq_n_u32(x, 5);
    vst1q_u32(v, x);
    *e = (v[3] & 0x80088) * 0x4801000 >> 29;
    return ok;
}

static inline bool unpack27x4c18(const char *s, uint32_t *v, unsigned *e)
{
    uint32x4_t x;
    bool ok = unpack24(s, &x);
    const uint8x16_t shuf = {
	    0, 1,  2,  4,  4,  5,  6,  8,
	    8, 9, 10, 12, 13, 14, -1, -1 };
    x = vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(x), shuf));
    const int64x2_t vshift64 = { 0, -1 };
    x = vreinterpretq_u32_u64(vshlq_u64(vreinterpretq_u64_u32(x), vshift64));
#ifdef SLOW_VSHIFT
    const uint32x4_t vmul = { 32, 4, 1, 1 << 17 };
    x = vmulq_u32(x, vmul);
#else
    const int32x4_t vshift = { 5, 2, 0, 17 };
    x = vshlq_u32(x, vshift);
#endif
    x = vshrq_n_u32(x, 5);
    vst1q_u32(v, x);
    int32_t lo = base64dec2(s + 16);
    if (lo < 0) return false;
    v[3] |= lo;
    return (void) e, ok;
}

static inline bool unpack28x3c14(const char *s, uint32_t *v, unsigned *e)
{
    uint32x4_t x;
    bool ok = unpack24(s - 2, &x);
    const uint8x16_t shuf = {
	     1,  2,  4,  5,  6,  8,  9, 10,
	    10, 12, 13, 14, -1, -1, -1, -1 };
    x = vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(x), shuf));
#ifdef SLOW_VSHIFT
    const uint32x4_t vmul = { 1, 16, 1, 0 };
    x = vmulq_u32(x, vmul);
#else
    const int32x4_t vshift = { 0, 4, 0, 0 };
    x = vshlq_u32(x, vshift);
#endif
    x = vshrq_n_u32(x, 4);
    vst1q_u32(v, x);
    return (void) e, ok;
}

static inline bool unpack29x3c15e3o1(const char *s, uint32_t *v, unsigned *e)
{
    uint8x16_t x;
    bool ok = unpack6(s, &x);
    const uint32x4_t mask = vdupq_n_u32((1 << 29) - 1);
    const uint8x16_t hi6 = {
	    -1, -1, -1, 12, -1, -1, -1, 13,
	    -1, -1, -1, 14, -1, -1, -1, -1 };
    uint16x8_t y = glue12(x);
    x = vqtbl1q_u8(x, hi6);
    uint32x4_t z = glue24(y);
    z = vorrq_u32(z, vreinterpretq_u32_u8(x));
    vst1q_u32(v, vandq_u32(z, mask));
    *e = (v[3] & 0x20820) * 0x1084000 >> 29;
    return ok;
}

static inline bool unpack30x3c15o1(const char *s, uint32_t *v, unsigned *e)
{
    uint8x16_t x;
    bool ok = unpack6(s, &x);
    const uint8x16_t hi6 = {
	    -1, -1, -1, 12, -1, -1, -1, 13,
	    -1, -1, -1, 14, -1, -1, -1, -1 };
    uint16x8_t y = glue12(x);
    x = vqtbl1q_u8(x, hi6);
    uint32x4_t z = glue24(y);
    z = vorrq_u32(z, vreinterpretq_u32_u8(x));
    vst1q_u32(v, z);
    return (void) e, ok;
}
