#include <array>
#include <cstddef>
#include <iostream>
#include <string_view>

int main() {
    std::cout << "up:";
    for (int i = 0; i < 5; ++i) {
        std::cout << ' ' << i;
    }
    std::cout << '\n';

    std::cout << "down:";
    for (int i = 5; i >= 1; --i) {
        std::cout << ' ' << i;
    }
    std::cout << '\n';

    std::cout << "by threes:";
    for (int i = 0; i < 10; i += 3) {
        std::cout << ' ' << i;
    }
    std::cout << '\n';

    std::cout << "halving:";
    int n = 100;
    while (n > 0) {
        std::cout << ' ' << n;
        n /= 2;
    }
    std::cout << '\n';

    constexpr std::array<std::string_view, 3> colors{"red", "green", "blue"};
    std::cout << "colors:";
    for (std::string_view color : colors) {  // range-based for (C++11)
        std::cout << ' ' << color;
    }
    std::cout << '\n';

    std::cout << "numbered:";
    for (std::size_t i = 0; i < colors.size(); ++i) {
        std::cout << ' ' << i + 1 << '=' << colors[i];
    }
    std::cout << '\n';

    // do-while tests the condition after the body, so the body always runs once.
    int m = 100;
    do {
        m *= 2;
    } while (m < 50);
    std::cout << "do-while: " << m << '\n';
}
