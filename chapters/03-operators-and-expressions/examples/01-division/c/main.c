#include <stdio.h>

int main(void) {
    // Integer division truncates: it drops the fraction, rounding toward zero.
    printf("7 / 2 = %d\n", 7 / 2);
    printf("-7 / 2 = %d\n", -7 / 2);
    // The remainder takes the sign of the left operand, so (a / b) * b + a % b == a.
    printf("7 %% 3 = %d\n", 7 % 3);  // %% prints a single % sign
    printf("-7 %% 3 = %d\n", -7 % 3);
    printf("7 %% -3 = %d\n", 7 % -3);
    // If either side is a double, the other is converted and the result is a double.
    printf("7.0 / 2 = %g\n", 7.0 / 2);
    return 0;
}
