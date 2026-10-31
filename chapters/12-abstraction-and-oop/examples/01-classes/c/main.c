#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// C has no private fields. A library would put only "struct fraction;" in its
// header, so other files could hold a pointer but not see inside. In one
// file, the rule is a comment: use the functions, not the fields.
struct fraction {
    long numerator;
    long denominator; // always positive, and in lowest terms
};

long gcd(long a, long b) {
    a = labs(a);
    while (b != 0) {
        long rest = a % b;
        a = b;
        b = rest;
    }
    return a;
}

// The "constructor": returns false instead of making a broken fraction.
bool fraction_make(long numerator, long denominator, struct fraction *out) {
    if (denominator == 0) {
        return false;
    }
    if (denominator < 0) {
        numerator = -numerator;
        denominator = -denominator;
    }
    long divisor = gcd(numerator, denominator);
    *out = (struct fraction){numerator / divisor, denominator / divisor};
    return true;
}

struct fraction fraction_add(struct fraction a, struct fraction b) {
    struct fraction sum;
    fraction_make(a.numerator * b.denominator + b.numerator * a.denominator,
                  a.denominator * b.denominator, &sum);
    return sum;
}

bool fraction_equal(struct fraction a, struct fraction b) {
    return a.numerator == b.numerator && a.denominator == b.denominator;
}

// Negative, zero, or positive, the shape that qsort wants.
int fraction_compare(const void *left, const void *right) {
    const struct fraction *a = left, *b = right;
    long x = a->numerator * b->denominator, y = b->numerator * a->denominator;
    return (x > y) - (x < y);
}

void fraction_print(struct fraction f) {
    if (f.denominator == 1) {
        printf("%ld", f.numerator);
    } else {
        printf("%ld/%ld", f.numerator, f.denominator);
    }
}

const char *yes_no(bool value) { return value ? "yes" : "no"; }

int main(void) {
    struct fraction half, third, other;
    fraction_make(1, 2, &half);
    fraction_make(1, 3, &third);

    printf("1/2 + 1/3 = ");
    fraction_print(fraction_add(half, third));
    printf("\n2/-4 is stored as ");
    fraction_make(2, -4, &other);
    fraction_print(other);
    printf("\n6/3 is stored as ");
    fraction_make(6, 3, &other);
    fraction_print(other);
    printf("\n");
    if (!fraction_make(1, 0, &other)) {
        printf("1/0: error: denominator is zero\n");
    }
    fraction_make(2, 4, &other);
    printf("1/2 == 2/4: %s\n", yes_no(fraction_equal(half, other)));
    printf("1/3 < 1/2: %s\n", yes_no(fraction_compare(&third, &half) < 0));

    struct fraction list[4];
    fraction_make(3, 4, &list[0]);
    list[1] = third;
    fraction_make(-1, 2, &list[2]);
    fraction_make(2, 4, &list[3]);
    qsort(list, 4, sizeof list[0], fraction_compare);
    printf("sorted: ");
    for (int i = 0; i < 4; i++) {
        printf("%s", i > 0 ? ", " : "");
        fraction_print(list[i]);
    }
    printf("\n");
    return 0;
}
