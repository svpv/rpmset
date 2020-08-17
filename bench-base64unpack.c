#undef NDEBUG
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "base64.h"
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
	if (!ok)
	    return 0;
	v += n, s += c, e++;
    } while (s < s_end);
    asm volatile("" ::: "memory");
    return rdtsc() - t;
}

#define WRAP_LOOP(unpack, N, C) \
uint64_t LOOP_##unpack(const char *restrict s, uint32_t *restrict v, unsigned *restrict e) \
{ return wrap_LOOP(unpack, s, v, e, N, C); }

WRAP_LOOP(unpack5x19c16e1,  19,  16)
WRAP_LOOP(unpack6x16c16,    16,  16)
WRAP_LOOP(unpack7x12c14,    12,  14)
WRAP_LOOP(unpack7x24c28,    24,  28)
WRAP_LOOP(unpack8x12c16,    12,  16)
WRAP_LOOP(unpack9x10c15,    10,  15)
WRAP_LOOP(unpack9x32c48,    32,  48)
WRAP_LOOP(unpack10x8c14e4,   8,  14)
WRAP_LOOP(unpack10x9c15,     9,  15)
WRAP_LOOP(unpack10x18c30,   18,  30)
WRAP_LOOP(unpack10x24c40,   24,  40)
WRAP_LOOP(unpack11x8c15e2,   8,  15)
WRAP_LOOP(unpack11x8c16e8,   8,  16)
WRAP_LOOP(unpack11x9c18e9,   9,  18)
WRAP_LOOP(unpack12x8c16,     8,  16)
WRAP_LOOP(unpack13x7c16e5,   7,  16)
WRAP_LOOP(unpack13x6c13o1,   6,  13)
WRAP_LOOP(unpack14x6c14,     6,  14)
WRAP_LOOP(unpack15x6c15,     6,  15)
WRAP_LOOP(unpack16x6c16,     6,  16)
WRAP_LOOP(unpack17x5c15e5,   5,  15)
WRAP_LOOP(unpack17x6c17,     6,  17)
WRAP_LOOP(unpack18x5c15,     5,  15)
WRAP_LOOP(unpack19x4c13e2o1, 4,  13)
WRAP_LOOP(unpack19x5c16e1,   5,  16)
WRAP_LOOP(unpack20x4c14e4,   4,  14)
WRAP_LOOP(unpack20x6c20,     6,  20)
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
    for (unsigned i = 0; i < (1<<16); i++) {
	uint64_t x = unpack_loop16(s, w, ew);
	assert(x);
	t += x;
    }
    for (unsigned i = 0; i < LOOP*n; i++)
	v[i] &= (1 << m) - 1;
    assert(memcmp(v, w, LOOP*n * 4) == 0);
    if (em) {
	for (unsigned i = 0; i < LOOP; i++)
	    ev[i] &= (1 << em) - 1;
	assert(memcmp(ev, ew, LOOP * 4) == 0);
    }
    fprintf(stderr, "%s: %.2f cycles per integer\n",
	    name, (double) t / (LOOP*n) / (1<<16));
    // Must fail on invalid inputs.
    for (unsigned i = 0; i < LOOP*c; i++) {
	char fita; // Gogol's indecent letter
	do
	    fita = rand32();
	while ((int32_t)base64dec1(&fita) >= 0);
	char save = s[i];
	s[i] = fita;
	uint64_t x = unpack_loop16(s, w, ew);
	assert(x == 0);
	s[i] = save;
    }
}

int main()
{
    bench_LOOP("unpack5x19",  pack5x19c16e1, LOOP_unpack5x19c16e1,    5, 19, 16, 1, 0);
    bench_LOOP("unpack6x16",  pack6x16c16,   LOOP_unpack6x16c16,      6, 16, 16, 0, 0);
    bench_LOOP("unpack7x12",  pack7x12c14,   LOOP_unpack7x12c14,      7, 12, 14, 0, 0);
    bench_LOOP("unpack7x24",  pack7x24c28,   LOOP_unpack7x24c28,      7, 24, 28, 0, 0);
    bench_LOOP("unpack8x12",  pack8x12c16,   LOOP_unpack8x12c16,      8, 12, 16, 0, 0);
    bench_LOOP("unpack9x10",  pack9x10c15,   LOOP_unpack9x10c15,      9, 10, 15, 0, 0);
    bench_LOOP("unpack9x32",  pack9x32c48,   LOOP_unpack9x32c48,      9, 32, 48, 0, 0);
    bench_LOOP("unpack10x8",  pack10x8c14e4, LOOP_unpack10x8c14e4,   10,  8, 14, 4, 0);
    bench_LOOP("unpack10x9",  pack10x9c15,   LOOP_unpack10x9c15,     10,  9, 15, 0, 0);
    bench_LOOP("unpack10x18", pack10x18c30,  LOOP_unpack10x18c30,    10, 18, 30, 0, 0);
    bench_LOOP("unpack10x24", pack10x24c40,  LOOP_unpack10x24c40,    10, 24, 40, 0, 0);
    bench_LOOP("unpack11x8",  pack11x8c15e2, LOOP_unpack11x8c15e2,   11,  8, 15, 2, 0);
    bench_LOOP("unpack11x8e", pack11x8c16e8, LOOP_unpack11x8c16e8,   11,  8, 16, 8, 0);
    bench_LOOP("unpack11x9",  pack11x9c18e9, LOOP_unpack11x9c18e9,   11,  9, 18, 9, 0);
    bench_LOOP("unpack12x8",  pack12x8c16,   LOOP_unpack12x8c16,     12,  8, 16, 0, 0);
    bench_LOOP("unpack13x6",  pack13x6c13,   LOOP_unpack13x6c13o1,   13,  6, 13, 0, 1);
    bench_LOOP("unpack13x7",  pack13x7c16e5, LOOP_unpack13x7c16e5,   13,  7, 16, 5, 0);
    bench_LOOP("unpack14x6",  pack14x6c14,   LOOP_unpack14x6c14,     14,  6, 14, 0, 0);
    bench_LOOP("unpack15x6",  pack15x6c15,   LOOP_unpack15x6c15,     15,  6, 15, 0, 0);
    bench_LOOP("unpack16x6",  pack16x6c16,   LOOP_unpack16x6c16,     16,  6, 16, 0, 0);
    bench_LOOP("unpack17x5",  pack17x5c15e5, LOOP_unpack17x5c15e5,   17,  5, 15, 5, 0);
    bench_LOOP("unpack17x6",  pack17x6c17,   LOOP_unpack17x6c17,     17,  6, 17, 0, 0);
    bench_LOOP("unpack18x5",  pack18x5c15,   LOOP_unpack18x5c15,     18,  5, 15, 0, 0);
    bench_LOOP("unpack19x4",  pack19x4c13e2, LOOP_unpack19x4c13e2o1, 19,  4, 13, 2, 1);
    bench_LOOP("unpack19x5",  pack19x5c16e1, LOOP_unpack19x5c16e1,   19,  5, 16, 1, 0);
    bench_LOOP("unpack20x4",  pack20x4c14e4, LOOP_unpack20x4c14e4,   20,  4, 14, 4, 0);
    bench_LOOP("unpack20x6",  pack20x6c20,   LOOP_unpack20x6c20,     20,  6, 20, 0, 0);
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
