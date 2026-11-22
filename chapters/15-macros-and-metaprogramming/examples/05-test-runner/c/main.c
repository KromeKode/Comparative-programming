#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A test returns NULL when it passes, or the text of the check that failed.
// A failed CHECK leaves the test at once.
#define CHECK(expr)                \
    do {                           \
        if (!(expr)) return #expr; \
    } while (0)

#define TEST(name) static const char *test_##name(void)

TEST(addition) {
    CHECK(2 + 2 == 4);
    CHECK(-3 + 3 == 0);
    return NULL;
}

TEST(strings) {
    char word[] = "abc";
    for (char *p = word; *p; p++) *p = (char)toupper((unsigned char)*p);
    CHECK(strcmp(word, "ABC") == 0);
    CHECK(strlen(word) == 3);
    return NULL;
}

TEST(rounding) {
    CHECK(0.5 + 0.25 == 0.75);
    CHECK(0.1 + 0.2 == 0.3);
    return NULL;
}

static int compare_ints(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

TEST(sorting) {
    int numbers[] = {3, 1, 2};
    qsort(numbers, 3, sizeof numbers[0], compare_ints);
    CHECK(numbers[0] == 1 && numbers[1] == 2 && numbers[2] == 3);
    return NULL;
}

// Standard C can't run code before main or find a function by its name, so
// the list of tests is written out. ENTRY turns each name into a string and
// the function's name.
#define ENTRY(name) {#name, test_##name}

static const struct {
    const char *name;
    const char *(*run)(void);
} tests[] = {
    ENTRY(addition),
    ENTRY(strings),
    ENTRY(rounding),
    ENTRY(sorting),
};

int main(void) {
    size_t count = sizeof tests / sizeof tests[0];
    int failed = 0;
    printf("running %zu tests\n", count);
    for (size_t i = 0; i < count; i++) {
        const char *failure = tests[i].run();
        if (failure) {
            printf("FAIL %s: %s\n", tests[i].name, failure);
            failed++;
        } else {
            printf("PASS %s\n", tests[i].name);
        }
    }
    printf("%d passed, %d failed\n", (int)count - failed, failed);
    return 0;  // a real runner would return 1 when a test fails
}
