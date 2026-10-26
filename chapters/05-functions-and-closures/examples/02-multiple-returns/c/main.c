#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// C returns one value. To give back more, the caller passes pointers to fill in.
static void min_max(const int *values, size_t count, int *lowest, int *highest) {
    *lowest = values[0];
    *highest = values[0];
    for (size_t i = 1; i < count; i++) {
        if (values[i] < *lowest) {
            *lowest = values[i];
        }
        if (values[i] > *highest) {
            *highest = values[i];
        }
    }
}

static void swap(const char **a, const char **b) {
    const char *temp = *a;
    *a = *b;
    *b = temp;
}

int main(void) {
    const int values[] = {3, -1, 4, 1, 5};
    int lowest, highest;
    min_max(values, 5, &lowest, &highest);  // & passes the address of each variable
    printf("min and max of [3, -1, 4, 1, 5]: %d %d\n", lowest, highest);

    // The other way: return a struct. The standard library's div() does this.
    div_t result = div(17, 5);
    printf("17 divided by 5: %d remainder %d\n", result.quot, result.rem);

    const char *first = "hello";
    const char *second = "world";
    swap(&first, &second);
    printf("swapped: %s %s\n", first, second);
    return 0;
}
