#include <stdio.h>

int main(void) {
    int a, b;
    while (scanf("%d %d", &a, &b) == 2) {
        // C's / and % truncate toward zero.
        int q = a / b;
        int r = a % b;
        // Floored division rounds down instead. The answers differ by one step when
        // the remainder is not zero and its sign differs from the divisor's.
        int fq = q;
        int fm = r;
        if (r != 0 && (r < 0) != (b < 0)) {
            fq -= 1;
            fm += b;
        }
        printf("%d %d -> truncated %d %d, floored %d %d\n", a, b, q, r, fq, fm);
    }
    return 0;
}
