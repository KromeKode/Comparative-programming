#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// C reports failure through the return value, an out parameter, or the global
// errno. strtol uses all three, so checking it takes three tests.
// Returns true and sets *out on success, or false and sets *error.
bool parse_long(const char *text, long *out, const char **error) {
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (end == text) {
        *error = "no digits";
        return false;
    }
    if (errno == ERANGE) {
        *error = "out of range";
        return false;
    }
    if (*end != '\0') {
        *error = "extra characters after the number";
        return false;
    }
    *out = value;
    return true;
}

int main(void) {
    const char *inputs[] = {"42", "4x2", "", "99999999999999999999"};
    printf("parse_long, built on strtol:\n");
    for (int i = 0; i < 4; i++) {
        long value;
        const char *error;
        if (parse_long(inputs[i], &value, &error)) {
            printf("  \"%s\" -> %ld\n", inputs[i], value);
        } else {
            printf("  \"%s\" -> error: %s\n", inputs[i], error);
        }
    }

    // atoi has no way to say it failed: it returns 0, or what it read so far.
    printf("atoi:\n");
    for (int i = 0; i < 3; i++) {
        printf("  \"%s\" -> %d\n", inputs[i], atoi(inputs[i]));
    }
    return 0;
}
