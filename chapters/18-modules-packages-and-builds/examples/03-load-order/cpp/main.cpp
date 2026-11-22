#include <iostream>

#include "settings.hpp"

int main() {
    std::cout << "main: start\n";
    std::cout << "main: the greeting is " << settings::greeting << '\n';
    std::cout << "main: the greeting is still " << settings::greeting << '\n';
}
