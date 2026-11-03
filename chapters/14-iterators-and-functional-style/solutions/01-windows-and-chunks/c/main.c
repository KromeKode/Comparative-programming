#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 4096
#define MAX_ITEMS (MAX_LINE / 2)  // a line can't hold more numbers than this

// The source: pulls the next number out of the text, one at a time.
struct numbers {
    const char *text;
};

static bool numbers_next(struct numbers *it, long long *out) {
    while (isspace((unsigned char)*it->text)) it->text++;
    if (*it->text == '\0') return false;
    char *end = NULL;
    *out = strtoll(it->text, &end, 10);
    it->text = end;
    return true;
}

// A group points into its adapter's buffer, so it is good until the next call.
struct group {
    const long long *items;
    size_t len;
};

// Every adapter has a next function of this shape, so show can print any of them.
typedef bool (*next_group_fn)(void *self, struct group *out);

struct windows {
    struct numbers numbers;
    size_t k, len;
    long long items[MAX_ITEMS];
};

static bool windows_next(void *self, struct group *out) {
    struct windows *it = self;
    if (it->len == it->k) {  // the oldest item leaves before the next one comes in
        memmove(it->items, it->items + 1, (it->k - 1) * sizeof it->items[0]);
        it->len--;
    }
    long long item = 0;
    while (it->len < it->k) {
        if (!numbers_next(&it->numbers, &item)) return false;
        it->items[it->len++] = item;
    }
    *out = (struct group){it->items, it->len};
    return true;
}

struct chunks {
    struct numbers numbers;
    size_t k;
    long long items[MAX_ITEMS];
};

static bool chunks_next(void *self, struct group *out) {
    struct chunks *it = self;
    size_t len = 0;
    long long item = 0;
    while (len < it->k && numbers_next(&it->numbers, &item)) it->items[len++] = item;
    *out = (struct group){it->items, len};
    return len > 0;
}

struct runs {
    struct numbers numbers;
    long long items[MAX_ITEMS];
};

static bool runs_next(void *self, struct group *out) {
    struct runs *it = self;
    long long first = 0, item = 0;
    if (!numbers_next(&it->numbers, &first)) return false;
    size_t len = 0;
    it->items[len++] = first;
    for (;;) {
        // The source is a plain value, so a copy of it can look ahead
        // without moving the real one.
        struct numbers ahead = it->numbers;
        if (!numbers_next(&ahead, &item) || item != first) break;
        it->numbers = ahead;
        it->items[len++] = item;
    }
    *out = (struct group){it->items, len};
    return true;
}

static void show(next_group_fn next, void *self) {
    struct group group;
    const char *sep = "";
    while (next(self, &group)) {
        printf("%s[", sep);
        for (size_t i = 0; i < group.len; i++) printf(i ? " %lld" : "%lld", group.items[i]);
        printf("]");
        sep = " ";
    }
    if (*sep == '\0') printf("(none)");
    printf("\n");
}

static void show_sums(struct windows *windows) {
    struct group group;
    const char *sep = "";
    while (windows_next(windows, &group)) {
        long long sum = 0;
        for (size_t i = 0; i < group.len; i++) sum += group.items[i];
        printf("%s%lld", sep, sum);
        sep = " ";
    }
    if (*sep == '\0') printf("(none)");
    printf("\n");
}

// The adapters are large, so they live here rather than on the stack.
static struct windows windows;
static struct chunks chunks;
static struct runs runs;

static void answer(const char *line) {
    char command[16];
    int used = 0;
    if (sscanf(line, " %15s%n", command, &used) != 1) return;  // a blank line
    struct numbers rest = {line + used};
    if (strcmp(command, "runs") == 0) {
        runs = (struct runs){.numbers = rest};
        show(runs_next, &runs);
        return;
    }
    long long k = 0;
    numbers_next(&rest, &k);
    if (strcmp(command, "chunks") == 0) {
        chunks = (struct chunks){.numbers = rest, .k = (size_t)k};
        show(chunks_next, &chunks);
        return;
    }
    windows = (struct windows){.numbers = rest, .k = (size_t)k};
    if (strcmp(command, "windows") == 0) {
        show(windows_next, &windows);
    } else {
        show_sums(&windows);
    }
}

int main(void) {
    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin) != NULL) answer(line);
    return 0;
}
