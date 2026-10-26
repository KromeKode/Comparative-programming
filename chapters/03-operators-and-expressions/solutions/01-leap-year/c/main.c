#include <stdbool.h>
#include <stdio.h>

static bool is_leap(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int main(void) {
    int year;
    while (scanf("%d", &year) == 1) {  // scanf skips blank lines for us
        printf("%d: %s\n", year, is_leap(year) ? "leap year" : "common year");
    }
    return 0;
}
