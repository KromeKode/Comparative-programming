#include <functional>
#include <iostream>

int add_one(int x) {
    return x + 1;
}

int triple(int x) {
    return x * 3;
}

// std::function holds anything callable: a function, a lambda, or an object.
int apply_twice(const std::function<int(int)>& f, int x) {
    return f(f(x));
}

// auto parameters (C++20) make this a template, so any two callables work.
auto compose(auto outer, auto inner) {
    return [outer, inner](int x) { return outer(inner(x)); };
}

int main() {
    std::cout << "apply_twice(add_one, 5) = " << apply_twice(add_one, 5) << '\n';
    std::cout << "apply_twice(triple, 5) = " << apply_twice(triple, 5) << '\n';
    auto square = [](int x) { return x * x; };  // a lambda: a function with no name
    std::cout << "apply_twice(square, 3) = " << apply_twice(square, 3) << '\n';
    auto add_then_triple = compose(triple, add_one);
    std::cout << "compose(triple, add_one)(5) = " << add_then_triple(5) << '\n';
}
