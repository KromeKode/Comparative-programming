#include <stdio.h>

#include "check.h"
#include "duration.h"

static void test_parse(void) {
    static const struct {
        const char *text;
        long long seconds;
    } cases[] = {
        {"45s", 45},
        {"1h30m", 5400},
        {"2d", 172800},
        {"1d2h3m4s", 93784},
        // A number can be more than the next unit up holds, and it can be 0.
        {"90m", 5400},
        {"0s", 0},
        {"1h0m", 3600},
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        int failures_before = check_failures;
        long long got = -1;
        CHECK(parse_duration(cases[i].text, &got));
        CHECK_INT_EQ(got, cases[i].seconds);
        if (check_failures > failures_before) printf("  in the case for \"%s\"\n", cases[i].text);
    }
}

static void test_parse_rejects(void) {
    static const char *const cases[] = {
        "",        // no parts
        "90",      // a number with no unit
        "1h30",    // the same, after a part
        "h",       // a unit with no number
        "1x",      // not a unit
        "1h1h",    // a unit twice
        "30m1h",   // units out of order
        "1h 30m",  // anything else
        "-1h",
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        int failures_before = check_failures;
        long long got = -1;
        CHECK(!parse_duration(cases[i], &got));
        CHECK_INT_EQ(got, -1);
        if (check_failures > failures_before) printf("  in the case for \"%s\"\n", cases[i]);
    }
}

static void test_format(void) {
    static const struct {
        long long seconds;
        const char *text;
    } cases[] = {
        {5400, "1h30m"},
        {93784, "1d2h3m4s"},
        {59, "59s"},
        // Units that are 0 are left out, wherever they are, unless everything is.
        {3600, "1h"},
        {86401, "1d1s"},
        {0, "0s"},
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        char got[32] = "";
        CHECK(format_duration(cases[i].seconds, got, sizeof got));
        CHECK_STR_EQ(got, cases[i].text);
    }
}

static void test_round_trip(void) {
    for (long long seconds = 0; seconds < 100000; seconds++) {
        char text[32] = "";
        long long got = -1;
        if (!format_duration(seconds, text, sizeof text) || !parse_duration(text, &got) ||
            got != seconds) {
            CHECK_INT_EQ(got, seconds);
            printf("  for \"%s\"\n", text);
            return;
        }
    }
}

int main(void) {
    RUN_TEST(test_parse);
    RUN_TEST(test_parse_rejects);
    RUN_TEST(test_format);
    RUN_TEST(test_round_trip);
    return check_exit_code();
}
