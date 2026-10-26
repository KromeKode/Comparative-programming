#include <stdio.h>

enum { READ = 1, WRITE = 2, EXEC = 4 };  // one bit each: 001, 010, 100

// C17's printf has no binary format (C23 adds %b), so print the bits by hand.
static void print_bits(const char *label, unsigned value) {
    char text[9];
    for (int i = 0; i < 8; i++) {
        text[i] = ((value >> (7 - i)) & 1u) ? '1' : '0';
    }
    text[8] = '\0';
    printf("%s: %s\n", label, text);
}

int main(void) {
    unsigned perms = READ | WRITE;  // turn bits on with |
    print_bits("read | write", perms);
    // Test a bit with &. The parentheses matter: == binds tighter than & in C.
    printf("has write? %s\n", (perms & WRITE) != 0 ? "yes" : "no");
    printf("has exec? %s\n", (perms & EXEC) != 0 ? "yes" : "no");
    perms &= ~WRITE;  // turn a bit off: AND with every bit except WRITE
    print_bits("clear write", perms);
    perms ^= EXEC;  // flip a bit with ^
    print_bits("toggle exec", perms);
    printf("5 << 2 = %d\n", 5 << 2);      // shifting left by n multiplies by 2^n
    printf("-16 >> 2 = %d\n", -16 >> 2);  // implementation-defined for negative numbers
    // 0x16 is 10110 in binary. Binary literals such as 0b10110 arrived in C23.
    printf("set bits in 10110: %d\n", __builtin_popcount(0x16u));  // GCC and Clang
    return 0;
}
