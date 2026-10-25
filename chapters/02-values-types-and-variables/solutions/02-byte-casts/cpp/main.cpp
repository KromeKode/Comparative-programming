#include <cstdint>
#include <iostream>

int main() {
    std::int64_t x;
    while (std::cin >> x) {
        auto u8 = static_cast<std::uint8_t>(x);  // keeps the lowest 8 bits
        auto i8 = static_cast<std::int8_t>(x);   // C++20 defines this as wrapping too
        // uint8_t and int8_t print as characters, so convert them to int first.
        std::cout << x << " -> u8 " << static_cast<int>(u8) << ", i8 " << static_cast<int>(i8)
                  << '\n';
    }
}
