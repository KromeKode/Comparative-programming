#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::int64_t target;
        char colon;
        if (!(in >> target >> colon)) {
            continue;  // a blank line
        }
        std::vector<std::int64_t> numbers;
        for (std::int64_t n; in >> n;) {
            numbers.push_back(n);
        }

        // Each number seen so far, mapped to the first index where it appeared.
        std::unordered_map<std::int64_t, std::size_t> first_index;
        bool found = false;
        for (std::size_t j = 0; j < numbers.size() && !found; ++j) {
            auto partner = first_index.find(target - numbers[j]);
            if (partner != first_index.end()) {
                std::cout << partner->second << ' ' << j << '\n';
                found = true;
            }
            first_index.try_emplace(numbers[j], j);  // does nothing if already there
        }
        if (!found) {
            std::cout << "none\n";
        }
    }
}
