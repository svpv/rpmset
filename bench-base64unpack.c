#undef NDEBUG
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <x86intrin.h>
#include "base64pack-sse4.h"

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
	bool (*unpack)(const char *s, uint32_t *v, unsigned *e),
	const char *s, uint32_t *v, unsigned *e,
	unsigned n, unsigned c)
{
    uint64_t t = __rdtsc();
    asm volatile("" ::: "memory");
    const char *s_end = s + 16*c;
    do {
	bool ok = unpack(s, v, e);
	assert(ok);
	v += n, s += c, e++;
    } while (s < s_end);
    asm volatile("" ::: "memory");
    return __rdtsc() - t;
}

uint64_t unpack9x32x16(const char *s, uint32_t *v, unsigned *e)
{
    return wrap_loop16(unpack9x32c48e0, s, v, e, 32, 48);
}

uint64_t unpack10x24x16(const char *s, uint32_t *v, unsigned *e)
{
    return wrap_loop16(unpack10x24c40e0, s, v, e, 24, 40);
}

uint64_t unpack29x3x16(const char *s, uint32_t *v, unsigned *e)
{
    return wrap_loop16(unpack29x3c15e3, s, v, e, 3, 15);
}

uint64_t unpack30x3x16(const char *s, uint32_t *v, unsigned *e)
{
    return wrap_loop16(unpack30x3c15e0, s, v, e, 3, 15);
}

void bench_loop16(const char *name,
	void (*pack)(const uint32_t *v, char *s, unsigned e),
	uint64_t (*unpack_loop16)(const char *s, uint32_t *v, unsigned *e),
	unsigned m, unsigned n, unsigned c, unsigned em)
{
    uint32_t v[16*n+1], w[16*n+1];
    for (unsigned i = 0; i < 16*n; i++)
	v[i] = rand32();
    unsigned ev[16], ew[16];
    for (unsigned i = 0; i < 16; i++)
	ev[i] = rand32();
    char s[16*c+1];
    for (unsigned i = 0; i < 16; i++)
	pack(v + n * i, s + c * i, ev[i]);
    uint64_t t = 0;
    for (unsigned i = 0; i < 64; i++)
	t += unpack_loop16(s, w, ew);
    for (unsigned i = 0; i < 16*n; i++)
	v[i] &= (1 << m) - 1;
    assert(memcmp(v, w, 16*n * 4) == 0);
    if (em) {
	for (unsigned i = 0; i < 16; i++)
	    ev[i] &= (1 << em) - 1;
	assert(memcmp(ev, ew, 16 * 4) == 0);
    }
    fprintf(stderr, "%s: %.2f cycles per integer\n",
	    name, (double) t / (16*n) / 64);
}

int main()
{
    bench_loop16("unpack9x32",  pack9x32c48e0,  unpack9x32x16,   9, 32, 48, 0);
    bench_loop16("unpack10x24", pack10x24c40e0, unpack10x24x16, 10, 24, 40, 0);
    bench_loop16("unpack29x3",  pack29x3c15e3,  unpack29x3x16,  29,  3, 15, 3);
    bench_loop16("unpack30x3",  pack30x3c15e0,  unpack30x3x16,  30,  3, 15, 0);
    return 0;
}
