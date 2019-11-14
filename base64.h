#pragma once
#include <stdint.h>

#pragma GCC visibility push(hidden)

extern const char base64[64];
extern const int8_t base64d0[256];
extern const int16_t base64d1[256];

static inline int base64dec1(const char *s)
{
    unsigned char c = *s;
    return base64d0[c];
}

static inline int base64dec2(const char *s)
{
    unsigned char c0 = s[0], c1 = s[1];
    return base64d0[c0] | base64d1[c1];
}

#pragma GCC visibility pop