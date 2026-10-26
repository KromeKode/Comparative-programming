#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

// The memo lives in a static array: zeroed at start, and kept between calls.
// fib(n) is never 0 for n >= 1, so 0 can mean "not worked out yet".
static uint64_t memo[93];

static uint64_t fib(int n) {
    if (n < 2) {
        return (uint64_t)n;
    }
    if (memo[n] == 0) {
        memo[n] = fib(n - 1) + fib(n - 2);
    }
    return memo[n];
}

int main(void) {
    int n;
    while (scanf("%d", &n) == 1) {
        printf("fib(%d) = %" PRIu64 "\n", n, fib(n));
    }
    return 0;
}
