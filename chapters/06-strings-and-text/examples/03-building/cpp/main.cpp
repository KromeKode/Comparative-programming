#include <format>
#include <iostream>
#include <string>
#include <vector>

int main() {
    const std::vector<std::string> fruits = {"apple", "banana", "cherry"};

    // std::string is mutable and grows as needed, so += in a loop is cheap.
    std::string joined;
    for (std::size_t i = 0; i < fruits.size(); ++i) {
        if (i > 0) {
            joined += ", ";
        }
        joined += fruits[i];
    }
    std::cout << "joined: " << joined << '\n';

    std::string built;
    for (int n = 1; n <= 5; ++n) {
        built += (n > 1 ? "-" : "") + std::to_string(n);
    }
    std::cout << "built: " << built << '\n';

    // std::format (C++20) uses Python-style format specs.
    std::cout << std::format("padded: [{:<8}] [{:>8}]\n", "Ada", 90);
    std::cout << std::format("zero-padded: {:05}\n", 42);
    std::cout << std::format("two decimals: {:.2f}\n", 3.14159);
}
