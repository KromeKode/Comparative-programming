#include <stdbool.h>
#include <stdio.h>

// The kinds of failure, plus a value that means success.
enum withdraw_kind {
    WITHDRAW_OK,
    INVALID_AMOUNT,
    INSUFFICIENT_FUNDS,
    ACCOUNT_FROZEN,
};

// The kind, plus the numbers a caller needs to recover.
struct withdraw_error {
    enum withdraw_kind kind;
    long amount;
    long available;
};

struct account {
    long balance;
    bool frozen;
};

struct withdraw_error withdraw(struct account *account, long amount) {
    if (amount <= 0) {
        return (struct withdraw_error){INVALID_AMOUNT, amount, 0};
    }
    if (account->frozen) {
        return (struct withdraw_error){ACCOUNT_FROZEN, amount, 0};
    }
    if (amount > account->balance) {
        return (struct withdraw_error){INSUFFICIENT_FUNDS, amount,
                                       account->balance};
    }
    account->balance -= amount;
    return (struct withdraw_error){WITHDRAW_OK, amount, 0};
}

void print_error(struct withdraw_error error) {
    // -Wall warns if this switch leaves out one of the enum's values.
    switch (error.kind) {
    case WITHDRAW_OK:
        printf("no error\n");
        break;
    case INVALID_AMOUNT:
        printf("invalid amount: %ld\n", error.amount);
        break;
    case INSUFFICIENT_FUNDS:
        printf("insufficient funds: need %ld, have %ld\n", error.amount,
               error.available);
        break;
    case ACCOUNT_FROZEN:
        printf("account is frozen\n");
        break;
    }
}

void attempt(struct account *account, long amount) {
    struct withdraw_error error = withdraw(account, amount);
    if (error.kind == WITHDRAW_OK) {
        printf("withdraw %ld: ok, %ld left\n", amount, account->balance);
        return;
    }
    printf("withdraw %ld: error: ", amount);
    print_error(error);
    if (error.kind == INSUFFICIENT_FUNDS) {
        withdraw(account, error.available);
        printf("  took %ld instead, %ld left\n", error.available,
               account->balance);
    } else if (error.kind == ACCOUNT_FROZEN) {
        printf("  call the bank to unfreeze it\n");
    }
}

int main(void) {
    struct account account = {100, false};
    printf("start with %ld\n", account.balance);
    long amounts[] = {30, -5, 100};
    for (int i = 0; i < 3; i++) {
        attempt(&account, amounts[i]);
    }
    printf("freeze the account\n");
    account.frozen = true;
    attempt(&account, 10);
    return 0;
}
