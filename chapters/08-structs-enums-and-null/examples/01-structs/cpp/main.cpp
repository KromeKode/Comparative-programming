#include <cstdlib>
#include <iostream>

struct Point {
    int x;
    int y;

    // A member function, called as p.steps_from_origin().
    int steps_from_origin() const { return std::abs(x) + std::abs(y); }

    // Since C++20, asking for the default == compares every field in order.
    bool operator==(const Point &) const = default;
};

// Teaches << how to print a Point.
std::ostream &operator<<(std::ostream &out, const Point &p) {
    return out << '(' << p.x << ", " << p.y << ')';
}

int main() {
    Point p{1, 2};
    std::cout << "p = " << p << '\n';
    std::cout << "p.x + p.y = " << p.x + p.y << '\n';

    // Designated initializers (C++20) name the fields they set.
    Point q{.x = 5, .y = p.y};
    std::cout << "q = p with x = 5: q = " << q << ", p = " << p << '\n';

    Point r = p; // a copy
    r.y = 9;
    std::cout << "r = p, then r.y = 9: r = " << r << ", p = " << p << '\n';

    Point a{3, -4};
    Point b{3, -4};
    std::cout << "a = " << a << ", b = " << b << ", a == b: " << (a == b ? "yes" : "no")
              << '\n';
    std::cout << "steps from (0, 0) to a: " << a.steps_from_origin() << '\n';
}
