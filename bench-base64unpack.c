#undef NDEBUG
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <x86intrin.h>
#include "pack.h"
#include "unpack.h"

// Lehmer random number generator
static inline uint32_t rand32(void)
{
#define R32K UINT64_C(6364136223846793005)
    static uint64_t state = R32K;
    uint32_t ret = state >> 32;
    state *= R32K;
    return ret;
}

static inline uint64_t wrap_loop16(
	unsigned (*unpack)(const char *s, uint32_t *v),
	const char *s, uint32_t *v,
	unsigned n, unsigned len)
{
    uint64_t t = __rdtsc();
    asm volatile("" ::: "memory");
    const char *s_end = s + 16*len;
    do {
	unsigned l = unpack(s, v);
	assert(l == len);
	v += n, s += l;
    } while (s < s_end);
    asm volatile("" ::: "memory");
    return __rdtsc() - t;
}

uint64_t unpack9x32x16(const char *s, uint32_t *v)
{
    return wrap_loop16(unpack9x32, s, v, 32, 48);
}

void bench_loop16(const char *name,
	unsigned (*pack)(const uint32_t *v, char *s),
	uint64_t (*unpack_loop16)(const char *s, uint32_t *v),
	unsigned m, unsigned n, unsigned len)
{
    uint32_t v[16*n], w[16*n];
    for (unsigned i = 0; i < 16*n; i++)
	v[i] = rand32();
    char s[16*len];
    for (unsigned i = 0; i < 16; i++) {
	unsigned l = pack(v + n * i, s + len * i);
	assert(l == len);
    }
    uint64_t t = 0;
    for (unsigned i = 0; i < 64; i++)
	t += unpack_loop16(s, w);
    for (unsigned i = 0; i < 16*n; i++)
	v[i] &= (1 << m) - 1;
    assert(memcmp(v, w, 16*n * 4) == 0);
    fprintf(stderr, "%s: %.2f cycles per integer\n",
	    name, (double) t / (16*n) / 64);
}

int main()
{
    bench_loop16("unpack9x32", pack9x32, unpack9x32x16, 9, 32, 48);
    return 0;
}
