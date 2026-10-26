#include <stdio.h>

// C has no closures. A closure is a function plus the variables it captured,
// so in C you keep those variables in a struct and pass it in yourself.
struct Adder {
    int n;
};

static int add(struct Adder adder, int x) {
    return x + adder.n;
}

struct Accumulator {
    int total;
};

static int accumulate(struct Accumulator *acc, int x) {
    acc->total += x;  // a pointer, so the change is kept between calls
    return acc->total;
}

int main(void) {
    struct Adder add5 = {5};
    struct Adder add10 = {10};
    printf("add5(10) = %d\n", add(add5, 10));
    printf("add10(10) = %d\n", add(add10, 10));

    struct Accumulator running = {0};
    // One call per statement: C does not fix the order it evaluates arguments in,
    // so printf("%d %d", accumulate(...), accumulate(...)) could run either first.
    printf("running total:");
    printf(" %d", accumulate(&running, 3));
    printf(" %d", accumulate(&running, 5));
    printf(" %d\n", accumulate(&running, 2));

    struct Adder made[3];
    for (int i = 0; i < 3; i++) {
        made[i].n = i;  // each struct stores its own copy of i
    }
    printf("made in a loop:");
    for (int i = 0; i < 3; i++) {
        printf(" %d", add(made[i], 0));
    }
    printf("\n");
    return 0;
}
