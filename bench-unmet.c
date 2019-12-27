#include <stdio.h>
#include <stdint.h>
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

double bench()
{
    uint64_t tsum = 0;
    uint64_t nsum = 0;
    for (size_t i = 0; i < npair; i++) {
	struct pair *p = &pairs[i];
	if (p->len0 >= 128 && cached(p->s0, p->len0))
	    goto R;
	int bpp;
#ifdef OLD
	int m;
	int rc = decode_set_init(p->s0, &bpp, &m);
	assert(rc == 0);
	rc = decode_set_size(p->len0, m);
	assert(rc > 0);
	size_t n = rc;
#else
	size_t n = setstring_decinit(p->s0, p->len0, &bpp);
	assert(n > 0);
#endif
	uint32_t *v = malloc(n * 4);
	assert(v);
	uint64_t t0 = rdtsc();
#ifdef OLD
	rc = decode_set(p->s0, m, v);
	assert(rc > 0);
	n = rc;
#else
	n = setstring_decode(p->s0, p->len0, bpp, v);
	assert(n > 0);
#endif
	tsum += rdtsc() - t0;
	nsum += n;
	free(v);

    R:
#ifdef OLD
	rc = decode_set_init(p->s0, &bpp, &m);
	assert(rc == 0);
	rc = decode_set_size(p->len0, m);
	assert(rc > 0);
	n = rc;
#else
	n = setstring_decinit(PAIR_S1(p), p->len1, &bpp);
	assert(n > 0);
#endif
	v = malloc(n * 4);
	assert(v);
	t0 = rdtsc();
#ifdef OLD
	rc = decode_set(p->s0, m, v);
	assert(rc > 0);
	n = rc;
#else
	n = setstring_decode(PAIR_S1(p), p->len1, bpp, v);
	assert(n > 0);
#endif
	tsum += rdtsc() - t0;
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
