#include <cstddef>
#include <string>
#include <vector>

#include "check.hpp"
#include "wrap.hpp"

using Lines = std::vector<std::string>;

// Lines as long as they can be, and a last line that is shorter.
TEST(fills_lines) {
    CHECK_EQ(wrap::wrap("the quick brown fox jumps", 10),
             Lines({"the quick", "brown fox", "jumps"}));
}

// A line can be exactly the width, and no longer: "ab cd" is 5.
TEST(exact_width) {
    CHECK_EQ(wrap::wrap("the quick brown", 9), Lines({"the quick", "brown"}));
    CHECK_EQ(wrap::wrap("ab cd", 4), Lines({"ab", "cd"}));
    CHECK_EQ(wrap::wrap("ab cd", 5), Lines({"ab cd"}));
}

// One space between words, whatever the text had.
TEST(spaces) { CHECK_EQ(wrap::wrap("  the   quick  ", 20), Lines({"the quick"})); }

// No words, no lines.
TEST(no_words) {
    CHECK_EQ(wrap::wrap("", 10), Lines{});
    CHECK_EQ(wrap::wrap("   ", 10), Lines{});
}

// A word longer than the width gets a line of its own, wherever it is.
TEST(long_words) {
    CHECK_EQ(wrap::wrap("extraordinary", 5), Lines({"extraordinary"}));
    CHECK_EQ(wrap::wrap("an extraordinary day", 5), Lines({"an", "extraordinary", "day"}));
    CHECK_EQ(wrap::wrap("a b", 1), Lines({"a", "b"}));
}

int main() { return check::run_all_tests(); }
