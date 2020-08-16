#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "setstring.h"
#include "set.c"

uint32_t *decode(const char *s, size_t len, size_t *pn, int *pbpp)
{
    int m;
    int rc = decode_set_init(s, pbpp, &m);
    assert(rc == 0);
    int n = decode_set_size(len, m);
    assert(n > 0);
    uint32_t *v = malloc(n * 4);
    assert(v);
    n = decode_set(s, m, v);
    assert(n > 0);
    return *pn = n, v;
}

void recheck(const char *ss, size_t len, const uint32_t *v, size_t n)
{
    int bpp;
    size_t n1 = setstring_decinit(ss, len, &bpp);
    assert(n1 >= n);
    uint32_t *w = malloc(n1 * 4);
    n1 = setstring_decode(ss, len, bpp, w);
    assert(n1 == n);
    assert(memcmp(v, w, n * 4) == 0);
    free(w);
}

uint32_t mix(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

void genlo(uint32_t *v, size_t n, int incbpp)
{
    uint32_t mask = (1 << incbpp) - 1;
    for (size_t i = 0; i < n; i++) {
	uint32_t lo = mix(v[i]) & mask;
	v[i] = v[i] << incbpp | lo;
    }
}

void conv(const char *s, size_t len, char nl, int incbpp)
{
    assert(strncmp(s, "set:", 4) == 0);
    s += 4;
    size_t n;
    int bpp, m;
    uint32_t *v = decode(s, len, &n, &bpp);
    if (incbpp) {
	bpp += incbpp;
	if (bpp > 32) {
	    incbpp = bpp - 32;
	    bpp = 32;
	}
	if (incbpp)
	    genlo(v, n, incbpp);
    }
    size_t len1 = setstring_encinit(v, n, bpp, &m);
    assert(len1 > 0);
    char *ss = malloc(len1 + 4);
    memcpy(ss, "set:", 4);
    size_t len2 = setstring_encode(v, n, bpp, m, ss + 4);
    assert(len2 > 0);
    assert(len2 < len1); // len1 accounts for '\0', len2 does not
    assert(len2 <= len || incbpp);
    assert(strlen(ss + 4) == len2);
    recheck(ss + 4, len2, v, n);
    ss[4+len2] = nl;
#define print(s, size) fwrite_unlocked(s, 1, size, stdout)
    print(ss, 4 + len2 + 1);
    free(v);
    free(ss);
}

int main(int argc, const char **argv)
{
    int incbpp = 0;
    if (argc > 1 && argv[1][0] == '+') {
	char c = argv[1][1];
	assert(c >= '1' && c <= '9');
	assert(argv[1][2] == '\0');
	incbpp = c - '0';
	argc--, argv++;
    }
    if (argc == 2) {
	conv(argv[1], strlen(argv[1]), '\n', incbpp);
	return 0;
    }
    assert(argc == 1);
    char *line = NULL;
    size_t alloc = 0, len;
    while ((len = getline(&line, &alloc, stdin)) != (size_t) -1) {
	assert(len > 1);
	len--;
	assert(line[len] == '\n');
	char sep = line[len] = ' ';
	char *s2 = rawmemchr(line, sep);
	if (s2 == line + len) {
	    sep = line[len] = '\t';
	    s2 = rawmemchr(line, sep);
	}
	*s2 = line[len] = '\0';
	if (s2 == line + len)
	    conv(line, len, '\n', incbpp);
	else {
	    conv(line, s2 - line, sep, incbpp);
	    s2++;
	    conv(s2, line + len - s2, '\n', incbpp);
	}
    }
    free(line);
    return 0;
}
