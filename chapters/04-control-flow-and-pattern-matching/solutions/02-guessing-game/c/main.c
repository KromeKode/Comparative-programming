#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Remove leading and trailing whitespace in place, and return the new start.
static char *trim(char *text) {
    while (isspace((unsigned char)*text)) {
        text++;
    }
    char *end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) {
        end--;
    }
    *end = '\0';
    return text;
}

static const char *noun(int count) {
    return count == 1 ? "guess" : "guesses";
}

int main(void) {
    char line[256];
    if (fgets(line, sizeof line, stdin) == NULL) {
        return 1;
    }
    long secret = strtol(line, NULL, 10);

    int guesses = 0;
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *text = trim(line);
        if (*text == '\0') {
            continue;  // a blank line
        }
        char *end;
        long guess = strtol(text, &end, 10);
        if (end == text || *end != '\0') {  // no digits, or text left over
            printf("%s: not a number\n", text);
            continue;
        }
        guesses++;
        if (guess < secret) {
            printf("%ld: too low\n", guess);
        } else if (guess > secret) {
            printf("%ld: too high\n", guess);
        } else {
            printf("%ld: correct in %d %s\n", guess, guesses, noun(guesses));
            return 0;
        }
    }
    printf("gave up after %d %s; the number was %ld\n", guesses, noun(guesses), secret);
    return 0;
}
