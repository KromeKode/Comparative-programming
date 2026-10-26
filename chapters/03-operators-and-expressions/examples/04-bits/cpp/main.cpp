#include <bit>
#include <bitset>
#include <iostream>

constexpr unsigned READ = 0b001;
constexpr unsigned WRITE = 0b010;
constexpr unsigned EXEC = 0b100;

int main() {
    unsigned perms = READ | WRITE;  // turn bits on with |
    std::cout << "read | write: " << std::bitset<8>(perms) << '\n';
    // Test a bit with &. The parentheses matter: == binds tighter than & in C++.
    std::cout << "has write? " << ((perms & WRITE) != 0 ? "yes" : "no") << '\n';
    std::cout << "has exec? " << ((perms & EXEC) != 0 ? "yes" : "no") << '\n';
    perms &= ~WRITE;  // turn a bit off: AND with every bit except WRITE
    std::cout << "clear write: " << std::bitset<8>(perms) << '\n';
    perms ^= EXEC;  // flip a bit with ^
    std::cout << "toggle exec: " << std::bitset<8>(perms) << '\n';
    std::cout << "5 << 2 = " << (5 << 2) << '\n';  // << also means "print", so wrap it
    std::cout << "-16 >> 2 = " << (-16 >> 2) << '\n';  // defined as arithmetic since C++20
    std::cout << "set bits in 10110: " << std::popcount(0b10110u) << '\n';
}
