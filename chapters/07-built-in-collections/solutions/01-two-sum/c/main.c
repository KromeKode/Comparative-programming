#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no map, so this is a small hash table from a number to the first index where
// it appeared. It uses open addressing: if a slot is taken, try the next one.
typedef struct {
    long long key;
    long index;  // -1 marks an empty slot
} Slot;

typedef struct {
    Slot *slots;
    size_t mask;  // the table size minus 1; the size is a power of two
} Table;

static Table make_table(size_t count) {
    size_t size = 16;
    while (size < count * 2) {
        size *= 2;  // keep the table at most half full
    }
    Table table = {malloc(size * sizeof(Slot)), size - 1};
    if (table.slots == NULL) {
        perror("malloc");
        exit(1);
    }
    for (size_t i = 0; i < size; i++) {
        table.slots[i].index = -1;
    }
    return table;
}

// Returns the slot holding `key`, or the empty slot where it belongs.
static Slot *find_slot(const Table *table, long long key) {
    // Multiplying by a large odd constant spreads nearby numbers across the table.
    unsigned long long hash = (unsigned long long)key * 0x9E3779B97F4A7C15ULL;
    size_t i = (size_t)(hash >> 32) & table->mask;
    while (table->slots[i].index != -1 && table->slots[i].key != key) {
        i = (i + 1) & table->mask;
    }
    return &table->slots[i];
}

int main(void) {
    static char line[1 << 20];  // room for long lines
    long long *numbers = NULL;
    size_t capacity = 0;
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *end;
        long long target = strtoll(line, &end, 10);
        if (end == line || *end != ':') {
            continue;  // a blank line
        }
        size_t count = 0;
        for (char *p = end + 1;; p = end) {
            long long n = strtoll(p, &end, 10);
            if (end == p) {
                break;  // no more numbers on this line
            }
            if (count == capacity) {
                capacity = capacity == 0 ? 64 : capacity * 2;
                numbers = realloc(numbers, capacity * sizeof *numbers);
                if (numbers == NULL) {
                    perror("realloc");
                    return 1;
                }
            }
            numbers[count++] = n;
        }

        Table seen = make_table(count);
        int found = 0;
        for (size_t j = 0; j < count && !found; j++) {
            Slot *partner = find_slot(&seen, target - numbers[j]);
            if (partner->index != -1) {
                printf("%ld %zu\n", partner->index, j);
                found = 1;
            }
            Slot *own = find_slot(&seen, numbers[j]);
            if (own->index == -1) {  // keep the first index only
                own->key = numbers[j];
                own->index = (long)j;
            }
        }
        if (!found) {
            printf("none\n");
        }
        free(seen.slots);
    }
    free(numbers);
    return 0;
}
