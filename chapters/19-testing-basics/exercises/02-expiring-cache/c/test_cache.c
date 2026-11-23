// Write tests for cache.c here, as test functions that main runs. One is here to
// start with, on the computer's clock, which can't show what happens as time passes
// without the test waiting for it. The README lists what the cache promises.
#include "cache.h"
#include "check.h"

static void test_put_then_get(void) {
    struct cache *cache = cache_new(60, cache_system_clock);
    CHECK(cache_put(cache, "a", "1"));
    CHECK_STR_EQ(cache_get(cache, "a"), "1");
    cache_free(cache);
}

int main(void) {
    RUN_TEST(test_put_then_get);
    return check_exit_code();
}
