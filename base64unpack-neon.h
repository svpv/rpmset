#include <stdint.h>
#include <stdbool.h>
#include <arm_neon.h>
#include "base64.h"

static inline bool unpack6(const char *s, uint8x16_t *x)
{
    *x = vld1q_u8((const uint8_t *) s);
    const uint8x16_t lut_lo = {
	    0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	    0x11, 0x11, 0x13, 0x1a, 0x1b, 0x1b, 0x1b, 0x1a };
    const uint8x16_t lut_hi = {
	    0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08,
	    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
    const uint8x16_t lut_roll = {
	    0, 16, 19, 4, -65, -65, -71, -71,
	    0,  0,  0, 0,   0,   0,   0,   0 };
    uint8x16_t lo_nib = vandq_u8(*x, vdupq_n_u8(0x0f));
    uint8x16_t hi_nib = vshrq_n_u8(*x, 4);
    uint8x16_t lo = vqtbl1q_u8(lut_lo, lo_nib);
    uint8x16_t hi = vqtbl1q_u8(lut_hi, hi_nib);
    uint8x16_t eq_2f = vceqq_u8(*x, vdupq_n_u8(0x2f));
    uint8x16_t roll = vqtbl1q_u8(lut_roll, vaddq_u8(eq_2f, hi_nib));
    uint8_t err = vmaxvq_u8(vandq_u8(lo, hi));
    *x = vaddq_u8(*x, roll);
    return !err;
}

static inline uint16x8_t glue12(uint8x16_t x)
{
    uint16x8_t y, z;
    y = vreinterpretq_u16_u8(x);
    z = vshrq_n_u16(y, 8);
    return vsliq_n_u16(y, z, 6);
}

static inline uint32x4_t glue24(uint16x8_t x)
{
    uint32x4_t y, z;
    y = vreinterpretq_u32_u16(x);
    z = vshrq_n_u32(y, 16);
    return vsliq_n_u32(y, z, 12);
}

static inline bool unpack24(const char *s, uint32x4_t *x)
{
    uint8x16_t y;
    bool ok = unpack6(s, &y);
    *x = glue24(glue12(y));
    return ok;
}

#define Mask(k) ((1U << k) - 1)

static inline bool unpack25x3c13e3o1(const char *s, uint32_t *v, unsigned *e)
{
    uint32x4_t x;
    bool ok = unpack24(s - 2, &x);
    const uint8x16_t shuf = {
	     1,  2,  4,  5,  5, 6, 8,  9,
	    10, 12, 13, 14, 14, 5, 9, -1 };
    x = vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(x), shuf));
#ifdef SLOW_VSHIFT
    const uint32x4_t vmul = { 8, 2, 128, 128 };
    x = vmulq_u32(x, vmul);
#else
    const int32x4_t vshift = { 3, 1, 7, 7 };
    x = vshlq_u32(x, vshift);
#endif
    x = vshrq_n_u32(x, 7);
    vst1q_u32(v, x);
    *e = (v[3] & 0x802002) * 0x40010080 >> 29;
    return ok;
}

static inline bool unpack25x4c17e2(const char *s, uint32_t *v, unsigned *e)
{
    uint32x4_t x;
    bool ok = unpack24(s, &x);
    const uint8x16_t shuf = {
	    0, 1,  2,  4,  4,  5,  6,  8,
	    8, 9, 10, 12, 12, 13, 14, -1 };
    x = vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(x), shuf));
#ifdef SLOW_VSHIFT
    const uint32x4_t vmul = { 128, 64, 32, 16 };
    x = vmulq_u32(x, vmul);
#else
    const int32x4_t vshift = { 7, 6, 5, 4 };
    x = vshlq_u32(x, vshift);
#endif
    x = vshrq_n_u32(x, 7);
    vst1q_u32(v, x);
    int32_t hi = base64dec1(s + 16);
    if (hi < 0) return false;
    v[3] |= (hi & Mask(4)) << 21;
    *e = (hi >> 4);
    return ok;
}

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
