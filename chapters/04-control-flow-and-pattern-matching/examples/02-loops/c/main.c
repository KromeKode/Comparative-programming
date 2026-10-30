#include <stdio.h>

int main(void) {
    printf("up:");
    for (int i = 0; i < 5; i++) {
        printf(" %d", i);
    }
    printf("\n");

    printf("down:");
    for (int i = 5; i >= 1; i--) {
        printf(" %d", i);
    }
    printf("\n");

    printf("by threes:");
    for (int i = 0; i < 10; i += 3) {
        printf(" %d", i);
    }
    printf("\n");

    printf("halving:");
    int n = 100;
    while (n > 0) {
        printf(" %d", n);
        n /= 2;
    }
    printf("\n");

    const char *colors[] = {"red", "green", "blue"};
    const size_t count = sizeof colors / sizeof colors[0];
    printf("colors:");
    for (size_t i = 0; i < count; i++) {
        printf(" %s", colors[i]);
    }
    printf("\n");

    printf("numbered:");
    for (size_t i = 0; i < count; i++) {
        printf(" %zu=%s", i + 1, colors[i]);
    }
    printf("\n");

    // do-while tests the condition after the body, so the body always runs once.
    int m = 100;
    do {
        m *= 2;
    } while (m < 50);
    printf("do-while: %d\n", m);
    return 0;
}
