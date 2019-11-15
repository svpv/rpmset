#include <stdint.h>
#include <stdbool.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)

static inline void pack27x3c14e3(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[ v[0] & Mask( 6)];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[0] & Mask(24)) >> 18];
    s[ 4] = base64[(v[0] & Mask(27)) >> 24 | (e & 1) << 3 |
	           (v[1] & Mask( 2)) <<  4];
    s[ 5] = base64[(v[1] & Mask( 8)) >>  2];
    s[ 6] = base64[(v[1] & Mask(14)) >>  8];
    s[ 7] = base64[(v[1] & Mask(20)) >> 14];
    s[ 8] = base64[(v[1] & Mask(26)) >> 20];
    s[ 9] = base64[(v[1] & Mask(27)) >> 26 | (e & 2) |
	           (v[2] & Mask( 4)) <<  2];
    s[10] = base64[(v[2] & Mask(10)) >>  4];
    s[11] = base64[(v[2] & Mask(16)) >> 10];
    s[12] = base64[(v[2] & Mask(22)) >> 16];
    s[13] = base64[(v[2] & Mask(27)) >> 22 | (e & 4) << 3];
}

static inline void pack27x4c18(const uint32_t *v, char *s, unsigned e)
{
    // 66663|36666
    s[ 0] = base64[ v[0] & Mask( 6)];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[0] & Mask(24)) >> 18];
    s[ 4] = base64[(v[0] & Mask(27)) >> 24 |
		   (v[1] & Mask( 3)) <<  3];
    s[ 5] = base64[(v[1] & Mask( 9)) >>  3];
    s[ 6] = base64[(v[1] & Mask(15)) >>  9];
    s[ 7] = base64[(v[1] & Mask(21)) >> 15];
    s[ 8] = base64[(v[1] & Mask(27)) >> 21];
    s[ 9] = base64[ v[2] & Mask( 6)];
    s[10] = base64[(v[2] & Mask(12)) >>  6];
    s[11] = base64[(v[2] & Mask(18)) >> 12];
    s[12] = base64[(v[2] & Mask(24)) >> 18];
    // The last number is skewed, low 12 bits in the last two bytes.
    s[13] = base64[(v[2] & Mask(27)) >> 24 |
		   (v[3] & Mask(15)) >> 12 << 3];
    s[14] = base64[(v[3] & Mask(21)) >> 15];
    s[15] = base64[(v[3] & Mask(27)) >> 21];
    s[16] = base64[ v[3] & Mask(6)];
    s[17] = base64[(v[3] & Mask(12)) >>  6];
    (void) e;
}

static inline void pack28x3c14(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[ v[0] & Mask( 6)];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[0] & Mask(24)) >> 18];
    s[ 4] = base64[(v[0] & Mask(28)) >> 24 |
	           (v[1] & Mask( 2)) <<  4];
    s[ 5] = base64[(v[1] & Mask( 8)) >>  2];
    s[ 6] = base64[(v[1] & Mask(14)) >>  8];
    s[ 7] = base64[(v[1] & Mask(20)) >> 14];
    s[ 8] = base64[(v[1] & Mask(26)) >> 20];
    s[ 9] = base64[(v[1] & Mask(28)) >> 26 |
	           (v[2] & Mask( 4)) <<  2];
    s[10] = base64[(v[2] & Mask(10)) >>  4];
    s[11] = base64[(v[2] & Mask(16)) >> 10];
    s[12] = base64[(v[2] & Mask(22)) >> 16];
    s[13] = base64[(v[2] & Mask(28)) >> 22];
    (void) e;
}

static inline void pack29x3c15e3(const uint32_t *v, char *s, unsigned e)
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
    s[12] = base64[(v[0] & Mask(29)) >> 24 | (e & 1) << 5];
    s[13] = base64[(v[1] & Mask(29)) >> 24 | (e & 2) << 4];
    s[14] = base64[(v[2] & Mask(29)) >> 24 | (e & 4) << 3];
}

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