#include <stdint.h>
#include <stdbool.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)

static inline void pack19x4c13e2(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(  e  & Mask( 2))       |
		   (v[0] & Mask( 4)) <<  2];
    s[ 1] = base64[(v[0] & Mask(10)) >>  4];
    s[ 2] = base64[(v[0] & Mask(16)) >> 10];
    s[ 3] = base64[(v[0] & Mask(19)) >> 16 |
		   (v[1] & Mask( 3)) <<  3];
    s[ 4] = base64[(v[1] & Mask( 9)) >>  3];
    s[ 5] = base64[(v[1] & Mask(15)) >>  9];
    s[ 6] = base64[(v[1] & Mask(19)) >> 15 |
		   (v[2] & Mask( 2)) <<  4];
    s[ 7] = base64[(v[2] & Mask( 8)) >>  2];
    s[ 8] = base64[(v[2] & Mask(14)) >>  8];
    s[ 9] = base64[(v[2] & Mask(19)) >> 14 |
		   (v[3] & Mask( 1)) <<  5];
    s[10] = base64[(v[3] & Mask( 7)) >>  1];
    s[11] = base64[(v[3] & Mask(13)) >>  7];
    s[12] = base64[(v[3] & Mask(19)) >> 13];
}

static inline void pack20x4c14e4(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(  e  & Mask( 4))       |
		   (v[0] & Mask( 2)) <<  4];
    s[ 1] = base64[(v[0] & Mask( 8)) >>  2];
    s[ 2] = base64[(v[0] & Mask(14)) >>  8];
    s[ 3] = base64[(v[0] & Mask(20)) >> 14];
    s[ 4] = base64[ v[1] & Mask( 6)];
    s[ 5] = base64[(v[1] & Mask(12)) >>  6];
    s[ 6] = base64[(v[1] & Mask(18)) >> 12];
    s[ 7] = base64[(v[1] & Mask(20)) >> 18 |
		   (v[2] & Mask( 4)) <<  2];
    s[ 8] = base64[(v[2] & Mask(10)) >>  4];
    s[ 9] = base64[(v[2] & Mask(16)) >> 10];
    s[10] = base64[(v[2] & Mask(20)) >> 16 |
		   (v[3] & Mask( 2)) <<  4];
    s[11] = base64[(v[3] & Mask( 8)) >>  2];
    s[12] = base64[(v[3] & Mask(14)) >>  8];
    s[13] = base64[(v[3] & Mask(20)) >> 14];
}

static inline void pack21x4c14(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[ v[0] & Mask( 6)];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[0] & Mask(21)) >> 18 |
		   (v[1] & Mask( 3)) <<  3];
    s[ 4] = base64[(v[1] & Mask( 9)) >>  3];
    s[ 5] = base64[(v[1] & Mask(15)) >>  9];
    s[ 6] = base64[(v[1] & Mask(21)) >> 15];
    s[ 7] = base64[ v[2] & Mask( 6)];
    s[ 8] = base64[(v[2] & Mask(12)) >>  6];
    s[ 9] = base64[(v[2] & Mask(18)) >> 12];
    s[10] = base64[(v[2] & Mask(21)) >> 18 |
		   (v[3] & Mask( 3)) <<  3];
    s[11] = base64[(v[3] & Mask( 9)) >>  3];
    s[12] = base64[(v[3] & Mask(15)) >>  9];
    s[13] = base64[(v[3] & Mask(21)) >> 15];
    (void) e;
}

static inline void pack22x4c15e2(const uint32_t *v, char *s, unsigned e)
{
    uint32_t x = (v[0] & Mask(10))
	       | (v[3] & Mask(22)) >> 16 << 10
	       | (  e  & Mask( 2)) << 16;
    s[ 0] = base64[(x & Mask( 6))];
    s[ 1] = base64[(x & Mask(12)) >>  6];
    s[ 2] = base64[(x >> 12)];
    s[ 3] = base64[(v[0] & Mask(16)) >> 10];
    s[ 4] = base64[(v[0] & Mask(22)) >> 16];
    s[ 5] = base64[ v[1] & Mask( 6)];
    s[ 6] = base64[(v[1] & Mask(12)) >>  6];
    s[ 7] = base64[(v[1] & Mask(18)) >> 12];
    s[ 8] = base64[(v[1] & Mask(22)) >> 18 |
		   (v[2] & Mask( 2)) <<  4];
    s[ 9] = base64[(v[2] & Mask( 8)) >>  2];
    s[10] = base64[(v[2] & Mask(14)) >>  8];
    s[11] = base64[(v[2] & Mask(20)) >> 14];
    s[12] = base64[(v[2] & Mask(22)) >> 20 |
		   (v[3] & Mask( 4)) <<  2];
    s[13] = base64[(v[3] & Mask(10)) >>  4];
    s[14] = base64[(v[3] & Mask(16)) >> 10];
}

static inline void pack23x4c16e4(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(v[0] & Mask( 5)) <<  1 | (e & 1)];
    s[ 1] = base64[(v[0] & Mask(11)) >>  5];
    s[ 2] = base64[(v[0] & Mask(17)) >> 11];
    s[ 3] = base64[(v[0] & Mask(23)) >> 17];
    s[ 4] = base64[(v[1] & Mask( 5)) <<  1 | (e & 2) >> 1];
    s[ 5] = base64[(v[1] & Mask(11)) >>  5];
    s[ 6] = base64[(v[1] & Mask(17)) >> 11];
    s[ 7] = base64[(v[1] & Mask(23)) >> 17];
    s[ 8] = base64[(v[2] & Mask( 5)) <<  1 | (e & 4) >> 2];
    s[ 9] = base64[(v[2] & Mask(11)) >>  5];
    s[10] = base64[(v[2] & Mask(17)) >> 11];
    s[11] = base64[(v[2] & Mask(23)) >> 17];
    s[12] = base64[(v[3] & Mask( 5)) <<  1 | (e & 8) >> 3];
    s[13] = base64[(v[3] & Mask(11)) >>  5];
    s[14] = base64[(v[3] & Mask(17)) >> 11];
    s[15] = base64[(v[3] & Mask(23)) >> 17];
}

static inline void pack24x4c16(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[ v[0] & Mask( 6)];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[0] & Mask(24)) >> 18];
    s[ 4] = base64[ v[1] & Mask( 6)];
    s[ 5] = base64[(v[1] & Mask(12)) >>  6];
    s[ 6] = base64[(v[1] & Mask(18)) >> 12];
    s[ 7] = base64[(v[1] & Mask(24)) >> 18];
    s[ 8] = base64[ v[2] & Mask( 6)];
    s[ 9] = base64[(v[2] & Mask(12)) >>  6];
    s[10] = base64[(v[2] & Mask(18)) >> 12];
    s[11] = base64[(v[2] & Mask(24)) >> 18];
    s[12] = base64[ v[3] & Mask( 6)];
    s[13] = base64[(v[3] & Mask(12)) >>  6];
    s[14] = base64[(v[3] & Mask(18)) >> 12];
    s[15] = base64[(v[3] & Mask(24)) >> 18];
    (void) e;
}

static inline void pack25x3c13e3(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[ v[0] & Mask( 6)];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[0] & Mask(24)) >> 18];
    s[ 4] = base64[(v[0] & Mask(25)) >> 24 | (e & 1) << 1 |
		   (v[1] & Mask( 4)) <<  2];
    s[ 5] = base64[(v[1] & Mask(10)) >>  4];
    s[ 6] = base64[(v[1] & Mask(16)) >> 10];
    s[ 7] = base64[(v[1] & Mask(22)) >> 16];
    s[ 8] = base64[(v[1] & Mask(25)) >> 22 | (e & 2) << 2 |
		   (v[2] & Mask( 2)) <<  4];
    s[ 9] = base64[(v[2] & Mask( 8)) >>  2];
    s[10] = base64[(v[2] & Mask(14)) >>  8];
    s[11] = base64[(v[2] & Mask(20)) >> 14];
    s[12] = base64[(v[2] & Mask(25)) >> 20 | (e & 4) << 3];
}

static inline void pack25x4c17e2(const uint32_t *v, char *s, unsigned e)
{
    // 66661|56662|46663|36664|2
    s[ 0] = base64[ v[0] & Mask( 6)];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[0] & Mask(24)) >> 18];
    s[ 4] = base64[(v[0] & Mask(25)) >> 24 |
		   (v[1] & Mask( 5)) <<  1];
    s[ 5] = base64[(v[1] & Mask(11)) >>  5];
    s[ 6] = base64[(v[1] & Mask(17)) >> 11];
    s[ 7] = base64[(v[1] & Mask(23)) >> 17];
    s[ 8] = base64[(v[1] & Mask(25)) >> 23 |
		   (v[2] & Mask( 4)) <<  2];
    s[ 9] = base64[(v[2] & Mask(10)) >>  4];
    s[10] = base64[(v[2] & Mask(16)) >> 10];
    s[11] = base64[(v[2] & Mask(22)) >> 16];
    s[12] = base64[(v[2] & Mask(25)) >> 22 |
		   (v[3] & Mask( 3)) <<  3];
    s[13] = base64[(v[3] & Mask( 9)) >>  3];
    s[14] = base64[(v[3] & Mask(15)) >>  9];
    s[15] = base64[(v[3] & Mask(21)) >> 15];
    s[16] = base64[(v[3] & Mask(25)) >> 21 |
		   (  e  & Mask( 2)) <<  4];
}

static inline void pack26x3c13(const uint32_t *v, char *s, unsigned e)
{
    // 66662|46664|26666
    s[ 0] = base64[ v[0] & Mask( 6)];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[0] & Mask(24)) >> 18];
    s[ 4] = base64[(v[0] & Mask(26)) >> 24 |
		   (v[1] & Mask( 4)) <<  2];
    s[ 5] = base64[(v[1] & Mask(10)) >>  4];
    s[ 6] = base64[(v[1] & Mask(16)) >> 10];
    s[ 7] = base64[(v[1] & Mask(22)) >> 16];
    s[ 8] = base64[(v[1] & Mask(26)) >> 22 |
		   (v[2] & Mask( 2)) <<  4];
    s[ 9] = base64[(v[2] & Mask( 8)) >>  2];
    s[10] = base64[(v[2] & Mask(14)) >>  8];
    s[11] = base64[(v[2] & Mask(20)) >> 14];
    s[12] = base64[(v[2] & Mask(26)) >> 20];
    (void) e;
}

static inline void pack26x4c18e4(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[ v[0] & Mask( 6)];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[0] & Mask(24)) >> 18];
    s[ 4] = base64[(v[0] & Mask(26)) >> 24 |
		   (v[1] & Mask( 4)) <<  2];
    s[ 5] = base64[(v[1] & Mask(10)) >>  4];
    s[ 6] = base64[(v[1] & Mask(16)) >> 10];
    s[ 7] = base64[(v[1] & Mask(22)) >> 16];
    s[ 8] = base64[(v[1] & Mask(26)) >> 22 |
		   (v[2] & Mask( 2)) <<  4];
    s[ 9] = base64[(v[2] & Mask( 8)) >>  2];
    s[10] = base64[(v[2] & Mask(14)) >>  8];
    s[11] = base64[(v[2] & Mask(20)) >> 14];
    s[12] = base64[(v[2] & Mask(26)) >> 20];
    s[13] = base64[ v[3] & Mask( 6)];
    s[14] = base64[(v[3] & Mask(12)) >>  6];
    s[15] = base64[(v[3] & Mask(18)) >> 12];
    s[16] = base64[(v[3] & Mask(24)) >> 18];
    s[17] = base64[(v[3] & Mask(26)) >> 24 |
		   (  e  & Mask( 4)) <<  2];
}

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
