#include <algorithm>
#include <iostream>
#include <span>
#include <string>
#include <vector>

// std::span is a view of some elements that live elsewhere: a pointer and a count.
void print(const std::string& label, std::span<const int> items) {
    std::cout << label << ": [";
    for (std::size_t i = 0; i < items.size(); ++i) {
        std::cout << (i > 0 ? ", " : "") << items[i];
    }
    std::cout << "]\n";
}

int main() {
    std::vector<int> numbers = {3, 1, 4, 1, 5};
    print("numbers", numbers);
    std::cout << "length: " << numbers.size() << '\n';
    std::cout << "first: " << numbers.front() << ", last: " << numbers.back() << '\n';

    numbers.push_back(9);
    print("append 9", numbers);
    numbers.insert(numbers.begin() + 1, 2);
    print("insert 2 at index 1", numbers);
    int removed = numbers.front();
    numbers.erase(numbers.begin());
    print("remove index 0 (was " + std::to_string(removed) + ")", numbers);

    bool found = std::ranges::find(numbers, 4) != numbers.end();
    std::cout << "contains 4: " << (found ? "yes" : "no") << '\n';
    auto first_one = std::ranges::find(numbers, 1) - numbers.begin();
    std::cout << "index of 1: " << first_one << '\n';

    std::ranges::sort(numbers);
    print("sorted", numbers);
    print("slice 1..4", std::span(numbers).subspan(1, 3));  // a view, no copy

    std::ranges::reverse(numbers);
    print("reversed", numbers);
}
