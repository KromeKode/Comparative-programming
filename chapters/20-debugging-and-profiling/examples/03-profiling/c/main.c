// Counts the words in its input, and prints the ten most common. It is slow on a long
// text, and section 20.7 runs it under a profiler to find out why.
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct count {
    char *word;
    int n;
};

struct counts {
    struct count *items;
    size_t length, capacity;
};

// Gives the index of the word's count, or -1 if it has none.
static long find(const struct counts *counts, const char *word) {
    for (size_t i = 0; i < counts->length; i++) {
        if (strcmp(counts->items[i].word, word) == 0) return (long)i;
    }
    return -1;
}

static char *copy(const char *s) {
    char *result = malloc(strlen(s) + 1);
    if (!result) exit(1);
    return strcpy(result, s);
}

static void add(struct counts *counts, const char *word) {
    long i = find(counts, word);
    if (i >= 0) {
        counts->items[i].n++;
        return;
    }
    if (counts->length == counts->capacity) {
        counts->capacity = counts->capacity ? 2 * counts->capacity : 64;
        counts->items = realloc(counts->items, counts->capacity * sizeof *counts->items);
        if (!counts->items) exit(1);
    }
    counts->items[counts->length++] = (struct count){copy(word), 1};
}

// Counts the words of the input: runs of the letters a to z, in lowercase, keeping
// the first 255 letters of a longer one.
static void count_words(FILE *in, struct counts *counts) {
    char word[256];
    size_t length = 0;
    int c;
    do {
        c = tolower(getc(in));
        if (c >= 'a' && c <= 'z') {
            if (length < sizeof word - 1) word[length++] = (char)c;
        } else if (length > 0) {
            word[length] = '\0';
            add(counts, word);
            length = 0;
        }
    } while (c != EOF);
}

static int by_count(const void *a, const void *b) {
    const struct count *x = a, *y = b;
    if (x->n != y->n) return y->n - x->n;
    return strcmp(x->word, y->word);
}

int main(void) {
    struct counts counts = {0};
    count_words(stdin, &counts);
    qsort(counts.items, counts.length, sizeof *counts.items, by_count);
    for (size_t i = 0; i < counts.length && i < 10; i++) {
        printf("%d %s\n", counts.items[i].n, counts.items[i].word);
    }
    for (size_t i = 0; i < counts.length; i++) free(counts.items[i].word);
    free(counts.items);
    return 0;
}
