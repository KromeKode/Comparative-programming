#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

// C has no default or named arguments, so every caller passes every argument.
static void greet(const char *name, const char *greeting, const char *punctuation) {
    printf("%s, %s%s\n", greeting, name, punctuation);
}

// A variadic function cannot tell how many arguments it got, so the caller says.
// Nothing checks that they really are ints: passing a double here is undefined behavior.
static int total(int count, ...) {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += va_arg(args, int);
    }
    va_end(args);
    return sum;
}

// For an existing array, C code passes a pointer and a length instead.
static int total_of(const int *values, size_t count) {
    int sum = 0;
    for (size_t i = 0; i < count; i++) {
        sum += values[i];
    }
    return sum;
}

int main(void) {
    greet("Ada", "Hello", "!");
    greet("Grace", "Hi", "!");
    greet("Linus", "Hello", "?");

    printf("total() = %d\n", total(0));
    printf("total(1, 2, 3) = %d\n", total(3, 1, 2, 3));
    const int numbers[] = {4, 3, 2, 1};
    printf("total of [4, 3, 2, 1] = %d\n", total_of(numbers, 4));
    return 0;
}
