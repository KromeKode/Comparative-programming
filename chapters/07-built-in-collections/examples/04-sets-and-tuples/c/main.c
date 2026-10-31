#include <stdint.h>
#include <stdio.h>

// C has no set. For small whole numbers, one bit per number does the job: bit n of a
// 64-bit integer says whether n is in the set, and the bitwise operators do the rest.
typedef uint64_t Set;

static Set with(Set set, int n) { return set | (UINT64_C(1) << n); }
static int has(Set set, int n) { return (set >> n) & 1; }

static void print_set(const char *label, Set set) {
    printf("%s:", label);
    for (int n = 0; n < 64; n++) {
        if (has(set, n)) {
            printf(" %d", n);
        }
    }
    printf("\n");
}

// A struct groups two values, like a tuple. C cannot compare structs with ==.
typedef struct {
    int x, y;
} Point;

static int same_point(Point p, Point q) { return p.x == q.x && p.y == q.y; }

static int contains_point(const Point *points, size_t count, Point p) {
    for (size_t i = 0; i < count; i++) {
        if (same_point(points[i], p)) {
            return 1;
        }
    }
    return 0;
}

int main(void) {
    Set a = with(with(with(with(0, 1), 2), 3), 4);
    Set b = with(with(with(0, 3), 4), 5);
    print_set("a", a);
    print_set("b", b);
    print_set("union", a | b);
    print_set("intersection", a & b);
    print_set("a minus b", a & ~b);
    printf("3 in a: %s\n", has(a, 3) ? "yes" : "no");

    const int values[] = {3, 1, 3, 2, 1};
    Set unique = 0;
    for (size_t i = 0; i < 5; i++) {
        unique = with(unique, values[i]);
    }
    print_set("unique of 3 1 3 2 1, sorted", unique);

    const Point path[] = {{0, 0}, {1, 0}, {0, 0}, {1, 1}};
    Point visited[4];
    size_t count = 0;
    for (size_t i = 0; i < 4; i++) {
        if (!contains_point(visited, count, path[i])) {
            visited[count++] = path[i];
        }
    }
    printf("visited 4 points, %zu distinct\n", count);
    Point target = {1, 0};
    printf("(1, 0) visited: %s\n", contains_point(visited, count, target) ? "yes" : "no");
    return 0;
}
