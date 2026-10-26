#include <iostream>

int main() {
    int a, b;
    while (std::cin >> a >> b) {
        // C++'s / and % truncate toward zero.
        int q = a / b;
        int r = a % b;
        // Floored division rounds down instead. The answers differ by one step when
        // the remainder is not zero and its sign differs from the divisor's.
        int fq = q;
        int fm = r;
        if (r != 0 && (r < 0) != (b < 0)) {
            fq -= 1;
            fm += b;
        }
        std::cout << a << ' ' << b << " -> truncated " << q << ' ' << r << ", floored " << fq
                  << ' ' << fm << '\n';
    }
}
