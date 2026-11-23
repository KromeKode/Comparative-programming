// Write tests for wrap.c here, as test functions that main runs. One is here to
// start with. The README lists what wrap promises, and check.h has CHECK,
// CHECK_INT_EQ, and CHECK_STR_EQ.
#include <stdlib.h>

#include "check.h"
#include "wrap.h"

static void test_one_line(void) {
    char *got = wrap("one two", 10);
    CHECK_STR_EQ(got, "one two\n");
    free(got);
}

int main(void) {
    RUN_TEST(test_one_line);
    return check_exit_code();
}
