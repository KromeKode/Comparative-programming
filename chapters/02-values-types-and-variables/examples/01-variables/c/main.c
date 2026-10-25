#include <stdbool.h>
#include <stdio.h>

#define MAX_PLAYERS 4  // a macro: the preprocessor pastes 4 wherever it appears

int main(void) {
    int count = 0;                // variables can change unless marked const
    const int birth_year = 1815;  // assigning to it again is a compile error
    const double pi = 3.14159;
    bool ready = true;            // bool needs <stdbool.h> before C23

    count += 3;

    printf("count = %d\n", count);
    printf("birth year = %d\n", birth_year);
    printf("pi = %g\n", pi);
    printf("max players = %d\n", MAX_PLAYERS);
    printf("ready = %s\n", ready ? "true" : "false");  // printf has no format for bool
    return 0;
}
