#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    long parsed = strtol("42", NULL, 10);  // text to number
    printf("\"42\" + 1 as a number: %ld\n", parsed + 1);

    double x = 3.99;
    printf("3.99 truncated: %d\n", (int)x);    // a cast drops the fraction
    printf("-3.99 truncated: %d\n", (int)-x);
    printf("2.5 rounded: %.0f\n", round(2.5));   // halfway rounds away from zero
    printf("-2.5 rounded: %.0f\n", round(-2.5));

    int seven = 7;
    int two = 2;
    printf("7 / 2 as integers: %d\n", seven / two);
    printf("7 / 2 as floats: %g\n", (double)seven / two);  // convert one side first
    return 0;
}
