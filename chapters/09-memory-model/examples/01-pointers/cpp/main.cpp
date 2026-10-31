#include <array>
#include <cstddef>
#include <iostream>

int main() {
    int x = 5;
    int *p = &x; // a pointer, exactly as in C
    *p = 7;
    std::cout << "after *p = 7: x = " << x << "\n";

    // A reference is another name for x. It must be set when it is made,
    // and it can never be made to refer to anything else.
    int &r = x;
    r = 9;
    std::cout << "after r = 9: x = " << x << "\n";

    int y = 1;
    r = y; // copies y's value into x; r still names x
    y = 2;
    std::cout << "after r = y, then y = 2: x = " << x << ", y = " << y << "\n";

    // std::array is a C array that does not turn into a pointer.
    std::array<int, 4> scores = {90, 85, 77, 68};
    int *q = scores.data();
    std::cout << "q[2] = " << q[2] << ", *(q + 2) = " << *(q + 2) << "\n";

    std::cout << "sizeof: x " << sizeof x << ", p " << sizeof p << ", r " << sizeof r
              << ", scores " << sizeof scores << "\n";

    // nullptr has its own type, so it cannot be mistaken for the number 0.
    int *nowhere = nullptr;
    std::cout << "nowhere " << (nowhere == nullptr ? "is nullptr" : "points somewhere")
              << "\n";
}
