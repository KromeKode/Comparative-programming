#include <algorithm>
#include <iostream>
#include <iterator>
#include <ranges>
#include <vector>

// Both functions say when they run, so the output shows the order of the work.
int square(int n) {
    std::cout << "  square " << n << "\n";
    return n * n;
}

bool even(int x) {
    std::cout << "  even? " << x << "\n";
    return x % 2 == 0;
}

void print_first_two(const std::vector<int>& items) {
    std::cout << "  first two: " << items[0] << " " << items[1] << "\n";
}

int main() {
    const std::vector numbers{1, 2, 3, 4, 5};

    // Eager: each algorithm finishes the whole vector before the next starts.
    std::cout << "eager:\n";
    std::vector<int> squares, evens;
    std::ranges::transform(numbers, std::back_inserter(squares), square);
    std::ranges::copy_if(squares, std::back_inserter(evens), even);
    print_first_two(evens);

    // Lazy: a view does nothing until the loop asks it for items. The output
    // also shows two costs of C++20 views (section 14.5): reading an item
    // through filter calls square again, and take steps one item too far.
    std::cout << "lazy:\n";
    std::vector<int> first_two;
    for (int x : numbers | std::views::transform(square) | std::views::filter(even) |
                     std::views::take(2)) {
        first_two.push_back(x);
    }
    print_first_two(first_two);

    // iota(1) counts up forever, so only a lazy pipeline can use it.
    std::cout << "endless:";
    auto big_even = [](int x) { return x % 2 == 0 && x > 50; };
    for (int x : std::views::iota(1) | std::views::transform([](int n) { return n * n; }) |
                     std::views::filter(big_even) | std::views::take(3)) {
        std::cout << " " << x;
    }
    std::cout << "\n";
}
