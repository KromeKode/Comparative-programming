#include <format>
#include <iostream>
#include <string>
#include <utility>

// C++ has no pattern matching yet. Structured bindings unpack the pair,
// and an if chain does the rest.
std::string classify(std::pair<int, int> point) {
    auto [x, y] = point;
    if (x == 0 && y == 0) {
        return "origin";
    }
    if (y == 0) {
        return std::format("on the x axis at x = {}", x);
    }
    if (x == 0) {
        return std::format("on the y axis at y = {}", y);
    }
    if (x == y) {
        return std::format("on the diagonal at {}", x);
    }
    return std::format("x = {}, y = {}", x, y);
}

int main() {
    for (auto point : {std::pair{0, 0}, {3, 0}, {0, -2}, {4, 4}, {-1, 5}}) {
        std::cout << std::format("({}, {}): {}\n", point.first, point.second, classify(point));
    }
}
