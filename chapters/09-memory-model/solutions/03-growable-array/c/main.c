#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define SPACES " \t\r\n"

// The struct is a small header, like the one inside a C++ vector or a Rust
// Vec. The items themselves live in one block on the heap.
typedef struct {
    int *items;      // NULL until the first push
    size_t length;   // how many items are in use
    size_t capacity; // how many items fit before the block must grow
} IntArray;

// Makes room for one more item. Returns false if there is no memory left.
static bool reserve_one(IntArray *a) {
    if (a->length < a->capacity) {
        return true;
    }
    size_t capacity = a->capacity == 0 ? 4 : a->capacity * 2;
    // realloc moves the items to a bigger block if it has to. It returns NULL
    // on failure and leaves the old block alone, so keep the old pointer
    // until it has succeeded.
    int *items = realloc(a->items, capacity * sizeof *items);
    if (items == NULL) {
        return false;
    }
    printf("grow: %zu -> %zu\n", a->capacity, capacity);
    a->items = items;
    a->capacity = capacity;
    return true;
}

static bool push(IntArray *a, int value) {
    if (!reserve_one(a)) {
        return false;
    }
    a->items[a->length++] = value;
    return true;
}

static void print_array(const IntArray *a) {
    printf("[");
    for (size_t i = 0; i < a->length; i++) {
        printf(i == 0 ? "%d" : ", %d", a->items[i]);
    }
    printf("] (length %zu, capacity %zu)\n", a->length, a->capacity);
}

// Reads a whole token as an int, or prints why it cannot.
static bool parse_int(const char *text, int *out) {
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (end == text || *end != '\0' || errno == ERANGE || value < INT_MIN ||
        value > INT_MAX) {
        printf("error: \"%s\" is not an int\n", text);
        return false;
    }
    *out = (int)value;
    return true;
}

// Reads an index that must point at an item in use.
static bool parse_index(const char *text, const IntArray *a, size_t *out) {
    int index;
    if (!parse_int(text, &index)) {
        return false;
    }
    if (index < 0 || (size_t)index >= a->length) {
        printf("error: index %d is out of range (length %zu)\n", index, a->length);
        return false;
    }
    *out = (size_t)index;
    return true;
}

static bool takes(const char *command, int want, int got) {
    if (got != want) {
        printf("error: %s takes %d value%s, got %d\n", command, want, want == 1 ? "" : "s",
               got);
        return false;
    }
    return true;
}

int main(void) {
    IntArray a = {NULL, 0, 0};
    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *parts[3];
        int n = 0;
        for (char *t = strtok(line, SPACES); t != NULL; t = strtok(NULL, SPACES)) {
            if (n < 3) {
                parts[n] = t;
            }
            n++;
        }
        if (n == 0) {
            continue;
        }
        const char *command = parts[0];
        int values = n - 1;
        int value;
        size_t index;
        if (strcmp(command, "push") == 0) {
            if (takes(command, 1, values) && parse_int(parts[1], &value) &&
                !push(&a, value)) {
                printf("error: out of memory\n");
                free(a.items);
                return 1;
            }
        } else if (strcmp(command, "pop") == 0) {
            if (!takes(command, 0, values)) {
                continue;
            }
            if (a.length == 0) {
                printf("error: pop from an empty array\n");
            } else {
                printf("pop: %d\n", a.items[--a.length]);
            }
        } else if (strcmp(command, "get") == 0) {
            if (takes(command, 1, values) && parse_index(parts[1], &a, &index)) {
                printf("get %zu: %d\n", index, a.items[index]);
            }
        } else if (strcmp(command, "set") == 0) {
            if (takes(command, 2, values) && parse_index(parts[1], &a, &index) &&
                parse_int(parts[2], &value)) {
                a.items[index] = value;
            }
        } else if (strcmp(command, "print") == 0) {
            if (takes(command, 0, values)) {
                print_array(&a);
            }
        } else {
            printf("error: unknown command \"%s\"\n", command);
        }
    }
    free(a.items); // one block, so one free; free(NULL) does nothing
    return 0;
}
