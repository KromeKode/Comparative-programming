#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ACCOUNTS 100
#define MAX_NAME 32
#define MAX_LINE 256
#define MAX_TEXT 128

#define OVERDRAFT_LIMIT 100
#define OVERDRAFT_FEE 5
#define SAVINGS_WITHDRAWALS 2

// Every account starts with this struct. The vtable holds the functions that
// differ between kinds of account.
struct account;
struct account_vtable {
    // Returns true if the withdrawal is allowed. If not, writes why into error.
    bool (*check_withdrawal)(const struct account *self, long long amount,
                             char *error, size_t size);
    // Called after each withdrawal that worked. NULL means do nothing.
    void (*after_withdrawal)(struct account *self);
    // Changes the balance as the kind requires, and writes what it did.
    void (*month_end)(struct account *self, char *note, size_t size);
};

struct account {
    const struct account_vtable *vtable;
    char name[MAX_NAME];
    long long balance;
};

void deposit(struct account *account, long long amount) {
    account->balance += amount;
}

bool withdraw(struct account *account, long long amount, char *error,
              size_t size) {
    if (!account->vtable->check_withdrawal(account, amount, error, size)) {
        return false;
    }
    account->balance -= amount;
    if (account->vtable->after_withdrawal != NULL) {
        account->vtable->after_withdrawal(account);
    }
    return true;
}

// A checking account adds nothing to the shared struct.
bool checking_check(const struct account *self, long long amount, char *error,
                    size_t size) {
    if (self->balance - amount < -OVERDRAFT_LIMIT) {
        snprintf(error, size, "over the overdraft limit of %d",
                 OVERDRAFT_LIMIT);
        return false;
    }
    return true;
}

void checking_month_end(struct account *self, char *note, size_t size) {
    if (self->balance < 0) {
        self->balance -= OVERDRAFT_FEE;
        snprintf(note, size, "fee %d", OVERDRAFT_FEE);
    } else {
        snprintf(note, size, "no fee");
    }
}

static const struct account_vtable checking_vtable = {
    checking_check, NULL, checking_month_end};

// A savings account has its own data after the shared part.
struct savings {
    struct account base;
    int withdrawals;
};

bool savings_check(const struct account *self, long long amount, char *error,
                   size_t size) {
    const struct savings *savings = (const struct savings *)self;
    if (savings->withdrawals == SAVINGS_WITHDRAWALS) {
        snprintf(error, size, "only %d withdrawals a month",
                 SAVINGS_WITHDRAWALS);
        return false;
    }
    if (amount > self->balance) {
        snprintf(error, size, "insufficient funds");
        return false;
    }
    return true;
}

void savings_after_withdrawal(struct account *self) {
    ((struct savings *)self)->withdrawals++;
}

void savings_month_end(struct account *self, char *note, size_t size) {
    long long interest = self->balance / 100;
    self->balance += interest;
    ((struct savings *)self)->withdrawals = 0;
    snprintf(note, size, "interest %lld", interest);
}

static const struct account_vtable savings_vtable = {
    savings_check, savings_after_withdrawal, savings_month_end};

// Makes an account of the named kind, or returns NULL for an unknown kind.
struct account *make_account(const char *kind, const char *name) {
    struct account *account = NULL;
    if (strcmp(kind, "checking") == 0) {
        account = calloc(1, sizeof(struct account));
        if (account != NULL) {
            account->vtable = &checking_vtable;
        }
    } else if (strcmp(kind, "savings") == 0) {
        struct savings *savings = calloc(1, sizeof(struct savings));
        if (savings != NULL) {
            savings->base.vtable = &savings_vtable;
            account = &savings->base;
        }
    } else {
        return NULL;
    }
    if (account == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    snprintf(account->name, sizeof account->name, "%s", name);
    return account;
}

struct bank {
    struct account *accounts[MAX_ACCOUNTS]; // in the order they were opened
    int count;
};

struct account *find(struct bank *bank, const char *name) {
    for (int i = 0; i < bank->count; i++) {
        if (strcmp(bank->accounts[i]->name, name) == 0) {
            return bank->accounts[i];
        }
    }
    return NULL;
}

void run(struct bank *bank, char **words, int count) {
    char error[MAX_TEXT];
    if (count == 3 && strcmp(words[0], "open") == 0) {
        const char *kind = words[1], *name = words[2];
        struct account *account = make_account(kind, name);
        if (account == NULL) {
            printf("error: unknown account kind: %s\n", kind);
        } else if (find(bank, name) != NULL) {
            printf("error: %s already has an account\n", name);
            free(account);
        } else if (bank->count == MAX_ACCOUNTS) {
            printf("error: too many accounts\n");
            free(account);
        } else {
            bank->accounts[bank->count++] = account;
            printf("%s: opened a %s account\n", name, kind);
        }
    } else if (count == 3 && (strcmp(words[0], "deposit") == 0 ||
                              strcmp(words[0], "withdraw") == 0)) {
        const char *name = words[1];
        long long amount = strtoll(words[2], NULL, 10);
        struct account *account = find(bank, name);
        if (account == NULL) {
            printf("error: no account named %s\n", name);
        } else if (strcmp(words[0], "deposit") == 0) {
            deposit(account, amount);
            printf("%s: deposited %lld, balance %lld\n", name, amount,
                   account->balance);
        } else if (!withdraw(account, amount, error, sizeof error)) {
            printf("error: %s: %s\n", name, error);
        } else {
            printf("%s: withdrew %lld, balance %lld\n", name, amount,
                   account->balance);
        }
    } else if (count == 1 && strcmp(words[0], "month") == 0) {
        for (int i = 0; i < bank->count; i++) {
            struct account *account = bank->accounts[i];
            account->vtable->month_end(account, error, sizeof error);
            printf("%s: %s, balance %lld\n", account->name, error,
                   account->balance);
        }
    }
}

int main(void) {
    struct bank bank = {.count = 0};
    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *words[4];
        int count = 0;
        for (char *word = strtok(line, " \t\r\n"); word != NULL;
             word = strtok(NULL, " \t\r\n")) {
            if (count == 4) {
                break;
            }
            words[count++] = word;
        }
        run(&bank, words, count);
    }
    for (int i = 0; i < bank.count; i++) {
        free(bank.accounts[i]);
    }
    return 0;
}
