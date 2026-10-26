#include <math.h>
#include <stdio.h>
#include <string.h>

static const char *yes_no(int condition) {
    return condition ? "yes" : "no";
}

int main(void) {
    char first[] = "hello";
    char second[] = "hello";  // a second array holding the same characters
    const char *a = first;
    const char *b = second;
    // == on two pointers compares the addresses, not the characters.
    printf("strings with ==: %s\n", yes_no(a == b));
    printf("strings with strcmp: %s\n", yes_no(strcmp(a, b) == 0));

    int xs[] = {1, 2, 3};
    int ys[] = {1, 2, 3};
    // memcmp compares raw bytes. Fine for ints; not for structs with padding.
    printf("arrays with memcmp: %s\n", yes_no(memcmp(xs, ys, sizeof xs) == 0));

    double nan = NAN;
    printf("NaN == NaN: %s\n", yes_no(nan == nan));
    return 0;
}
