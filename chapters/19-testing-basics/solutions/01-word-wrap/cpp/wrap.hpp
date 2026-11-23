#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace wrap {

// Wraps text into lines of at most width characters, breaking only at spaces.
std::vector<std::string> wrap(std::string_view text, std::size_t width);

}  // namespace wrap
