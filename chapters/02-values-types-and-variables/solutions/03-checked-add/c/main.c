#include <limits.h>
#include <stdio.h>

int main(void) {
    long long a, b;
    while (scanf("%lld %lld", &a, &b) == 2) {
        // Check before adding: computing a + b would itself be undefined behavior.
        if ((b > 0 && a > LLONG_MAX - b) || (b < 0 && a < LLONG_MIN - b)) {
            printf("overflow\n");
        } else {
            printf("%lld\n", a + b);
        }
    }
    return 0;
}
