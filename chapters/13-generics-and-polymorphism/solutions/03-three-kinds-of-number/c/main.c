#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 512

// One number of any kind. Int and Mod7 use only top; Frac uses both.
struct number {
    long long top, bottom;
};

// A kind is a table of functions: everything eval needs to know about a type
// of number. In Rust this is a trait, and the compiler passes the table for
// you; in C you pass it by hand.
struct kind {
    const char *name;
    bool (*parse)(const char *text, struct number *out);
    struct number (*add)(struct number a, struct number b);
    struct number (*sub)(struct number a, struct number b);
    struct number (*mul)(struct number a, struct number b);
    bool (*div)(struct number a, struct number b, struct number *out);  // false for / 0
    void (*print)(struct number n);
};

// A whole number: an optional minus sign and at least one digit.
static bool parse_whole(const char *text, size_t length, long long *out) {
    const char *digits = text[0] == '-' ? text + 1 : text;
    size_t digit_count = length - (size_t)(digits - text);
    if (digit_count == 0 || strspn(digits, "0123456789") < digit_count) return false;
    char copy[32];
    if (length >= sizeof copy) return false;
    memcpy(copy, text, length);
    copy[length] = '\0';
    errno = 0;
    *out = strtoll(copy, NULL, 10);
    return errno == 0;
}

static bool parse_int(const char *text, struct number *out) {
    out->bottom = 1;
    return parse_whole(text, strlen(text), &out->top);
}

static struct number int_add(struct number a, struct number b) {
    return (struct number){a.top + b.top, 1};
}

static struct number int_sub(struct number a, struct number b) {
    return (struct number){a.top - b.top, 1};
}

static struct number int_mul(struct number a, struct number b) {
    return (struct number){a.top * b.top, 1};
}

// C's / rounds toward zero.
static bool int_div(struct number a, struct number b, struct number *out) {
    if (b.top == 0) return false;
    *out = (struct number){a.top / b.top, 1};
    return true;
}

static void print_whole(struct number n) { printf("%lld\n", n.top); }

static long long gcd(long long a, long long b) {
    while (b != 0) {
        long long rest = a % b;
        a = b;
        b = rest;
    }
    return a < 0 ? -a : a;
}

// Lowest terms, with a positive bottom.
static struct number frac(long long top, long long bottom) {
    if (bottom < 0) {
        top = -top;
        bottom = -bottom;
    }
    long long divisor = gcd(top, bottom);
    return (struct number){top / divisor, bottom / divisor};
}

static bool parse_frac(const char *text, struct number *out) {
    const char *slash = strchr(text, '/');
    long long top, bottom = 1;
    if (!parse_whole(text, slash ? (size_t)(slash - text) : strlen(text), &top)) return false;
    if (slash && !parse_whole(slash + 1, strlen(slash + 1), &bottom)) return false;
    // "1/-2" and "1/0" are not fractions: the bottom must be above zero.
    if (bottom <= 0) return false;
    *out = frac(top, bottom);
    return true;
}

static struct number frac_add(struct number a, struct number b) {
    return frac(a.top * b.bottom + b.top * a.bottom, a.bottom * b.bottom);
}

static struct number frac_sub(struct number a, struct number b) {
    return frac(a.top * b.bottom - b.top * a.bottom, a.bottom * b.bottom);
}

static struct number frac_mul(struct number a, struct number b) {
    return frac(a.top * b.top, a.bottom * b.bottom);
}

static bool frac_div(struct number a, struct number b, struct number *out) {
    if (b.top == 0) return false;
    *out = frac(a.top * b.bottom, a.bottom * b.top);
    return true;
}

static void print_frac(struct number n) {
    if (n.bottom == 1) {
        printf("%lld\n", n.top);
    } else {
        printf("%lld/%lld\n", n.top, n.bottom);
    }
}

// C's % keeps the sign of the left side, so -1 % 7 is -1. Adding 7 fixes it.
static struct number mod7(long long n) { return (struct number){(n % 7 + 7) % 7, 1}; }

static bool parse_mod7(const char *text, struct number *out) {
    long long n;
    if (!parse_whole(text, strlen(text), &n)) return false;
    *out = mod7(n);
    return true;
}

static struct number mod7_add(struct number a, struct number b) { return mod7(a.top + b.top); }
static struct number mod7_sub(struct number a, struct number b) { return mod7(a.top - b.top); }
static struct number mod7_mul(struct number a, struct number b) { return mod7(a.top * b.top); }

// x to the power 5 is the inverse of x when 7 is prime: x * x^5 = x^6 = 1.
static bool mod7_div(struct number a, struct number b, struct number *out) {
    if (b.top == 0) return false;
    struct number inverse = mod7(1);
    for (int i = 0; i < 5; i++) inverse = mod7_mul(inverse, b);
    *out = mod7_mul(a, inverse);
    return true;
}

static const struct kind kinds[] = {
    {"int", parse_int, int_add, int_sub, int_mul, int_div, print_whole},
    {"frac", parse_frac, frac_add, frac_sub, frac_mul, frac_div, print_frac},
    {"mod7", parse_mod7, mod7_add, mod7_sub, mod7_mul, mod7_div, print_whole},
};

// Runs the tokens on a stack of numbers. Returns NULL and sets *result, or
// returns an error message (in a static buffer).
static const char *eval(const struct kind *kind, char **tokens, size_t count,
                        struct number *result) {
    static char message[MAX_LINE];
    struct number stack[MAX_LINE / 2];
    size_t size = 0;
    for (size_t i = 0; i < count; i++) {
        const char *token = tokens[i];
        if (strlen(token) == 1 && strchr("+-*/", token[0])) {
            if (size < 2) return "not enough numbers";
            struct number left = stack[size - 2], right = stack[size - 1];
            size -= 2;
            switch (token[0]) {
                case '+': stack[size] = kind->add(left, right); break;
                case '-': stack[size] = kind->sub(left, right); break;
                case '*': stack[size] = kind->mul(left, right); break;
                default:
                    if (!kind->div(left, right, &stack[size])) return "division by zero";
            }
            size++;
        } else if (kind->parse(token, &stack[size])) {
            size++;
        } else {
            snprintf(message, sizeof message, "bad token \"%s\"", token);
            return message;
        }
    }
    if (size != 1) {
        snprintf(message, sizeof message, "expected one result, found %zu", size);
        return message;
    }
    *result = stack[0];
    return NULL;
}

int main(void) {
    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *words[MAX_LINE / 2];
        size_t count = 0;
        for (char *word = strtok(line, " \t\r\n"); word; word = strtok(NULL, " \t\r\n")) {
            words[count++] = word;
        }
        if (count == 0) continue;
        const struct kind *kind = NULL;
        for (size_t i = 0; i < sizeof kinds / sizeof kinds[0]; i++) {
            if (strcmp(words[0], kinds[i].name) == 0) kind = &kinds[i];
        }
        if (kind == NULL) {
            printf("error: unknown kind \"%s\"\n", words[0]);
            continue;
        }
        struct number result;
        const char *error = eval(kind, words + 1, count - 1, &result);
        if (error) {
            printf("error: %s\n", error);
        } else {
            kind->print(result);
        }
    }
    return 0;
}
