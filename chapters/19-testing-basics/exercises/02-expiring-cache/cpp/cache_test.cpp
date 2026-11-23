// Write tests for cache.cpp here, with TEST. One is here to start with, on the
// computer's clock, which can't show what happens as time passes without the test
// waiting for it. The README lists what the cache promises.
#include "cache.hpp"
#include "check.hpp"

TEST(put_then_get) {
    cache::SystemClock clock;
    cache::Cache cache(60, clock);
    cache.put("a", "1");
    CHECK_EQ(cache.get("a"), "1");
}

int main() { return check::run_all_tests(); }
