// Tests for leap.cpp. TEST registers each one, so main doesn't have to list them.
#include "check.hpp"
#include "leap.hpp"

TEST(divisible_by_4) {
    CHECK(leap::is_leap_year(2024));
    CHECK(!leap::is_leap_year(2023));
}

TEST(centuries) {
    CHECK(!leap::is_leap_year(1900));
    CHECK(leap::is_leap_year(2000));
}

int main() { return check::run_all_tests(); }
