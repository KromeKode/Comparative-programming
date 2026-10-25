#include <iostream>
#include <string>

int main() {
    std::string line;
    std::getline(std::cin, line);

    const char* whitespace = " \t\r\n";
    const auto first = line.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        std::cout << "Hello, stranger!\n";
        return 0;
    }
    const auto last = line.find_last_not_of(whitespace);
    std::cout << "Hello, " << line.substr(first, last - first + 1) << "!\n";
}
