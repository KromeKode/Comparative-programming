// check.h: a test framework in a few lines, since C's standard library has none.
//
// CHECK(condition) reports the file, line, and text of a condition that is false,
// and carries on, so that one run shows every failure. CHECK_INT_EQ and
// CHECK_STR_EQ compare two values and print both when they differ. RUN_TEST(name)
// calls a test function and prints whether its checks passed, and
// check_exit_code() is what main returns: 0 if everything passed, 1 if not.
#ifndef CHECK_H
#define CHECK_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int check_failures = 0;

#define CHECK(condition)                                                         \
    do {                                                                         \
        if (!(condition)) {                                                      \
            printf("%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #condition); \
            check_failures++;                                                    \
        }                                                                        \
    } while (0)

#define CHECK_INT_EQ(actual, expected) \
    check_int_eq((actual), (expected), #actual, #expected, __FILE__, __LINE__)

#define CHECK_STR_EQ(actual, expected) \
    check_str_eq((actual), (expected), #actual, #expected, __FILE__, __LINE__)

#define RUN_TEST(name)                                                         \
    do {                                                                       \
        int failures_before = check_failures;                                  \
        name();                                                                \
        printf("%s %s\n", check_failures == failures_before ? "ok  " : "FAIL", \
               #name);                                                         \
    } while (0)

static inline void check_int_eq(long long actual, long long expected, const char *actual_text,
                                const char *expected_text, const char *file, int line) {
    if (actual == expected) return;
    printf("%s:%d: CHECK_INT_EQ(%s, %s)\n  actual:   %lld\n  expected: %lld\n", file, line,
           actual_text, expected_text, actual, expected);
    check_failures++;
}

// Prints a string for a failure message: NULL, or the string in quotes, with any
// newline, tab, quote, or backslash in it written as \n, \t, \", or \\.
static inline void check_print_str(const char *label, const char *s) {
    printf("  %s", label);
    if (s == NULL) {
        printf("NULL\n");
        return;
    }
    putchar('"');
    for (; *s != '\0'; s++) {
        switch (*s) {
            case '\n': printf("\\n"); break;
            case '\t': printf("\\t"); break;
            case '"': printf("\\\""); break;
            case '\\': printf("\\\\"); break;
            default: putchar(*s);
        }
    }
    printf("\"\n");
}

// NULL counts as a value, which equals only NULL.
static inline void check_str_eq(const char *actual, const char *expected, const char *actual_text,
                                const char *expected_text, const char *file, int line) {
    bool equal = actual == NULL || expected == NULL ? actual == expected
                                                    : strcmp(actual, expected) == 0;
    if (equal) return;
    printf("%s:%d: CHECK_STR_EQ(%s, %s)\n", file, line, actual_text, expected_text);
    check_print_str("actual:   ", actual);
    check_print_str("expected: ", expected);
    check_failures++;
}

static inline int check_exit_code(void) {
    if (check_failures > 0) {
        printf("%d checks failed\n", check_failures);
        return 1;
    }
    printf("all checks passed\n");
    return 0;
}

#endif
