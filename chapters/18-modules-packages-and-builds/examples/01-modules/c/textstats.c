#include "textstats.h"

#include <ctype.h>
#include <string.h>

// static makes next_word private to this file: the linker never sees its name,
// so no other file can call it, and another file can have a next_word of its own.
static const char *next_word(const char *text, size_t *length) {
    while (*text != '\0' && !isalpha((unsigned char)*text)) {
        text++;
    }
    const char *start = text;
    while (isalpha((unsigned char)*text)) {
        text++;
    }
    *length = (size_t)(text - start);
    return start;
}

size_t word_count(const char *text) {
    size_t count = 0;
    size_t length;
    for (const char *word = next_word(text, &length); length > 0;
         word = next_word(word + length, &length)) {
        count++;
    }
    return count;
}

void longest_word(const char *text, char *out, size_t size) {
    const char *best = text;
    size_t best_length = 0;
    size_t length;
    for (const char *word = next_word(text, &length); length > 0;
         word = next_word(word + length, &length)) {
        if (length > best_length) {
            best = word;
            best_length = length;
        }
    }
    if (best_length >= size) {
        best_length = size - 1;
    }
    memcpy(out, best, best_length);
    out[best_length] = '\0';
}
