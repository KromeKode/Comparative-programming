#include <bit>
#include <cstdint>
#include <format>
#include <iostream>

int main() {
    std::uint32_t n;
    while (std::cin >> n) {
        // <bit> (C++20) has these as single CPU instructions on most machines.
        std::cout << std::format("{}: {:b} (ones {}, width {})\n", n, n, std::popcount(n),
                                 std::bit_width(n));
    }
}
