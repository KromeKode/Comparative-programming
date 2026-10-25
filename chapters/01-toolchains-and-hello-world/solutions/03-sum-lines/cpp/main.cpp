#include <cstdint>
#include <iostream>

int main() {
    std::int64_t total = 0;
    std::int64_t value = 0;
    while (std::cin >> value) {  // >> skips spaces and newlines, so blank lines vanish
        total += value;
    }
    std::cout << total << '\n';
}
