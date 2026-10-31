#include <array>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Each call gets its own stack frame with its own `here`.
void countdown(int n) {
    int here = n * 10;
    if (n > 0) {
        countdown(n - 1);
    }
    std::cout << " " << here;
}

int main() {
    std::cout << "each frame keeps its own local:";
    countdown(3);
    std::cout << "\n";

    std::array<int, 1000> on_stack{}; // 4000 bytes inside main's frame
    std::vector<int> on_heap(1000);   // a small header here, the items on the heap
    std::cout << "std::array of 1000 ints: " << sizeof on_stack << " bytes\n";
    std::cout << "std::vector of 1000 ints: " << sizeof on_heap << " bytes here, "
              << on_heap.size() * sizeof(int) << " on the heap\n";

    // The header holds a pointer to the items, the end of the items, and the end
    // of the space reserved for them. Adding past the end moves everything to a
    // bigger block on the heap.
    std::vector<int> growing;
    std::size_t capacity = growing.capacity();
    std::cout << "capacity as 100 items are added:";
    for (int i = 0; i < 100; i++) {
        growing.push_back(i);
        if (growing.capacity() != capacity) {
            capacity = growing.capacity();
            std::cout << " " << capacity;
        }
    }
    std::cout << "\nsizeof growing is still " << sizeof growing << "\n";

    // make_unique puts one value on the heap and frees it when `boxed` goes away
    // (chapter 10).
    auto boxed = std::make_unique<int>(42);
    std::cout << "*boxed = " << *boxed << ", sizeof boxed = " << sizeof boxed << "\n";

    std::string word = "hello";
    std::cout << "sizeof a std::string: " << sizeof word << "\n";
}
