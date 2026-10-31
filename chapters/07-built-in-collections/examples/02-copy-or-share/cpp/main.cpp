#include <iostream>
#include <string>
#include <vector>

std::string show(const std::vector<int>& items) {
    std::string text = "[";
    for (std::size_t i = 0; i < items.size(); ++i) {
        text += (i > 0 ? ", " : "") + std::to_string(items[i]);
    }
    return text + "]";
}

// A reference parameter (&) refers to the caller's vector. Without the &, the
// function would get its own copy.
void set_first(std::vector<int>& items) { items[0] = -1; }

int main() {
    std::vector<int> a = {1, 2, 3};

    std::vector<int> b = a;  // copies every element
    b[0] = 99;
    std::cout << "b = a, then b[0] = 99: a is " << show(a) << '\n';

    auto c = a;  // also a copy: C++ needs no separate copy call
    c[0] = 0;
    std::cout << "c = copy of a, then c[0] = 0: a is " << show(a) << '\n';

    set_first(a);
    std::cout << "set_first(a) sets a[0] = -1: a is " << show(a) << '\n';

    std::vector<std::vector<int>> grid = {{0, 0}, {0, 0}};
    auto g = grid;  // copies the outer vector and every row in it
    g[0][0] = 1;
    std::cout << "g = copy of grid, then g[0][0] = 1: grid is [" << show(grid[0]) << ", "
              << show(grid[1]) << "]\n";
}
