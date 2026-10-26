#include <stdbool.h>
#include <stdio.h>

static int calls = 0;  // counts how many times the right side ran

static bool big_ratio(int x) {
    calls++;
    return 10 / x > 1;  // dividing by zero here would be undefined behavior
}

static const char *display_name(const char *name) {
    // C's || always gives 0 or 1, never one of its operands, so use ?: for defaults.
    return name[0] != '\0' ? name : "stranger";
}

int main(void) {
    int values[] = {0, 5};
    for (int i = 0; i < 2; i++) {
        int x = values[i];
        // && stops as soon as the left side is false, so big_ratio never sees 0.
        bool ok = x != 0 && big_ratio(x);
        printf("x = %d: %s\n", x, ok ? "yes" : "no");
    }
    printf("right side ran %d of 2 times\n", calls);
    printf("name \"\": %s\n", display_name(""));
    printf("name \"Ada\": %s\n", display_name("Ada"));
    return 0;
}
