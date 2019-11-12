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

#define LOOP 256

static inline uint64_t wrap_LOOP(
	bool (*unpack)(const char *s, uint32_t *v, unsigned *e),
	const char *s, uint32_t *v, unsigned *e,
	unsigned n, unsigned c)
{
    uint64_t t = __rdtsc();
    asm volatile("" ::: "memory");
    const char *s_end = s + LOOP*c;
    do {
	bool ok = unpack(s, v, e);
	assert(ok);
	v += n, s += c, e++;
    } while (s < s_end);
    asm volatile("" ::: "memory");
    return __rdtsc() - t;
}

#define WRAP_LOOP(unpack, N, C) \
uint64_t LOOP_##unpack(const char *s, uint32_t *v, unsigned *e) \
{ return wrap_LOOP(unpack, s, v, e, N, C); }

WRAP_LOOP(unpack9x32c48e0,  32, 48)
WRAP_LOOP(unpack10x24c40e0, 24, 40)
WRAP_LOOP(unpack28x6c28e0,  6,  28)
WRAP_LOOP(unpack29x3c15e3,  3,  15)
WRAP_LOOP(unpack30x3c15e0,  3,  15)

void bench_LOOP(const char *name,
	void (*pack)(const uint32_t *v, char *s, unsigned e),
	uint64_t (*unpack_loop16)(const char *s, uint32_t *v, unsigned *e),
	unsigned m, unsigned n, unsigned c, unsigned em)
{
    uint32_t v[LOOP*n+1], w[LOOP*n+1];
    for (unsigned i = 0; i < LOOP*n; i++)
	v[i] = rand32();
    unsigned ev[LOOP], ew[LOOP];
    for (unsigned i = 0; i < LOOP; i++)
	ev[i] = rand32();
    char s[LOOP*c+1];
    for (unsigned i = 0; i < LOOP; i++)
	pack(v + n * i, s + c * i, ev[i]);
    uint64_t t = 0;
    for (unsigned i = 0; i < (1<<16); i++)
	t += unpack_loop16(s, w, ew);
    for (unsigned i = 0; i < LOOP*n; i++)
	v[i] &= (1 << m) - 1;
    assert(memcmp(v, w, LOOP*n * 4) == 0);
    if (em) {
	for (unsigned i = 0; i < 16; i++)
	    ev[i] &= (1 << em) - 1;
	assert(memcmp(ev, ew, 16 * 4) == 0);
    }
    fprintf(stderr, "%s: %.2f cycles per integer\n",
	    name, (double) t / (LOOP*n) / (1<<16));
}

int main()
{
    bench_LOOP("unpack9x32",  pack9x32c48e0,  LOOP_unpack9x32c48e0,   9, 32, 48, 0);
    bench_LOOP("unpack10x24", pack10x24c40e0, LOOP_unpack10x24c40e0, 10, 24, 40, 0);
    bench_LOOP("unpack28x6",  pack28x6c28e0,  LOOP_unpack28x6c28e0,  28,  6, 28, 0);
    bench_LOOP("unpack29x3",  pack29x3c15e3,  LOOP_unpack29x3c15e3,  29,  3, 15, 3);
    bench_LOOP("unpack30x3",  pack30x3c15e0,  LOOP_unpack30x3c15e0,  30,  3, 15, 0);
    return 0;
}
