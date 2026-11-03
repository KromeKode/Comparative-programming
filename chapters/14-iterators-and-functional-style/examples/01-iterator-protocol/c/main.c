#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// A ring keeps the last CAPACITY items pushed into it, oldest first.
#define CAPACITY 3

struct ring {
    int items[CAPACITY];
    size_t start, len;
};

static void ring_push(struct ring *ring, int item) {
    ring->items[(ring->start + ring->len) % CAPACITY] = item;
    if (ring->len < CAPACITY) {
        ring->len++;
    } else {
        ring->start = (ring->start + 1) % CAPACITY;  // the oldest item is overwritten
    }
}

// C has no iterator protocol, so this is the usual shape: a struct that
// remembers the position, and a function that hands out the next item.
struct ring_iter {
    const struct ring *ring;
    size_t index;
};

static struct ring_iter ring_iter(const struct ring *ring) {
    return (struct ring_iter){ring, 0};
}

// Stores the next item in *out and returns true, or returns false at the end.
static bool ring_next(struct ring_iter *it, int *out) {
    if (it->index == it->ring->len) return false;
    *out = it->ring->items[(it->ring->start + it->index) % CAPACITY];
    it->index++;
    return true;
}

static void print_all(const char *label, struct ring_iter it) {
    printf("%s:", label);
    int item = 0;
    while (ring_next(&it, &item)) printf(" %d", item);
    printf("\n");
}

int main(void) {
    struct ring ring = {0};
    for (int i = 1; i <= 5; i++) ring_push(&ring, i);

    print_all("loop", ring_iter(&ring));
    print_all("again", ring_iter(&ring));  // a new iterator starts at the beginning

    struct ring_iter it = ring_iter(&ring);
    int item = 0;
    printf("by hand:");
    for (const char *sep = " "; ring_next(&it, &item); sep = ", then ") {
        printf("%s%d", sep, item);
    }
    printf(", then nothing\n");

    it = ring_iter(&ring);
    ring_next(&it, &item);
    printf("resumed: took %d, then the loop got", item);
    while (ring_next(&it, &item)) printf(" %d", item);
    printf("\n");
    return 0;
}
