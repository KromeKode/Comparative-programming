// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
#include <array>
#include <cctype>
#include <iostream>
#include <string>

std::array<int, 26> count_letters(std::istream& in) {
    std::array<int, 26> counts{};
    char c;
    while (in.get(c)) {
        if (std::isalpha(static_cast<unsigned char>(c))) counts[c - 'a']++;
    }
    return counts;
}

int main() {
    std::array<int, 26> counts = count_letters(std::cin);
    for (int i = 0; i < 26; i++) {
        if (counts[i] == 0) continue;
        std::cout << char('a' + i) << ' ' << counts[i] << ' ' << std::string(counts[i], '#')
                  << '\n';
    }
}
