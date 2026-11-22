#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define MAX_ARGS 8

typedef long long num;

static struct {
    bool on;
    int depth;
    long calls;
} tracer;

static void indent(void) {
    for (int i = 0; i < tracer.depth; i++) printf("  ");
}

static void print_call(const char *name, int argc, const num *args) {
    printf("%s(", name);
    for (int i = 0; i < argc; i++) printf(i ? ", %lld" : "%lld", args[i]);
    printf(")");
}

static void trace_enter(const char *name, int argc, const num *args) {
    tracer.calls++;
    if (tracer.on) {
        indent();
        print_call(name, argc, args);
        printf("\n");
    }
    tracer.depth++;
}

static num trace_leave(num result) {
    tracer.depth--;
    if (tracer.on) {
        indent();
        printf("-> %lld\n", result);
    }
    return result;
}

// TRACED_1(fib, n) { ... } defines fib(n) as a wrapper that traces the call
// and runs the body written after the macro, which becomes fib_body. It also
// defines fib_call, which takes the arguments as an array, for the table.
#define TRACED_1(name, a)                                                        \
    static num name##_body(num a);                                               \
    static num name(num a) {                                                     \
        num args[] = {a};                                                        \
        trace_enter(#name, 1, args);                                             \
        return trace_leave(name##_body(a));                                      \
    }                                                                            \
    static num name##_call(const num *args) { return name(args[0]); }            \
    enum { name##_arity = 1 };                                                   \
    static num name##_body(num a)

#define TRACED_2(name, a, b)                                                     \
    static num name##_body(num a, num b);                                        \
    static num name(num a, num b) {                                              \
        num args[] = {a, b};                                                     \
        trace_enter(#name, 2, args);                                             \
        return trace_leave(name##_body(a, b));                                   \
    }                                                                            \
    static num name##_call(const num *args) { return name(args[0], args[1]); }   \
    enum { name##_arity = 2 };                                                   \
    static num name##_body(num a, num b)

// The bodies call fib, gcd, and power, which are the wrappers, so the
// recursive calls are traced too.
TRACED_1(fib, n) { return n < 2 ? n : fib(n - 1) + fib(n - 2); }

TRACED_2(gcd, a, b) { return b == 0 ? a : gcd(b, a % b); }

TRACED_2(power, base, exponent) {
    if (exponent == 0) return 1;
    if (exponent % 2 == 0) {
        num half = power(base, exponent / 2);
        return half * half;
    }
    return base * power(base, exponent - 1);
}

#define ENTRY(name) {#name, name##_arity, name##_call}

static const struct function {
    const char *name;
    int arity;
    num (*call)(const num *args);
} functions[] = {ENTRY(fib), ENTRY(gcd), ENTRY(power)};

static const struct function *find(const char *name) {
    for (size_t i = 0; i < sizeof functions / sizeof functions[0]; i++) {
        if (strcmp(functions[i].name, name) == 0) return &functions[i];
    }
    return NULL;
}

static void run(char *line) {
    const char *command = strtok(line, " \t\r\n");
    const char *name = strtok(NULL, " \t\r\n");
    num args[MAX_ARGS];
    int argc = 0;
    for (char *word; (word = strtok(NULL, " \t\r\n")) != NULL;) {
        if (argc < MAX_ARGS) args[argc] = strtoll(word, NULL, 10);
        argc++;
    }
    bool trace = strcmp(command, "trace") == 0;
    if (!trace && strcmp(command, "count") != 0) {
        printf("unknown command: %s\n", command);
        return;
    }
    const struct function *function = find(name);
    if (function == NULL) {
        printf("unknown function: %s\n", name);
        return;
    }
    if (argc != function->arity) {
        printf("%s takes %d argument%s, got %d\n", name, function->arity,
               function->arity == 1 ? "" : "s", argc);
        return;
    }
    tracer.on = trace;
    tracer.depth = 0;
    tracer.calls = 0;
    num result = function->call(args);
    print_call(name, argc, args);
    printf(" = %lld in %ld call%s\n", result, tracer.calls, tracer.calls == 1 ? "" : "s");
}

int main(void) {
    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin) != NULL) {
        if (strspn(line, " \t\r\n") != strlen(line)) run(line);
    }
    return 0;
}
