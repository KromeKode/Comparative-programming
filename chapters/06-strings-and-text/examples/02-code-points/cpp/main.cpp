#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

// Decodes the code point that starts at s[i] and moves i past it.
// It assumes valid UTF-8; chapter 36 shows how to check.
char32_t decode_utf8(std::string_view s, std::size_t& i) {
    auto byte = [&](std::size_t k) -> char32_t { return static_cast<unsigned char>(s[k]); };
    char32_t first = byte(i);
    if (first < 0x80) {  // 0xxxxxxx: one byte, plain ASCII
        i += 1;
        return first;
    }
    if (first < 0xE0) {  // 110xxxxx 10xxxxxx
        char32_t cp = ((first & 0x1F) << 6) | (byte(i + 1) & 0x3F);
        i += 2;
        return cp;
    }
    if (first < 0xF0) {  // 1110xxxx 10xxxxxx 10xxxxxx
        char32_t cp = ((first & 0x0F) << 12) | ((byte(i + 1) & 0x3F) << 6) | (byte(i + 2) & 0x3F);
        i += 3;
        return cp;
    }
    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    char32_t cp = ((first & 0x07) << 18) | ((byte(i + 1) & 0x3F) << 12) |
                  ((byte(i + 2) & 0x3F) << 6) | (byte(i + 3) & 0x3F);
    i += 4;
    return cp;
}

int main() {
    const std::string s = "Zo\u00eb \U0001F44D\U0001F3FD";

    std::cout << "UTF-8 bytes:";
    for (unsigned char b : s) {
        std::cout << std::format(" {:02X}", b);
    }
    std::cout << '\n';

    std::cout << "code points:";
    for (std::size_t i = 0; i < s.size();) {
        std::cout << std::format(" U+{:04X}", static_cast<std::uint32_t>(decode_utf8(s, i)));
    }
    std::cout << '\n';

    // substr counts bytes, so find the byte where the third code point ends.
    std::size_t end = 0;
    for (int n = 0; n < 3; ++n) {
        decode_utf8(s, end);
    }
    std::cout << "first 3 code points: " << s.substr(0, end) << '\n';
}
