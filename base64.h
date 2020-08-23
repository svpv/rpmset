#pragma once
#include <stdint.h>

#pragma GCC visibility push(hidden)

extern const char base64[64];

struct base64dtab {
    const int8_t d0[256];
    const int16_t d1[256];
    const int32_t d2[256];
    const int32_t d3[256];
    const int32_t d4[256];
};

extern const struct base64dtab base64dtab;
#define base64d0 base64dtab.d0
#define base64d1 base64dtab.d1
#define base64d2 base64dtab.d2
#define base64d3 base64dtab.d3
#define base64d4 base64dtab.d4

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

static inline uint32_t base64dec5c(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
{
    return base64d0[c0] | base64d1[c1] | (base64d0[c2] | base64d1[c3] | base64d2[c4]) << 12;
}

static inline uint64_t base64dec6c(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5)
{
    return  (int64_t) base64d0[c0] | (int64_t) base64d1[c1] | (int64_t) base64d2[c2] |
	   ((int64_t) base64d0[c3] | (int64_t) base64d1[c4] | (int64_t) base64d2[c5]) << 18;
}

static inline uint64_t base64dec7c(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5, unsigned char c6)
{
    return  (int64_t) base64d0[c0] | (int64_t) base64d1[c1] |
	   ((int64_t) base64d0[c2] | (int64_t) base64d1[c3]) << 12 |
	   ((int64_t) base64d0[c4] | (int64_t) base64d1[c5] | (int64_t) base64d2[c6]) << 24;
}

static inline uint64_t base64dec8c(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5, unsigned char c6, unsigned char c7)
{
    return  (int64_t) base64d0[c0] | (int64_t) base64d1[c1] |
	   ((int64_t) base64d0[c2] | (int64_t) base64d1[c3] | (int64_t) base64d2[c4]) << 12 |
	   ((int64_t) base64d0[c5] | (int64_t) base64d1[c6] | (int64_t) base64d2[c7]) << 30;
}

static inline uint64_t base64dec9c(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5, unsigned char c6, unsigned char c7, unsigned char c8)
{
    return  (int64_t) base64d0[c0] | (int64_t) base64d1[c1] | (int64_t) base64d2[c2] |
	   ((int64_t) base64d0[c3] | (int64_t) base64d1[c4] | (int64_t) base64d2[c5]) << 18 |
	   ((int64_t) base64d0[c6] | (int64_t) base64d1[c7] | (int64_t) base64d2[c8]) << 36;
}

static inline uint64_t base64dec10c(unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char c5, unsigned char c6, unsigned char c7, unsigned char c8, unsigned char c9)
{
    return  (int64_t) base64d0[c0] | (int64_t) base64d1[c1] |
	   ((int64_t) base64d0[c2] | (int64_t) base64d1[c3]) << 12 |
	   ((int64_t) base64d0[c4] | (int64_t) base64d1[c5] | (int64_t) base64d2[c6]) << 24 |
	   ((int64_t) base64d0[c7] | (int64_t) base64d1[c8] | (int64_t) base64d2[c9]) << 42;
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

static inline uint32_t base64dec5(const char *s)
{
    return base64dec5c(s[0], s[1], s[2], s[3], s[4]);
}

static inline uint64_t base64dec6(const char *s)
{
    return base64dec6c(s[0], s[1], s[2], s[3], s[4], s[5]);
}

static inline uint64_t base64dec7(const char *s)
{
    return base64dec7c(s[0], s[1], s[2], s[3], s[4], s[5], s[6]);
}

static inline uint64_t base64dec8(const char *s)
{
    return base64dec8c(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
}

static inline uint64_t base64dec9(const char *s)
{
    return base64dec9c(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8]);
}

static inline uint64_t base64dec10(const char *s)
{
    return base64dec10c(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9]);
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
