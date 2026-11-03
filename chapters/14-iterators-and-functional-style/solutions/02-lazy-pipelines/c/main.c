#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_STAGES 64

static bool is_prime(long long n) {
    if (n < 2) return false;
    for (long long d = 2; d * d <= n; d++) {
        if (n % d == 0) return false;
    }
    return true;
}

// A predicate or a function, with its number, such as "above 5" or "add 3".
struct op {
    enum { EVEN, ODD, PRIME, ABOVE, BELOW, SQUARE, NEGATE, ADD, MUL } kind;
    long long n;
};

static bool test(struct op op, long long x) {
    switch (op.kind) {
        case EVEN: return x % 2 == 0;
        case ODD: return x % 2 != 0;
        case PRIME: return is_prime(x);
        case ABOVE: return x > op.n;
        case BELOW: return x < op.n;
        default: return false;
    }
}

static long long apply(struct op op, long long x) {
    switch (op.kind) {
        case SQUARE: return x * x;
        case NEGATE: return -x;
        case ADD: return x + op.n;
        case MUL: return x * op.n;
        default: return x;
    }
}

// One link in the chain. Every stage, sources included, has a next function
// that stores the next item in *out and returns true, or returns false at the
// end. A stage pulls from the one before it only when it is asked for an item.
struct stage {
    bool (*next)(struct stage *self, long long *out);
    struct stage *inner;  // the stage this one pulls from; a source has none
    struct op op;         // for map, filter, takewhile, and dropwhile
    long long n;          // the number in take 3, skip 3, or step 3
    long long a, b;       // state kept between calls; each kind says how it uses them
};

static bool pull(struct stage *stage, long long *out) { return stage->next(stage, out); }

static bool naturals_next(struct stage *self, long long *out) {  // a: the last one given
    *out = ++self->a;
    return true;
}

static bool primes_next(struct stage *self, long long *out) {  // a: the last one checked
    while (!is_prime(++self->a)) {
    }
    *out = self->a;
    return true;
}

static bool fibonacci_next(struct stage *self, long long *out) {  // a, b: the next two
    *out = self->a;
    long long sum = self->a + self->b;
    self->a = self->b;
    self->b = sum;
    return true;
}

static bool range_next(struct stage *self, long long *out) {  // a: next; b: the last
    if (self->a > self->b) return false;
    *out = self->a++;
    return true;
}

static bool repeat_next(struct stage *self, long long *out) {  // a: the item
    *out = self->a;
    return true;
}

static bool map_next(struct stage *self, long long *out) {
    if (!pull(self->inner, out)) return false;
    *out = apply(self->op, *out);
    return true;
}

static bool filter_next(struct stage *self, long long *out) {
    while (pull(self->inner, out)) {
        if (test(self->op, *out)) return true;
    }
    return false;
}

static bool takewhile_next(struct stage *self, long long *out) {  // a: 1 once done
    if (self->a || !pull(self->inner, out)) return false;
    if (test(self->op, *out)) return true;
    self->a = 1;  // once one item fails, the rest are not looked at
    return false;
}

static bool dropwhile_next(struct stage *self, long long *out) {  // a: 1 once done dropping
    while (pull(self->inner, out)) {
        if (self->a || !test(self->op, *out)) {
            self->a = 1;
            return true;
        }
    }
    return false;
}

// Checking the count before pulling is what stops take from asking for one
// item more than it needs.
static bool take_next(struct stage *self, long long *out) {  // a: how many are left
    if (self->a == 0) return false;
    self->a--;
    return pull(self->inner, out);
}

static bool skip_next(struct stage *self, long long *out) {  // a: how many to skip
    for (; self->a > 0; self->a--) {
        if (!pull(self->inner, out)) return false;
    }
    return pull(self->inner, out);
}

static bool step_next(struct stage *self, long long *out) {  // a: 1 after the first item
    for (long long i = 1; self->a && i < self->n; i++) {
        if (!pull(self->inner, out)) return false;
    }
    self->a = 1;
    return pull(self->inner, out);
}

static struct op parse_op(const char *name, const char *number) {
    static const char *names[] = {"even",   "odd",    "prime", "above", "below",
                                  "square", "negate", "add",   "mul"};
    struct op op = {EVEN, strtoll(number, NULL, 10)};
    for (int i = 0; i < (int)(sizeof names / sizeof names[0]); i++) {
        if (strcmp(name, names[i]) == 0) op.kind = i;
    }
    return op;
}

static bool is(const char *word, const char *name) { return strcmp(word, name) == 0; }

// Fills in a stage from its words, with the state it starts in. Returns false
// for an unknown name.
static bool parse_stage(struct stage *stage, char words[3][24]) {
    const char *name = words[0];
    long long n = strtoll(words[1], NULL, 10), m = strtoll(words[2], NULL, 10);  // or zeros
    struct op op = parse_op(words[1], words[2]);
    if (is(name, "naturals")) *stage = (struct stage){.next = naturals_next};
    else if (is(name, "primes")) *stage = (struct stage){.next = primes_next};
    else if (is(name, "fibonacci")) *stage = (struct stage){.next = fibonacci_next, .b = 1};
    else if (is(name, "range")) *stage = (struct stage){.next = range_next, .a = n, .b = m};
    else if (is(name, "repeat")) *stage = (struct stage){.next = repeat_next, .a = n};
    else if (is(name, "map")) *stage = (struct stage){.next = map_next, .op = op};
    else if (is(name, "filter")) *stage = (struct stage){.next = filter_next, .op = op};
    else if (is(name, "takewhile")) *stage = (struct stage){.next = takewhile_next, .op = op};
    else if (is(name, "dropwhile")) *stage = (struct stage){.next = dropwhile_next, .op = op};
    else if (is(name, "take")) *stage = (struct stage){.next = take_next, .a = n};
    else if (is(name, "skip")) *stage = (struct stage){.next = skip_next, .a = n};
    else if (is(name, "step")) *stage = (struct stage){.next = step_next, .n = n};
    else return false;
    return true;
}

static void run(char *line) {
    static struct stage stages[MAX_STAGES];
    enum { LIST, SUM, COUNT } finish = LIST;
    int count = 0;
    for (char *part = strtok(line, "|"); part != NULL && count < MAX_STAGES;
         part = strtok(NULL, "|")) {
        char words[3][24] = {""};
        int got = sscanf(part, "%23s %23s %23s", words[0], words[1], words[2]);
        // A sum or count at the end is not a stage: it says what to do with the items.
        if (got == 1 && (is(words[0], "sum") || is(words[0], "count"))) {
            finish = is(words[0], "sum") ? SUM : COUNT;
        } else if (!parse_stage(&stages[count], words)) {
            fprintf(stderr, "unknown stage %s\n", words[0]);
            return;
        } else {
            stages[count].inner = count > 0 ? &stages[count - 1] : NULL;
            count++;
        }
    }
    // Nothing has run yet. Only the loop below pulls items through.
    long long item = 0, total = 0, items = 0;
    while (pull(&stages[count - 1], &item)) {
        total += item;
        items++;
        if (finish == LIST) printf(items > 1 ? " %lld" : "%lld", item);
    }
    if (finish == SUM) printf("%lld", total);
    if (finish == COUNT) printf("%lld", items);
    if (finish == LIST && items == 0) printf("(empty)");
    printf("\n");
}

int main(void) {
    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin) != NULL) {
        if (strspn(line, " \t\r\n") < strlen(line)) run(line);
    }
    return 0;
}
