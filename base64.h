#pragma once
#include <stdint.h>

#pragma GCC visibility push(hidden)

extern const char base64[64];
extern const int8_t base64d0[256];
extern const int16_t base64d1[256];
extern const int32_t base64d2[256];
extern const int32_t base64d3[256];

// These routines deliberately return uint32_t, so that left shifts do not
// trigger undefined behavior.  To check the result, assign to int32_t:
//
//	int32_t b24 = base64dec2(s) | base64dec2(s + 2) << 12;
//	if (b24 < 0) return false;

static inline uint32_t base64dec1(const char *s)
{
    unsigned char c = *s;
    return base64d0[c];
}

static inline uint32_t base64dec1shl6(const char *s)
{
    unsigned char c = *s;
    return base64d1[c];
}

static inline uint32_t base64dec1shl12(const char *s)
{
    unsigned char c = *s;
    return base64d2[c];
}

static inline uint32_t base64dec2c(unsigned char c0, unsigned char c1)
{
    return base64d0[c0] | base64d1[c1];
}

static inline uint32_t base64dec3c(unsigned char c0, unsigned char c1, unsigned char c2)
{
    return base64d0[c0] | base64d1[c1] | base64d2[c2];
}

static inline uint32_t base64dec4c(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3)
{
    return base64d0[c0] | base64d1[c1] | base64d2[c2] | base64d3[c3];
}

static inline uint32_t base64dec2(const char *s)
{
    return base64dec2c(s[0], s[1]);
}

static inline uint32_t base64dec3(const char *s)
{
    return base64dec3c(s[0], s[1], s[2]);
}

static inline uint32_t base64dec4(const char *s)
{
    return base64dec4c(s[0], s[1], s[2], s[3]);
}

static inline uint64_t base64wdec2(const char *s)
{
    unsigned char c0 = s[0], c1 = s[1];
    return base64d0[c0] | base64d1[c1];
}

static inline uint64_t base64wdec3(const char *s)
{
    unsigned char c0 = s[0], c1 = s[1], c2 = s[2];
    return base64d0[c0] | base64d1[c1] | base64d2[c2];
}

#pragma GCC visibility pop
