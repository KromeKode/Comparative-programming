// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. cJSON reads JSON into a
// tree of cJSON nodes, and this checks each node's type itself. cJSON's printer indents
// with tabs, so this writes the report itself too.
#include <cJSON.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *title;
    const char *author;
    int year;
    const cJSON *tags;  // an array of strings, or NULL for none
} Book;

// Reads a whole file into a string. Returns NULL, with errno set, if it can't.
static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) return NULL;
    char *text = NULL;
    size_t length = 0, capacity = 0;
    for (;;) {
        if (length + 1 >= capacity) {  // room for the '\0' at the end
            capacity = capacity == 0 ? 4096 : capacity * 2;
            char *bigger = realloc(text, capacity);
            if (bigger == NULL) break;
            text = bigger;
        }
        size_t got = fread(text + length, 1, capacity - length - 1, file);
        length += got;
        if (got == 0) break;
    }
    int error = ferror(file) || length + 1 >= capacity ? errno : 0;
    fclose(file);
    if (error != 0) {
        free(text);
        errno = error;
        return NULL;
    }
    text[length] = '\0';
    return text;
}

// Fills in book from a node, or returns a message saying what is wrong with it.
static const char *book_from_json(const cJSON *json, Book *book) {
    const cJSON *title = cJSON_GetObjectItemCaseSensitive(json, "title");
    const cJSON *author = cJSON_GetObjectItemCaseSensitive(json, "author");
    const cJSON *year = cJSON_GetObjectItemCaseSensitive(json, "year");
    const cJSON *tags = cJSON_GetObjectItemCaseSensitive(json, "tags");
    // A missing key gives NULL, and cJSON_IsString(NULL) is false. valueint is the
    // number cut to an int, so it differs from valuedouble for 1965.5.
    if (!cJSON_IsObject(json) || !cJSON_IsString(title) || !cJSON_IsString(author) ||
        !cJSON_IsNumber(year) || year->valuedouble != year->valueint) {
        return "each book needs a title, an author, and a year";
    }
    if (tags != NULL) {
        int ok = cJSON_IsArray(tags);
        const cJSON *tag;
        cJSON_ArrayForEach(tag, tags) ok = ok && cJSON_IsString(tag);
        if (!ok) return "tags must be a list of strings";
    }
    *book = (Book){title->valuestring, author->valuestring, year->valueint, tags};
    return NULL;
}

// Writes text as a JSON string: in quotes, with " and \ and control characters escaped,
// and everything else, UTF-8 included, as it is.
static void print_string(const char *text) {
    putchar('"');
    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
        switch (*p) {
        case '"': fputs("\\\"", stdout); break;
        case '\\': fputs("\\\\", stdout); break;
        case '\n': fputs("\\n", stdout); break;
        case '\r': fputs("\\r", stdout); break;
        case '\t': fputs("\\t", stdout); break;
        default:
            if (*p < 0x20) printf("\\u%04x", *p);
            else putchar(*p);
        }
    }
    putchar('"');
}

// Writes an array of strings, one to a line, the way JSON.stringify(value, null, 2)
// does; indent is the indent of the line the array starts on.
static void print_strings(const char **items, size_t count, const char *indent) {
    if (count == 0) {
        fputs("[]", stdout);
        return;
    }
    puts("[");
    for (size_t i = 0; i < count; i++) {
        printf("%s  ", indent);
        print_string(items[i]);
        puts(i + 1 < count ? "," : "");
    }
    printf("%s]", indent);
}

static int has_tag(const Book *book, const char *tag) {
    const cJSON *item;
    cJSON_ArrayForEach(item, book->tags) {
        if (strcmp(item->valuestring, tag) == 0) return 1;
    }
    return 0;
}

static int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static void print_report(const Book *books, size_t count) {
    // Every title and tag fits in arrays as long as the list of books and its tags.
    size_t tag_count = 0, all_tags = 0;
    for (size_t i = 0; i < count; i++) all_tags += (size_t)cJSON_GetArraySize(books[i].tags);
    const char **tags = malloc((all_tags + 1) * sizeof *tags);
    const char **titles = malloc((count + 1) * sizeof *titles);
    if (tags == NULL || titles == NULL) {
        fputs("books: out of memory\n", stderr);
        exit(1);
    }

    const Book *oldest = NULL;  // the first of the oldest, when two have the same year
    for (size_t i = 0; i < count; i++) {
        if (oldest == NULL || books[i].year < oldest->year) oldest = &books[i];
        const cJSON *tag;
        cJSON_ArrayForEach(tag, books[i].tags) {
            int seen = 0;
            for (size_t j = 0; j < tag_count && !seen; j++) {
                seen = strcmp(tags[j], tag->valuestring) == 0;
            }
            if (!seen) tags[tag_count++] = tag->valuestring;
        }
    }
    qsort(tags, tag_count, sizeof *tags, compare_strings);  // strcmp: by byte

    printf("{\n  \"count\": %zu,\n  \"oldest\": ", count);
    if (oldest == NULL) {
        fputs("null", stdout);
    } else {
        size_t size = strlen(oldest->title) + strlen(oldest->author) + sizeof ", by ";
        char *text = malloc(size);
        if (text == NULL) exit(1);
        snprintf(text, size, "%s, by %s", oldest->title, oldest->author);
        print_string(text);
        free(text);
    }
    fputs(",\n  \"tags\": ", stdout);
    if (tag_count == 0) fputs("{}", stdout);
    for (size_t t = 0; t < tag_count; t++) {
        fputs(t == 0 ? "{\n    " : ",\n    ", stdout);
        print_string(tags[t]);
        fputs(": ", stdout);
        size_t n = 0;
        for (size_t i = 0; i < count; i++) {
            if (has_tag(&books[i], tags[t])) titles[n++] = books[i].title;
        }
        print_strings(titles, n, "    ");
        if (t + 1 == tag_count) fputs("\n  }", stdout);
    }
    fputs(",\n  \"untagged\": ", stdout);
    size_t n = 0;
    for (size_t i = 0; i < count; i++) {
        if (cJSON_GetArraySize(books[i].tags) == 0) titles[n++] = books[i].title;
    }
    print_strings(titles, n, "  ");
    puts("\n}");
    free(tags);
    free(titles);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fputs("usage: books FILE\n", stderr);
        return 2;
    }
    const char *path = argv[1];
    char *text = read_file(path);
    if (text == NULL) {
        fprintf(stderr, "books: %s: %s\n", path, strerror(errno));
        return 1;
    }
    // The 1 says that nothing may follow the JSON value; cJSON_Parse(text) would
    // read "{} and more" as {}, and stop.
    cJSON *json = cJSON_ParseWithOpts(text, NULL, 1);
    free(text);
    if (json == NULL) {
        fprintf(stderr, "books: %s: not valid JSON\n", path);
        return 1;
    }
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, "books");
    size_t count = 0;
    Book *books = malloc(((size_t)cJSON_GetArraySize(list) + 1) * sizeof *books);
    const char *problem = cJSON_IsArray(list) ? NULL : "expected an object with a list of books";
    if (books == NULL) problem = "out of memory";
    const cJSON *item;
    cJSON_ArrayForEach(item, list) {  // a loop over the array's nodes, which are a list
        if (problem == NULL) problem = book_from_json(item, &books[count++]);
    }
    if (problem == NULL) print_report(books, count);
    else fprintf(stderr, "books: %s: %s\n", path, problem);
    free(books);
    cJSON_Delete(json);  // frees every node, and the strings the books point to
    return problem == NULL ? 0 : 1;
}
