#include <iostream>
#include <string>
#include <vector>

struct Point {
    int x, y;
};

// Overloads: one name, a separate function for each type. The compiler picks
// one from the type of the argument.
std::string show(int value) { return std::to_string(value); }

std::string show(bool value) { return value ? "true" : "false"; }

std::string show(const std::string& value) { return "\"" + value + "\""; }

std::string show(Point point) {
    return "(" + std::to_string(point.x) + ", " + std::to_string(point.y) + ")";
}

// A template covers every vector whose items have a show, including vectors
// of vectors, which call this same template one level down.
template <typename T>
std::string show(const std::vector<T>& items) {
    std::string text = "[";
    for (std::size_t i = 0; i < items.size(); i++) {
        if (i > 0) text += ", ";
        text += show(items[i]);
    }
    return text + "]";
}

int main() {
    using namespace std::string_literals;  // "hi"s is a std::string, not a char array
    std::cout << show(42) << "\n";
    std::cout << show("hi"s) << "\n";
    std::cout << show(true) << "\n";
    std::cout << show(std::vector{1, 2, 3}) << "\n";
    std::cout << show(std::vector<std::vector<std::string>>{{"a", "b"}, {}, {"c"}}) << "\n";
    std::cout << show(Point{1, 2}) << "\n";
    std::cout << show(std::vector<Point>{{1, 2}, {3, 4}}) << "\n";
}
