#include "textstats.hpp"

#include <cctype>
#include <vector>

namespace textstats {

// An unnamed namespace makes words private to this file, as static does in C.
namespace {

std::vector<std::string_view> words(std::string_view text) {
    std::vector<std::string_view> found;
    std::size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && !std::isalpha(static_cast<unsigned char>(text[i]))) {
            i++;
        }
        std::size_t start = i;
        while (i < text.size() && std::isalpha(static_cast<unsigned char>(text[i]))) {
            i++;
        }
        if (i > start) {
            found.push_back(text.substr(start, i - start));
        }
    }
    return found;
}

}  // namespace

std::size_t word_count(std::string_view text) { return words(text).size(); }

std::string longest_word(std::string_view text) {
    std::string_view best;
    for (std::string_view word : words(text)) {
        if (word.size() > best.size()) {
            best = word;
        }
    }
    return std::string(best);
}

}  // namespace textstats
