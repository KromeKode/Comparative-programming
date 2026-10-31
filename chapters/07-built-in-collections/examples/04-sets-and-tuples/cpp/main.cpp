#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>

// std::set keeps its items sorted; std::unordered_set is a hash set.
void print(const std::string& label, const std::set<int>& items) {
    std::cout << label << ':';
    for (int n : items) {
        std::cout << ' ' << n;
    }
    std::cout << '\n';
}

int main() {
    const std::set<int> a = {1, 2, 3, 4};
    const std::set<int> b = {3, 4, 5};
    print("a", a);
    print("b", b);

    // The set algorithms walk two sorted ranges and write the result somewhere.
    std::set<int> all, both, only_a;
    std::ranges::set_union(a, b, std::inserter(all, all.end()));
    std::ranges::set_intersection(a, b, std::inserter(both, both.end()));
    std::ranges::set_difference(a, b, std::inserter(only_a, only_a.end()));
    print("union", all);
    print("intersection", both);
    print("a minus b", only_a);
    std::cout << "3 in a: " << (a.contains(3) ? "yes" : "no") << '\n';

    const std::vector<int> values = {3, 1, 3, 2, 1};
    print("unique of 3 1 3 2 1, sorted", std::set<int>(values.begin(), values.end()));

    // std::pair is a two-item tuple. Pairs compare item by item, so a set can sort them.
    const std::vector<std::pair<int, int>> path = {{0, 0}, {1, 0}, {0, 0}, {1, 1}};
    const std::set<std::pair<int, int>> visited(path.begin(), path.end());
    std::cout << "visited " << path.size() << " points, " << visited.size() << " distinct\n";
    std::cout << "(1, 0) visited: " << (visited.contains({1, 0}) ? "yes" : "no") << '\n';
}
