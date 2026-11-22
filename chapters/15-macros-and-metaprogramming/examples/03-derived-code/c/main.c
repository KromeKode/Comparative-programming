#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// The fields are listed once. Each entry gives a type, a name, and a kind,
// and the kind picks the helper functions below that print and compare it.
#define BOOK_FIELDS(X)              \
    X(const char *, title, string)  \
    X(const char *, author, string) \
    X(int, year, int)               \
    X(bool, in_print, bool)

struct book {
#define X(type, name, kind) type name;
    BOOK_FIELDS(X)
#undef X
};

static void print_string(const char *s) { printf("\"%s\"", s); }
static void print_int(int n) { printf("%d", n); }
static void print_bool(bool b) { printf("%s", b ? "true" : "false"); }

static int compare_string(const char *a, const char *b) { return strcmp(a, b); }
static int compare_int(int a, int b) { return (a > b) - (a < b); }
static int compare_bool(bool a, bool b) { return (int)a - (int)b; }

static const char *const book_fields[] = {
#define X(type, name, kind) #name,
    BOOK_FIELDS(X)
#undef X
};

// Prints a book the way C would write it, as a compound literal.
static void print_book(const struct book *book) {
    const char *sep = "";
    printf("(struct book){");
#define X(type, name, kind)         \
    printf("%s." #name " = ", sep); \
    print_##kind(book->name);       \
    sep = ", ";
    BOOK_FIELDS(X)
#undef X
    printf("}\n");
}

// Compares field by field, in the order of the list, as a dictionary does.
static int compare_books(const struct book *a, const struct book *b) {
    int order = 0;
#define X(type, name, kind) \
    if ((order = compare_##kind(a->name, b->name)) != 0) return order;
    BOOK_FIELDS(X)
#undef X
    return 0;
}

static bool books_equal(const struct book *a, const struct book *b) {
    return compare_books(a, b) == 0;
}

int main(void) {
    printf("fields:");
    for (size_t i = 0; i < sizeof book_fields / sizeof book_fields[0]; i++) {
        printf(" %s", book_fields[i]);
    }
    printf("\n");

    struct book dune = {"Dune", "Frank Herbert", 1965, true};
    printf("dune: ");
    print_book(&dune);

    struct book copy = dune;  // assignment copies every field
    printf("copy == dune: %s\n", books_equal(&copy, &dune) ? "true" : "false");

    struct book reissue = dune;
    reissue.year = 1966;
    printf("reissue: ");
    print_book(&reissue);
    printf("reissue == dune: %s\n", books_equal(&reissue, &dune) ? "true" : "false");
    printf("dune < reissue: %s\n", compare_books(&dune, &reissue) < 0 ? "true" : "false");

    // C has no sets, so count the books that differ from every earlier one.
    struct book books[] = {dune, copy, reissue};
    int distinct = 0;
    for (size_t i = 0; i < 3; i++) {
        bool seen = false;
        for (size_t j = 0; j < i; j++) seen = seen || books_equal(&books[i], &books[j]);
        distinct += !seen;
    }
    printf("distinct books: %d\n", distinct);
    return 0;
}
