#include <algorithm>
#include <cstdint>
#include <iostream>

int main() {
    std::uint64_t start;
    while (std::cin >> start) {
        // The values climb far past 2^32 for some starts, so use 64 bits.
        std::uint64_t n = start;
        std::uint64_t peak = n;
        int steps = 0;
        while (n != 1) {
            n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
            peak = std::max(peak, n);
            ++steps;
        }
        std::cout << start << ": steps " << steps << ", peak " << peak << '\n';
    }
}
