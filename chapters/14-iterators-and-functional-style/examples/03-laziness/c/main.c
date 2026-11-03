#include <stdbool.h>
#include <stdio.h>

// Both functions say when they run, so the output shows the order of the work.
static int square(int n) {
    printf("  square %d\n", n);
    return n * n;
}

static bool even(int x) {
    printf("  even? %d\n", x);
    return x % 2 == 0;
}

int main(void) {
    // Eager: each step finishes the whole list before the next step starts.
    printf("eager:\n");
    int squares[5], evens[5];
    int found = 0;
    for (int i = 0; i < 5; i++) squares[i] = square(i + 1);
    for (int i = 0; i < 5; i++) {
        if (even(squares[i])) evens[found++] = squares[i];
    }
    printf("  first two: %d %d\n", evens[0], evens[1]);

    // Lazy: one loop takes each number through every step, and stops as soon
    // as it has two. In C, lazy means writing the steps into one loop.
    printf("lazy:\n");
    int first_two[2];
    found = 0;
    for (int n = 1; n <= 5 && found < 2; n++) {
        int x = square(n);
        if (even(x)) first_two[found++] = x;
    }
    printf("  first two: %d %d\n", first_two[0], first_two[1]);

    // With no end to the numbers, only the lazy way can work.
    printf("endless:");
    found = 0;
    for (int n = 1; found < 3; n++) {
        int x = n * n;
        if (x % 2 == 0 && x > 50) {
            printf(" %d", x);
            found++;
        }
    }
    printf("\n");
    return 0;
}
