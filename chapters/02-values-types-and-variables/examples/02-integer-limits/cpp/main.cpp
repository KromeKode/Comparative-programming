#include <cstdint>
#include <iostream>
#include <limits>

int main() {
    std::int32_t i32 = std::numeric_limits<std::int32_t>::max();
    std::cout << "int32 max: " << i32 << '\n';

    // As in C, i32 + 1 is undefined behavior. Use the compiler's checked add:
    std::int32_t sum;
    if (__builtin_add_overflow(i32, 1, &sum)) {
        std::cout << "int32 max + 1: overflow\n";
    }

    std::uint32_t u32 = std::numeric_limits<std::uint32_t>::max();
    u32 += 1;  // unsigned types wrap around
    std::cout << "uint32 max + 1: " << u32 << '\n';

    std::cout << "int64 max: " << std::numeric_limits<std::int64_t>::max() << '\n';
}
