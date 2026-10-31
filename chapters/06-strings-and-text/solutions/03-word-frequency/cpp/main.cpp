#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

// A word character is an ASCII letter or any byte of a non-ASCII character (0x80 and up).
bool is_word_byte(unsigned char b) {
    return (b >= 'a' && b <= 'z') || (b >= 'A' && b <= 'Z') || b >= 0x80;
}

int main() {
    // istreambuf_iterator reads every byte of stdin, whitespace included.
    const std::string text(std::istreambuf_iterator<char>(std::cin), {});

    std::map<std::string, int> counts;  // a std::map keeps its keys sorted
    std::string word;
    for (std::size_t i = 0; i <= text.size(); ++i) {
        const unsigned char b = i < text.size() ? static_cast<unsigned char>(text[i]) : ' ';
        if (is_word_byte(b)) {
            word += static_cast<char>(b >= 'A' && b <= 'Z' ? b - 'A' + 'a' : b);
        } else if (!word.empty()) {
            ++counts[word];
            word.clear();
        }
    }

    // The map is already in byte order, which for UTF-8 is code point order.
    // stable_sort keeps that order among words with the same count.
    std::vector<std::pair<std::string, int>> sorted(counts.begin(), counts.end());
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const auto& a, const auto& b) { return a.second > b.second; });
    for (const auto& [w, n] : sorted) {
        std::cout << n << ' ' << w << '\n';
    }
}
