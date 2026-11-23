#include <optional>

#include "cache.hpp"
#include "check.hpp"

// A clock that says whatever time the test sets.
struct FakeClock : cache::Clock {
    long long time = 0;
    long long now() const override { return time; }
};

TEST(put_then_get) {
    FakeClock clock;
    cache::Cache cache(10, clock);
    cache.put("a", "1");
    CHECK_EQ(cache.get("a"), "1");
    CHECK_EQ(cache.get("b"), std::nullopt);
    CHECK_EQ(cache.size(), 1u);
}

// An entry put at time t is there until t + ttl, and gone from then on.
TEST(expires_after_ttl) {
    FakeClock clock;
    clock.time = 100;
    cache::Cache cache(10, clock);
    cache.put("a", "1");
    clock.time = 109;
    CHECK_EQ(cache.get("a"), "1");
    clock.time = 110;
    CHECK_EQ(cache.get("a"), std::nullopt);
    CHECK_EQ(cache.size(), 0u);
}

// Getting an entry doesn't make it last longer.
TEST(get_does_not_refresh) {
    FakeClock clock;
    cache::Cache cache(10, clock);
    cache.put("a", "1");
    clock.time = 5;
    CHECK_EQ(cache.get("a"), "1");
    clock.time = 10;
    CHECK_EQ(cache.get("a"), std::nullopt);
}

// Putting a key again replaces its value, and its ttl starts again.
TEST(put_replaces) {
    FakeClock clock;
    cache::Cache cache(10, clock);
    cache.put("a", "1");
    clock.time = 5;
    cache.put("a", "2");
    CHECK_EQ(cache.size(), 1u);
    clock.time = 14;
    CHECK_EQ(cache.get("a"), "2");
    clock.time = 15;
    CHECK_EQ(cache.get("a"), std::nullopt);
}

// Each key has its own time: one expiring, or a put of another, leaves the rest.
TEST(keys_are_separate) {
    FakeClock clock;
    cache::Cache cache(10, clock);
    cache.put("a", "1");
    clock.time = 5;
    cache.put("b", "2");
    CHECK_EQ(cache.get("a"), "1");
    CHECK_EQ(cache.get("b"), "2");
    CHECK_EQ(cache.size(), 2u);
    clock.time = 10;
    CHECK_EQ(cache.get("a"), std::nullopt);
    CHECK_EQ(cache.get("b"), "2");
    CHECK_EQ(cache.size(), 1u);
}

int main() { return check::run_all_tests(); }
