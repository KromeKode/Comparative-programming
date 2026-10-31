#include <iostream>
#include <string>
#include <string_view>

// A byte of the form 10xxxxxx continues a code point; every other byte starts one.
std::size_t count_code_points(std::string_view s) {
    std::size_t count = 0;
    for (unsigned char byte : s) {
        if ((byte & 0xC0) != 0x80) {
            ++count;
        }
    }
    return count;
}

// Code points above U+FFFF start with a byte 11110xxx and need two UTF-16 code units.
std::size_t count_utf16_units(std::string_view s) {
    std::size_t count = 0;
    for (unsigned char byte : s) {
        if ((byte & 0xC0) != 0x80) {
            count += byte >= 0xF0 ? 2 : 1;
        }
    }
    return count;
}

int main() {
    // std::string holds bytes. GCC and Clang store this literal as UTF-8.
    const std::string s = "Zo\u00eb \U0001F44D\U0001F3FD";

    std::cout << "text: " << s << '\n';
    std::cout << "built-in length: " << s.size() << " (bytes)\n";
    std::cout << "UTF-8 bytes: " << s.size() << '\n';
    std::cout << "UTF-16 code units: " << count_utf16_units(s) << '\n';
    std::cout << "code points: " << count_code_points(s) << '\n';
}
