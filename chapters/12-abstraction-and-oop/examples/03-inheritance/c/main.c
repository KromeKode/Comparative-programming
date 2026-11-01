#include <stdio.h>

#define CAPACITY 16

// C has no inheritance, so a counting bag is built by composition: it holds a
// bag and calls the bag's functions.
struct bag {
    const char *items[CAPACITY];
    int size;
};

void bag_add(struct bag *bag, const char *item) {
    if (bag->size < CAPACITY) {
        bag->items[bag->size++] = item;
    }
}

void bag_add_all(struct bag *bag, const char *const *items, int count) {
    for (int i = 0; i < count; i++) {
        bag_add(bag, items[i]); // always the plain bag_add
    }
}

struct counting_bag {
    struct bag bag;
    int count;
};

void counting_bag_add(struct counting_bag *bag, const char *item) {
    bag->count++;
    bag_add(&bag->bag, item);
}

void counting_bag_add_all(struct counting_bag *bag, const char *const *items,
                          int count) {
    bag->count += count;
    bag_add_all(&bag->bag, items, count);
}

int main(void) {
    struct counting_bag bag = {0};
    const char *fruit[] = {"apple", "pear", "plum"};
    counting_bag_add_all(&bag, fruit, 3);
    counting_bag_add(&bag, "fig");
    printf("composition: the bag holds %d items, the count says %d\n",
           bag.bag.size, bag.count);
    return 0;
}
