#include <stdlib.h>

#include "check.h"
#include "wrap.h"

static const struct {
    const char *text;
    size_t width;
    const char *want;
} cases[] = {
    // Lines as long as they can be, and a last line that is shorter.
    {"the quick brown fox jumps", 10, "the quick\nbrown fox\njumps\n"},
    // A line can be exactly the width, and no longer: "ab cd" is 5.
    {"the quick brown", 9, "the quick\nbrown\n"},
    {"ab cd", 4, "ab\ncd\n"},
    {"ab cd", 5, "ab cd\n"},
    // One space between words, whatever the text had.
    {"  the   quick  ", 20, "the quick\n"},
    // No words, no lines.
    {"", 10, ""},
    {"   ", 10, ""},
    // A word longer than the width gets a line of its own, wherever it is.
    {"extraordinary", 5, "extraordinary\n"},
    {"an extraordinary day", 5, "an\nextraordinary\nday\n"},
    {"a b", 1, "a\nb\n"},
};

static void test_wrap(void) {
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        char *got = wrap(cases[i].text, cases[i].width);
        CHECK_STR_EQ(got, cases[i].want);
        free(got);
    }
}

int main(void) {
    RUN_TEST(test_wrap);
    return check_exit_code();
}
