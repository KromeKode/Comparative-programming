// Simulates a day of payments between bank accounts, and prints a summary. It gives
// the right answers, but slowly: ../README.md says what it prints, and how fast it
// has to be.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { START = 1000 };  // what each account starts with

// The Park-Miller generator, which every language's version uses, so that they all
// make up the same transfers from the same seed.
static long long next_random(long long *state) {
    *state = *state * 48271 % 2147483647;
    return *state;
}

struct account {
    char name[16];
    long long balance;
};

static void account_name(char name[16], int i) { snprintf(name, 16, "acct%06d", i); }

// Gives the account with this name.
static struct account *find(struct account *accounts, int count, const char *name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(accounts[i].name, name) == 0) return &accounts[i];
    }
    return NULL;
}

static long long total(const struct account *accounts, int count) {
    long long sum = 0;
    for (int i = 0; i < count; i++) sum += accounts[i].balance;
    return sum;
}

static int richest_first(const void *a, const void *b) {
    const struct account *x = a, *y = b;
    if (x->balance != y->balance) return x->balance < y->balance ? 1 : -1;
    return strcmp(x->name, y->name);
}

int main(void) {
    int count;
    long long transfers, seed;
    if (scanf("%d %lld %lld", &count, &transfers, &seed) != 3) return 1;
    struct account *accounts = malloc((size_t)count * sizeof *accounts);
    if (!accounts) return 1;
    for (int i = 0; i < count; i++) {
        account_name(accounts[i].name, i);
        accounts[i].balance = START;
    }

    long long state = seed, refused = 0;
    char name[16];
    for (long long t = 0; t < transfers; t++) {
        account_name(name, (int)(next_random(&state) % count));
        struct account *source = find(accounts, count, name);
        account_name(name, (int)(next_random(&state) % count));
        struct account *target = find(accounts, count, name);
        long long amount = next_random(&state) % 500 + 1;
        if (source->balance < amount) {
            refused++;
        } else {
            source->balance -= amount;
            target->balance += amount;
        }
        // Money is never made or lost, so the total stays the same.
        if (total(accounts, count) != (long long)count * START) {
            fprintf(stderr, "error: the total has changed\n");
            return 1;
        }
    }

    printf("transfers: %lld, refused: %lld\n", transfers, refused);
    printf("total: %lld\n", total(accounts, count));
    qsort(accounts, (size_t)count, sizeof *accounts, richest_first);
    for (int i = 0; i < count && i < 5; i++) {
        printf("%s %lld\n", accounts[i].name, accounts[i].balance);
    }
    free(accounts);
    return 0;
}
