#include <stdexcept>
#include <string>
#include <utility>

#include "check.hpp"
#include "roman.hpp"

// A table of cases, and one loop that checks them all. CHECK_EQ prints the
// expected value when a case fails, which says which case it was.
TEST(numerals) {
    const std::pair<int, std::string> cases[] = {
        {1, "I"},
        {4, "IV"},
        {9, "IX"},
        {14, "XIV"},
        {40, "XL"},
        {90, "XC"},
        {400, "CD"},
        {1994, "MCMXCIV"},
        {2024, "MMXXIV"},
        {3999, "MMMCMXCIX"},
    };
    for (const auto& [n, want] : cases) {
        CHECK_EQ(roman::to_roman(n), want);
    }
}

// The errors are part of what to_roman promises, so they get tests too.
TEST(out_of_range) {
    CHECK_THROWS(roman::to_roman(0), std::out_of_range);
    CHECK_THROWS(roman::to_roman(4000), std::out_of_range);
    CHECK_THROWS(roman::to_roman(-1), std::out_of_range);
}

int main() { return check::run_all_tests(); }
