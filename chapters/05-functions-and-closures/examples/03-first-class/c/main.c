#include <stdio.h>

// A function pointer type: "pointer to a function taking an int and returning an int".
typedef int (*IntFn)(int);

static int add_one(int x) {
    return x + 1;
}

static int triple(int x) {
    return x * 3;
}

static int square(int x) {  // C has no anonymous functions, so this needs a name too
    return x * x;
}

static int apply_twice(IntFn f, int x) {
    return f(f(x));
}

// C cannot build a new function at run time. To compose, store both pointers
// in a struct and call through it.
struct Composed {
    IntFn outer;
    IntFn inner;
};

static int call_composed(struct Composed c, int x) {
    return c.outer(c.inner(x));
}

int main(void) {
    printf("apply_twice(add_one, 5) = %d\n", apply_twice(add_one, 5));
    printf("apply_twice(triple, 5) = %d\n", apply_twice(triple, 5));
    printf("apply_twice(square, 3) = %d\n", apply_twice(square, 3));
    struct Composed add_then_triple = {triple, add_one};
    printf("compose(triple, add_one)(5) = %d\n", call_composed(add_then_triple, 5));
    return 0;
}
