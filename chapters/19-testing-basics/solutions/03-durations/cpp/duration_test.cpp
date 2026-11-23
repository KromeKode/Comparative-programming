#include <stdexcept>
#include <string>
#include <utility>

#include "check.hpp"
#include "duration.hpp"

using duration::format_duration;
using duration::parse_duration;

TEST(parses) {
    const std::pair<std::string, long long> cases[] = {
        {"45s", 45},
        {"1h30m", 5400},
        {"2d", 172800},
        {"1d2h3m4s", 93784},
        // A number can be more than the next unit up holds, and it can be 0.
        {"90m", 5400},
        {"0s", 0},
        {"1h0m", 3600},
    };
    for (const auto& [text, seconds] : cases) CHECK_EQ(parse_duration(text), seconds);
}

// CHECK_THROWS prints the statement that didn't throw, which says which case it was.
TEST(rejects) {
    CHECK_THROWS(parse_duration(""), std::invalid_argument);        // no parts
    CHECK_THROWS(parse_duration("90"), std::invalid_argument);      // a number with no unit
    CHECK_THROWS(parse_duration("1h30"), std::invalid_argument);    // the same, after a part
    CHECK_THROWS(parse_duration("h"), std::invalid_argument);       // a unit with no number
    CHECK_THROWS(parse_duration("1x"), std::invalid_argument);      // not a unit
    CHECK_THROWS(parse_duration("1h1h"), std::invalid_argument);    // a unit twice
    CHECK_THROWS(parse_duration("30m1h"), std::invalid_argument);   // units out of order
    CHECK_THROWS(parse_duration("1h 30m"), std::invalid_argument);  // anything else
    CHECK_THROWS(parse_duration("-1h"), std::invalid_argument);
}

TEST(formats) {
    const std::pair<long long, std::string> cases[] = {
        {5400, "1h30m"},
        {93784, "1d2h3m4s"},
        {59, "59s"},
        // Units that are 0 are left out, wherever they are, unless everything is.
        {3600, "1h"},
        {86401, "1d1s"},
        {0, "0s"},
    };
    for (const auto& [seconds, text] : cases) CHECK_EQ(format_duration(seconds), text);
}

// If parse_duration throws, run_all_tests catches it, and the test fails.
TEST(round_trips) {
    for (long long seconds = 0; seconds < 100000; seconds++) {
        long long got = parse_duration(format_duration(seconds));
        if (got != seconds) {
            CHECK_EQ(got, seconds);
            return;
        }
    }
}

int main() { return check::run_all_tests(); }
