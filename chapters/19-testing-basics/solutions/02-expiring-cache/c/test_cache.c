#include "cache.h"
#include "check.h"

// The fake clock reads the time from a variable that the test sets: context points
// at it.
static long long read_time(void *context) { return *(long long *)context; }

static void test_put_then_get(void) {
    long long time = 0;
    struct cache *cache = cache_new(10, (struct cache_clock){read_time, &time});
    CHECK(cache_put(cache, "a", "1"));
    CHECK_STR_EQ(cache_get(cache, "a"), "1");
    CHECK_STR_EQ(cache_get(cache, "b"), NULL);
    CHECK_INT_EQ(cache_size(cache), 1);
    cache_free(cache);
}

// An entry put at time t is there until t + ttl, and gone from then on.
static void test_expires_after_ttl(void) {
    long long time = 100;
    struct cache *cache = cache_new(10, (struct cache_clock){read_time, &time});
    CHECK(cache_put(cache, "a", "1"));
    time = 109;
    CHECK_STR_EQ(cache_get(cache, "a"), "1");
    time = 110;
    CHECK_STR_EQ(cache_get(cache, "a"), NULL);
    CHECK_INT_EQ(cache_size(cache), 0);
    cache_free(cache);
}

// Getting an entry doesn't make it last longer.
static void test_get_does_not_refresh(void) {
    long long time = 0;
    struct cache *cache = cache_new(10, (struct cache_clock){read_time, &time});
    CHECK(cache_put(cache, "a", "1"));
    time = 5;
    CHECK_STR_EQ(cache_get(cache, "a"), "1");
    time = 10;
    CHECK_STR_EQ(cache_get(cache, "a"), NULL);
    cache_free(cache);
}

// Putting a key again replaces its value, and its ttl starts again.
static void test_put_replaces(void) {
    long long time = 0;
    struct cache *cache = cache_new(10, (struct cache_clock){read_time, &time});
    CHECK(cache_put(cache, "a", "1"));
    time = 5;
    CHECK(cache_put(cache, "a", "2"));
    CHECK_INT_EQ(cache_size(cache), 1);
    time = 14;
    CHECK_STR_EQ(cache_get(cache, "a"), "2");
    time = 15;
    CHECK_STR_EQ(cache_get(cache, "a"), NULL);
    cache_free(cache);
}

// Each key has its own time: one expiring, or a put of another, leaves the rest.
static void test_keys_are_separate(void) {
    long long time = 0;
    struct cache *cache = cache_new(10, (struct cache_clock){read_time, &time});
    CHECK(cache_put(cache, "a", "1"));
    time = 5;
    CHECK(cache_put(cache, "b", "2"));
    CHECK_STR_EQ(cache_get(cache, "a"), "1");
    CHECK_STR_EQ(cache_get(cache, "b"), "2");
    CHECK_INT_EQ(cache_size(cache), 2);
    time = 10;
    CHECK_STR_EQ(cache_get(cache, "a"), NULL);
    CHECK_STR_EQ(cache_get(cache, "b"), "2");
    CHECK_INT_EQ(cache_size(cache), 1);
    cache_free(cache);
}

int main(void) {
    RUN_TEST(test_put_then_get);
    RUN_TEST(test_expires_after_ttl);
    RUN_TEST(test_get_does_not_refresh);
    RUN_TEST(test_put_replaces);
    RUN_TEST(test_keys_are_separate);
    return check_exit_code();
}
