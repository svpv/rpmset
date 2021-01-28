#include <stdint.h>
#include <string.h>
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(x, 0)

#ifdef __x86_64__
#include <emmintrin.h>
#define SENTINELS 9
static inline void install_sentinels(uint32_t *v, size_t n, int bpp)
{
    __m128i x = _mm_set1_epi32(-1);
    if (likely(bpp != 32))
	x = _mm_srli_epi32(x, 1); // UINT32_MAX -> INT32_MAX, for pcmpgtd
    v += n;
    _mm_storeu_si128((void *)(v + 0), x);
    _mm_storeu_si128((void *)(v + 4), x);
    v[8] = _mm_cvtsi128_si32(x);
}
#else
#define SENTINELS 2
#define install_sentinels(v, n, bpp) memset((v) + (n), 0xff, SENTINELS * 4)
#endif

#pragma GCC visibility push(hidden)
size_t cache_decode(const char *s, size_t len, const uint32_t **pv);
size_t downsample(const uint32_t *v, size_t n, uint32_t *w, int bpp);
int setcmploop(const uint32_t *Pv, size_t Pn, const uint32_t *Rv, size_t Rn);
#pragma GCC visibility pop
