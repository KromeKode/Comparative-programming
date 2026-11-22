#include "check.h"
#include "roman.h"

// A table of cases, and one loop that checks them all. A failure inside the loop
// says which line of code failed, but not which case, so the loop says that too.
static void test_numerals(void) {
    static const struct {
        int n;
        const char *want;
    } cases[] = {
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
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        int failures_before = check_failures;
        char got[16] = "";
        CHECK(to_roman(cases[i].n, got, sizeof got));
        CHECK_STR_EQ(got, cases[i].want);
        if (check_failures > failures_before) printf("  in the case for %d\n", cases[i].n);
    }
}

// The errors are part of what to_roman promises, so they get tests too.
static void test_out_of_range(void) {
    char got[16] = "unchanged";
    CHECK(!to_roman(0, got, sizeof got));
    CHECK(!to_roman(4000, got, sizeof got));
    CHECK(!to_roman(-1, got, sizeof got));
    CHECK_STR_EQ(got, "unchanged");
}

int main(void) {
    RUN_TEST(test_numerals);
    RUN_TEST(test_out_of_range);
    return check_exit_code();
}
