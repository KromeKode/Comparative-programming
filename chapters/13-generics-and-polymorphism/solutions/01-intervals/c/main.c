#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 512
#define MAX_WORD 64
#define MAX_INTERVALS 128

struct version {
    long major, minor, patch;
};

// C has no generics, so one union holds a value of any kind, and each kind's
// functions know which member to use.
union value {
    long long number;
    char word[MAX_WORD];
    struct version version;
};

struct interval {
    union value low, high;
};

// A kind is a table of functions. It plays the part of a type parameter's
// constraint: everything merge needs to know about the values.
struct kind {
    const char *name;
    bool (*parse)(const char *text, union value *out);
    int (*compare)(const union value *a, const union value *b);
    void (*print)(const union value *value);
};

static bool is_digits(const char *text) {
    if (*text == '\0') return false;
    for (; *text; text++) {
        if (*text < '0' || *text > '9') return false;
    }
    return true;
}

static bool parse_int(const char *text, union value *out) {
    if (!is_digits(text[0] == '-' ? text + 1 : text)) return false;
    errno = 0;
    out->number = strtoll(text, NULL, 10);
    return errno == 0;
}

static int compare_ints(const union value *a, const union value *b) {
    return (a->number > b->number) - (a->number < b->number);
}

static void print_int(const union value *value) { printf("%lld", value->number); }

static bool parse_word(const char *text, union value *out) {
    size_t length = strlen(text);
    if (length == 0 || length >= MAX_WORD) return false;
    for (size_t i = 0; i < length; i++) {
        if (text[i] < 'a' || text[i] > 'z') return false;
    }
    memcpy(out->word, text, length + 1);
    return true;
}

static int compare_words(const union value *a, const union value *b) {
    return strcmp(a->word, b->word);
}

static void print_word(const union value *value) { printf("%s", value->word); }

static bool parse_version(const char *text, union value *out) {
    char copy[MAX_WORD];
    if (strlen(text) >= sizeof copy) return false;
    strcpy(copy, text);
    long parts[3];
    char *rest = copy;
    for (int i = 0; i < 3; i++) {
        char *dot = strchr(rest, '.');
        if ((i < 2) != (dot != NULL)) return false;  // exactly two dots
        if (dot) *dot = '\0';
        if (!is_digits(rest)) return false;
        parts[i] = strtol(rest, NULL, 10);
        if (dot) rest = dot + 1;
    }
    out->version = (struct version){parts[0], parts[1], parts[2]};
    return true;
}

static int compare_versions(const union value *a, const union value *b) {
    const struct version *x = &a->version, *y = &b->version;
    if (x->major != y->major) return x->major < y->major ? -1 : 1;
    if (x->minor != y->minor) return x->minor < y->minor ? -1 : 1;
    return (x->patch > y->patch) - (x->patch < y->patch);
}

static void print_version(const union value *value) {
    const struct version *v = &value->version;
    printf("%ld.%ld.%ld", v->major, v->minor, v->patch);
}

static const struct kind kinds[] = {
    {"int", parse_int, compare_ints, print_int},
    {"word", parse_word, compare_words, print_word},
    {"version", parse_version, compare_versions, print_version},
};

// qsort's compare function takes no extra argument, so the kind being sorted
// waits in a static variable. (glibc's qsort_r and C11's optional qsort_s
// take one, but neither is everywhere.)
static const struct kind *sorting;

static int compare_lows(const void *a, const void *b) {
    return sorting->compare(&((const struct interval *)a)->low,
                            &((const struct interval *)b)->low);
}

// Sorts by the low ends, then joins each interval to the last one if they
// overlap. Returns how many intervals are left.
static size_t merge(const struct kind *kind, struct interval *intervals, size_t count) {
    sorting = kind;
    qsort(intervals, count, sizeof intervals[0], compare_lows);
    size_t merged = 0;
    for (size_t i = 0; i < count; i++) {
        if (merged > 0 && kind->compare(&intervals[i].low, &intervals[merged - 1].high) <= 0) {
            struct interval *last = &intervals[merged - 1];
            if (kind->compare(&intervals[i].high, &last->high) > 0) last->high = intervals[i].high;
        } else {
            intervals[merged++] = intervals[i];
        }
    }
    return merged;
}

// Parses every interval on a line, merges them, and prints the result.
static void run(const struct kind *kind, char **words, size_t count) {
    static struct interval intervals[MAX_INTERVALS];
    for (size_t i = 0; i < count; i++) {
        char *word = words[i];
        char *dots = strstr(word, "..");
        if (dots == NULL || strstr(dots + 2, "..") != NULL) {
            printf("error: bad interval \"%s\"\n", word);
            return;
        }
        *dots = '\0';  // split the word in two, and put it back for messages
        char *high_text = dots + 2;
        bool low_ok = kind->parse(word, &intervals[i].low);
        bool high_ok = low_ok && kind->parse(high_text, &intervals[i].high);
        if (!low_ok || !high_ok) {
            printf("error: bad %s \"%s\"\n", kind->name, low_ok ? high_text : word);
            return;
        }
        *dots = '.';
        if (kind->compare(&intervals[i].high, &intervals[i].low) < 0) {
            printf("error: %s is backwards\n", word);
            return;
        }
    }
    size_t merged = merge(kind, intervals, count);
    printf("%s", kind->name);
    for (size_t i = 0; i < merged; i++) {
        printf(" ");
        kind->print(&intervals[i].low);
        printf("..");
        kind->print(&intervals[i].high);
    }
    printf("\n");
}

int main(void) {
    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *words[MAX_INTERVALS + 1];
        size_t count = 0;
        for (char *word = strtok(line, " \t\r\n"); word && count < MAX_INTERVALS + 1;
             word = strtok(NULL, " \t\r\n")) {
            words[count++] = word;
        }
        if (count == 0) continue;
        const struct kind *kind = NULL;
        for (size_t i = 0; i < sizeof kinds / sizeof kinds[0]; i++) {
            if (strcmp(words[0], kinds[i].name) == 0) kind = &kinds[i];
        }
        if (kind == NULL) {
            printf("error: unknown kind \"%s\"\n", words[0]);
            continue;
        }
        run(kind, words + 1, count - 1);
    }
    return 0;
}
