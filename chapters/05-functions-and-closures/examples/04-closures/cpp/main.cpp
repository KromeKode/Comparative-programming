#include <functional>
#include <iostream>
#include <vector>

// [n] captures n by value: the lambda keeps its own copy.
auto make_adder(int n) {
    return [n](int x) { return x + n; };
}

// An init-capture creates a variable inside the lambda. mutable lets calls change it.
auto make_accumulator() {
    return [total = 0](int x) mutable {
        total += x;
        return total;
    };
}

int main() {
    auto add5 = make_adder(5);
    auto add10 = make_adder(10);
    std::cout << "add5(10) = " << add5(10) << '\n';
    std::cout << "add10(10) = " << add10(10) << '\n';

    auto running = make_accumulator();
    const int a = running(3);
    const int b = running(5);
    const int c = running(2);
    std::cout << "running total: " << a << ' ' << b << ' ' << c << '\n';

    std::vector<std::function<int()>> made;
    for (int i = 0; i < 3; ++i) {
        made.push_back([i] { return i; });  // [&i] would dangle once the loop ends
    }
    std::cout << "made in a loop:";
    for (const auto& f : made) {
        std::cout << ' ' << f();
    }
    std::cout << '\n';
}
