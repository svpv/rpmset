#include <stdint.h>
#include <stdbool.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)

static inline bool unpack22x4c15e2o1(const char *s, uint32_t *v, unsigned *e)
{
    int32_t x;
    x = base64dec2(s + 0) | base64dec2(s + 2) << 12;
    if (x < 0) return false;
    *e    = (x & Mask(2));
    v[0]  = (x >> 2);
    x = base64dec2(s + 4) | base64dec2(s + 6) << 12;
    if (x < 0) return false;
    v[1]  = (x & Mask(22));
    v[2]  = (x >> 22);
    x = base64dec2(s + 8) | base64dec2(s + 10) << 12;
    if (x < 0) return false;
    v[2] |= (x & Mask(20)) << 2;
    v[3]  = (x >> 20);
    x = base64dec2(s + 12) | base64dec1(s + 14) << 12;
    if (x < 0) return false;
    v[3] |= (x << 4);
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
