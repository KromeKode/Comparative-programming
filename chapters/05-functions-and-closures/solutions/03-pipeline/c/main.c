#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C cannot make a closure that remembers n, so each step stores a function
// pointer and its n side by side.
typedef long long (*Op)(long long x, long long n);

static long long op_add(long long x, long long n) {
    return x + n;
}
static long long op_sub(long long x, long long n) {
    return x - n;
}
static long long op_mul(long long x, long long n) {
    return x * n;
}
static long long op_min(long long x, long long n) {
    return x < n ? x : n;
}
static long long op_max(long long x, long long n) {
    return x > n ? x : n;
}

static Op lookup(const char *name) {
    if (strcmp(name, "add") == 0) return op_add;
    if (strcmp(name, "sub") == 0) return op_sub;
    if (strcmp(name, "mul") == 0) return op_mul;
    if (strcmp(name, "min") == 0) return op_min;
    if (strcmp(name, "max") == 0) return op_max;
    return NULL;
}

struct Step {
    Op op;
    long long n;
};

int main(void) {
    char line[512];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *word = strtok(line, " \t\r\n");
        if (word == NULL) {
            continue;  // a blank line
        }
        long long start = atoll(word);

        struct Step steps[64];
        int count = 0;
        while ((word = strtok(NULL, " \t\r\n")) != NULL) {
            Op op = lookup(word);
            char *number = strtok(NULL, " \t\r\n");
            if (op == NULL || number == NULL || count == 64) {
                fprintf(stderr, "bad step: %s\n", word);
                return 1;
            }
            steps[count++] = (struct Step){op, atoll(number)};
        }

        long long value = start;
        for (int i = 0; i < count; i++) {
            value = steps[i].op(value, steps[i].n);
        }
        printf("%lld -> %lld\n", start, value);
    }
    return 0;
}
