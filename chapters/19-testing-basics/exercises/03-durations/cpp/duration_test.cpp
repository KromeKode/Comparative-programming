// Write tests for duration.cpp here, with TEST. One is here to start with. The
// README lists what parse_duration and format_duration promise, and check.hpp has
// CHECK, CHECK_EQ, and CHECK_THROWS.
#include "check.hpp"
#include "duration.hpp"

TEST(parses) { CHECK_EQ(duration::parse_duration("1h30m"), 5400); }

int main() { return check::run_all_tests(); }
