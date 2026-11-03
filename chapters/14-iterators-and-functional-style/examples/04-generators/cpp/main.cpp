#include <iostream>

// C++20 has coroutines but no generator type to use them with; C++23 adds
// std::generator. So these are written the other way round: they call visit
// with each value, and stop when visit returns false.
template <typename Visit>
void fibonacci(Visit visit) {
    long a = 0, b = 1;
    while (visit(a)) {
        long next = a + b;
        a = b;
        b = next;
    }
}

struct Node {
    int value;
    const Node* left = nullptr;
    const Node* right = nullptr;
};

// Returns whether to keep going, so a false from visit stops every level of
// the recursion at once.
template <typename Visit>
bool in_order(const Node* node, Visit& visit) {
    return !node || (in_order(node->left, visit) && visit(node->value) &&
                     in_order(node->right, visit));
}

int main() {
    const Node one{1}, three{3}, five{5}, seven{7};
    const Node two{2, &one, &three}, six{6, &five, &seven};
    const Node tree{4, &two, &six};

    int count = 0;
    std::cout << "fibonacci:";
    fibonacci([&](long value) {
        if (count++ == 10) return false;
        std::cout << " " << value;
        return true;
    });

    auto print_all = [](int value) {
        std::cout << " " << value;
        return true;
    };
    std::cout << "\nin order:";
    in_order(&tree, print_all);

    // Returning false at the first value over 3 means 5, 6, and 7 are never visited.
    auto print_up_to_3 = [](int value) {
        if (value > 3) return false;
        std::cout << " " << value;
        return true;
    };
    std::cout << "\nup to 3:";
    in_order(&tree, print_up_to_3);
    std::cout << "\n";
}
