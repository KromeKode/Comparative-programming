#include <iostream>
#include <string>
#include <utility>
#include <vector>

std::string show(const std::vector<int> &v) {
    std::string out;
    for (int item : v) {
        out += (out.empty() ? "" : " ") + std::to_string(item);
    }
    return out;
}

// const means this function will not change amount. It does not mean that
// nothing else will.
void add_each(std::vector<int> &values, const int &amount) {
    for (int &item : values) {
        item += amount;
    }
}

int main() {
    int x = 1;
    int &a = x;
    int &b = x; // two references to one int
    a = 5;
    std::cout << "after a = 5: b = " << b << "\n";

    int y = 3;
    std::swap(x, y); // uses a temporary, so it is safe even for std::swap(x, x)
    std::cout << "after std::swap(x, y): x = " << x << ", y = " << y << "\n";

    std::vector<int> values = {1, 2, 3};
    add_each(values, values[0]); // amount is values[0] itself
    std::cout << "add_each(values, values[0]): " << show(values) << "\n";

    std::vector<int> fixed = {1, 2, 3};
    int amount = fixed[0];
    add_each(fixed, amount);
    std::cout << "with a copy of the amount: " << show(fixed) << "\n";

    // Two vectors are two separate blocks of memory, even after assignment.
    std::vector<int> other = fixed;
    other[0] = 100;
    std::cout << "other = fixed, then other[0] = 100: fixed = " << show(fixed) << "\n";
}
