#undef NDEBUG
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

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
uint64_t LOOP_##unpack(const char *s, uint32_t *v, unsigned *e) \
{ return wrap_LOOP(unpack, s, v, e, N, C); }

WRAP_LOOP(unpack9x32c48,   32, 48)
WRAP_LOOP(unpack10x24c40,  24, 40)
WRAP_LOOP(unpack27x3c14e3, 3,  14)
WRAP_LOOP(unpack27x4c18,   4,  18)
WRAP_LOOP(unpack28x3c14,   3,  14)
WRAP_LOOP(unpack29x3c15e3, 3,  15)
WRAP_LOOP(unpack30x3c15,   3,  15)

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
    char s_[2+LOOP*c+1], *s = s_ + 2;
    s_[0] = 'b', s_[1] = 'M';
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
    bench_LOOP("unpack9x32",  pack9x32c48,   LOOP_unpack9x32c48,    9, 32, 48, 0);
    bench_LOOP("unpack10x24", pack10x24c40,  LOOP_unpack10x24c40,  10, 24, 40, 0);
    bench_LOOP("unpack27x3",  pack27x3c14e3, LOOP_unpack27x3c14e3, 27,  3, 14, 3);
    bench_LOOP("unpack27x4",  pack27x4c18,   LOOP_unpack27x4c18,   27,  4, 18, 0);
    bench_LOOP("unpack28x3",  pack28x3c14,   LOOP_unpack28x3c14,   28,  3, 14, 0);
    bench_LOOP("unpack29x3",  pack29x3c15e3, LOOP_unpack29x3c15e3, 29,  3, 15, 3);
    bench_LOOP("unpack30x3",  pack30x3c15,   LOOP_unpack30x3c15,   30,  3, 15, 0);
    return 0;
}
