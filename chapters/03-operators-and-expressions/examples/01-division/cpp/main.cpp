#include <iostream>

int main() {
    // Integer division truncates toward zero, exactly as in C.
    std::cout << "7 / 2 = " << 7 / 2 << '\n';
    std::cout << "-7 / 2 = " << -7 / 2 << '\n';
    // The remainder takes the sign of the left operand.
    std::cout << "7 % 3 = " << 7 % 3 << '\n';
    std::cout << "-7 % 3 = " << -7 % 3 << '\n';
    std::cout << "7 % -3 = " << 7 % -3 << '\n';
    // If either side is a double, the other is converted and the result is a double.
    std::cout << "7.0 / 2 = " << 7.0 / 2 << '\n';
}
