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
    int bpp, m;
    size_t n1 = setstring_decodeInit(ss, len, &bpp, &m);
    assert(n1 >= n);
    uint32_t *w = malloc(n1 * 4);
    n1 = setstring_decode(ss, len, bpp, m, w);
    assert(n1 == n);
    assert(memcmp(v, w, n * 4) == 0);
    free(w);
}

void conv(const char *s, size_t len, char nl)
{
    assert(strncmp(s, "set:", 4) == 0);
    s += 4;
    size_t n;
    int bpp, m;
    uint32_t *v = decode(s, len, &n, &bpp);
    len = setstring_encodeInit(v, n, bpp, &m);
    assert(len > 0);
    char *ss = malloc(len + 4);
    memcpy(ss, "set:", 4);
    len = setstring_encode(v, n, bpp, m, ss + 4);
    assert(len > 0);
    recheck(ss + 4, len, v, n);
    ss[4+len] = nl;
#define print(s, len) fwrite_unlocked(s, 1, len, stdout)
    print(ss, 4 + len + 1);
    free(v);
    free(ss);
}

int main(int argc, const char **argv)
{
    if (argc == 2) {
	conv(argv[1], strlen(argv[1]), '\n');
	return 0;
    }
    assert(argc == 1);
    char *line = NULL;
    size_t alloc = 0, len;
    while ((len = getline(&line, &alloc, stdin)) != -1) {
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
	    conv(line, len, '\n');
	else {
	    conv(line, s2 - line, sep);
	    s2++;
	    conv(s2, line + len - s2, '\n');
	}
    }
    free(line);
    return 0;
}
