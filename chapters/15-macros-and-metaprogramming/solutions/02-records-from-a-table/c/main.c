#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_COLUMNS 64

// The three kinds of field. A text is an array, so it lives in the book.
typedef char text[MAX_LINE];
typedef long long whole;
typedef bool flag;

// The fields are written once, here, as a kind and a name. Every use of the
// list below turns them into different code.
#define BOOK_FIELDS(X) \
    X(text, title)     \
    X(text, author)    \
    X(whole, year)     \
    X(whole, pages)    \
    X(flag, in_print)

struct book {
#define X(kind, name) kind name;
    BOOK_FIELDS(X)
#undef X
};

enum field {
#define X(kind, name) FIELD_##name,
    BOOK_FIELDS(X)
#undef X
    FIELD_COUNT
};

static const char *const field_names[] = {
#define X(kind, name) #name,
    BOOK_FIELDS(X)
#undef X
};

// Each parser returns NULL when the text is right, or else what is wrong.
static const char *parse_text(const char *s, text *out) {
    strcpy(*out, s);
    return NULL;
}

static const char *parse_whole(const char *s, whole *out) {
    const char *digits = s[0] == '-' ? s + 1 : s;
    if (*digits == '\0' || strspn(digits, "0123456789") != strlen(digits)) {
        return "is not a whole number";
    }
    *out = strtoll(s, NULL, 10);
    return NULL;
}

static const char *parse_flag(const char *s, flag *out) {
    if (strcmp(s, "true") != 0 && strcmp(s, "false") != 0) return "is not true or false";
    *out = strcmp(s, "true") == 0;
    return NULL;
}

static void print_text(const text *value) { printf("\"%s\"", *value); }
static void print_whole(const whole *value) { printf("%lld", *value); }
static void print_flag(const flag *value) { printf("%s", *value ? "true" : "false"); }

// A switch with a case for each field, which calls the parser for its kind.
static const char *set_field(struct book *book, enum field field, const char *s) {
    switch (field) {
#define X(kind, name)  \
    case FIELD_##name: \
        return parse_##kind(s, &book->name);
        BOOK_FIELDS(X)
#undef X
    case FIELD_COUNT:
        break;
    }
    return "is not a field";
}

static void print_book(const struct book *book) {
    printf("Book(");
#define X(kind, name)                                     \
    printf("%s" #name "=", FIELD_##name > 0 ? ", " : ""); \
    print_##kind(&book->name);
    BOOK_FIELDS(X)
#undef X
    printf(")\n");
}

// Splits line at each comma, in place, and trims spaces from each part.
static int split(char *line, char **parts) {
    int count = 0;
    for (char *start = line;; start++) {
        char *end = strchr(start, ',');
        if (end) *end = '\0';
        while (isspace((unsigned char)*start)) start++;
        char *last = start + strlen(start);
        while (last > start && isspace((unsigned char)last[-1])) *--last = '\0';
        if (count < MAX_COLUMNS) parts[count] = start;
        count++;
        if (!end) return count;
        start = end;
    }
}

static bool blank(const char *line) { return strspn(line, " \t\r\n") == strlen(line); }

static const char *plural(int count) { return count == 1 ? "" : "s"; }

// Finds the field for each column, or prints what is wrong and returns false.
static bool read_header(char **parts, int count, enum field *columns) {
    for (int i = 0; i < count; i++) {
        int found = -1;
        for (int f = 0; f < FIELD_COUNT; f++) {
            if (strcmp(parts[i], field_names[f]) == 0) found = f;
        }
        if (found < 0) {
            printf("unknown column: %s\n", parts[i]);
            return false;
        }
        for (int j = 0; j < i; j++) {
            if (columns[j] == (enum field)found) {
                printf("repeated column: %s\n", parts[i]);
                return false;
            }
        }
        columns[i] = (enum field)found;
    }
    return true;
}

int main(void) {
    char line[MAX_LINE];
    char *parts[MAX_COLUMNS];
    enum field columns[MAX_COLUMNS];
    int column_count = -1, books = 0, bad = 0, row = 0;
    while (fgets(line, sizeof line, stdin) != NULL) {
        if (blank(line)) continue;
        int count = split(line, parts);
        if (column_count < 0) {
            column_count = count;
            if (!read_header(parts, count, columns)) return 0;
            continue;
        }
        row++;
        if (count != column_count) {
            printf("row %d: expected %d values, got %d\n", row, column_count, count);
            bad++;
            continue;
        }
        struct book book = {0};  // every field starts at "", 0, or false
        const char *problem = NULL;
        int column = 0;
        for (; column < count; column++) {
            problem = set_field(&book, columns[column], parts[column]);
            if (problem) break;
        }
        if (problem) {
            printf("row %d: %s: \"%s\" %s\n", row, field_names[columns[column]], parts[column],
                   problem);
            bad++;
        } else {
            print_book(&book);
            books++;
        }
    }
    if (column_count >= 0) {
        printf("%d book%s, %d bad row%s\n", books, plural(books), bad, plural(bad));
    }
    return 0;
}
