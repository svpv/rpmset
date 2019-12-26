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

int main()
{
    getpairs();
    return 0;
}
