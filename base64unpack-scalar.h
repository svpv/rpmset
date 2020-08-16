#include <stdint.h>
#include <stdbool.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)

static inline bool unpack5x19c16e1(const char *s, uint32_t *v, unsigned *e)
{
    int64_t x;
    x = base64wdec3(s + 0) | base64wdec3(s + 3) << 18;
    if (x < 0) return false;
    *e    = (x & 1);
    v[ 0] = (x >>  1) & Mask(5);
    v[ 1] = (x >>  6) & Mask(5);
    v[ 2] = (x >> 11) & Mask(5);
    v[ 3] = (x >> 16) & Mask(5);
    v[ 4] = (x >> 21) & Mask(5);
    v[ 5] = (x >> 26) & Mask(5);
    v[ 6] = (x >> 31);
    int32_t y;
    y = base64dec2(s + 6) | base64dec3(s + 8) << 12;
    if (y < 0) return false;
    v[ 7] = (y      ) & Mask(5);
    v[ 8] = (y >>  5) & Mask(5);
    v[ 9] = (y >> 10) & Mask(5);
    v[10] = (y >> 15) & Mask(5);
    v[11] = (y >> 20) & Mask(5);
    v[12] = (y >> 25);
    y = base64dec2(s + 11) | base64dec3(s + 13) << 12;
    if (y < 0) return false;
    v[13] = (y      ) & Mask(5);
    v[14] = (y >>  5) & Mask(5);
    v[15] = (y >> 10) & Mask(5);
    v[16] = (y >> 15) & Mask(5);
    v[17] = (y >> 20) & Mask(5);
    v[18] = (y >> 25);
    return (void) e, true;
}

static inline bool unpack6x16c16(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = v[ 0] = base64dec1(s +  0); if (x < 0) return false;
    x = v[ 4] = base64dec1(s +  1); if (x < 0) return false;
    x = v[ 8] = base64dec1(s +  2); if (x < 0) return false;
    x = v[12] = base64dec1(s +  3); if (x < 0) return false;
    x = v[ 1] = base64dec1(s +  4); if (x < 0) return false;
    x = v[ 5] = base64dec1(s +  5); if (x < 0) return false;
    x = v[ 9] = base64dec1(s +  6); if (x < 0) return false;
    x = v[13] = base64dec1(s +  7); if (x < 0) return false;
    x = v[ 2] = base64dec1(s +  8); if (x < 0) return false;
    x = v[ 6] = base64dec1(s +  9); if (x < 0) return false;
    x = v[10] = base64dec1(s + 10); if (x < 0) return false;
    x = v[14] = base64dec1(s + 11); if (x < 0) return false;
    x = v[ 3] = base64dec1(s + 12); if (x < 0) return false;
    x = v[ 7] = base64dec1(s + 13); if (x < 0) return false;
    x = v[11] = base64dec1(s + 14); if (x < 0) return false;
    x = v[15] = base64dec1(s + 15); if (x < 0) return false;
    return (void) e, true;
}

static inline bool unpack7x12c14(const char *s, uint32_t *v, unsigned *e)
{
    int64_t x;
    x = base64wdec2(s + 0) | base64wdec2(s + 2) << 12 | base64wdec3(s + 4) << 24;
    if (x < 0) return false;
    v[ 0] = (x      ) & Mask(7);
    v[ 4] = (x >>  7) & Mask(7);
    v[ 8] = (x >> 14) & Mask(7);
    v[ 1] = (x >> 21) & Mask(7);
    v[ 5] = (x >> 28) & Mask(7);
    v[ 9] = (x >> 35);
    x = base64wdec2(s + 7) | base64wdec2(s + 9) << 12 | base64wdec3(s + 11) << 24;
    if (x < 0) return false;
    v[ 2] = (x      ) & Mask(7);
    v[ 6] = (x >>  7) & Mask(7);
    v[10] = (x >> 14) & Mask(7);
    v[ 3] = (x >> 21) & Mask(7);
    v[ 7] = (x >> 28) & Mask(7);
    v[11] = (x >> 35);
    return (void) e, true;
}

static inline bool unpack7x24c28(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x, y;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[ 0] = (x      ) & Mask(7);
    v[ 4] = (x >>  7) & Mask(7);
    v[ 8] = (x >> 14) & Mask(7);
    y = base64dec3(s + 16);
    if (y < 0) return false;
    v[12] = (x >> 21 | y << 3) & Mask(7);
    v[16] = (y >>  4) & Mask(7);
    v[20] = (y >> 11);
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[ 1] = (x      ) & Mask(7);
    v[ 5] = (x >>  7) & Mask(7);
    v[ 9] = (x >> 14) & Mask(7);
    y = base64dec3(s + 19);
    if (y < 0) return false;
    v[13] = (x >> 21 | y << 3) & Mask(7);
    v[17] = (y >>  4) & Mask(7);
    v[21] = (y >> 11);
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[ 2] = (x      ) & Mask(7);
    v[ 6] = (x >>  7) & Mask(7);
    v[10] = (x >> 14) & Mask(7);
    y = base64dec3(s + 22);
    if (y < 0) return false;
    v[14] = (x >> 21 | y << 3) & Mask(7);
    v[18] = (y >>  4) & Mask(7);
    v[22] = (y >> 11);
    x = base64dec2(s + 12) | base64dec2(s + 14) << 12;
    if (x < 0) return false;
    v[ 3] = (x      ) & Mask(7);
    v[ 7] = (x >>  7) & Mask(7);
    v[11] = (x >> 14) & Mask(7);
    y = base64dec3(s + 25);
    if (y < 0) return false;
    v[15] = (x >> 21 | y << 3) & Mask(7);
    v[19] = (y >>  4) & Mask(7);
    v[23] = (y >> 11);
    return (void) e, true;
}

static inline bool unpack8x12c16(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[ 0] = (uint8_t) x;
    v[ 4] = (uint8_t)(x >> 8);
    v[ 8] = (x >> 16);
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[ 1] = (uint8_t) x;
    v[ 5] = (uint8_t)(x >> 8);
    v[ 9] = (x >> 16);
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[ 2] = (uint8_t) x;
    v[ 6] = (uint8_t)(x >> 8);
    v[10] = (x >> 16);
    x = base64dec2(s + 12) | base64dec2(s + 14) << 12;
    if (x < 0) return false;
    v[ 3] = (uint8_t) x;
    v[ 7] = (uint8_t)(x >> 8);
    v[11] = (x >> 16);
    return (void) e, true;
}

static inline bool unpack9x10c15(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec3(s + 0);
    if (x < 0) return false;
    v[0] = (x & Mask(9));
    v[1] = (x >> 9);
    x = base64dec3(s + 3);
    if (x < 0) return false;
    v[2] = (x & Mask(9));
    v[3] = (x >> 9);
    x = base64dec3(s + 6);
    if (x < 0) return false;
    v[4] = (x & Mask(9));
    v[5] = (x >> 9);
    x = base64dec3(s + 9);
    if (x < 0) return false;
    v[6] = (x & Mask(9));
    v[7] = (x >> 9);
    x = base64dec3(s + 12);
    if (x < 0) return false;
    v[8] = (x & Mask(9));
    v[9] = (x >> 9);
    return (void) e, true;
}

static inline bool unpack9x32c48(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec3(s + 0);
    if (x < 0) return false;
    v[ 0] = (x & Mask(9));
    v[ 4] = (x >> 9);
    x = base64dec3c(s[3], s[16], s[17]);
    if (x < 0) return false;
    v[ 8] = (x & Mask(9));
    v[12] = (x >> 9);
    x = base64dec3c(s[18], s[19], s[32]);
    if (x < 0) return false;
    v[16] = (x & Mask(9));
    v[20] = (x >> 9);
    x = base64dec3(s + 33);
    if (x < 0) return false;
    v[24] = (x & Mask(9));
    v[28] = (x >> 9);

    x = base64dec3(s + 4);
    if (x < 0) return false;
    v[ 1] = (x & Mask(9));
    v[ 5] = (x >> 9);
    x = base64dec3c(s[7], s[20], s[21]);
    if (x < 0) return false;
    v[ 9] = (x & Mask(9));
    v[13] = (x >> 9);
    x = base64dec3c(s[22], s[23], s[36]);
    if (x < 0) return false;
    v[17] = (x & Mask(9));
    v[21] = (x >> 9);
    x = base64dec3(s + 37);
    if (x < 0) return false;
    v[25] = (x & Mask(9));
    v[29] = (x >> 9);

    x = base64dec3(s + 8);
    if (x < 0) return false;
    v[ 2] = (x & Mask(9));
    v[ 6] = (x >> 9);
    x = base64dec3c(s[11], s[24], s[25]);
    if (x < 0) return false;
    v[10] = (x & Mask(9));
    v[14] = (x >> 9);
    x = base64dec3c(s[26], s[27], s[40]);
    if (x < 0) return false;
    v[18] = (x & Mask(9));
    v[22] = (x >> 9);
    x = base64dec3(s + 41);
    if (x < 0) return false;
    v[26] = (x & Mask(9));
    v[30] = (x >> 9);

    x = base64dec3(s + 12);
    if (x < 0) return false;
    v[ 3] = (x & Mask(9));
    v[ 7] = (x >> 9);
    x = base64dec3c(s[15], s[28], s[29]);
    if (x < 0) return false;
    v[11] = (x & Mask(9));
    v[15] = (x >> 9);
    x = base64dec3c(s[30], s[31], s[44]);
    if (x < 0) return false;
    v[19] = (x & Mask(9));
    v[23] = (x >> 9);
    x = base64dec3(s + 45);
    if (x < 0) return false;
    v[27] = (x & Mask(9));
    v[31] = (x >> 9);
    return (void) e, true;
}

static inline bool unpack10x8c14e4(const char *s, uint32_t *v, unsigned *e)
{
#if 0
    int32_t x, y;
    x = base64dec3(s + 0);
    if (x < 0) return false;
    v[0]  = (x & Mask(4));
    v[0] |= (x >> 12) << 4;
    *e    = (x & Mask(4)<<8) >> 8;
    y = base64dec2(s + 3) | base64dec3(s + 5) << 12;
    if (y < 0) return false;
    v[1]  = (y & Mask(10));
    v[2]  = (y & Mask(20)) >> 10;
    v[3]  = (y >> 20);
    y = base64dec2(s + 8) | base64dec3(s + 10) << 12;
    if (y < 0) return false;
    v[4]  = (y & Mask(10));
    v[5]  = (y & Mask(20)) >> 10;
    v[6]  = (y >> 20);
    y = base64dec1(s + 13);
    if (y < 0) return false;
    v[7]  =  y;
    v[7] |= (x & Mask(4)<<4) << 2;
#else
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    *e    = (x & Mask(4));
    v[0]  = (x & Mask(14)) >> 4;
    v[1]  = (x >> 14);
    x = base64dec2(s + 4) | base64dec3(s + 6) << 12;
    if (x < 0) return false;
    v[2]  = (x & Mask(10));
    v[3]  = (x & Mask(20)) >> 10;
    v[4]  = (x >> 20);
    x = base64dec2(s + 9) | base64dec3(s + 11) << 12;
    if (x < 0) return false;
    v[5]  = (x & Mask(10));
    v[6]  = (x & Mask(20)) >> 10;
    v[7]  = (x >> 20);
#endif
    return true;
}

static inline bool unpack10x9c15(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec3(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = (x & Mask(10));
    v[1]  = (x & Mask(20)) >> 10;
    v[2]  = (x >> 20);
    x = base64dec2(s + 5) | base64dec3(s + 7) << 12;
    if (x < 0) return false;
    v[3]  = (x & Mask(10));
    v[4]  = (x & Mask(20)) >> 10;
    v[5]  = (x >> 20);
    x = base64dec2(s + 10) | base64dec3(s + 12) << 12;
    if (x < 0) return false;
    v[6]  = (x & Mask(10));
    v[7]  = (x & Mask(20)) >> 10;
    v[8]  = (x >> 20);
    return (void) e, true;
}

static inline bool unpack10x18c30(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x0, x1, x2, x3;
    x0 = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x0 < 0) return false;
    x1 = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x1 < 0) return false;
    x2 = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x2 < 0) return false;
    x3 = base64dec2(s + 12) | base64dec2(s + 14) << 12;
    if (x3 < 0) return false;
    v[ 0] = (x0 & Mask(10));
    v[ 1] = (x1 & Mask(10));
    v[ 2] = (x2 & Mask(10));
    v[ 3] = (x3 & Mask(10));
    v[ 4] = (x0 & Mask(20)) >> 10;
    v[ 5] = (x1 & Mask(20)) >> 10;
    v[ 6] = (x2 & Mask(20)) >> 10;
    v[ 7] = (x3 & Mask(20)) >> 10;
    int64_t y;
    y = base64wdec3(s + 16) | base64wdec3(s + 19) << 18;
    if (y < 0) return false;
    v[16] = (y & Mask(10));
    v[17] = (y & Mask(20)) >> 10;
    y = x0 >> 20 | y >> 20 << 4;
    v[ 8] = (y & Mask(10));
    v[12] = (y >> 10);
    y =  base64wdec2(s + 22) | base64wdec2(s + 24) << 12
      | (base64wdec2(s + 26) | base64wdec2(s + 28) << 12) << 24;
    if (y < 0) return false;
    y = x1 >> 20 | y << 4;
    v[ 9] = (y & Mask(10));
    v[13] = (y & Mask(20)) >> 10;
    y = x2 >> 20 | y >> 20 << 4;
    v[10] = (y & Mask(10));
    v[14] = (y & Mask(20)) >> 10;
    y = x3 >> 20 | y >> 20 << 4;
    v[11] = (y & Mask(10));
    v[15] = (y >> 10);
    return (void) e, true;
}

static inline bool unpack10x24c40(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | (base64dec2(s + 2) | base64dec1shl12(s + 16)) << 12;
    if (x < 0) return false;
    v[ 0] = (x & Mask(10));
    v[ 4] = (x & Mask(20)) >> 10;
    v[ 8] = (x >> 20);
    x = base64dec2(s + 4) | (base64dec2(s + 6) | base64dec1shl12(s + 20)) << 12;
    if (x < 0) return false;
    v[ 1] = (x & Mask(10));
    v[ 5] = (x & Mask(20)) >> 10;
    v[ 9] = (x >> 20);
    x = base64dec2(s + 8) | (base64dec2(s + 10) | base64dec1shl12(s + 24)) << 12;
    if (x < 0) return false;
    v[ 2] = (x & Mask(10));
    v[ 6] = (x & Mask(20)) >> 10;
    v[10] = (x >> 20);
    x = base64dec2(s + 12) | (base64dec2(s + 14) | base64dec1shl12(s + 28)) << 12;
    if (x < 0) return false;
    v[ 3] = (x & Mask(10));
    v[ 7] = (x & Mask(20)) >> 10;
    v[11] = (x >> 20);
    x = base64dec3(s + 17) | base64dec2(s + 32) << 18;
    if (x < 0) return false;
    v[12] = (x & Mask(10));
    v[16] = (x & Mask(20)) >> 10;
    v[20] = (x >> 20);
    x = base64dec3(s + 21) | base64dec2(s + 34) << 18;
    if (x < 0) return false;
    v[13] = (x & Mask(10));
    v[17] = (x & Mask(20)) >> 10;
    v[21] = (x >> 20);
    x = base64dec3(s + 25) | base64dec2(s + 36) << 18;
    if (x < 0) return false;
    v[14] = (x & Mask(10));
    v[18] = (x & Mask(20)) >> 10;
    v[22] = (x >> 20);
    x = base64dec3(s + 29) | base64dec2(s + 38) << 18;
    if (x < 0) return false;
    v[15] = (x & Mask(10));
    v[19] = (x & Mask(20)) >> 10;
    v[23] = (x >> 20);
    return (void) e, true;
}

static inline bool unpack11x8c15e2(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x, y;
    x = base64dec3(s + 0);
    if (x < 0) return false;
    v[0]  = (x & Mask(10));
    *e    = (x >> 16);
    y = base64dec2(s + 3);
    if (y < 0) return false;
    v[0] |= (y & Mask(1)) << 10;
    v[1]  = (y >> 1);
    y = base64dec2(s + 5) | base64dec3(s + 7) << 12;
    if (y < 0) return false;
    v[2]  = (y & Mask(11));
    v[3]  = (y & Mask(22)) >> 11;
    v[4]  = (y >> 22);
    y = base64dec2(s + 10) | base64dec3(s + 12) << 12;
    if (y < 0) return false;
    v[4] |= (y & Mask(3)) << 8;
    v[5]  = (y & Mask(14)) >> 3;
    v[6]  = (y & Mask(25)) >> 14;
    v[7]  = (y >> 25);
    v[7] |= (x & Mask(6)<<10) >> 5;
    return true;
}

static inline bool unpack11x8c16e8(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x, y;
    x = base64dec5(s + 0);
    if (x < 0) return false;
    *e    = (uint8_t) x;
    v[0]  = (x & Mask(19)) >> 8;
    v[1]  = (x >> 19);
    x = base64dec5(s + 5);
    if (x < 0) return false;
    v[2]  = (x & Mask(11));
    v[3]  = (x & Mask(22)) >> 11;
    v[4]  = (x >> 22);
    y = base64dec5(s + 10);
    if (y < 0) return false;
    v[4] |= (y & Mask(3)) << 8;
    v[5]  = (y & Mask(14)) >> 3;
    v[6]  = (y & Mask(25)) >> 14;
    v[7]  = (y >> 25);
    x = base64dec1(s + 15);
    if (x < 0) return false;
    v[7] |= (x << 5);
    return true;
}

static inline bool unpack12x8c16(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s +  0); if (x < 0) return false; else v[0] = x;
    x = base64dec2(s +  2); if (x < 0) return false; else v[4] = x;
    x = base64dec2(s +  4); if (x < 0) return false; else v[1] = x;
    x = base64dec2(s +  6); if (x < 0) return false; else v[5] = x;
    x = base64dec2(s +  8); if (x < 0) return false; else v[2] = x;
    x = base64dec2(s + 10); if (x < 0) return false; else v[6] = x;
    x = base64dec2(s + 12); if (x < 0) return false; else v[3] = x;
    x = base64dec2(s + 14); if (x < 0) return false; else v[7] = x;
    return (void) e, true;
}

static inline bool unpack13x6c13o1(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec3(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = (x & Mask(13));
    v[1]  = (x >> 13) & Mask(13);
    v[2]  = (x >> 26);
    x = base64dec2(s + 5) | base64dec3(s + 7) << 12;
    if (x < 0) return false;
    v[2] |= (x & Mask(9)) << 4;
    v[3]  = (x >> 9) & Mask(13);
    v[4]  = (x >> 22);
    x = base64dec3(s + 10);
    if (x < 0) return false;
    v[4] |= (x & Mask(5)) << 8;
    v[5]  = (x >> 5) & Mask(13);
    return (void) e, true;
}

static inline bool unpack13x7c16e5(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec3(s + 0);
    if (x < 0) return false;
    *e    = (x & Mask(5));
    v[0]  = (x >> 5);
    x = base64dec3(s + 3);
    if (x < 0) return false;
    v[1]  = (x & Mask(13));
    v[2]  = (x >> 13);
    x = base64dec2(s + 6) | base64dec3(s + 8) << 12;
    if (x < 0) return false;
    v[2] |= (uint8_t) x << 5;
    v[3]  = (x & Mask(21)) >> 8;
    v[4]  = (x >> 21);
    x = base64dec2(s + 11) | base64dec3(s + 13) << 12;
    if (x < 0) return false;
    v[4] |= (x & Mask(4)) << 9;
    v[5]  = (x & Mask(17)) >> 4;
    v[6]  = (x >> 17);
    return true;
}

static inline bool unpack14x6c14(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = (x & Mask(14));
    v[1]  = (x >> 14);
    x = base64dec3(s + 4);
    if (x < 0) return false;
    v[1] |= (x & Mask(4)) << 10;
    v[2]  = (x >> 4);
    x = base64dec2(s + 7) | base64dec2(s + 9) << 12;
    if (x < 0) return false;
    v[3]  = (x & Mask(14));
    v[4]  = (x >> 14);
    x = base64dec3(s + 11);
    if (x < 0) return false;
    v[4] |= (x & Mask(4)) << 10;
    v[5]  = (x >> 4);
    return (void) e, true;
}

static inline bool unpack15x6c15(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec3(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = (x & Mask(15));
    v[1]  = (x >> 15);
    x = base64dec2(s + 5) | base64dec3(s + 7) << 12;
    if (x < 0) return false;
    v[2]  = (x & Mask(15));
    v[3]  = (x >> 15);
    x = base64dec2(s + 10) | base64dec3(s + 12) << 12;
    if (x < 0) return false;
    v[4]  = (x & Mask(15));
    v[5]  = (x >> 15);
    return (void) e, true;
}

static inline bool unpack16x6c16(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = (x & Mask(16));
    v[1]  = (x >> 16);
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[1] |= (x & Mask(8)) << 8;
    v[2]  = (x >> 8);
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[3]  = (x & Mask(16));
    v[4]  = (x >> 16);
    x = base64dec2(s + 12) | base64dec2(s + 14) << 12;
    if (x < 0) return false;
    v[4] |= (x & Mask(8)) << 8;
    v[5]  = (x >> 8);
    return (void) e, true;
}

static inline bool unpack17x5c15e5(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x, y;
    x = base64dec3(s + 0);
    if (x < 0) return false;
    v[1]  = (x & Mask(10));
    *e    = (x >> 13);
    y = base64dec2(s + 3) | base64dec2(s + 5) << 12;
    if (y < 0) return false;
    v[0]  = (y >> 7);
    v[1] |= (y & Mask(7)) << 10;
    y = base64dec2(s + 7) | base64dec2(s + 9) << 12;
    if (y < 0) return false;
    v[2]  = (y & Mask(17));
    v[3]  = (y >> 17);
    y = base64dec2(s + 11) | base64dec2(s + 13) << 12;
    if (y < 0) return false;
    v[3] |= (y & Mask(10)) << 7;
    v[4]  = (y >> 10);
    v[4] |= (x & Mask(3)<<10) << 4;
    return true;
}

static inline bool unpack17x6c17(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = (x & Mask(17));
    v[1]  = (x >> 17);
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[1] |= (x & Mask(10)) << 7;
    v[2]  = (x >> 10);
    x = base64dec3(s + 8);
    if (x < 0) return false;
    v[2] |= (x & Mask(3)) << 14;
    v[3]  = (x >> 3);
    x = base64dec3(s + 11);
    if (x < 0) return false;
    v[3] |= (x & Mask(2)) << 15;
    v[4]  = (x >> 2);
    x = base64dec3(s + 14);
    if (x < 0) return false;
    v[4] |= (x & Mask(1)) << 16;
    v[5]  = (x >> 1);
    return (void) e, true;
}

static inline bool unpack18x5c15(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec3(s + 0);
    if (x < 0) return false;
    v[0] = x;
    x = base64dec3(s + 3);
    if (x < 0) return false;
    v[1] = x;
    x = base64dec3(s + 6);
    if (x < 0) return false;
    v[2] = x;
    x = base64dec3(s + 9);
    if (x < 0) return false;
    v[3] = x;
    x = base64dec3(s + 12);
    if (x < 0) return false;
    v[4] = x;
    return (void) e, true;
}

static inline bool unpack19x4c13e2o1(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec3(s + 0);
    if (x < 0) return false;
    *e    = (x & Mask(2));
    v[0]  = (x >> 2);
    x = base64dec3(s + 3);
    if (x < 0) return false;
    v[0] |= (x & Mask(3)) << 16;
    v[1]  = (x >> 3);
    x = base64dec3(s + 6);
    if (x < 0) return false;
    v[1] |= (x & Mask(4)) << 15;
    v[2]  = (x >> 4);
    x = base64dec2(s + 9) | base64dec2(s + 11) << 12;
    if (x < 0) return false;
    v[2] |= (x & Mask(5)) << 14;
    v[3]  = (x >> 5);
    return true;
}

static inline bool unpack19x5c16e1(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec3(s + 0);
    if (x < 0) return false;
    *e    = (x & Mask(1));
    v[0]  = (x >> 1);
    x = base64dec3(s + 3);
    if (x < 0) return false;
    v[0] |= (x & Mask(2)) << 17;
    v[1]  = (x >> 2);
    x = base64dec3(s + 6);
    if (x < 0) return false;
    v[1] |= (x & Mask(3)) << 16;
    v[2]  = (x >> 3);
    x = base64dec3(s + 9);
    if (x < 0) return false;
    v[2] |= (x & Mask(4)) << 15;
    v[3]  = (x >> 4);
    x = base64dec2(s + 12) | base64dec2(s + 14) << 12;
    if (x < 0) return false;
    v[3] |= (x & Mask(5)) << 14;
    v[4]  = (x >> 5);
    return true;
}

static inline bool unpack20x4c14e4(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    *e    = (x & Mask(4));
    v[0]  = (x >> 4);
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[1]  = (x & Mask(20));
    v[2]  = (x >> 20);
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[2] |= (x & Mask(16)) << 4;
    v[3]  = (x >> 16);
    x = base64dec2(s + 12);
    if (x < 0) return false;
    v[3] |= (x << 8);
    return true;
}

static inline bool unpack20x6c20(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec5(s + 0);
    if (x < 0) return false;
    v[0]  = (x & Mask(20));
    v[1]  = (x >> 20);
    x = base64dec5(s + 5);
    if (x < 0) return false;
    v[1] |= (x & Mask(10)) << 10;
    v[2]  = (x >> 10);
    x = base64dec5(s + 10);
    if (x < 0) return false;
    v[3]  = (x & Mask(20));
    v[4]  = (x >> 20);
    x = base64dec5(s + 15);
    if (x < 0) return false;
    v[4] |= (x & Mask(10)) << 10;
    v[5]  = (x >> 10);
    return (void) e, true;
}

static inline bool unpack21x4c14(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = (x & Mask(21));
    v[1]  = (x >> 21);
    x = base64dec3(s + 4);
    if (x < 0) return false;
    v[1] |= (x << 3);
    x = base64dec2(s + 7) | base64dec2(s + 9) << 12;
    if (x < 0) return false;
    v[2]  = (x & Mask(21));
    v[3]  = (x >> 21);
    x = base64dec3(s + 11);
    if (x < 0) return false;
    v[3] |= (x << 3);
    return (void) e, true;
}

static inline bool unpack22x4c15e2(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x, y;
    x = base64dec3(s + 0);
    if (x < 0) return false;
    v[0]  = (x & Mask(10));
    *e    = (x >> 16);
    y = base64dec2(s + 3);
    if (y < 0) return false;
    v[0] |= (y << 10);
    y = base64dec2(s + 5) | base64dec3(s + 7) << 12;
    if (y < 0) return false;
    v[1]  = (y & Mask(22));
    v[2]  = (y >> 22);
    y = base64dec2(s + 10) | base64dec3(s + 12) << 12;
    if (y < 0) return false;
    v[2] |= (y & Mask(14)) << 8;
    v[3]  = (y >> 14);
    v[3] |= (x & Mask(6)<<10) << 6;
    return true;
}

static inline bool unpack23x4c16e4(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    uint32_t e0, e1, e2, e3;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0] = x >> 1, e0 = (x & 1);
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[1] = x >> 1, e1 = (x & 1) << 1;
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[2] = x >> 1, e2 = (x & 1) << 2;
    x = base64dec2(s + 12) | base64dec2(s + 14) << 12;
    if (x < 0) return false;
    v[3] = x >> 1, e3 = (x & 1) << 3;
    *e = e0 | e1 | e2 | e3;
    return true;
}

static inline bool unpack24x4c16(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = v[0] = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    x = v[1] = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    x = v[2] = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    x = v[3] = base64dec2(s + 12) | base64dec2(s + 14) << 12;
    if (x < 0) return false;
    return (void) e, true;
}

static inline bool unpack25x3c13e3o1(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    uint32_t e0, e1, e2;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = x;
    x = (e0 = base64dec1(s + 4)) | base64dec1shl6(s + 5) | base64dec1shl12(s + 6);
    if (x < 0) return false;
    v[0] |= (x & Mask(1)) << 24;
    v[1]  = (x >> 2);
    x = base64dec1(s + 7) | (e1 = base64dec1shl6(s + 8)) | base64dec1shl12(s + 9);
    if (x < 0) return false;
    v[1] |= (x & Mask(9)) << 16;
    v[2]  = (x >> 10);
    x = base64dec1(s + 10) | base64dec1shl6(s + 11) | (e2 = base64dec1shl12(s + 12));
    if (x < 0) return false;
    v[2] |= (x & Mask(17)) << 8;
    *e = ((e0 | e1 | e2) & 0x20202) * 0x10204000 >> 29;
    return true;
}

static inline bool unpack25x4c17e2(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = x;
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[0] |= (x & Mask(1)) << 24;
    v[1]  = (x >> 1);
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[1] |= (x & Mask(2)) << 23;
    v[2]  = (x >> 2);
    x = base64dec2(s + 12) | base64dec2(s + 14) << 12;
    if (x < 0) return false;
    v[2] |= (x & Mask(3)) << 22;
    v[3]  = (x >> 3);
    x = base64dec1(s + 16);
    if (x < 0) return false;
    v[3] |= (x & Mask(4)) << 21;
    *e    = (x >> 4);
    return true;
}

static inline bool unpack26x3c13o1(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = x;
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[0] |= (x & Mask(2)) << 24;
    v[1]  = (x >> 2);
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[1] |= (x & Mask(4)) << 22;
    v[2]  = (x >> 4);
    x = base64dec1(s + 12);
    if (x < 0) return false;
    v[2] |= (x << 20);
    return (void) e, true;
}

static inline bool unpack26x4c18e4(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = x;
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[0] |= (x & Mask(2)) << 24;
    v[1]  = (x >> 2);
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[1] |= (x & Mask(4)) << 22;
    v[2]  = (x >> 4);
    x = base64dec2(s + 12) | base64dec2(s + 14) << 12;
    if (x < 0) return false;
    v[2] |= (x & Mask(6)) << 20;
    v[3]  = (x >> 6);
    x = base64dec2(s + 16);
    if (x < 0) return false;
    v[3] |= (x & Mask(8)) << 18;
    *e    = (x >> 8);
    return true;
}

static inline bool unpack27x3c14e3(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    uint32_t e0, e1, e2;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = x;
    x = (e0 = base64dec1(s + 4)) | base64dec1shl6(s + 5) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[0] |= (x & Mask(3)) << 24;
    v[1]  = (x >> 4);
    x = base64dec1(s + 8) | (e1 = base64dec1shl6(s + 9)) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[1] |= (x & Mask(7)) << 20;
    v[2]  = (x >> 8);
    x = base64dec2(s + 12);
    if (x < 0) return false;
    v[2] |= (e2 = (x << 16)) & Mask(27);
    *e = ((e0 | e1 | e2) & 0x8000088) * 0x4800010 >> 29;
    return true;
}

static inline bool unpack27x4c18(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = x;
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[0] |= (x & Mask(3)) << 24;
    v[1]  = (x >> 3);
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[1] |= (x & Mask(6)) << 21;
    v[2]  = (x >> 6);
    x = base64dec2(s + 12) | base64dec2(s + 14) << 12;
    if (x < 0) return false;
    v[2] |= (x & Mask(9)) << 18;
    v[3]  = (x >> 9 << 12);
    x = base64dec2(s + 16);
    if (x < 0) return false;
    v[3] |= x;
    return (void) e, true;
}

static inline bool unpack28x3c14(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    v[0]  = x;
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[0] |= (x & Mask(4)) << 24;
    v[1]  = (x >> 4);
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[1] |= (x & Mask(8)) << 20;
    v[2]  = (x >> 8);
    x = base64dec2(s + 12);
    if (x < 0) return false;
    v[2] |= (x << 16);
    return (void) e, true;
}

static inline bool unpack29x3c15e3o1(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x0, x1, x2;
    uint32_t e0, e1, e2;
    uint32_t mask = (1 << 29) - 1;
    x0 = base64dec2(s + 0) | base64dec2(s + 2) << 12 | (e0 = base64dec1(s + 12)) << 24;
    if (x0 < 0) return false;
    v[0] = x0 & mask;
    x1 = base64dec2(s + 4) | base64dec2(s + 6) << 12 | (e1 = base64dec1shl6(s + 13)) << 18;
    if (x1 < 0) return false;
    v[1] = x1 & mask;
    x2 = base64dec2(s + 8) | base64dec2(s + 10) << 12 | (e2 = (base64dec1(s + 14) << 24));
    if (x2 < 0) return false;
    v[2] = x2 & mask;
    *e = ((e0 | e1 | e2) & 0xa0100823) * 0x10b3014 >> 29;
    return true;
}

static inline bool unpack30x3c15o1(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12 | base64dec1(s + 12) << 24;
    if (x < 0) return false;
    v[0] = x;
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12 | base64dec1(s + 13) << 24;
    if (x < 0) return false;
    v[1] = x;
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12 | base64dec1(s + 14) << 24;
    if (x < 0) return false;
    v[2] = x;
    return (void) e, true;
}
