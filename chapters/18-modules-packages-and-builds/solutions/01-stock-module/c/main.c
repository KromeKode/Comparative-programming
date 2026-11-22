// Exercise 18.1: a stock module. This file is given: write stock.h and stock.c so
// that it builds. It reads the commands, and leaves keeping count to the module.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stock.h"

#define MAX_LINE 1024
#define MAX_WORDS 4
#define SPACE " \t\r\n"

// An amount is a whole number from 1 to 999,999,999,999,999: digits, with any
// zeros in front, and at most 15 more after them. It is stored in *amount.
static bool parse_amount(const char *word, long long *amount) {
    while (*word == '0') word++;
    size_t length = strlen(word);
    if (length == 0 || length > 15 || strspn(word, "0123456789") != length) return false;
    *amount = strtoll(word, NULL, 10);
    return true;
}

// list prints the items through stock_each, which calls this once for each item
// with the same context: here, whether an item has been printed yet.
static void print_item(const char *item, long long count, void *context) {
    bool *first = context;
    printf("%s%s %lld", *first ? "" : ", ", item, count);
    *first = false;
}

int main(void) {
    Stock *stock = stock_new();
    if (stock == NULL) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }
    char line[MAX_LINE];
    for (int number = 1; fgets(line, sizeof line, stdin) != NULL; number++) {
        char *words[MAX_WORDS];
        int count = 0;
        for (char *word = strtok(line, SPACE); word != NULL; word = strtok(NULL, SPACE)) {
            if (count == MAX_WORDS) break;
            words[count++] = word;
        }
        if (count == 0) continue;
        long long amount;
        if (count == 3 && strcmp(words[0], "add") == 0 && parse_amount(words[2], &amount)) {
            if (!stock_add(stock, words[1], amount)) {
                fprintf(stderr, "out of memory\n");
                stock_free(stock);
                return 1;
            }
            printf("added %lld %s, now %lld\n", amount, words[1], stock_count(stock, words[1]));
        } else if (count == 3 && strcmp(words[0], "take") == 0 && parse_amount(words[2], &amount)) {
            if (stock_take(stock, words[1], amount)) {
                printf("took %lld %s, now %lld\n", amount, words[1], stock_count(stock, words[1]));
            } else {
                printf("error: line %d: only %lld %s in stock\n", number,
                       stock_count(stock, words[1]), words[1]);
            }
        } else if (count == 2 && strcmp(words[0], "count") == 0) {
            printf("%s: %lld\n", words[1], stock_count(stock, words[1]));
        } else if (count == 1 && strcmp(words[0], "list") == 0) {
            bool first = true;
            printf("stock: ");
            stock_each(stock, print_item, &first);
            printf("%s\n", first ? "empty" : "");
        } else {
            printf("error: line %d: expected add, take, count, or list\n", number);
        }
    }
    stock_free(stock);
    return 0;
}
