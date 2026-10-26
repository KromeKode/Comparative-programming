#include <iostream>

bool is_leap(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int main() {
    int year;
    while (std::cin >> year) {  // >> skips blank lines for us
        std::cout << year << ": " << (is_leap(year) ? "leap year" : "common year") << '\n';
    }
}
