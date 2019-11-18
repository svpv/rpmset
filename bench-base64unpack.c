#undef NDEBUG
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "base64pack.h"

#if defined(__i386__) || defined(__x86_64__)
#include <x86intrin.h>
#define rdtsc() __rdtsc()
#elif defined(__aarch64__)
static inline uint64_t rdtsc(void)
{
    uint64_t t;
    asm volatile("mrs %0, cntvct_el0" : "=r"(t));
    return t;
}
#else
#error "rdtsc not supported"
#endif

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
    uint64_t t = rdtsc();
    asm volatile("" ::: "memory");
    const char *s_end = s + LOOP*c;
    do {
	bool ok = unpack(s, v, e);
	assert(ok);
	v += n, s += c, e++;
    } while (s < s_end);
    asm volatile("" ::: "memory");
    return rdtsc() - t;
}

#define WRAP_LOOP(unpack, N, C) \
uint64_t LOOP_##unpack(const char *restrict s, uint32_t *restrict v, unsigned *restrict e) \
{ return wrap_LOOP(unpack, s, v, e, N, C); }

WRAP_LOOP(unpack21x4c14,     4,  14)
WRAP_LOOP(unpack22x4c15e2,   4,  15)
WRAP_LOOP(unpack23x4c16e4,   4,  16)
WRAP_LOOP(unpack24x4c16,     4,  16)
WRAP_LOOP(unpack25x3c13e3o1, 3,  13)
WRAP_LOOP(unpack25x4c17e2,   4,  17)
WRAP_LOOP(unpack26x3c13o1,   3,  13)
WRAP_LOOP(unpack26x4c18e4,   4,  18)
WRAP_LOOP(unpack27x3c14e3,   3,  14)
WRAP_LOOP(unpack27x4c18,     4,  18)
WRAP_LOOP(unpack28x3c14,     3,  14)
WRAP_LOOP(unpack29x3c15e3o1, 3,  15)
WRAP_LOOP(unpack30x3c15o1,   3,  15)

void bench_LOOP(const char *name,
	void (*pack)(const uint32_t *v, char *s, unsigned e),
	uint64_t (*unpack_loop16)(const char *s, uint32_t *v, unsigned *e),
	unsigned m, unsigned n, unsigned c, unsigned em, unsigned o)
{
    uint32_t v[LOOP*n+1], w[LOOP*n+1];
    for (unsigned i = 0; i < LOOP*n; i++)
	v[i] = rand32(), w[i] = rand32();
    unsigned ev[LOOP], ew[LOOP];
    for (unsigned i = 0; i < LOOP; i++)
	ev[i] = rand32(), ew[i] = rand32();
    char s_[2+LOOP*c+2], *s = s_ + 2;
    s_[0] = 'b', s_[1] = 'M';
    for (unsigned i = 0; i < LOOP; i++)
	pack(v + n * i, s + c * i, ev[i]);
    s[LOOP*c] = 'x', s[LOOP*c+o] = '\0';
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
    bench_LOOP("unpack21x4",  pack21x4c14,   LOOP_unpack21x4c14,     21,  4, 14, 0, 0);
    bench_LOOP("unpack22x4",  pack22x4c15e2, LOOP_unpack22x4c15e2,   22,  4, 15, 2, 0);
    bench_LOOP("unpack23x4",  pack23x4c16e4, LOOP_unpack23x4c16e4,   23,  4, 16, 4, 0);
    bench_LOOP("unpack24x4",  pack24x4c16,   LOOP_unpack24x4c16,     24,  4, 16, 0, 0);
    bench_LOOP("unpack25x3",  pack25x3c13e3, LOOP_unpack25x3c13e3o1, 25,  3, 13, 3, 1);
    bench_LOOP("unpack25x4",  pack25x4c17e2, LOOP_unpack25x4c17e2,   25,  4, 17, 2, 0);
    bench_LOOP("unpack26x3",  pack26x3c13,   LOOP_unpack26x3c13o1,   26,  3, 13, 0, 1);
    bench_LOOP("unpack26x4",  pack26x4c18e4, LOOP_unpack26x4c18e4,   26,  4, 18, 4, 0);
    bench_LOOP("unpack27x3",  pack27x3c14e3, LOOP_unpack27x3c14e3,   27,  3, 14, 3, 0);
    bench_LOOP("unpack27x4",  pack27x4c18,   LOOP_unpack27x4c18,     27,  4, 18, 0, 0);
    bench_LOOP("unpack28x3",  pack28x3c14,   LOOP_unpack28x3c14,     28,  3, 14, 0, 0);
    bench_LOOP("unpack29x3",  pack29x3c15e3, LOOP_unpack29x3c15e3o1, 29,  3, 15, 3, 1);
    bench_LOOP("unpack30x3",  pack30x3c15,   LOOP_unpack30x3c15o1,   30,  3, 15, 0, 1);
    return 0;
}
