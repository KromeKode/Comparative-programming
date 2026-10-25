#include <cstdint>
#include <iostream>
#include <limits>

int main() {
    constexpr auto max = std::numeric_limits<std::int64_t>::max();
    constexpr auto min = std::numeric_limits<std::int64_t>::min();

    std::int64_t a, b;
    while (std::cin >> a >> b) {
        // Check before adding: computing a + b would itself be undefined behavior.
        if ((b > 0 && a > max - b) || (b < 0 && a < min - b)) {
            std::cout << "overflow\n";
        } else {
            std::cout << a + b << '\n';
        }
    }
}
