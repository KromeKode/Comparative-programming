#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// A struct groups named fields. Assigning one copies every field.
typedef struct {
    int x;
    int y;
} Point;

// C has no methods: a function that takes the struct does the same job.
static int steps_from_origin(Point p) {
    return abs(p.x) + abs(p.y);
}

// C cannot compare structs with ==, so equality is a function you write.
static bool point_equal(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

int main(void) {
    Point p = {1, 2};
    printf("p = (%d, %d)\n", p.x, p.y);
    printf("p.x + p.y = %d\n", p.x + p.y);

    // Designated initializers name the fields they set.
    Point q = {.x = 5, .y = p.y};
    printf("q = p with x = 5: q = (%d, %d), p = (%d, %d)\n", q.x, q.y, p.x, p.y);

    Point r = p; // a copy
    r.y = 9;
    printf("r = p, then r.y = 9: r = (%d, %d), p = (%d, %d)\n", r.x, r.y, p.x, p.y);

    Point a = {3, -4};
    Point b = {3, -4};
    printf("a = (%d, %d), b = (%d, %d), a == b: %s\n", a.x, a.y, b.x, b.y,
           point_equal(a, b) ? "yes" : "no");
    printf("steps from (0, 0) to a: %d\n", steps_from_origin(a));
    return 0;
}
