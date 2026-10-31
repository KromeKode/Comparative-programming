#include <stdio.h>

// A tagged union: the tag says which member of the union holds the data.
typedef enum { CASH, CARD, VOUCHER } PaymentKind;

typedef struct {
    PaymentKind kind;
    union {
        struct {
            const char *last4;
        } card;
        struct {
            const char *code;
            long cents;
        } voucher;
    };
} Payment;

// Writes cents as dollars and cents, such as 12.50, into buf.
static const char *money(long cents, char buf[static 32]) {
    snprintf(buf, 32, "%ld.%02ld", cents / 100, cents % 100);
    return buf;
}

static void describe(const Payment *payment, long total) {
    char a[32], b[32];
    // Nothing stops this code from reading payment->voucher when kind is CARD.
    // Checking the tag first is up to you.
    switch (payment->kind) {
    case CASH:
        printf("cash: pay %s in cash\n", money(total, a));
        break;
    case CARD:
        printf("card ending %s: charge %s to the card\n", payment->card.last4,
               money(total, a));
        break;
    case VOUCHER: {
        long cents = payment->voucher.cents;
        if (cents >= total) {
            printf("voucher %s (%s): covers the order\n", payment->voucher.code,
                   money(cents, a));
        } else {
            printf("voucher %s (%s): %s left to pay\n", payment->voucher.code,
                   money(cents, a), money(total - cents, b));
        }
        break;
    }
    }
}

int main(void) {
    long total = 1250;
    const Payment payments[] = {
        {.kind = CASH},
        {.kind = CARD, .card = {"4242"}},
        {.kind = VOUCHER, .voucher = {"SPRING", 500}},
        {.kind = VOUCHER, .voucher = {"BIG", 2000}},
    };
    char buf[32];
    printf("order total: %s\n", money(total, buf));
    for (size_t i = 0; i < sizeof payments / sizeof payments[0]; i++) {
        describe(&payments[i], total);
    }
    return 0;
}
