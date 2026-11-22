#include <stdio.h>
#include <stdlib.h>

#include "textstats.h"

int main(void) {
    // Read all of stdin into one string.
    size_t size = 0, capacity = 256;
    char *text = malloc(capacity);
    if (text == NULL) {
        return 1;
    }
    size_t got;
    while ((got = fread(text + size, 1, capacity - size - 1, stdin)) > 0) {
        size += got;
        if (capacity - size == 1) {
            char *bigger = realloc(text, capacity * 2);
            if (bigger == NULL) {
                free(text);
                return 1;
            }
            text = bigger;
            capacity *= 2;
        }
    }
    text[size] = '\0';

    char longest[64];
    longest_word(text, longest, sizeof longest);
    printf("textstats %s\n", TEXTSTATS_VERSION);
    printf("words: %zu\n", word_count(text));
    printf("longest: %s\n", longest);
    free(text);
    return 0;
}
