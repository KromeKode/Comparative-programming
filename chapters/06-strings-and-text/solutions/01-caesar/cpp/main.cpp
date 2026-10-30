#include <iostream>
#include <string>

// Shifts an ASCII letter and leaves every other byte alone. That is safe for UTF-8:
// every byte of a non-ASCII character is 0x80 or above, so none looks like a letter.
char shift_letter(char c, int shift) {
    if (c >= 'a' && c <= 'z') {
        return static_cast<char>('a' + (c - 'a' + shift) % 26);
    }
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>('A' + (c - 'A' + shift) % 26);
    }
    return c;
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        const auto start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos) {
            continue;  // a blank line
        }
        const auto space = line.find(' ', start);
        const long shift = std::stol(line.substr(start, space - start));
        // C++'s % keeps the sign of the left side, so turn the shift into 0 to 25 first.
        const int positive = static_cast<int>((shift % 26 + 26) % 26);
        std::string text = space == std::string::npos ? "" : line.substr(space + 1);
        if (!text.empty() && text.back() == '\r') {
            text.pop_back();
        }
        for (char& c : text) {
            c = shift_letter(c, positive);
        }
        std::cout << text << '\n';
    }
}
