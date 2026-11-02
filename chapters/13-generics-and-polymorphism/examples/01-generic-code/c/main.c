#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no generics. One way round that is a macro that writes the code for
// each type you name, which is what a C++ template does for you.
#define DEFINE_STACK(T, Name, FORMAT)                             \
    typedef struct {                                              \
        T *items;                                                 \
        size_t len, cap;                                          \
    } Name;                                                       \
                                                                  \
    static inline void Name##_push(Name *s, T item) {             \
        if (s->len == s->cap) {                                   \
            s->cap = s->cap ? s->cap * 2 : 4;                     \
            T *grown = realloc(s->items, s->cap * sizeof(T));     \
            if (!grown) abort();                                  \
            s->items = grown;                                     \
        }                                                         \
        s->items[s->len++] = item;                                \
    }                                                             \
                                                                  \
    static inline bool Name##_pop(Name *s, T *out) {              \
        if (s->len == 0) return false;                            \
        *out = s->items[--s->len];                                \
        return true;                                              \
    }                                                             \
                                                                  \
    static inline bool Name##_peek(const Name *s, T *out) {       \
        if (s->len == 0) return false;                            \
        *out = s->items[s->len - 1];                              \
        return true;                                              \
    }                                                             \
                                                                  \
    static inline void Name##_report(Name *s, const char *name) { \
        T popped; /* not "T popped, top;", see section 13.8 */    \
        T top;                                                    \
        printf("%s: popped ", name);                              \
        if (Name##_pop(s, &popped)) printf(FORMAT, popped);       \
        else printf("none");                                      \
        printf(", top is ");                                      \
        if (Name##_peek(s, &top)) printf(FORMAT, top);            \
        else printf("none");                                      \
        printf(", %zu left\n", s->len);                           \
    }                                                             \
                                                                  \
    static inline void Name##_free(Name *s) { free(s->items); }

DEFINE_STACK(int, IntStack, "%d")
DEFINE_STACK(const char *, WordStack, "%s")

// The other way is void *, as qsort does: the caller passes the size of an
// item and a function that compares two. Nothing checks that they match.
static const void *largest(const void *items, size_t count, size_t size,
                           int (*compare)(const void *, const void *)) {
    if (count == 0) return NULL;
    const char *bytes = items;
    const void *best = bytes;
    for (size_t i = 1; i < count; i++) {
        if (compare(bytes + i * size, best) > 0) best = bytes + i * size;
    }
    return best;
}

static int compare_ints(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static int compare_words(const void *a, const void *b) {
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

// Without generics, each combination of types needs its own struct.
struct int_word {
    int first;
    const char *second;
};

struct word_int {
    const char *first;
    int second;
};

static struct word_int swap_int_word(struct int_word pair) {
    return (struct word_int){pair.second, pair.first};
}

int main(void) {
    IntStack numbers = {0};
    IntStack_push(&numbers, 3);
    IntStack_push(&numbers, 1);
    IntStack_push(&numbers, 4);
    IntStack_report(&numbers, "numbers");
    IntStack_free(&numbers);

    WordStack words = {0};
    WordStack_push(&words, "pear");
    WordStack_push(&words, "fig");
    WordStack_report(&words, "words");
    WordStack_free(&words);

    IntStack empty = {0};
    IntStack_report(&empty, "empty");
    IntStack_free(&empty);

    int digits[] = {3, 1, 4, 1, 5};
    const int *most = largest(digits, 5, sizeof digits[0], compare_ints);
    printf("largest number: %d\n", *most);
    const char *fruit[] = {"pear", "fig", "apple"};
    const char *const *last = largest(fruit, 3, sizeof fruit[0], compare_words);
    printf("largest word: %s\n", *last);
    printf("largest of nothing: %s\n",
           largest(digits, 0, sizeof digits[0], compare_ints) ? "?" : "none");

    struct int_word pair = {7, "seven"};
    struct word_int swapped = swap_int_word(pair);
    printf("(%d, %s) swapped is (%s, %d)\n", pair.first, pair.second, swapped.first,
           swapped.second);
    return 0;
}
