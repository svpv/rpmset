#include <stdint.h>
#include "base64.h"

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
