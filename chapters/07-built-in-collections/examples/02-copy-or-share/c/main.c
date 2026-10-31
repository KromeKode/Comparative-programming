#include <stdio.h>
#include <string.h>

static void print_items(const int *items, size_t count) {
    printf("[");
    for (size_t i = 0; i < count; i++) {
        printf(i > 0 ? ", %d" : "%d", items[i]);
    }
    printf("]");
}

// An array parameter is really a pointer, so the function changes the caller's array.
static void set_first(int items[]) { items[0] = -1; }

int main(void) {
    int a[] = {1, 2, 3};

    // An array cannot be assigned, so "b = a" in C means a pointer to the same items.
    int *b = a;
    b[0] = 99;
    printf("b = a, then b[0] = 99: a is ");
    print_items(a, 3);
    printf("\n");

    int c[3];
    memcpy(c, a, sizeof a);  // copies the bytes of all three items
    c[0] = 0;
    printf("c = copy of a, then c[0] = 0: a is ");
    print_items(a, 3);
    printf("\n");

    set_first(a);
    printf("set_first(a) sets a[0] = -1: a is ");
    print_items(a, 3);
    printf("\n");

    // A 2D array is one block of memory, so memcpy copies the rows too.
    int grid[2][2] = {{0, 0}, {0, 0}};
    int g[2][2];
    memcpy(g, grid, sizeof grid);
    g[0][0] = 1;
    printf("g = copy of grid, then g[0][0] = 1: grid is [");
    for (size_t row = 0; row < 2; row++) {
        printf(row > 0 ? ", " : "");
        print_items(grid[row], 2);
    }
    printf("]\n");
    return 0;
}
