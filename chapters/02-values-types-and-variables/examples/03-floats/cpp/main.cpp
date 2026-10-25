#include <cmath>
#include <iomanip>
#include <iostream>

int main() {
    double a = 0.1;
    double b = 0.2;
    double sum = a + b;

    // std::cout shows 6 significant digits by default, which would print 0.3.
    std::cout << "0.1 + 0.2 = " << std::setprecision(17) << sum << '\n';
    std::cout << "equal to 0.3? " << (sum == 0.3 ? "yes" : "no") << '\n';
    std::cout << "close to 0.3? " << (std::abs(sum - 0.3) < 1e-9 ? "yes" : "no") << '\n';
}
