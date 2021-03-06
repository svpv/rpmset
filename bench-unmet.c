#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
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
    while ((len = getline(&line, &alloc, stdin)) != (size_t) -1) {
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
	*s1++ = '\0';
	assert(strncmp(s1, "set:", 4) == 0);
	s1 += 4;
	p->len1 = end - s1;
	line = NULL, alloc = 0;
    }
}

#if defined(__i386__) || defined(__x86_64__)
#include <x86intrin.h>
#define rdtsc() __rdtsc()
#else
static inline uint64_t rdtsc(void)
{
    uint64_t t;
#if defined(__aarch64__)
    asm volatile("mrs %0, cntvct_el0" : "=r"(t));
#elif defined(__powerpc64__)
    asm volatile("mfspr %0, 268" : "=r"(t));
#else
#error "rdtsc not supported"
#endif
    return t;
}
#endif

#ifdef OLD
#include "set.c"
#else
#include "setstring.h"
#endif

#include <stdbool.h>
#include <t1ha.h>

bool cached(const char *s, size_t len)
{
    uint64_t h = t1ha2_atonce(s, len, 0);
#undef CACHE_SIZE
#define CACHE_SIZE 255
#define MIDPOINT 224
#define MOVSTEP 32
    static struct {
	uint64_t hv[CACHE_SIZE+1];
	unsigned n;
    } C;
    uint64_t *hv = C.hv;
    uint64_t *hp = C.hv;
    hv[C.n] = h;
    while (*hp != h)
	hp++;
    size_t i = hp - hv;
    if (i < C.n) { // found
	if (i > MOVSTEP) { // move to front
	    hv += i - MOVSTEP;
	    memmove(hv + 1, hv, MOVSTEP * 8);
	    hv[0] = h;
	}
	return true;
    }
    // not found
    if (C.n < MIDPOINT)
	i = C.n++;
    else {
	if (C.n < CACHE_SIZE)
	    C.n++;
	i = MIDPOINT;
	memmove(hv + i + 1, hv + i, (CACHE_SIZE - i - 1) * 8);
    }
    hv[i] = h;
    return false;
}

bool opt_R;

void dec1(const char *s, size_t len, uint64_t *nsum, uint64_t *tsum)
{
    int bpp;
#ifdef OLD
    int m;
    int rc = decode_set_init(s, &bpp, &m);
    assert(rc == 0);
    rc = decode_set_size(len, m);
    assert(rc > 0);
    size_t n = rc;
#else
    size_t n = setstring_decinit(s, len, &bpp);
    assert(n > 0);
#endif
    uint32_t *v = malloc(n * 4);
    assert(v);
    uint64_t t0 = rdtsc();
#ifdef OLD
    rc = decode_set(s, m, v);
    assert(rc > 0);
    n = rc;
#else
    n = setstring_decode(s, len, bpp, v);
    assert(n > 0);
#endif
    *tsum += rdtsc() - t0;
    *nsum += n;
    free(v);
}

double bench(void)
{
    uint64_t tsum = 0;
    uint64_t nsum = 0;
    for (size_t i = 0; i < npair; i++) {
	struct pair *p = &pairs[i];
	if (!opt_R) {
	    if (p->len0 < 128) {
		assert(strlen(p->s0) == p->len0); // prefetch
		dec1(p->s0, p->len0, &nsum, &tsum);
	    }
	    else if (!cached(p->s0, p->len0))
		dec1(p->s0, p->len0, &nsum, &tsum);
	}
	assert(strlen(PAIR_S1(p)) == p->len1);
	dec1(PAIR_S1(p), p->len1, &nsum, &tsum);
    }
    return tsum / (double) nsum;
}

#include <unistd.h>

int main(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "R")) != -1)
	switch (opt) {
	case 'R':
	    opt_R = 1;
	    break;
	default:
	    assert(!"getopt");
	}
    getpairs();
    double cpi = bench();
    fprintf(stderr, "%.2f cycles per integer\n", cpi);
    return 0;
}
