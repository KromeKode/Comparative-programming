#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no growable array, so here is a small one: a pointer to the items, how many
// there are, and how many fit before the memory has to grow.
typedef struct {
    int *items;
    size_t length;
    size_t capacity;
} IntList;

static void reserve(IntList *list, size_t needed) {
    if (needed <= list->capacity) {
        return;
    }
    size_t capacity = list->capacity == 0 ? 4 : list->capacity * 2;  // double it
    while (capacity < needed) {
        capacity *= 2;
    }
    int *items = realloc(list->items, capacity * sizeof *items);
    if (items == NULL) {
        perror("realloc");
        exit(1);
    }
    list->items = items;
    list->capacity = capacity;
}

static void append(IntList *list, int value) {
    reserve(list, list->length + 1);
    list->items[list->length++] = value;
}

static void insert_at(IntList *list, size_t index, int value) {
    reserve(list, list->length + 1);
    // Move everything from index onward one place to the right.
    memmove(&list->items[index + 1], &list->items[index],
            (list->length - index) * sizeof *list->items);
    list->items[index] = value;
    list->length++;
}

static int remove_at(IntList *list, size_t index) {
    int removed = list->items[index];
    memmove(&list->items[index], &list->items[index + 1],
            (list->length - index - 1) * sizeof *list->items);
    list->length--;
    return removed;
}

// Searching is a loop. Returns the index of the first `value`, or -1 if it is missing.
static long index_of(const IntList *list, int value) {
    for (size_t i = 0; i < list->length; i++) {
        if (list->items[i] == value) {
            return (long)i;
        }
    }
    return -1;
}

static void print_items(const char *label, const int *items, size_t count) {
    printf("%s: [", label);
    for (size_t i = 0; i < count; i++) {
        printf(i > 0 ? ", %d" : "%d", items[i]);
    }
    printf("]\n");
}

static int compare_ints(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void) {
    IntList numbers = {NULL, 0, 0};
    const int start[] = {3, 1, 4, 1, 5};
    for (size_t i = 0; i < 5; i++) {
        append(&numbers, start[i]);
    }
    print_items("numbers", numbers.items, numbers.length);
    printf("length: %zu\n", numbers.length);
    printf("first: %d, last: %d\n", numbers.items[0], numbers.items[numbers.length - 1]);

    append(&numbers, 9);
    print_items("append 9", numbers.items, numbers.length);
    insert_at(&numbers, 1, 2);
    print_items("insert 2 at index 1", numbers.items, numbers.length);
    int removed = remove_at(&numbers, 0);
    char label[64];
    snprintf(label, sizeof label, "remove index 0 (was %d)", removed);
    print_items(label, numbers.items, numbers.length);

    printf("contains 4: %s\n", index_of(&numbers, 4) >= 0 ? "yes" : "no");
    printf("index of 1: %ld\n", index_of(&numbers, 1));

    qsort(numbers.items, numbers.length, sizeof *numbers.items, compare_ints);
    print_items("sorted", numbers.items, numbers.length);
    print_items("slice 1..4", numbers.items + 1, 3);  // a pointer and a count, no copy

    for (size_t i = 0, j = numbers.length - 1; i < j; i++, j--) {
        int swap = numbers.items[i];
        numbers.items[i] = numbers.items[j];
        numbers.items[j] = swap;
    }
    print_items("reversed", numbers.items, numbers.length);

    free(numbers.items);
    return 0;
}
