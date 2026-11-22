// Tests for leap.c. Each test is a function, and main runs them: C has no way to
// find them on its own.
#include "check.h"
#include "leap.h"

static void test_divisible_by_4(void) {
    CHECK(is_leap_year(2024));
    CHECK(!is_leap_year(2023));
}

static void test_centuries(void) {
    CHECK(!is_leap_year(1900));
    CHECK(is_leap_year(2000));
}

int main(void) {
    RUN_TEST(test_divisible_by_4);
    RUN_TEST(test_centuries);
    return check_exit_code();
}
