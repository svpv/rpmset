#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(x, 0)

#define CACHE_SIZE (256 - 2)

struct cache {
    // Hash values, for linear search with a sentinel.
    uint16_t hv[CACHE_SIZE+1];
    // The number of entries in ev[].
    uint16_t n;
    // Malloc'd cache entries.
    struct cache_ent *ev[CACHE_SIZE];
};

// The cache object is thread-local: it is created when a thread first calls
// cache_decode, and purged when the thread exits.

static void cache_free(void *arg)
{
    struct cache *C = arg;
    struct cache_ent **ep = C->ev;
    struct cache_ent **ev_end = C->ev + C->n;
    while (ep < ev_end)
	free(*ep++);
    free(C);
}

static pthread_key_t cache_key;

static __attribute__((constructor)) void cache_init(void)
{
    int rc = pthread_key_create(&cache_key, cache_free);
    assert(rc == 0);
}

static struct cache *cache_tlsobj(void)
{
    struct cache *C = pthread_getspecific(cache_key);
    if (likely(C))
	return C;
    C = aligned_alloc(64, sizeof *C); // to CPU cache line
    assert(C);
    memset(C, 0, sizeof *C);
    int rc = pthread_setspecific(cache_key, C);
    assert(rc == 0);
    return C;
}
