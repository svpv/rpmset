#include <stdint.h>
#include <string.h>
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(x, 0)

#pragma GCC visibility push(hidden)
size_t cache_decode(const char *s, size_t len, const uint32_t **pv);
size_t downsample(const uint32_t *v, size_t n, uint32_t *w, int bpp);

#ifdef __x86_64__
#define SENTINELS 9
int setcmploop(const uint32_t *Pv, size_t Pn, const uint32_t *Rv, size_t Rn, int bpp);
#else
#define SENTINELS 2
int setcmploop(const uint32_t *Pv, size_t Pn, const uint32_t *Rv, size_t Rn);
#define setcmploop(Pv, Pn, Rv, Rn, bpp) setcmploop(Pv, Pn, Rv, Rn)
#endif

#define install_sentinels(v, n) memset(v + n, 0xff, SENTINELS * 4)
#pragma GCC visibility pop
