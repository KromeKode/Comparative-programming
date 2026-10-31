#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_WORDS 100
#define SPACES " \t\r\n"

// Each word is a char *, so swapping two words swaps two pointers. No
// characters move, however long the words are.
static void swap(char **a, char **b) {
    char *tmp = *a;
    *a = *b;
    *b = tmp;
}

// Reverses words[from..to] and returns how many swaps that took.
static int reverse(char **words, int from, int to) {
    int swaps = 0;
    for (int i = from, j = to; i < j; i++, j--) {
        swap(&words[i], &words[j]);
        swaps++;
    }
    return swaps;
}

// Reads a 1-based position into *out, or prints why it cannot.
static bool parse_position(const char *text, int count, int *out) {
    for (const char *c = text; *c != '\0'; c++) {
        if (!isdigit((unsigned char)*c)) {
            printf("error: \"%s\" is not a position\n", text);
            return false;
        }
    }
    // strtoul sets errno to ERANGE when the number is too big for it.
    errno = 0;
    unsigned long value = strtoul(text, NULL, 10);
    if (errno == ERANGE || value < 1 || value > (unsigned long)count) {
        printf("error: position %s is out of range (1 to %d)\n", text, count);
        return false;
    }
    *out = (int)value;
    return true;
}

static void print_words(char **words, int count) {
    for (int i = 0; i < count; i++) {
        printf(i == 0 ? "%s" : " %s", words[i]);
    }
    printf("\n");
}

int main(void) {
    // The words point into this buffer, so it must outlive them. A static
    // array lives for the whole program.
    static char first[MAX_LINE];
    char *words[MAX_WORDS];
    int count = 0;
    int swaps = 0;

    if (fgets(first, sizeof first, stdin) != NULL) {
        for (char *w = strtok(first, SPACES); w != NULL && count < MAX_WORDS;
             w = strtok(NULL, SPACES)) {
            words[count++] = w;
        }
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *parts[3];
        int n = 0;
        for (char *t = strtok(line, SPACES); t != NULL; t = strtok(NULL, SPACES)) {
            if (n < 3) {
                parts[n] = t;
            }
            n++;
        }
        if (n == 0) {
            continue;
        }
        const char *command = parts[0];
        if (strcmp(command, "swap") != 0 && strcmp(command, "reverse") != 0) {
            printf("error: unknown command \"%s\"\n", command);
            continue;
        }
        if (n != 3) {
            printf("error: %s needs 2 positions, got %d\n", command, n - 1);
            continue;
        }
        int i, j;
        if (!parse_position(parts[1], count, &i) || !parse_position(parts[2], count, &j)) {
            continue;
        }
        if (strcmp(command, "swap") == 0) {
            swap(&words[i - 1], &words[j - 1]);
            swaps++;
        } else if (i <= j) {
            swaps += reverse(words, i - 1, j - 1);
        } else {
            swaps += reverse(words, j - 1, i - 1);
        }
        print_words(words, count);
    }
    printf("swaps: %d\n", swaps);
    return 0;
}
