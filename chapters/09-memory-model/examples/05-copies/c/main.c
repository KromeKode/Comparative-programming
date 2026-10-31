#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The characters are inside the struct, so copying the struct copies them.
typedef struct {
    char name[16];
    int score;
} Inline;

// Only an address is inside the struct. Copying the struct copies the address,
// so both copies share one set of characters: a shallow copy.
typedef struct {
    char *name;
    int score;
} Pointing;

// A new heap copy of a string, or NULL if there is no memory.
static char *copy_string(const char *s) {
    size_t size = strlen(s) + 1; // + 1 for the '\0' at the end
    char *copy = malloc(size);
    if (copy != NULL) {
        memcpy(copy, s, size);
    }
    return copy;
}

int main(void) {
    Inline a = {"Ada", 90};
    Inline b = a;
    b.name[0] = 'E';
    b.score = 70;
    printf("inline array: a = %s %d, b = %s %d\n", a.name, a.score, b.name, b.score);

    Pointing c = {copy_string("Ada"), 90};
    if (c.name == NULL) {
        return 1;
    }
    Pointing d = c; // copies the pointer, not the characters
    d.name[0] = 'E';
    d.score = 70;
    printf("shallow copy: c = %s %d, d = %s %d\n", c.name, c.score, d.name, d.score);

    // A deep copy gives the new struct characters of its own.
    Pointing e = c;
    e.name = copy_string(c.name);
    if (e.name == NULL) {
        return 1;
    }
    e.name[0] = 'I';
    printf("deep copy: c = %s, e = %s\n", c.name, e.name);

    free(e.name);
    free(c.name); // c and d shared this block, so it is freed once, not twice
    return 0;
}
