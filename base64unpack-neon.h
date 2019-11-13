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
