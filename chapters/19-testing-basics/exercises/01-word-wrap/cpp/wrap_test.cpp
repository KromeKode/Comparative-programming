// Write tests for wrap.cpp here, with TEST. One is here to start with. The README
// lists what wrap promises, and check.hpp has CHECK, CHECK_EQ, and CHECK_THROWS.
#include <string>
#include <vector>

#include "check.hpp"
#include "wrap.hpp"

using Lines = std::vector<std::string>;

TEST(one_line) { CHECK_EQ(wrap::wrap("one two", 10), Lines({"one two"})); }

int main() { return check::run_all_tests(); }
