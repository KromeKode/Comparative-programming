#include <array>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

constexpr std::array<std::pair<int, std::string_view>, 13> table{{
    {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"}, {90, "XC"}, {50, "L"},
    {40, "XL"}, {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"},
}};

std::string to_roman(int n) {
    std::string out;
    for (auto [value, symbol] : table) {
        while (n >= value) {  // take the biggest value that still fits
            out += symbol;
            n -= value;
        }
    }
    return out;
}

int letter_value(char letter) {
    switch (letter) {
        case 'I':
            return 1;
        case 'V':
            return 5;
        case 'X':
            return 10;
        case 'L':
            return 50;
        case 'C':
            return 100;
        case 'D':
            return 500;
        case 'M':
            return 1000;
        default:
            return 0;
    }
}

int from_roman(std::string_view text) {
    int total = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const int value = letter_value(text[i]);
        const int next = i + 1 < text.size() ? letter_value(text[i + 1]) : 0;
        // A smaller letter before a bigger one is subtracted: IV is 5 - 1.
        total += value < next ? -value : value;
    }
    return total;
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(line[0]))) {
            const int n = std::stoi(line);
            std::cout << n << " -> " << to_roman(n) << '\n';
        } else {
            std::cout << line << " -> " << from_roman(line) << '\n';
        }
    }
}
