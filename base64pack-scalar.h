#include <stdint.h>
#include <stdbool.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)

static inline void pack30x3c15(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[ v[0] & Mask(6)];
    s[ 1] = base64[(v[0] & Mask(12)) >> 6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[0] & Mask(24)) >> 18];
    s[ 4] = base64[ v[1] & Mask(6)];
    s[ 5] = base64[(v[1] & Mask(12)) >> 6];
    s[ 6] = base64[(v[1] & Mask(18)) >> 12];
    s[ 7] = base64[(v[1] & Mask(24)) >> 18];
    s[ 8] = base64[ v[2] & Mask(6)];
    s[ 9] = base64[(v[2] & Mask(12)) >> 6];
    s[10] = base64[(v[2] & Mask(18)) >> 12];
    s[11] = base64[(v[2] & Mask(24)) >> 18];
    s[12] = base64[(v[0] & Mask(30)) >> 24];
    s[13] = base64[(v[1] & Mask(30)) >> 24];
    s[14] = base64[(v[2] & Mask(30)) >> 24];
    (void) e;
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
