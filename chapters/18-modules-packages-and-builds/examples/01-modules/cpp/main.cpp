#include <iostream>
#include <iterator>
#include <string>

#include "textstats.hpp"

int main() {
    std::string text(std::istreambuf_iterator<char>(std::cin), {});
    std::cout << "textstats " << textstats::version << '\n';
    std::cout << "words: " << textstats::word_count(text) << '\n';
    std::cout << "longest: " << textstats::longest_word(text) << '\n';
}
