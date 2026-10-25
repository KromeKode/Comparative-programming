#include <cmath>
#include <iostream>
#include <string>

int main() {
    int parsed = std::stoi("42");  // throws std::invalid_argument on bad input
    std::cout << "\"42\" + 1 as a number: " << parsed + 1 << '\n';

    double x = 3.99;
    std::cout << "3.99 truncated: " << static_cast<int>(x) << '\n';
    std::cout << "-3.99 truncated: " << static_cast<int>(-x) << '\n';
    std::cout << "2.5 rounded: " << std::lround(2.5) << '\n';  // halfway rounds away from zero
    std::cout << "-2.5 rounded: " << std::lround(-2.5) << '\n';

    int seven = 7;
    int two = 2;
    std::cout << "7 / 2 as integers: " << seven / two << '\n';
    std::cout << "7 / 2 as floats: " << static_cast<double>(seven) / two << '\n';
}
