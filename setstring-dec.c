#include "setstring.h"

size_t setstring_decinit(const char *s, size_t len, int *bpp)
{
    if (len < 3)
	return 0;
    *bpp = s[0] - 'a' + 7;
    if (*bpp < 7 || *bpp > 32)
	return 0;
    int m = s[1] - 'A' + 5;
    if (m < 5 || m > 30)
	return 0;
    if (m > *bpp)
	return 0;
    // Each character carries 6 bits.
    size_t bits = (len - 2) * 6;
    // Each (m + 1) bits can make a value.
    return bits / (m + 1);
}
