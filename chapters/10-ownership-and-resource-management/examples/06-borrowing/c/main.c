#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Returns a or b, whichever is longer. The result points into one of them, so
// it is good only as long as both are. Only this comment says so.
const char *longest(const char *a, const char *b) {
    return strlen(b) > strlen(a) ? b : a;
}

// Walks the words of a string without copying them. It borrows the text, so
// the text must outlive it.
typedef struct {
    const char *rest;
} Words;

// Finds the next word and points *start at it. Returns false at the end.
bool words_next(Words *w, const char **start, size_t *length) {
    w->rest += strspn(w->rest, " ");
    if (*w->rest == '\0') {
        return false;
    }
    *start = w->rest;
    *length = strcspn(w->rest, " ");
    w->rest += *length;
    return true;
}

int main(void) {
    char a[] = "apple";
    char b[] = "fig";
    const char *result = longest(a, b);
    printf("longest(\"%s\", \"%s\") = %s\n", a, b, result);

    const char *text = "the quick brown fox";
    printf("words of \"%s\":", text);
    Words words = {text};
    const char *start;
    size_t length;
    while (words_next(&words, &start, &length)) {
        printf(" [%.*s]", (int)length, start);
    }
    printf("\n");

    // The words are pointers into text, not copies of it.
    Words again = {text};
    words_next(&again, &start, &length);
    if (words_next(&again, &start, &length)) {
        printf("\"%.*s\" starts %td bytes into the text\n", (int)length, start,
               start - text);
    }
    return 0;
}
