#include <stdio.h>

// C has no pattern matching: test each shape with if, most specific first.
static void classify(int x, int y) {
    printf("(%d, %d): ", x, y);
    if (x == 0 && y == 0) {
        printf("origin\n");
    } else if (y == 0) {
        printf("on the x axis at x = %d\n", x);
    } else if (x == 0) {
        printf("on the y axis at y = %d\n", y);
    } else if (x == y) {
        printf("on the diagonal at %d\n", x);
    } else {
        printf("x = %d, y = %d\n", x, y);
    }
}

int main(void) {
    const int points[][2] = {{0, 0}, {3, 0}, {0, -2}, {4, 4}, {-1, 5}};
    for (size_t i = 0; i < sizeof points / sizeof points[0]; i++) {
        classify(points[i][0], points[i][1]);
    }
    return 0;
}
