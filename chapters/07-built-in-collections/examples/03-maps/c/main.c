#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no map. For a handful of keys, an array of pairs searched from the start is
// enough; chapter 24 builds a hash table, which stays fast with millions of keys.
typedef struct {
    const char *key;
    int value;
} Entry;

typedef struct {
    Entry entries[16];
    size_t count;
} Map;

// Returns the position of `key` in the array, or -1 if it is not there.
static long index_of(const Map *map, const char *key) {
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            return (long)i;
        }
    }
    return -1;
}

// Returns a pointer to the value stored for `key`, or NULL if there is none.
static int *find(Map *map, const char *key) {
    long i = index_of(map, key);
    return i >= 0 ? &map->entries[i].value : NULL;
}

static void put(Map *map, const char *key, int value) {
    int *existing = find(map, key);
    if (existing != NULL) {
        *existing = value;
    } else if (map->count < sizeof map->entries / sizeof map->entries[0]) {
        map->entries[map->count++] = (Entry){key, value};
    }
}

static void remove_key(Map *map, const char *key) {
    long i = index_of(map, key);
    if (i >= 0) {
        map->entries[i] = map->entries[--map->count];  // move the last entry into the gap
    }
}

static int compare_keys(const void *a, const void *b) {
    return strcmp(((const Entry *)a)->key, ((const Entry *)b)->key);
}

// Prints the entries sorted by key, since removing one changed their order.
static void print_map(const char *label, const Map *map) {
    Map sorted = *map;
    qsort(sorted.entries, sorted.count, sizeof sorted.entries[0], compare_keys);
    printf("%s:", label);
    for (size_t i = 0; i < sorted.count; i++) {
        printf(" %s=%d", sorted.entries[i].key, sorted.entries[i].value);
    }
    printf("\n");
}

int main(void) {
    Map stock = {.count = 0};
    put(&stock, "apple", 3);
    put(&stock, "banana", 5);
    print_map("stock", &stock);

    put(&stock, "cherry", 7);
    print_map("add cherry", &stock);

    printf("banana: %d\n", *find(&stock, "banana"));
    int *durian = find(&stock, "durian");
    printf("durian: %d%s\n", durian != NULL ? *durian : 0, durian != NULL ? "" : " (missing)");

    *find(&stock, "apple") += 2;
    print_map("apple += 2", &stock);
    remove_key(&stock, "banana");
    print_map("remove banana", &stock);

    printf("has cherry: %s\n", find(&stock, "cherry") != NULL ? "yes" : "no");
    printf("size: %zu\n", stock.count);
    return 0;
}
