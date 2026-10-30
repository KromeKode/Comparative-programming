#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

// Decodes the code point that starts at s[i] and moves i past it (valid UTF-8 only).
char32_t decode_utf8(std::string_view s, std::size_t& i) {
    auto byte = [&](std::size_t k) -> char32_t { return static_cast<unsigned char>(s[k]); };
    char32_t first = byte(i);
    if (first < 0x80) {
        i += 1;
        return first;
    }
    if (first < 0xE0) {
        char32_t cp = ((first & 0x1F) << 6) | (byte(i + 1) & 0x3F);
        i += 2;
        return cp;
    }
    if (first < 0xF0) {
        char32_t cp = ((first & 0x0F) << 12) | ((byte(i + 1) & 0x3F) << 6) | (byte(i + 2) & 0x3F);
        i += 3;
        return cp;
    }
    char32_t cp = ((first & 0x07) << 18) | ((byte(i + 1) & 0x3F) << 12) |
                  ((byte(i + 2) & 0x3F) << 6) | (byte(i + 3) & 0x3F);
    i += 4;
    return cp;
}

// std::string holds bytes, so reversing it would scramble multi-byte characters.
// A std::u32string holds one code point per element.
bool is_palindrome(std::string_view line) {
    std::u32string kept;
    for (std::size_t i = 0; i < line.size();) {
        char32_t cp = decode_utf8(line, i);
        if (cp >= 0x80) {
            kept += cp;
        } else if (std::isalnum(static_cast<int>(cp))) {
            kept += static_cast<char32_t>(std::tolower(static_cast<int>(cp)));
        }
    }
    return std::equal(kept.begin(), kept.end(), kept.rbegin());
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        const auto first = line.find_first_not_of(" \t\r");
        if (first == std::string::npos) {
            continue;
        }
        const auto last = line.find_last_not_of(" \t\r");
        const std::string text = line.substr(first, last - first + 1);
        std::cout << text << ": " << (is_palindrome(text) ? "palindrome" : "not a palindrome")
                  << '\n';
    }
}
