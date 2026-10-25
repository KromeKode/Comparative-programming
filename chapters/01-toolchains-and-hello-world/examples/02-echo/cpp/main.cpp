#include <iostream>
#include <string>

int main() {
    std::string line;
    std::getline(std::cin, line);        // removes the '\n', not a '\r'
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    std::cout << "You typed: " << line << '\n';
}
