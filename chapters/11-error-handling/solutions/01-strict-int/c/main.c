#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum parse_error {
    PARSE_OK,
    EMPTY,
    NO_DIGITS,
    INVALID_CHARACTER,
    TOO_LARGE,
    TOO_SMALL,
};

// What parse_int found: a value, or an error and where it happened.
struct parse_result {
    enum parse_error error;
    int32_t value;
    char character; // for INVALID_CHARACTER
    int position;   // for INVALID_CHARACTER, counting from 1
};

struct parse_result parse_int(const char *text) {
    struct parse_result result = {PARSE_OK, 0, 0, 0};
    size_t length = strlen(text);
    if (length == 0) {
        result.error = EMPTY;
        return result;
    }
    int negative = text[0] == '-';
    size_t start = (text[0] == '+' || text[0] == '-') ? 1 : 0;
    if (start == length) {
        result.error = NO_DIGITS;
        return result;
    }
    // Check every character first, so a bad one is reported even when the
    // digits before it are already too large.
    for (size_t i = start; i < length; i++) {
        if (text[i] < '0' || text[i] > '9') {
            result.error = INVALID_CHARACTER;
            result.character = text[i];
            result.position = (int)i + 1;
            return result;
        }
    }
    // Stop growing once the number is past every limit, so it cannot
    // overflow however many digits there are.
    int64_t magnitude = 0;
    for (size_t i = start; i < length; i++) {
        magnitude = magnitude * 10 + (text[i] - '0');
        if (magnitude > (int64_t)INT32_MAX + 1) {
            magnitude = (int64_t)INT32_MAX + 2;
        }
    }
    if (!negative && magnitude > INT32_MAX) {
        result.error = TOO_LARGE;
    } else if (negative && magnitude > (int64_t)INT32_MAX + 1) {
        result.error = TOO_SMALL;
    } else {
        result.value = (int32_t)(negative ? -magnitude : magnitude);
    }
    return result;
}

void print_result(const char *text, struct parse_result result) {
    printf("\"%s\" -> ", text);
    switch (result.error) {
    case PARSE_OK:
        printf("%d\n", (int)result.value);
        break;
    case EMPTY:
        printf("error: empty input\n");
        break;
    case NO_DIGITS:
        printf("error: no digits after the sign\n");
        break;
    case INVALID_CHARACTER:
        printf("error: invalid character '%c' at position %d\n",
               result.character, result.position);
        break;
    case TOO_LARGE:
        printf("error: too large, the maximum is %d\n", (int)INT32_MAX);
        break;
    case TOO_SMALL:
        printf("error: too small, the minimum is %d\n", (int)INT32_MIN);
        break;
    }
}

int main(void) {
    char line[256];
    while (fgets(line, sizeof line, stdin) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        print_result(line, parse_int(line));
    }
    return 0;
}
