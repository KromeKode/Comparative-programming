#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    uint32_t n;
    while (scanf("%" SCNu32, &n) == 1) {
        // Width: how many times can we shift right before nothing is left?
        int width = 0;
        for (uint32_t rest = n; rest != 0; rest >>= 1) {
            width++;
        }

        // Digits, lowest bit last. C17's printf has no %b (C23 adds it).
        char digits[33] = "0";
        for (int i = 0; i < width; i++) {
            digits[width - 1 - i] = (char)('0' + ((n >> i) & 1u));
        }
        if (width > 0) {
            digits[width] = '\0';
        }

        // Kernighan's trick: rest & (rest - 1) clears the lowest set bit.
        int ones = 0;
        for (uint32_t rest = n; rest != 0; rest &= rest - 1) {
            ones++;
        }

        printf("%" PRIu32 ": %s (ones %d, width %d)\n", n, digits, ones, width);
    }
    return 0;
}
