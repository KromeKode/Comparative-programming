#include "wrap.hpp"

namespace wrap {

std::vector<std::string> wrap(std::string_view text, std::size_t width) {
    std::vector<std::string> lines;
    std::string line;
    std::size_t start = 0;
    while (true) {
        start = text.find_first_not_of(' ', start);
        if (start == std::string_view::npos) break;
        std::size_t end = text.find(' ', start);
        if (end == std::string_view::npos) end = text.size();
        std::string_view word = text.substr(start, end - start);
        start = end;
        if (line.empty()) {
            line = word;
        } else if (line.size() + 1 + word.size() <= width) {
            line += ' ';
            line += word;
        } else {
            lines.push_back(line);
            line = word;
        }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

}  // namespace wrap
