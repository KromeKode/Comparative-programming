#pragma once

#include <string>

namespace roman {

// n in Roman numerals. Throws std::out_of_range if n is outside 1 to 3999.
std::string to_roman(int n);

}  // namespace roman
