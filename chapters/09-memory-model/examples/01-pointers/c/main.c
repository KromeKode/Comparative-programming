#include <stddef.h>
#include <stdio.h>

int main(void) {
    int x = 5;
    int *p = &x; // p holds the address of x
    printf("x = %d, *p = %d\n", x, *p);

    *p = 7; // writing through p changes x
    printf("after *p = 7: x = %d\n", x);

    // An array's name turns into a pointer to its first element.
    int scores[4] = {90, 85, 77, 68};
    int *q = scores;
    printf("q[2] = %d, *(q + 2) = %d\n", q[2], *(q + 2));

    // Adding 2 to an int pointer moves it 2 ints, which is 8 bytes.
    ptrdiff_t bytes = (char *)(q + 2) - (char *)q;
    printf("q + 2 is %td bytes past q\n", bytes);

    // sizeof on the array gives the whole array; on the pointer, just the address.
    printf("sizeof: x %zu, p %zu, scores %zu, q %zu\n", sizeof x, sizeof p,
           sizeof scores, sizeof q);

    // A pointer to a pointer: pp holds the address of p.
    int **pp = &p;
    **pp = 9;
    printf("after **pp = 9: x = %d\n", x);

    int *nowhere = NULL;
    printf("nowhere %s\n", nowhere == NULL ? "is NULL" : "points somewhere");
    return 0;
}
