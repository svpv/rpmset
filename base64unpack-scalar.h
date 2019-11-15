#include <stdint.h>
#include <stdbool.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)

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

static inline bool unpack29x3c15e3(const char *s, uint32_t *v, unsigned *e)
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

static inline bool unpack30x3c15(const char *s, uint32_t *v, unsigned *e)
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
