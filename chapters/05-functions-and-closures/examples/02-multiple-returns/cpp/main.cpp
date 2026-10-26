#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// Return several values as a std::pair (or std::tuple for more than two).
std::pair<int, int> min_max(const std::vector<int>& values) {
    int lowest = values[0];
    int highest = values[0];
    for (int value : values) {
        lowest = std::min(lowest, value);
        highest = std::max(highest, value);
    }
    return {lowest, highest};
}

std::pair<int, int> div_mod(int a, int b) {
    return {a / b, a % b};
}

int main() {
    // Structured bindings (C++17) unpack the pair into two named variables.
    auto [lowest, highest] = min_max({3, -1, 4, 1, 5});
    std::cout << "min and max of [3, -1, 4, 1, 5]: " << lowest << ' ' << highest << '\n';

    auto [quotient, remainder] = div_mod(17, 5);
    std::cout << "17 divided by 5: " << quotient << " remainder " << remainder << '\n';

    std::string first = "hello";
    std::string second = "world";
    std::swap(first, second);  // takes both by reference and swaps them in place
    std::cout << "swapped: " << first << ' ' << second << '\n';
}
