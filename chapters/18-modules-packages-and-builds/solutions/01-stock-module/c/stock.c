// The stock module: a list of items kept in strcmp's order, so that stock_each can
// go through them in order and a binary search can find one.
#include "stock.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    long long count;
} Entry;

// The full type, which only this file can see. Other files can't read or change
// the entries except through the functions in stock.h.
struct Stock {
    Entry *entries;
    size_t length;
    size_t capacity;
};

Stock *stock_new(void) {
    // calloc sets every field to zero: no entries, and no array yet.
    return calloc(1, sizeof(Stock));
}

void stock_free(Stock *stock) {
    if (stock == NULL) return;
    for (size_t i = 0; i < stock->length; i++) free(stock->entries[i].name);
    free(stock->entries);
    free(stock);
}

// static: a helper that other files can't call, or even see. It returns where item
// is, or where it would go, and sets *found to say which.
static size_t find(const Stock *stock, const char *item, bool *found) {
    size_t low = 0, high = stock->length;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        int order = strcmp(stock->entries[middle].name, item);
        if (order == 0) {
            *found = true;
            return middle;
        }
        if (order < 0) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    *found = false;
    return low;
}

bool stock_add(Stock *stock, const char *item, long long amount) {
    bool found;
    size_t at = find(stock, item, &found);
    if (found) {
        stock->entries[at].count += amount;
        return true;
    }
    if (stock->length == stock->capacity) {
        size_t capacity = stock->capacity ? 2 * stock->capacity : 8;
        Entry *entries = realloc(stock->entries, capacity * sizeof(Entry));
        if (entries == NULL) return false;
        stock->entries = entries;
        stock->capacity = capacity;
    }
    size_t size = strlen(item) + 1;
    char *name = malloc(size);
    if (name == NULL) return false;
    memcpy(name, item, size);
    memmove(&stock->entries[at + 1], &stock->entries[at], (stock->length - at) * sizeof(Entry));
    stock->entries[at] = (Entry){name, amount};
    stock->length++;
    return true;
}

bool stock_take(Stock *stock, const char *item, long long amount) {
    bool found;
    size_t at = find(stock, item, &found);
    if (!found || stock->entries[at].count < amount) return false;
    stock->entries[at].count -= amount;
    return true;
}

long long stock_count(const Stock *stock, const char *item) {
    bool found;
    size_t at = find(stock, item, &found);
    return found ? stock->entries[at].count : 0;
}

void stock_each(const Stock *stock,
                void (*visit)(const char *item, long long count, void *context),
                void *context) {
    for (size_t i = 0; i < stock->length; i++) {
        if (stock->entries[i].count > 0) {
            visit(stock->entries[i].name, stock->entries[i].count, context);
        }
    }
}
