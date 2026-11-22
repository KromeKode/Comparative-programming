// The interface of the textstats module. #pragma once does what an include
// guard does: a second #include of this file in the same translation unit adds
// nothing. It isn't in the standard, but every major compiler supports it.
#pragma once

#include <cstddef>
#include <string>
#include <string_view>

// A namespace keeps the module's names apart from everyone else's, so callers
// write textstats::word_count, or bring the names in with a using-declaration.
namespace textstats {

// inline lets every file that includes the header share one definition.
inline constexpr std::string_view version = "1.0";

// The number of words in text. A word is a run of ASCII letters.
std::size_t word_count(std::string_view text);

// The first of the longest words in text.
std::string longest_word(std::string_view text);

}  // namespace textstats
