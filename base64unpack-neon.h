#include <stdint.h>
#include <stdbool.h>
#include <arm_neon.h>

static inline uint8x16_t pack6(uint8x16_t x)
{
    const uint8x16_t lut = {
	    65, 71, -4, -4,  -4,  -4, -4, -4,
	    -4, -4, -4, -4, -19, -16,  0,  0 };
    uint8x16_t y = vqsubq_u8(x, vdupq_n_u8(51));
    uint8x16_t z = vcgtq_u8(x, vdupq_n_u8(25));
    y = vsubq_u8(y, z);
    return vaddq_u8(x, vqtbl1q_u8(lut, y));
}

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

static inline uint8x16_t unglue(uint32x4_t x)
{
    uint32x4_t x0 =             vandq_u32(x, vdupq_n_u32(63));
    uint32x4_t x1 = vshlq_n_u32(vandq_u32(x, vdupq_n_u32(63<<6)), 2);
    uint32x4_t x2 = vshlq_n_u32(vandq_u32(x, vdupq_n_u32(63<<12)), 4);
    uint32x4_t x3 = vshlq_n_u32(vandq_u32(x, vdupq_n_u32(63<<18)), 6);
    uint32x4_t y = vorrq_u32(vorrq_u32(x0, x1), vorrq_u32(x2, x3));
    return vreinterpretq_u8_u32(y);
}

static inline uint8x16_t pack24(uint32x4_t x)
{
    uint8x16_t y = unglue(x);
    return pack6(y);
}

static inline uint16x8_t glue12(uint8x16_t x)
{
    uint16x8_t y, z;
    z = vreinterpretq_u16_u8(x);
    y = vandq_u16(z, vdupq_n_u16(63<<8));
    z = vandq_u16(z, vdupq_n_u16(63));
    y = vshrq_n_u16(y, 2);
    return vorrq_u16(y, z);
}

static inline uint32x4_t glue24(uint16x8_t x)
{
    uint32x4_t y, z;
    z = vreinterpretq_u32_u16(x);
    y = vandq_u32(z, vdupq_n_u32(4095<<16));
    z = vandq_u32(z, vdupq_n_u32(4095));
    y = vshrq_n_u32(y, 4);
    return vorrq_u32(y, z);
}

static inline bool unpack24(const char *s, uint32x4_t *x)
{
    uint8x16_t y;
    bool ok = unpack6(s, &y);
    *x = glue24(glue12(y));
    return ok;
}

static inline void pack30x3c15(const uint32_t *v, char *s, unsigned e)
{
    const uint32x4_t mask = vdupq_n_u32((1 << 30) - 1);
    const uint8x16_t hi6 = {
	    -1, -1, -1, -1, -1, -1, -1, -1,
	    -1, -1, -1, -1,  3,  7, 11, -1 };
    uint32x4_t x = vandq_u32(mask, vld1q_u32(v));
    uint8x16_t y = vqtbl1q_u8(vreinterpretq_u8_u32(x), hi6);
    const uint32x4_t keep3 = { -1, -1, -1, 0 };
    x = vandq_u32(x, keep3);
    uint8x16_t z = unglue(x);
    z = vorrq_u8(z, y);
    vst1q_u8((void *) s, pack6(z));
    (void) e;
}

static inline bool unpack30x3c15(const char *s, uint32_t *v, unsigned *e)
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
