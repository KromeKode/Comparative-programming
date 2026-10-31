#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Entry {
    char *word;
    int count;
};

static struct Entry *entries = NULL;
static size_t entry_count = 0;
static size_t entry_capacity = 0;

// A word character is an ASCII letter or any byte of a non-ASCII character (0x80 and up).
static int is_word_byte(unsigned char b) {
    return (b >= 'a' && b <= 'z') || (b >= 'A' && b <= 'Z') || b >= 0x80;
}

// A linear search keeps this short. Chapter 24 builds a hash table that scales.
static void add_word(const char *word, size_t length) {
    for (size_t i = 0; i < entry_count; i++) {
        if (strlen(entries[i].word) == length && memcmp(entries[i].word, word, length) == 0) {
            entries[i].count++;
            return;
        }
    }
    if (entry_count == entry_capacity) {
        entry_capacity = entry_capacity == 0 ? 16 : entry_capacity * 2;
        entries = realloc(entries, entry_capacity * sizeof *entries);
        if (entries == NULL) {
            exit(1);
        }
    }
    char *copy = malloc(length + 1);
    if (copy == NULL) {
        exit(1);
    }
    memcpy(copy, word, length);
    copy[length] = '\0';
    entries[entry_count++] = (struct Entry){copy, 1};
}

// Most frequent first. strcmp compares unsigned bytes, and UTF-8 byte order is the
// same as code point order, so ties come out in code point order.
static int compare_entries(const void *a, const void *b) {
    const struct Entry *x = a;
    const struct Entry *y = b;
    if (x->count != y->count) {
        return y->count - x->count;
    }
    return strcmp(x->word, y->word);
}

int main(void) {
    char word[256];
    size_t length = 0;
    int c;
    // Read one byte at a time, and treat the end of input like a separator.
    do {
        c = getchar();
        if (c != EOF && is_word_byte((unsigned char)c)) {
            if (length < sizeof word) {
                word[length++] = (char)(c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c);
            }
        } else if (length > 0) {
            add_word(word, length);
            length = 0;
        }
    } while (c != EOF);

    qsort(entries, entry_count, sizeof *entries, compare_entries);
    for (size_t i = 0; i < entry_count; i++) {
        printf("%d %s\n", entries[i].count, entries[i].word);
        free(entries[i].word);
    }
    free(entries);
    return 0;
}
