#include <stddef.h>
#include <stdio.h>

// Swaps two ints without a temporary. It works only while a and b point to
// different places.
static void xor_swap(int *a, int *b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

// Adds *amount to every value. If amount points into values, the amount
// changes partway through the loop.
static void add_each(int *values, size_t len, const int *amount) {
    for (size_t i = 0; i < len; i++) {
        values[i] += *amount;
    }
}

int main(void) {
    int x = 1;
    int *a = &x;
    int *b = &x; // a and b are aliases: two names for one int
    *a = 5;
    printf("after *a = 5: *b = %d\n", *b);

    int y = 3;
    xor_swap(&x, &y);
    printf("after xor_swap(&x, &y): x = %d, y = %d\n", x, y);
    xor_swap(&x, &x);
    printf("after xor_swap(&x, &x): x = %d\n", x);

    int values[3] = {1, 2, 3};
    add_each(values, 3, &values[0]);
    printf("add_each(values, 3, &values[0]): %d %d %d\n", values[0], values[1],
           values[2]);

    int fixed[3] = {1, 2, 3};
    int amount = fixed[0]; // a copy, so it cannot change under the loop
    add_each(fixed, 3, &amount);
    printf("with a copy of the amount: %d %d %d\n", fixed[0], fixed[1], fixed[2]);
    return 0;
}
