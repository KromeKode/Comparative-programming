#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no set, so this is a small hash set of strings with open addressing. The
// strings stay in the line buffer; the set only stores pointers to them.
typedef struct {
    const char **slots;  // NULL marks an empty slot
    size_t mask;         // the table size minus 1; the size is a power of two
} StringSet;

static uint64_t hash(const char *s) {
    uint64_t h = 14695981039346656037ULL;  // FNV-1a: mix in one byte at a time
    for (; *s != '\0'; s++) {
        h = (h ^ (unsigned char)*s) * 1099511628211ULL;
    }
    return h;
}

// Adds `word` and returns 1, or returns 0 if it was already there.
static int add(StringSet *set, const char *word) {
    size_t i = (size_t)hash(word) & set->mask;
    while (set->slots[i] != NULL) {
        if (strcmp(set->slots[i], word) == 0) {
            return 0;
        }
        i = (i + 1) & set->mask;
    }
    set->slots[i] = word;
    return 1;
}

int main(void) {
    static char line[1 << 20];  // room for long lines
    while (fgets(line, sizeof line, stdin) != NULL) {
        // A line of n bytes has at most n/2 + 1 words; keep the table at most half full.
        size_t size = 16;
        while (size < strlen(line) + 2) {
            size *= 2;
        }
        StringSet seen = {calloc(size, sizeof(const char *)), size - 1};
        if (seen.slots == NULL) {
            perror("calloc");
            return 1;
        }
        int printed = 0;
        for (char *word = strtok(line, " \t\r\n"); word != NULL; word = strtok(NULL, " \t\r\n")) {
            if (add(&seen, word)) {
                printf(printed++ > 0 ? " %s" : "%s", word);
            }
        }
        if (printed > 0) {
            printf("\n");  // a blank line prints nothing
        }
        free(seen.slots);
    }
    return 0;
}
