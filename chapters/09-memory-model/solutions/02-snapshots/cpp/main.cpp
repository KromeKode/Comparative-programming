#include <algorithm>
#include <charconv>
#include <cstddef>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

// A vector of strings has value semantics: assigning one copies every row.
using Grid = std::vector<std::string>;

// Reads a number made of digits. One too big for size_t becomes the largest
// size_t, which is outside any grid.
std::optional<std::size_t> parse_number(const std::string &text) {
    if (!std::all_of(text.begin(), text.end(), [](char c) { return c >= '0' && c <= '9'; })) {
        std::cout << "error: \"" << text << "\" is not a number\n";
        return std::nullopt;
    }
    std::size_t value = 0;
    auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc()) {
        value = std::numeric_limits<std::size_t>::max();
    }
    return value;
}

bool one_character(const std::string &text) {
    if (text.size() != 1) {
        std::cout << "error: \"" << text << "\" is not one character\n";
        return false;
    }
    return true;
}

bool takes(const std::string &command, std::size_t want, std::size_t got) {
    if (got != want) {
        std::cout << "error: " << command << " takes " << want << " value"
                  << (want == 1 ? "" : "s") << ", got " << got << "\n";
        return false;
    }
    return true;
}

int main() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        return 0;
    }
    std::size_t width = 0, height = 0;
    std::istringstream(line) >> width >> height;
    Grid grid(height, std::string(width, '.'));
    std::optional<Grid> saved;

    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::vector<std::string> parts;
        for (std::string part; in >> part;) {
            parts.push_back(part);
        }
        if (parts.empty()) {
            continue;
        }
        const std::string &command = parts[0];
        std::size_t values = parts.size() - 1;
        if (command == "paint") {
            if (!takes(command, 3, values)) {
                continue;
            }
            auto x = parse_number(parts[1]);
            if (!x) {
                continue;
            }
            auto y = parse_number(parts[2]);
            if (!y) {
                continue;
            }
            if (*x < 1 || *x > width || *y < 1 || *y > height) {
                std::cout << "error: (" << parts[1] << ", " << parts[2]
                          << ") is outside the " << width << " by " << height << " grid\n";
                continue;
            }
            if (one_character(parts[3])) {
                grid[*y - 1][*x - 1] = parts[3][0];
            }
        } else if (command == "fill") {
            if (takes(command, 1, values) && one_character(parts[1])) {
                for (std::string &row : grid) {
                    row.assign(width, parts[1][0]);
                }
            }
        } else if (command == "save") {
            if (takes(command, 0, values)) {
                saved = grid; // a full copy
            }
        } else if (command == "restore") {
            if (!takes(command, 0, values)) {
                continue;
            }
            if (saved) {
                grid = *saved; // another full copy, so saved stays as it was
            } else {
                std::cout << "error: nothing saved\n";
            }
        } else if (command == "show") {
            if (takes(command, 0, values)) {
                for (const std::string &row : grid) {
                    std::cout << row << "\n";
                }
                std::cout << "\n";
            }
        } else {
            std::cout << "error: unknown command \"" << command << "\"\n";
        }
    }
}
