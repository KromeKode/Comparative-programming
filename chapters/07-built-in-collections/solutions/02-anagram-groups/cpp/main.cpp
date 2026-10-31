#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

int main() {
    // An unordered_map forgets the order keys arrived in, so it maps each key to a
    // group number, and the groups live in a vector in the order they were created.
    std::unordered_map<std::string, std::size_t> group_of;
    std::vector<std::vector<std::string>> groups;

    for (std::string word; std::cin >> word;) {
        for (char& c : word) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        std::string key = word;
        std::ranges::sort(key);  // anagrams have the same letters once sorted
        auto [it, inserted] = group_of.try_emplace(key, groups.size());
        if (inserted) {
            groups.emplace_back();
        }
        groups[it->second].push_back(word);
    }

    for (const auto& group : groups) {
        for (std::size_t i = 0; i < group.size(); ++i) {
            std::cout << (i > 0 ? " " : "") << group[i];
        }
        std::cout << '\n';
    }
}
