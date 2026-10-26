#include <charconv>
#include <compare>
#include <iostream>
#include <string>
#include <string_view>

std::string_view trim(std::string_view text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        return {};
    }
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::string guesses_text(int count) {
    return std::to_string(count) + (count == 1 ? " guess" : " guesses");
}

int main() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        return 1;
    }
    const long secret = std::stol(line);

    int guesses = 0;
    while (std::getline(std::cin, line)) {
        const std::string_view text = trim(line);
        if (text.empty()) {
            continue;  // a blank line
        }
        long guess = 0;
        const char* end = text.data() + text.size();
        auto [ptr, error] = std::from_chars(text.data(), end, guess);
        if (error != std::errc{} || ptr != end) {  // no digits, or text left over
            std::cout << text << ": not a number\n";
            continue;
        }
        ++guesses;
        // <=> (C++20) compares once and reports less, greater, or equal.
        const auto order = guess <=> secret;
        if (order < 0) {
            std::cout << guess << ": too low\n";
        } else if (order > 0) {
            std::cout << guess << ": too high\n";
        } else {
            std::cout << guess << ": correct in " << guesses_text(guesses) << '\n';
            return 0;
        }
    }
    std::cout << "gave up after " << guesses_text(guesses) << "; the number was " << secret
              << '\n';
}
