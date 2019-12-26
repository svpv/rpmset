#include <stdio.h>
#include <string.h>
#include <assert.h>

struct pair {
    char *s0;
    unsigned len0;
    unsigned len1;
};

#define PAIR_S1(p) ((p)->s0 + (p)->len0 + 5)

size_t npair;
struct pair pairs[1<<20];

void getpairs(void)
{
    size_t len;
    char *line = NULL;
    size_t alloc = 0;
    while ((len = getline(&line, &alloc, stdin)) != -1) {
	assert(len > 0);
	len--;
	assert(line[len] == '\n');
	line[len] = '\0';
	char *end = line + len;
	struct pair *p = &pairs[npair++];
	assert(strncmp(line, "set:", 4) == 0);
	p->s0 = line + 4;
	char *s1 = memchr(p->s0, ' ', end - p->s0);
	assert(s1);
	p->len0 = s1 - p->s0;
	s1++;
	assert(strncmp(s1, "set:", 4) == 0);
	s1 += 4;
	p->len1 = end - s1;
	line = NULL, alloc = 0;
    }
}

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

#include "setstring.h"

double bench()
{
    uint64_t tsum = 0;
    uint64_t nsum = 0;
    for (size_t i = 0; i < npair; i++) {
	struct pair *p = &pairs[i];
	int bpp;
	size_t n = setstring_decinit(p->s0, p->len0, &bpp);
	assert(n > 0);
	uint32_t *v = malloc(n * 4);
	assert(v);
	uint64_t t0 = rdtsc();
	n = setstring_decode(p->s0, p->len0, bpp, v);
	tsum += rdtsc() - t0;
	assert(n > 0);
	nsum += n;
	free(v);

	n = setstring_decinit(PAIR_S1(p), p->len1, &bpp);
	assert(n > 0);
	v = malloc(n * 4);
	assert(v);
	t0 = rdtsc();
	n = setstring_decode(PAIR_S1(p), p->len1, bpp, v);
	tsum += rdtsc() - t0;
	assert(n > 0);
	nsum += n;
	free(v);
    }
    return tsum / (double) nsum;
}

int main()
{
    getpairs();
    double cpi = bench();
    fprintf(stderr, "%.1f cycles per integer\n", cpi);
    return 0;
}
