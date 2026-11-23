// Reads a trace of function calls and prints a flat profile: for each function, how
// many calls it got, its total time, and its self time. See ../README.md.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct function {
    char *name;
    long long calls, total, self;
    long long running;          // how many of its calls are running now
    long long outermost_start;  // when the outermost of them started
};

struct call {
    size_t function;
    long long start;
    long long in_callees;  // time spent in the functions it called
};

static struct function *functions;
static size_t function_count, function_capacity;
static struct call *stack;
static size_t depth, stack_capacity;

static void *grow(void *items, size_t *capacity, size_t size) {
    *capacity = *capacity ? 2 * *capacity : 16;
    items = realloc(items, *capacity * size);
    if (!items) exit(1);
    return items;
}

// Gives the index of the function with this name, adding it if it is new. A trace
// names a few hundred functions at most, few enough to look through.
static size_t find(const char *name) {
    for (size_t i = 0; i < function_count; i++) {
        if (strcmp(functions[i].name, name) == 0) return i;
    }
    if (function_count == function_capacity) {
        functions = grow(functions, &function_capacity, sizeof *functions);
    }
    char *copy = malloc(strlen(name) + 1);
    if (!copy) exit(1);
    functions[function_count] = (struct function){.name = strcpy(copy, name)};
    return function_count++;
}

// Reads a line of any length, without its newline, into *line. Returns false at the
// end of the input.
static bool read_line(char **line, size_t *capacity) {
    size_t length = 0;
    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        if (length + 1 >= *capacity) *line = grow(*line, capacity, 1);
        (*line)[length++] = (char)c;
    }
    if (c == EOF && length == 0) return false;
    if (*capacity == 0) *line = grow(*line, capacity, 1);
    (*line)[length] = '\0';
    return true;
}

static bool is_time(const char *s) {
    size_t length = strlen(s);
    if (length == 0 || length > 12) return false;
    for (size_t i = 0; i < length; i++) {
        if (s[i] < '0' || s[i] > '9') return false;
    }
    return true;
}

static bool is_name(const char *s) {
    for (; *s; s++) {
        if (*s < '!' || *s > '~') return false;
    }
    return true;
}

static int by_self_time(const void *a, const void *b) {
    const struct function *x = a, *y = b;
    if (x->self != y->self) return x->self < y->self ? 1 : -1;
    return strcmp(x->name, y->name);
}

// Reads the trace, and returns false after printing an error if it is wrong.
static bool profile(void) {
    char *line = NULL;
    size_t capacity = 0;
    long long now = 0;
    bool ok = true;
    for (long number = 1; ok && read_line(&line, &capacity); number++) {
        char *fields[4];
        int count = 0;
        for (char *field = strtok(line, " \t"); field; field = strtok(NULL, " \t")) {
            if (count < 4) fields[count] = field;
            count++;
        }
        if (count == 0) continue;
        if (count != 3 || !is_time(fields[0]) || !is_name(fields[2]) ||
            (strcmp(fields[1], "enter") != 0 && strcmp(fields[1], "exit") != 0)) {
            printf("error: line %ld: expected TIME enter NAME or TIME exit NAME\n", number);
            ok = false;
            break;
        }
        long long time = atoll(fields[0]);
        const char *name = fields[2];
        if (time < now) {
            printf("error: line %ld: time goes back from %lld to %lld\n", number, now, time);
            ok = false;
            break;
        }
        now = time;

        if (strcmp(fields[1], "enter") == 0) {
            size_t f = find(name);
            functions[f].calls++;
            if (functions[f].running == 0) functions[f].outermost_start = time;
            functions[f].running++;
            if (depth == stack_capacity) stack = grow(stack, &stack_capacity, sizeof *stack);
            stack[depth++] = (struct call){f, time, 0};
            continue;
        }

        if (depth == 0) {
            printf("error: line %ld: exit from %s, but nothing is running\n", number, name);
            ok = false;
            break;
        }
        struct call call = stack[depth - 1];
        struct function *function = &functions[call.function];
        if (strcmp(function->name, name) != 0) {
            printf("error: line %ld: exit from %s, but the innermost call is to %s\n", number,
                   name, function->name);
            ok = false;
            break;
        }
        depth--;
        long long duration = time - call.start;
        function->self += duration - call.in_callees;
        function->running--;
        // A recursive call's time is already in the total of the call around it.
        if (function->running == 0) function->total += time - function->outermost_start;
        if (depth > 0) stack[depth - 1].in_callees += duration;
    }
    if (ok && depth > 0) {
        const char *name = functions[stack[depth - 1].function].name;
        printf("error: end of trace: %s is still running\n", name);
        ok = false;
    }
    free(line);
    return ok;
}

int main(void) {
    if (profile()) {
        if (function_count == 0) printf("no calls\n");
        qsort(functions, function_count, sizeof *functions, by_self_time);
        for (size_t i = 0; i < function_count; i++) {
            const struct function *f = &functions[i];
            printf("%s %lld %lld %lld\n", f->name, f->calls, f->total, f->self);
        }
    }
    for (size_t i = 0; i < function_count; i++) free(functions[i].name);
    free(functions);
    free(stack);
    return 0;
}
