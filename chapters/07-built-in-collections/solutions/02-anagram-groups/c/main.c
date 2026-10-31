#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD 64

// A word, its key (its letters sorted), and where it appeared in the input.
typedef struct {
    char word[MAX_WORD];
    char key[MAX_WORD];
    size_t position;
} Word;

static int compare_chars(const void *a, const void *b) {
    return *(const char *)a - *(const char *)b;
}

// Sorting by key brings each group together; ties keep the input order.
static int by_key_then_position(const void *a, const void *b) {
    const Word *x = a, *y = b;
    int c = strcmp(x->key, y->key);
    return c != 0 ? c : (x->position > y->position) - (x->position < y->position);
}

// A group is a run of words in the sorted array.
typedef struct {
    size_t start, end, first_position;
} Group;

static int by_first_position(const void *a, const void *b) {
    const Group *x = a, *y = b;
    return (x->first_position > y->first_position) - (x->first_position < y->first_position);
}

int main(void) {
    Word *words = NULL;
    size_t count = 0, capacity = 0;
    char buffer[MAX_WORD];
    while (scanf("%63s", buffer) == 1) {  // reads one whitespace-separated word
        if (count == capacity) {
            capacity = capacity == 0 ? 64 : capacity * 2;
            words = realloc(words, capacity * sizeof *words);
            if (words == NULL) {
                perror("realloc");
                return 1;
            }
        }
        Word *w = &words[count];
        size_t length = strlen(buffer);
        for (size_t i = 0; i <= length; i++) {  // <= copies the '\0' too
            w->word[i] = (char)tolower((unsigned char)buffer[i]);
        }
        strcpy(w->key, w->word);
        qsort(w->key, strlen(w->key), 1, compare_chars);
        w->position = count++;
    }

    // C has no map, so group by sorting: equal keys end up next to each other.
    qsort(words, count, sizeof *words, by_key_then_position);
    Group *groups = malloc((count + 1) * sizeof *groups);
    if (groups == NULL) {
        perror("malloc");
        return 1;
    }
    size_t group_count = 0;
    for (size_t i = 0; i < count; i++) {
        if (i == 0 || strcmp(words[i].key, words[i - 1].key) != 0) {
            groups[group_count++] = (Group){i, i, words[i].position};
        }
        groups[group_count - 1].end = i + 1;
    }
    // Then put the groups back in the order their first words appeared.
    qsort(groups, group_count, sizeof *groups, by_first_position);

    for (size_t g = 0; g < group_count; g++) {
        for (size_t i = groups[g].start; i < groups[g].end; i++) {
            printf(i > groups[g].start ? " %s" : "%s", words[i].word);
        }
        printf("\n");
    }
    free(groups);
    free(words);
    return 0;
}
