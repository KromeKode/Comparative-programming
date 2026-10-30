#include <stdio.h>
#include <string.h>

int main(void) {
    const char *fruits[] = {"apple", "banana", "cherry"};
    const size_t count = sizeof fruits / sizeof fruits[0];

    // C has no growable string. Build into a fixed buffer, and keep track of the end.
    char joined[64] = "";
    for (size_t i = 0; i < count; i++) {
        if (i > 0) {
            strncat(joined, ", ", sizeof joined - strlen(joined) - 1);
        }
        strncat(joined, fruits[i], sizeof joined - strlen(joined) - 1);
    }
    printf("joined: %s\n", joined);

    // snprintf returns how many characters it wrote, so `used` marks the end.
    char built[64];
    int used = 0;
    for (int n = 1; n <= 5; n++) {
        used += snprintf(built + used, sizeof built - (size_t)used, n > 1 ? "-%d" : "%d", n);
    }
    printf("built: %s\n", built);

    printf("padded: [%-8s] [%8d]\n", "Ada", 90);  // - means left-align
    printf("zero-padded: %05d\n", 42);
    printf("two decimals: %.2f\n", 3.14159);
    return 0;
}
