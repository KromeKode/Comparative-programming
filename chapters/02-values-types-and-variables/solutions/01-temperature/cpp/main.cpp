#include <iomanip>
#include <iostream>

int main() {
    double value;
    char unit;
    std::cout << std::fixed << std::setprecision(1);  // every double after this: 1 decimal
    while (std::cin >> value >> unit) {
        if (unit == 'C') {
            std::cout << value << " C = " << value * 9.0 / 5.0 + 32.0 << " F\n";
        } else {
            std::cout << value << " F = " << (value - 32.0) * 5.0 / 9.0 << " C\n";
        }
    }
}
