#include <algorithm>
#include <charconv>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// References give swap the caller's two strings. std::swap trades the
// strings' insides (a pointer and two sizes) without copying any characters.
void swap(std::string &a, std::string &b) {
    std::swap(a, b);
}

// Reverses words[from..to] and returns how many swaps that took.
int reverse(std::vector<std::string> &words, std::size_t from, std::size_t to) {
    int swaps = 0;
    for (; from < to; from++, to--) {
        swap(words[from], words[to]);
        swaps++;
    }
    return swaps;
}

std::optional<std::size_t> parse_position(const std::string &text, std::size_t count) {
    if (!std::all_of(text.begin(), text.end(), [](char c) { return c >= '0' && c <= '9'; })) {
        std::cout << "error: \"" << text << "\" is not a position\n";
        return std::nullopt;
    }
    // from_chars reports a number too big for value instead of throwing.
    std::size_t value = 0;
    auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc() || value < 1 || value > count) {
        std::cout << "error: position " << text << " is out of range (1 to " << count
                  << ")\n";
        return std::nullopt;
    }
    return value;
}

int main() {
    std::vector<std::string> words;
    std::string line;
    if (std::getline(std::cin, line)) {
        std::istringstream in(line);
        for (std::string word; in >> word;) {
            words.push_back(word);
        }
    }

    int swaps = 0;
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
        if (command != "swap" && command != "reverse") {
            std::cout << "error: unknown command \"" << command << "\"\n";
            continue;
        }
        if (parts.size() != 3) {
            std::cout << "error: " << command << " needs 2 positions, got "
                      << parts.size() - 1 << "\n";
            continue;
        }
        auto i = parse_position(parts[1], words.size());
        if (!i) {
            continue;
        }
        auto j = parse_position(parts[2], words.size());
        if (!j) {
            continue;
        }
        if (command == "swap") {
            swap(words[*i - 1], words[*j - 1]);
            swaps++;
        } else {
            swaps += reverse(words, std::min(*i, *j) - 1, std::max(*i, *j) - 1);
        }
        for (std::size_t k = 0; k < words.size(); k++) {
            std::cout << (k == 0 ? "" : " ") << words[k];
        }
        std::cout << "\n";
    }
    std::cout << "swaps: " << swaps << "\n";
}
