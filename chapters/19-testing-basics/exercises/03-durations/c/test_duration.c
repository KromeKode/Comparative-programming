// Write tests for duration.c here, as test functions that main runs. One is here to
// start with. The README lists what parse_duration and format_duration promise, and
// check.h has CHECK, CHECK_INT_EQ, and CHECK_STR_EQ.
#include "check.h"
#include "duration.h"

static void test_parse(void) {
    long long got = -1;
    CHECK(parse_duration("1h30m", &got));
    CHECK_INT_EQ(got, 5400);
}

int main(void) {
    RUN_TEST(test_parse);
    return check_exit_code();
}
