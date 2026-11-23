// Counts the words in its input, and prints the ten most common. It is slow on a long
// text, and section 20.7 runs it under a profiler to find out why.
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

using Counts = std::vector<std::pair<std::string, int>>;

// Gives the words of the text: runs of the letters a to z, in lowercase.
std::vector<std::string> split_words(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (char c : text) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (c >= 'a' && c <= 'z') {
            word += c;
        } else if (!word.empty()) {
            words.push_back(std::move(word));
            word.clear();
        }
    }
    if (!word.empty()) words.push_back(std::move(word));
    return words;
}

// Gives the word's count, or end() if it has none.
Counts::iterator find(Counts& counts, const std::string& word) {
    return std::find_if(counts.begin(), counts.end(),
                        [&](const auto& pair) { return pair.first == word; });
}

Counts count_words(const std::vector<std::string>& words) {
    Counts counts;
    for (const std::string& word : words) {
        auto it = find(counts, word);
        if (it == counts.end()) {
            counts.emplace_back(word, 1);
        } else {
            it->second++;
        }
    }
    return counts;
}

int main() {
    std::string text(std::istreambuf_iterator<char>(std::cin), {});
    Counts counts = count_words(split_words(text));
    std::sort(counts.begin(), counts.end(), [](const auto& a, const auto& b) {
        return a.second != b.second ? a.second > b.second : a.first < b.first;
    });
    for (std::size_t i = 0; i < counts.size() && i < 10; i++) {
        std::cout << counts[i].second << ' ' << counts[i].first << '\n';
    }
}
