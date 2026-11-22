// Exercise 18.1: a stock module. This file is given: write stock.hpp and stock.cpp
// so that it builds. It reads the commands, and leaves keeping count to the module.
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "stock.hpp"

// An amount is a whole number from 1 to 999,999,999,999,999: digits, with any
// zeros in front, and at most 15 more after them.
static std::optional<long long> parse_amount(const std::string& word) {
    std::size_t start = word.find_first_not_of('0');
    if (start == std::string::npos || word.size() - start > 15) return std::nullopt;
    if (word.find_first_not_of("0123456789") != std::string::npos) return std::nullopt;
    return std::stoll(word.substr(start));
}

int main() {
    stock::Stock stock;
    std::string line;
    for (int number = 1; std::getline(std::cin, line); number++) {
        std::istringstream in(line);
        std::vector<std::string> words;
        for (std::string word; in >> word;) words.push_back(word);
        if (words.empty()) continue;
        std::optional<long long> amount;
        if (words.size() == 3 && words[0] == "add" && (amount = parse_amount(words[2]))) {
            stock.add(words[1], *amount);
            std::cout << "added " << *amount << ' ' << words[1] << ", now "
                      << stock.count(words[1]) << '\n';
        } else if (words.size() == 3 && words[0] == "take" && (amount = parse_amount(words[2]))) {
            if (stock.take(words[1], *amount)) {
                std::cout << "took " << *amount << ' ' << words[1] << ", now "
                          << stock.count(words[1]) << '\n';
            } else {
                std::cout << "error: line " << number << ": only " << stock.count(words[1])
                          << ' ' << words[1] << " in stock\n";
            }
        } else if (words.size() == 2 && words[0] == "count") {
            std::cout << words[1] << ": " << stock.count(words[1]) << '\n';
        } else if (words.size() == 1 && words[0] == "list") {
            std::string text;
            for (const auto& [item, count] : stock.items()) {
                text += (text.empty() ? "" : ", ") + item + ' ' + std::to_string(count);
            }
            std::cout << "stock: " << (text.empty() ? "empty" : text) << '\n';
        } else {
            std::cout << "error: line " << number << ": expected add, take, count, or list\n";
        }
    }
}
