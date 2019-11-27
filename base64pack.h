#include <stdint.h>
#include <stdbool.h>
#include "base64.h"

#define Mask(k) ((1U << k) - 1)
#define Mask64(k) ((1ULL << k) - 1)

static inline void pack8x12c16(const uint32_t *v, char *s, unsigned e)
{
    uint32_t x;
    x = (v[0] & Mask(8)) | (v[4] & Mask(8)) << 8 | (v[8] & Mask(8)) << 16;
    s[ 0] = base64[(x & Mask(6))];
    s[ 1] = base64[(x & Mask(12)) >> 6];
    s[ 2] = base64[(x & Mask(18)) >> 12];
    s[ 3] = base64[(x >> 18)];
    x = (v[1] & Mask(8)) | (v[5] & Mask(8)) << 8 | (v[9] & Mask(8)) << 16;
    s[ 4] = base64[(x & Mask(6))];
    s[ 5] = base64[(x & Mask(12)) >> 6];
    s[ 6] = base64[(x & Mask(18)) >> 12];
    s[ 7] = base64[(x >> 18)];
    x = (v[2] & Mask(8)) | (v[6] & Mask(8)) << 8 | (v[10] & Mask(8)) << 16;
    s[ 8] = base64[(x & Mask(6))];
    s[ 9] = base64[(x & Mask(12)) >> 6];
    s[10] = base64[(x & Mask(18)) >> 12];
    s[11] = base64[(x >> 18)];
    x = (v[3] & Mask(8)) | (v[7] & Mask(8)) << 8 | (v[11] & Mask(8)) << 16;
    s[12] = base64[(x & Mask(6))];
    s[13] = base64[(x & Mask(12)) >> 6];
    s[14] = base64[(x & Mask(18)) >> 12];
    s[15] = base64[(x >> 18)];
    (void) e;
}

static inline void pack9x10c15(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(v[0] & Mask(6))];
    s[ 1] = base64[(v[0] & Mask(9)) >> 6 |
		   (v[1] & Mask(3)) << 3];
    s[ 2] = base64[(v[1] & Mask(9)) >> 3];
    s[ 3] = base64[(v[2] & Mask(6))];
    s[ 4] = base64[(v[2] & Mask(9)) >> 6 |
		   (v[3] & Mask(3)) << 3];
    s[ 5] = base64[(v[3] & Mask(9)) >> 3];
    s[ 6] = base64[(v[4] & Mask(6))];
    s[ 7] = base64[(v[4] & Mask(9)) >> 6 |
		   (v[5] & Mask(3)) << 3];
    s[ 8] = base64[(v[5] & Mask(9)) >> 3];
    s[ 9] = base64[(v[6] & Mask(6))];
    s[10] = base64[(v[6] & Mask(9)) >> 6 |
		   (v[7] & Mask(3)) << 3];
    s[11] = base64[(v[7] & Mask(9)) >> 3];
    s[12] = base64[(v[8] & Mask(6))];
    s[13] = base64[(v[8] & Mask(9)) >> 6 |
		   (v[9] & Mask(3)) << 3];
    s[14] = base64[(v[9] & Mask(9)) >> 3];
    (void) e;
}

static inline void pack9x32c48(const uint32_t *v, char *s, unsigned e)
{
    uint32_t x;
    x = (v[0] & Mask(9)) | (v[4] & Mask(9)) << 9;
    s[ 0] = base64[(x & Mask(6))];
    s[ 1] = base64[(x & Mask(12)) >> 6];
    s[ 2] = base64[(x & Mask(18)) >> 12];
    x = (v[8] & Mask(9)) | (v[12] & Mask(9)) << 9;
    s[ 3] = base64[(x & Mask(6))];
    s[16] = base64[(x & Mask(12)) >> 6];
    s[17] = base64[(x & Mask(18)) >> 12];
    x = (v[16] & Mask(9)) | (v[20] & Mask(9)) << 9;
    s[18] = base64[(x & Mask(6))];
    s[19] = base64[(x & Mask(12)) >> 6];
    s[32] = base64[(x & Mask(18)) >> 12];
    x = (v[24] & Mask(9)) | (v[28] & Mask(9)) << 9;
    s[33] = base64[(x & Mask(6))];
    s[34] = base64[(x & Mask(12)) >> 6];
    s[35] = base64[(x & Mask(18)) >> 12];

    x = (v[1] & Mask(9)) | (v[5] & Mask(9)) << 9;
    s[ 4] = base64[(x & Mask(6))];
    s[ 5] = base64[(x & Mask(12)) >> 6];
    s[ 6] = base64[(x & Mask(18)) >> 12];
    x = (v[9] & Mask(9)) | (v[13] & Mask(9)) << 9;
    s[ 7] = base64[(x & Mask(6))];
    s[20] = base64[(x & Mask(12)) >> 6];
    s[21] = base64[(x & Mask(18)) >> 12];
    x = (v[17] & Mask(9)) | (v[21] & Mask(9)) << 9;
    s[22] = base64[(x & Mask(6))];
    s[23] = base64[(x & Mask(12)) >> 6];
    s[36] = base64[(x & Mask(18)) >> 12];
    x = (v[25] & Mask(9)) | (v[29] & Mask(9)) << 9;
    s[37] = base64[(x & Mask(6))];
    s[38] = base64[(x & Mask(12)) >> 6];
    s[39] = base64[(x & Mask(18)) >> 12];

    x = (v[2] & Mask(9)) | (v[6] & Mask(9)) << 9;
    s[ 8] = base64[(x & Mask(6))];
    s[ 9] = base64[(x & Mask(12)) >> 6];
    s[10] = base64[(x & Mask(18)) >> 12];
    x = (v[10] & Mask(9)) | (v[14] & Mask(9)) << 9;
    s[11] = base64[(x & Mask(6))];
    s[24] = base64[(x & Mask(12)) >> 6];
    s[25] = base64[(x & Mask(18)) >> 12];
    x = (v[18] & Mask(9)) | (v[22] & Mask(9)) << 9;
    s[26] = base64[(x & Mask(6))];
    s[27] = base64[(x & Mask(12)) >> 6];
    s[40] = base64[(x & Mask(18)) >> 12];
    x = (v[26] & Mask(9)) | (v[30] & Mask(9)) << 9;
    s[41] = base64[(x & Mask(6))];
    s[42] = base64[(x & Mask(12)) >> 6];
    s[43] = base64[(x & Mask(18)) >> 12];

    x = (v[3] & Mask(9)) | (v[7] & Mask(9)) << 9;
    s[12] = base64[(x & Mask(6))];
    s[13] = base64[(x & Mask(12)) >> 6];
    s[14] = base64[(x & Mask(18)) >> 12];
    x = (v[11] & Mask(9)) | (v[15] & Mask(9)) << 9;
    s[15] = base64[(x & Mask(6))];
    s[28] = base64[(x & Mask(12)) >> 6];
    s[29] = base64[(x & Mask(18)) >> 12];
    x = (v[19] & Mask(9)) | (v[23] & Mask(9)) << 9;
    s[30] = base64[(x & Mask(6))];
    s[31] = base64[(x & Mask(12)) >> 6];
    s[44] = base64[(x & Mask(18)) >> 12];
    x = (v[27] & Mask(9)) | (v[31] & Mask(9)) << 9;
    s[45] = base64[(x & Mask(6))];
    s[46] = base64[(x & Mask(12)) >> 6];
    s[47] = base64[(x & Mask(18)) >> 12];
    (void) e;
}

static inline void pack10x8c14e4(const uint32_t *v, char *s, unsigned e)
{
#if 0
    uint32_t x;
    x = (v[0] & Mask( 4))
      | (v[7] & Mask(10)) >> 6 << 4
      | (  e  & Mask( 4)) << 8;
    s[ 0] = base64[(x & Mask(6))];
    s[ 1] = base64[(x >> 6)];
    s[ 2] = base64[(v[0] & Mask(10)) >> 4];
    s[ 3] = base64[(v[1] & Mask( 6))];
    s[ 4] = base64[(v[1] & Mask(10)) >> 6 |
		   (v[2] & Mask( 2)) << 4];
    s[ 5] = base64[(v[2] & Mask( 8)) >> 2];
    s[ 6] = base64[(v[2] & Mask(10)) >> 8 |
		   (v[3] & Mask( 4)) << 2];
    s[ 7] = base64[(v[3] & Mask(10)) >> 4];
    s[ 8] = base64[(v[4] & Mask( 6))];
    s[ 9] = base64[(v[4] & Mask(10)) >> 6 |
		   (v[5] & Mask( 2)) << 4];
    s[10] = base64[(v[5] & Mask( 8)) >> 2];
    s[11] = base64[(v[5] & Mask(10)) >> 8 |
		   (v[6] & Mask( 4)) << 2];
    s[12] = base64[(v[6] & Mask(10)) >> 4];
    s[13] = base64[(v[7] & Mask( 6))];
#else
    s[ 0] = base64[(  e  & Mask( 4))      |
		   (v[0] & Mask( 2)) << 4];
    s[ 1] = base64[(v[0] & Mask( 8)) >> 2];
    s[ 2] = base64[(v[0] & Mask(10)) >> 8 |
		   (v[1] & Mask( 4)) << 2];
    s[ 3] = base64[(v[1] & Mask(10)) >> 4];
    s[ 4] = base64[(v[2] & Mask(6))];
    s[ 5] = base64[(v[2] & Mask(10)) >> 6 |
		   (v[3] & Mask( 2)) << 4];
    s[ 6] = base64[(v[3] & Mask( 8)) >> 2];
    s[ 7] = base64[(v[3] & Mask(10)) >> 8 |
		   (v[4] & Mask( 4)) << 2];
    s[ 8] = base64[(v[4] & Mask(10)) >> 4];
    s[ 9] = base64[(v[5] & Mask(6))];
    s[10] = base64[(v[5] & Mask(10)) >> 6 |
		   (v[6] & Mask( 2)) << 4];
    s[11] = base64[(v[6] & Mask( 8)) >> 2];
    s[12] = base64[(v[6] & Mask(10)) >> 8 |
		   (v[7] & Mask( 4)) << 2];
    s[13] = base64[(v[7] & Mask(10)) >> 4];
#endif
}

static inline void pack10x9c15(const uint32_t *v, char *s, unsigned e)
{
    uint32_t x;
    x = (v[0] & Mask(10)) | (v[1] & Mask(10)) << 10 | (v[2] & Mask(10)) << 20;
    s[ 0] = base64[(x & Mask( 6))];
    s[ 1] = base64[(x & Mask(12)) >> 6];
    s[ 2] = base64[(x & Mask(18)) >> 12];
    s[ 3] = base64[(x & Mask(24)) >> 18];
    s[ 4] = base64[(x >> 24)];
    x = (v[3] & Mask(10)) | (v[4] & Mask(10)) << 10 | (v[5] & Mask(10)) << 20;
    s[ 5] = base64[(x & Mask( 6))];
    s[ 6] = base64[(x & Mask(12)) >> 6];
    s[ 7] = base64[(x & Mask(18)) >> 12];
    s[ 8] = base64[(x & Mask(24)) >> 18];
    s[ 9] = base64[(x >> 24)];
    x = (v[6] & Mask(10)) | (v[7] & Mask(10)) << 10 | (v[8] & Mask(10)) << 20;
    s[10] = base64[(x & Mask( 6))];
    s[11] = base64[(x & Mask(12)) >> 6];
    s[12] = base64[(x & Mask(18)) >> 12];
    s[13] = base64[(x & Mask(24)) >> 18];
    s[14] = base64[(x >> 24)];
    (void) e;
}

static inline void pack10x18c30(const uint32_t *v, char *s, unsigned e)
{
#if 1
    uint32_t x;
    x = (v[0] & Mask(10)) | (v[4] & Mask(10)) << 10 | (v[8] & Mask(4)) << 20;
    s[ 0] = base64[(x & Mask(6))];
    s[ 1] = base64[(x & Mask(12)) >> 6];
    s[ 2] = base64[(x & Mask(18)) >> 12];
    s[ 3] = base64[(x >> 18)];
    x = (v[1] & Mask(10)) | (v[5] & Mask(10)) << 10 | (v[9] & Mask(4)) << 20;
    s[ 4] = base64[(x & Mask(6))];
    s[ 5] = base64[(x & Mask(12)) >> 6];
    s[ 6] = base64[(x & Mask(18)) >> 12];
    s[ 7] = base64[(x >> 18)];
    x = (v[2] & Mask(10)) | (v[6] & Mask(10)) << 10 | (v[10] & Mask(4)) << 20;
    s[ 8] = base64[(x & Mask(6))];
    s[ 9] = base64[(x & Mask(12)) >> 6];
    s[10] = base64[(x & Mask(18)) >> 12];
    s[11] = base64[(x >> 18)];
    x = (v[3] & Mask(10)) | (v[7] & Mask(10)) << 10 | (v[11] & Mask(4)) << 20;
    s[12] = base64[(x & Mask(6))];
    s[13] = base64[(x & Mask(12)) >> 6];
    s[14] = base64[(x & Mask(18)) >> 12];
    s[15] = base64[(x >> 18)];
    uint64_t y;
    y = (v[16] & Mask(10)) | (v[17] & Mask(10)) << 10
      | (v[8] & Mask(10)) >> 4 << 20 | (uint64_t)(v[12] & Mask(10)) << 26;
    s[16] = base64[(y & Mask(6))];
    s[17] = base64[(y & Mask(12)) >> 6];
    s[18] = base64[(y & Mask(18)) >> 12];
    s[19] = base64[(y & Mask(24)) >> 18];
    s[20] = base64[(y & Mask(30)) >> 24];
    s[21] = base64[(y >> 30)];
    y = (v[9] & Mask(10)) >> 4 | (v[13] & Mask(10)) << 6
      | (v[10] & Mask(10)) >> 4 << 16 | (v[14] & Mask(10)) << 22
      | (uint64_t)(v[11] & Mask(10)) >> 4 << 32 | (uint64_t)(v[15] & Mask(10)) << 38;
    s[22] = base64[(y & Mask(6))];
    s[23] = base64[(y & Mask(12)) >> 6];
    s[24] = base64[(y & Mask(18)) >> 12];
    s[25] = base64[(y & Mask(24)) >> 18];
    s[26] = base64[(y & Mask(30)) >> 24];
    s[27] = base64[(y & Mask64(36)) >> 30];
    s[28] = base64[(y & Mask64(42)) >> 36];
    s[29] = base64[(y >> 42)];
    (void) e;
#else
    pack10x9c15(v + 0, s + 00, e);
    pack10x9c15(v + 9, s + 15, e);
#endif
}

static inline void pack10x24c40(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(v[ 0] & Mask( 6))];
    s[ 1] = base64[(v[ 0] & Mask(10)) >> 6 |
		   (v[ 4] & Mask( 2)) << 4];
    s[ 2] = base64[(v[ 4] & Mask( 8)) >> 2];
    s[ 3] = base64[(v[ 4] & Mask(10)) >> 8 |
		   (v[ 8] & Mask( 4)) << 2];
    s[ 4] = base64[(v[ 1] & Mask( 6))];
    s[ 5] = base64[(v[ 1] & Mask(10)) >> 6 |
		   (v[ 5] & Mask( 2)) << 4];
    s[ 6] = base64[(v[ 5] & Mask( 8)) >> 2];
    s[ 7] = base64[(v[ 5] & Mask(10)) >> 8 |
		   (v[ 9] & Mask( 4)) << 2];
    s[ 8] = base64[(v[ 2] & Mask( 6))];
    s[ 9] = base64[(v[ 2] & Mask(10)) >> 6 |
		   (v[ 6] & Mask( 2)) << 4];
    s[10] = base64[(v[ 6] & Mask( 8)) >> 2];
    s[11] = base64[(v[ 6] & Mask(10)) >> 8 |
		   (v[10] & Mask( 4)) << 2];
    s[12] = base64[(v[ 3] & Mask( 6))];
    s[13] = base64[(v[ 3] & Mask(10)) >> 6 |
		   (v[ 7] & Mask( 2)) << 4];
    s[14] = base64[(v[ 7] & Mask( 8)) >> 2];
    s[15] = base64[(v[ 7] & Mask(10)) >> 8 |
		   (v[11] & Mask( 4)) << 2];
    s[16] = base64[(v[ 8] & Mask(10)) >> 4];
    s[17] = base64[(v[12] & Mask( 6))];
    s[18] = base64[(v[12] & Mask(10)) >> 6 |
		   (v[16] & Mask( 2)) << 4];
    s[19] = base64[(v[16] & Mask( 8)) >> 2];
    s[20] = base64[(v[ 9] & Mask(10)) >> 4];
    s[21] = base64[(v[13] & Mask( 6))];
    s[22] = base64[(v[13] & Mask(10)) >> 6 |
		   (v[17] & Mask( 2)) << 4];
    s[23] = base64[(v[17] & Mask( 8)) >> 2];
    s[24] = base64[(v[10] & Mask(10)) >> 4];
    s[25] = base64[(v[14] & Mask( 6))];
    s[26] = base64[(v[14] & Mask(10)) >> 6 |
		   (v[18] & Mask( 2)) << 4];
    s[27] = base64[(v[18] & Mask( 8)) >> 2];
    s[28] = base64[(v[11] & Mask(10)) >> 4];
    s[29] = base64[(v[15] & Mask( 6))];
    s[30] = base64[(v[15] & Mask(10)) >> 6 |
		   (v[19] & Mask( 2)) << 4];
    s[31] = base64[(v[19] & Mask( 8)) >> 2];
    s[32] = base64[(v[16] & Mask(10)) >> 8 |
		   (v[20] & Mask( 4)) << 2];
    s[33] = base64[(v[20] & Mask(10)) >> 4];
    s[34] = base64[(v[17] & Mask(10)) >> 8 |
		   (v[21] & Mask( 4)) << 2];
    s[35] = base64[(v[21] & Mask(10)) >> 4];
    s[36] = base64[(v[18] & Mask(10)) >> 8 |
		   (v[22] & Mask( 4)) << 2];
    s[37] = base64[(v[22] & Mask(10)) >> 4];
    s[38] = base64[(v[19] & Mask(10)) >> 8 |
		   (v[23] & Mask( 4)) << 2];
    s[39] = base64[(v[23] & Mask(10)) >> 4];
    (void) e;
}

static inline void pack11x8c15e2(const uint32_t *v, char *s, unsigned e)
{
    uint32_t x;
    x = (v[0] & Mask(10))
      | (v[7] & Mask(11)) >> 5 << 10
      | (  e  & Mask( 2)) << 16;
    s[ 0] = base64[(x & Mask(6))];
    s[ 1] = base64[(x & Mask(12)) >> 6];
    s[ 2] = base64[(x >> 12)];
    s[ 3] = base64[(v[0] & Mask(11)) >> 10 |
		   (v[1] & Mask( 5)) <<  1];
    s[ 4] = base64[(v[1] & Mask(11)) >>  5];
    s[ 5] = base64[(v[2] & Mask( 6))];
    s[ 6] = base64[(v[2] & Mask(11)) >>  6 |
		   (v[3] & Mask( 1)) <<  5];
    s[ 7] = base64[(v[3] & Mask( 7)) >>  1];
    s[ 8] = base64[(v[3] & Mask(11)) >>  7 |
		   (v[4] & Mask( 2)) <<  4];
    s[ 9] = base64[(v[4] & Mask( 8)) >>  2];
    s[10] = base64[(v[4] & Mask(11)) >>  8 |
		   (v[5] & Mask( 3)) <<  3];
    s[11] = base64[(v[5] & Mask( 9)) >>  3];
    s[12] = base64[(v[5] & Mask(11)) >>  9 |
		   (v[6] & Mask( 4)) <<  2];
    s[13] = base64[(v[6] & Mask(10)) >>  4];
    s[14] = base64[(v[6] & Mask(11)) >> 10 |
		   (v[7] & Mask( 5)) <<  1];
}

static inline void pack12x8c16(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(v[0] & Mask( 6))];
    s[ 1] = base64[(v[0] & Mask(12)) >> 6];
    s[ 2] = base64[(v[4] & Mask( 6))];
    s[ 3] = base64[(v[4] & Mask(12)) >> 6];
    s[ 4] = base64[(v[1] & Mask( 6))];
    s[ 5] = base64[(v[1] & Mask(12)) >> 6];
    s[ 6] = base64[(v[5] & Mask( 6))];
    s[ 7] = base64[(v[5] & Mask(12)) >> 6];
    s[ 8] = base64[(v[2] & Mask( 6))];
    s[ 9] = base64[(v[2] & Mask(12)) >> 6];
    s[10] = base64[(v[6] & Mask( 6))];
    s[11] = base64[(v[6] & Mask(12)) >> 6];
    s[12] = base64[(v[3] & Mask( 6))];
    s[13] = base64[(v[3] & Mask(12)) >> 6];
    s[14] = base64[(v[7] & Mask( 6))];
    s[15] = base64[(v[7] & Mask(12)) >> 6];
    (void) e;
}

static inline void pack13x6c13(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(v[0] & Mask( 6))];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(13)) >> 12 |
		   (v[1] & Mask( 5)) <<  1];
    s[ 3] = base64[(v[1] & Mask(11)) >>  5];
    s[ 4] = base64[(v[1] & Mask(13)) >> 11 |
		   (v[2] & Mask( 4)) <<  2];
    s[ 5] = base64[(v[2] & Mask(10)) >>  4];
    s[ 6] = base64[(v[2] & Mask(13)) >> 10 |
		   (v[3] & Mask( 3)) <<  3];
    s[ 7] = base64[(v[3] & Mask( 9)) >>  3];
    s[ 8] = base64[(v[3] & Mask(13)) >>  9 |
		   (v[4] & Mask( 2)) <<  4];
    s[ 9] = base64[(v[4] & Mask( 8)) >>  2];
    s[10] = base64[(v[4] & Mask(13)) >>  8 |
		   (v[5] & Mask( 1)) <<  5];
    s[11] = base64[(v[5] & Mask( 7)) >>  1];
    s[12] = base64[(v[5] & Mask(13)) >>  7];
    (void) e;
}

static inline void pack13x7c16e5(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(  e  & Mask( 5))       |
		   (v[0] & Mask( 1)) <<  5];
    s[ 1] = base64[(v[0] & Mask( 7)) >>  1];
    s[ 2] = base64[(v[0] & Mask(13)) >>  7];
    s[ 3] = base64[(v[1] & Mask( 6))];
    s[ 4] = base64[(v[1] & Mask(12)) >>  6];
    s[ 5] = base64[(v[1] & Mask(13)) >> 12 |
		   (v[2] & Mask( 5)) <<  1];
    s[ 6] = base64[(v[2] & Mask(11)) >>  5];
    s[ 7] = base64[(v[2] & Mask(13)) >> 11 |
		   (v[3] & Mask( 4)) <<  2];
    s[ 8] = base64[(v[3] & Mask(10)) >>  4];
    s[ 9] = base64[(v[3] & Mask(13)) >> 10 |
		   (v[4] & Mask( 3)) <<  3];
    s[10] = base64[(v[4] & Mask( 9)) >>  3];
    s[11] = base64[(v[4] & Mask(13)) >>  9 |
		   (v[5] & Mask( 2)) <<  4];
    s[12] = base64[(v[5] & Mask( 8)) >>  2];
    s[13] = base64[(v[5] & Mask(13)) >>  8 |
		   (v[6] & Mask( 1)) <<  5];
    s[14] = base64[(v[6] & Mask( 7)) >>  1];
    s[15] = base64[(v[6] & Mask(13)) >>  7];
}

static inline void pack14x6c14(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(v[0] & Mask( 6))];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(14)) >> 12 |
		   (v[1] & Mask( 4)) <<  2];
    s[ 3] = base64[(v[1] & Mask(10)) >>  4];
    s[ 4] = base64[(v[1] & Mask(14)) >> 10 |
		   (v[2] & Mask( 2)) <<  4];
    s[ 5] = base64[(v[2] & Mask( 8)) >>  2];
    s[ 6] = base64[(v[2] & Mask(14)) >>  8];
    s[ 7] = base64[(v[3] & Mask( 6))];
    s[ 8] = base64[(v[3] & Mask(12)) >>  6];
    s[ 9] = base64[(v[3] & Mask(14)) >> 12 |
		   (v[4] & Mask( 4)) <<  2];
    s[10] = base64[(v[4] & Mask(10)) >>  4];
    s[11] = base64[(v[4] & Mask(14)) >> 10 |
		   (v[5] & Mask( 2)) <<  4];
    s[12] = base64[(v[5] & Mask( 8)) >>  2];
    s[13] = base64[(v[5] & Mask(14)) >>  8];
    (void) e;
}

static inline void pack15x6c15(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(v[0] & Mask( 6))];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(15)) >> 12 |
		   (v[1] & Mask( 3)) <<  3];
    s[ 3] = base64[(v[1] & Mask( 9)) >>  3];
    s[ 4] = base64[(v[1] & Mask(15)) >>  9];
    s[ 5] = base64[(v[2] & Mask( 6))];
    s[ 6] = base64[(v[2] & Mask(12)) >>  6];
    s[ 7] = base64[(v[2] & Mask(15)) >> 12 |
		   (v[3] & Mask( 3)) <<  3];
    s[ 8] = base64[(v[3] & Mask( 9)) >>  3];
    s[ 9] = base64[(v[3] & Mask(15)) >>  9];
    s[10] = base64[(v[4] & Mask( 6))];
    s[11] = base64[(v[4] & Mask(12)) >>  6];
    s[12] = base64[(v[4] & Mask(15)) >> 12 |
		   (v[5] & Mask( 3)) <<  3];
    s[13] = base64[(v[5] & Mask( 9)) >>  3];
    s[14] = base64[(v[5] & Mask(15)) >>  9];
    (void) e;
}

static inline void pack16x6c16(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(v[0] & Mask( 6))];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(16)) >> 12 |
		   (v[1] & Mask( 2)) <<  4];
    s[ 3] = base64[(v[1] & Mask( 8)) >>  2];
    s[ 4] = base64[(v[1] & Mask(14)) >>  8];
    s[ 5] = base64[(v[1] & Mask(16)) >> 14 |
		   (v[2] & Mask( 4)) <<  2];
    s[ 6] = base64[(v[2] & Mask(10)) >>  4];
    s[ 7] = base64[(v[2] & Mask(16)) >> 10];
    s[ 8] = base64[(v[3] & Mask( 6))];
    s[ 9] = base64[(v[3] & Mask(12)) >>  6];
    s[10] = base64[(v[3] & Mask(16)) >> 12 |
		   (v[4] & Mask( 2)) <<  4];
    s[11] = base64[(v[4] & Mask( 8)) >>  2];
    s[12] = base64[(v[4] & Mask(14)) >>  8];
    s[13] = base64[(v[4] & Mask(16)) >> 14 |
		   (v[5] & Mask( 4)) <<  2];
    s[14] = base64[(v[5] & Mask(10)) >>  4];
    s[15] = base64[(v[5] & Mask(16)) >> 10];
    (void) e;
}

static inline void pack17x5c15e5(const uint32_t *v, char *s, unsigned e)
{
    uint32_t x;
    x = (v[1] & Mask(10))
      | (v[4] & Mask(17)) >> 14 << 10
      | (  e  & Mask( 5)) << 13;
    s[ 0] = base64[(x & Mask(6))];
    s[ 1] = base64[(x & Mask(12)) >> 6];
    s[ 2] = base64[(x >> 12)];
    x = (v[1] & Mask(17)) >> 10
      | (v[0] & Mask(17)) << 7;
    s[ 3] = base64[(x & Mask(6))];
    s[ 4] = base64[(x & Mask(12)) >> 6];
    s[ 5] = base64[(x & Mask(18)) >> 12];
    s[ 6] = base64[(x >> 18)];
    x = (v[2] & Mask(17))
      | (v[3] & Mask(7)) << 17;
    s[ 7] = base64[(x & Mask(6))];
    s[ 8] = base64[(x & Mask(12)) >> 6];
    s[ 9] = base64[(x & Mask(18)) >> 12];
    s[10] = base64[(x >> 18)];
    x = (v[3] & Mask(17)) >> 7
      | (v[4] & Mask(14)) << 10;
    s[11] = base64[(x & Mask(6))];
    s[12] = base64[(x & Mask(12)) >> 6];
    s[13] = base64[(x & Mask(18)) >> 12];
    s[14] = base64[(x >> 18)];
}

static inline void pack17x6c17(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(v[0] & Mask( 6))];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(17)) >> 12 |
		   (v[1] & Mask( 1)) <<  5];
    s[ 3] = base64[(v[1] & Mask( 7)) >>  1];
    s[ 4] = base64[(v[1] & Mask(13)) >>  7];
    s[ 5] = base64[(v[1] & Mask(17)) >> 13 |
		   (v[2] & Mask( 2)) <<  4];
    s[ 6] = base64[(v[2] & Mask( 8)) >>  2];
    s[ 7] = base64[(v[2] & Mask(14)) >>  8];
    s[ 8] = base64[(v[2] & Mask(17)) >> 14 |
		   (v[3] & Mask( 3)) <<  3];
    s[ 9] = base64[(v[3] & Mask( 9)) >>  3];
    s[10] = base64[(v[3] & Mask(15)) >>  9];
    s[11] = base64[(v[3] & Mask(17)) >> 15 |
		   (v[4] & Mask( 4)) <<  2];
    s[12] = base64[(v[4] & Mask(10)) >>  4];
    s[13] = base64[(v[4] & Mask(16)) >> 10];
    s[14] = base64[(v[4] & Mask(17)) >> 16 |
		   (v[5] & Mask( 5)) <<  1];
    s[15] = base64[(v[5] & Mask(11)) >>  5];
    s[16] = base64[(v[5] & Mask(17)) >> 11];
    (void) e;
}

static inline void pack18x5c15(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(v[0] & Mask( 6))];
    s[ 1] = base64[(v[0] & Mask(12)) >>  6];
    s[ 2] = base64[(v[0] & Mask(18)) >> 12];
    s[ 3] = base64[(v[1] & Mask( 6))];
    s[ 4] = base64[(v[1] & Mask(12)) >>  6];
    s[ 5] = base64[(v[1] & Mask(18)) >> 12];
    s[ 6] = base64[(v[2] & Mask( 6))];
    s[ 7] = base64[(v[2] & Mask(12)) >>  6];
    s[ 8] = base64[(v[2] & Mask(18)) >> 12];
    s[ 9] = base64[(v[3] & Mask( 6))];
    s[10] = base64[(v[3] & Mask(12)) >>  6];
    s[11] = base64[(v[3] & Mask(18)) >> 12];
    s[12] = base64[(v[4] & Mask( 6))];
    s[13] = base64[(v[4] & Mask(12)) >>  6];
    s[14] = base64[(v[4] & Mask(18)) >> 12];
    (void) e;
}

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

static inline void pack19x5c16e1(const uint32_t *v, char *s, unsigned e)
{
    s[ 0] = base64[(  e  & Mask( 1))       |
		   (v[0] & Mask( 5)) <<  1];
    s[ 1] = base64[(v[0] & Mask(11)) >>  5];
    s[ 2] = base64[(v[0] & Mask(17)) >> 11];
    s[ 3] = base64[(v[0] & Mask(19)) >> 17 |
		   (v[1] & Mask( 4)) <<  2];
    s[ 4] = base64[(v[1] & Mask(10)) >>  4];
    s[ 5] = base64[(v[1] & Mask(16)) >> 10];
    s[ 6] = base64[(v[1] & Mask(19)) >> 16 |
		   (v[2] & Mask( 3)) <<  3];
    s[ 7] = base64[(v[2] & Mask( 9)) >>  3];
    s[ 8] = base64[(v[2] & Mask(15)) >>  9];
    s[ 9] = base64[(v[2] & Mask(19)) >> 15 |
		   (v[3] & Mask( 2)) <<  4];
    s[10] = base64[(v[3] & Mask( 8)) >>  2];
    s[11] = base64[(v[3] & Mask(14)) >>  8];
    s[12] = base64[(v[3] & Mask(19)) >> 14 |
		   (v[4] & Mask( 1)) <<  5];
    s[13] = base64[(v[4] & Mask( 7)) >>  1];
    s[14] = base64[(v[4] & Mask(13)) >>  7];
    s[15] = base64[(v[4] & Mask(19)) >> 13];
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
