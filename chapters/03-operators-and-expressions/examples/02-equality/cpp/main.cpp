#include <iostream>
#include <limits>
#include <string>
#include <vector>

const char* yes_no(bool condition) {
    return condition ? "yes" : "no";
}

int main() {
    std::string a = "hello";
    std::string b = "hel";
    b += "lo";  // built separately, same characters
    // std::string defines == to compare the characters.
    std::cout << "strings with ==: " << yes_no(a == b) << '\n';

    std::vector<int> xs{1, 2, 3};
    std::vector<int> ys{1, 2, 3};
    std::cout << "vectors with ==: " << yes_no(xs == ys) << '\n';

    double nan = std::numeric_limits<double>::quiet_NaN();
    std::cout << "NaN == NaN: " << yes_no(nan == nan) << '\n';
}
