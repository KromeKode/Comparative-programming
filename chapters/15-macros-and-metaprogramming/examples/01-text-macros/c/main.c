#include <stdio.h>

// An object-like macro. Before the compiler sees the code, the preprocessor
// pastes 3 wherever the name LIMIT appears.
#define LIMIT 3

// Function-like macros paste in the text of their arguments, not the values.
#define SQUARE_BAD(x) x * x
#define SQUARE(x) ((x) * (x))

// #expr turns the argument's text into a string literal, and __LINE__ is the
// line the macro is used on, so SHOW can print the code it was given.
#define SHOW(expr) printf("line %d: %s = %d\n", __LINE__, #expr, (expr))

// ## joins two tokens into one. Each use of DEFINE_COUNTER makes a variable
// and a function, with names built from the one it is given.
#define DEFINE_COUNTER(name)     \
    static int name##_count = 0; \
    static int bump_##name(void) { return ++name##_count; }

DEFINE_COUNTER(clicks)
DEFINE_COUNTER(views)

// __VA_ARGS__ stands for every argument after the format.
#define LOG(format, ...) printf("log: " format "\n", __VA_ARGS__)

// The build can set this one: cc -DVERBOSE=1 main.c
#ifndef VERBOSE
#define VERBOSE 0
#endif

int main(void) {
    SHOW(LIMIT * 2);
    SHOW(SQUARE_BAD(1 + 2));  // becomes 1 + 2 * 1 + 2
    SHOW(SQUARE(1 + 2));      // becomes ((1 + 2) * (1 + 2))

    // The argument is pasted in twice, so the call in it runs twice.
    SHOW(SQUARE(bump_clicks()));
    SHOW(clicks_count);

    bump_views();
    LOG("%d clicks, %d view", clicks_count, views_count);

#if VERBOSE
    LOG("built with VERBOSE=%d", VERBOSE);
#else
    printf("VERBOSE is off, so the line that logs it was never compiled\n");
#endif
    return 0;
}
