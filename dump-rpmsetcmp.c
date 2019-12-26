#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>

int rpmsetcmp(const char *s1, const char *s2)
{
    static int (*next)(const char *s1, const char *s2);
    if (next == NULL) {
	next = dlsym(RTLD_NEXT, __func__);
	assert(next);
    }
    int ret = next(s1, s2);
    if (strncmp(s1, "set:", 4) == 0) s1 += 4;
    if (strncmp(s2, "set:", 4) == 0) s2 += 4;
    printf("set:%s set:%s\n", s1, s2);
    return ret;
}
