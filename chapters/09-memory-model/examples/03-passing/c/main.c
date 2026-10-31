#include <stddef.h>
#include <stdio.h>

typedef struct {
    int x;
    int y;
} Point;

// C passes every argument by value: the function gets its own copy.
static void bump_copy(int n) {
    n++;
    printf("inside bump_copy: n = %d\n", n);
}

// To change the caller's variable, pass its address.
static void bump(int *n) {
    (*n)++;
}

// An array parameter is really a pointer to the first item, so the function
// changes the caller's array. It cannot tell how long the array is, either.
static void double_all(int values[], size_t len) {
    for (size_t i = 0; i < len; i++) {
        values[i] *= 2;
    }
}

static void shift_copy(Point p) {
    p.x += 10;
    printf("inside shift_copy: p = (%d, %d)\n", p.x, p.y);
}

static void shift(Point *p) {
    p->x += 10; // p->x means (*p).x
}

// A pointer to const reads the caller's struct without copying it, and the
// compiler refuses any write through it.
static int sum(const Point *p) {
    return p->x + p->y;
}

int main(void) {
    int n = 1;
    bump_copy(n);
    printf("after bump_copy(n): n = %d\n", n);
    bump(&n);
    printf("after bump(&n): n = %d\n", n);

    int values[3] = {1, 2, 3};
    double_all(values, 3);
    printf("after double_all(values, 3): %d %d %d\n", values[0], values[1], values[2]);

    Point p = {1, 2};
    shift_copy(p);
    printf("after shift_copy(p): p = (%d, %d)\n", p.x, p.y);
    shift(&p);
    printf("after shift(&p): p = (%d, %d)\n", p.x, p.y);
    printf("sum(&p) = %d\n", sum(&p));
    return 0;
}
