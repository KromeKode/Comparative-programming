#include <stdint.h>
#include <stdio.h>

int main(void) {
    long long x;
    while (scanf("%lld", &x) == 1) {
        uint8_t u8 = (uint8_t)x;             // keeps the lowest 8 bits (defined for unsigned)
        int i8 = u8 >= 128 ? u8 - 256 : u8;  // read those same bits as a signed number
        printf("%lld -> u8 %d, i8 %d\n", x, u8, i8);
    }
    return 0;
}
