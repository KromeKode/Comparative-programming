#include <iostream>
#include <vector>

void bump_copy(int n) {
    n++;
    std::cout << "inside bump_copy: n = " << n << "\n";
}

// A reference parameter is another name for the caller's variable.
void bump(int &n) {
    n++;
}

// The C way still works.
void bump_pointer(int *n) {
    (*n)++;
}

// Passing a vector by value copies all of its items.
void add_copy(std::vector<int> v) {
    v.push_back(4);
    std::cout << "inside add_copy: " << v.size() << " items\n";
}

void add(std::vector<int> &v) {
    v.push_back(4);
}

// A const reference reads the caller's vector without copying it.
int total(const std::vector<int> &v) {
    int sum = 0;
    for (int item : v) {
        sum += item;
    }
    return sum;
}

int main() {
    int n = 1;
    bump_copy(n);
    std::cout << "after bump_copy(n): n = " << n << "\n";
    bump(n); // looks just like the call above, but changes n
    std::cout << "after bump(n): n = " << n << "\n";
    bump_pointer(&n);
    std::cout << "after bump_pointer(&n): n = " << n << "\n";

    std::vector<int> v = {1, 2, 3};
    add_copy(v);
    std::cout << "after add_copy(v): v has " << v.size() << " items\n";
    add(v);
    std::cout << "after add(v): v has " << v.size() << " items\n";
    std::cout << "total(v) = " << total(v) << "\n";
}
