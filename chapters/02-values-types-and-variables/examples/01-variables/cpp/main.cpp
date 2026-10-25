#include <iostream>

constexpr int max_players = 4;  // a compile-time constant with a real type

int main() {
    int count = 0;
    const int birth_year = 1815;
    const double pi = 3.14159;
    bool ready = true;

    count += 3;

    std::cout << "count = " << count << '\n';
    std::cout << "birth year = " << birth_year << '\n';
    std::cout << "pi = " << pi << '\n';
    std::cout << "max players = " << max_players << '\n';
    std::cout << "ready = " << std::boolalpha << ready << '\n';  // without boolalpha: 1
}
