// The stock module: how many of each item there are.
//
// A Stock is an opaque type: other files can hold a pointer to one, and pass it to
// these functions, but only stock.c knows what is inside it.
#ifndef STOCK_H
#define STOCK_H

#include <stdbool.h>

typedef struct Stock Stock;

// A new, empty stock, or NULL if there is no memory for one. Free it with
// stock_free.
Stock *stock_new(void);
void stock_free(Stock *stock);

// Adds amount of item, and returns false only if there was no memory to.
bool stock_add(Stock *stock, const char *item, long long amount);
// Takes amount of item away, and returns true, if there are that many.
bool stock_take(Stock *stock, const char *item, long long amount);
// How many of item there are: 0 for an item never added.
long long stock_count(const Stock *stock, const char *item);
// Calls visit for each item there are any of, in strcmp's order, with its count and
// the context pointer, which is for visit's own use.
void stock_each(const Stock *stock,
                void (*visit)(const char *item, long long count, void *context),
                void *context);

#endif
