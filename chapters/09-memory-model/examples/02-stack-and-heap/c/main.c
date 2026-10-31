#include <stdio.h>
#include <stdlib.h>

// Static data: one copy for the whole run, set up before main starts.
static int squares_made = 0;

// Each call gets its own stack frame with its own `here`. The frames stack up
// as the calls go deeper and are popped as each call returns.
static void countdown(int n) {
    int here = n * 10;
    if (n > 0) {
        countdown(n - 1);
    }
    printf(" %d", here);
}

// The squares go on the heap, so they outlive this call. The caller frees them.
static int *make_squares(size_t n) {
    int *squares = malloc(n * sizeof *squares);
    if (squares == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < n; i++) {
        squares[i] = (int)((i + 1) * (i + 1));
    }
    squares_made++;
    return squares;
}

int main(void) {
    printf("each frame keeps its own local:");
    countdown(3);
    printf("\n");

    int on_stack[1000] = {0}; // 4000 bytes inside main's frame
    printf("an array of 1000 ints on the stack: %zu bytes\n", sizeof on_stack);

    int *on_heap = make_squares(1000);
    if (on_heap == NULL) {
        return 1;
    }
    // sizeof sees only the pointer. The 4000 bytes it points to are elsewhere.
    printf("1000 squares on the heap: sizeof on_heap = %zu, on_heap[999] = %d\n",
           sizeof on_heap, on_heap[999]);
    free(on_heap); // give the memory back; on_heap now points at nothing usable
    on_heap = NULL;

    const char *literal = "hello"; // the characters are static data, read-only
    char copy[] = "hello";         // a writable copy of them on the stack
    copy[0] = 'j';
    printf("literal = %s, copy = %s, sizeof literal = %zu, sizeof copy = %zu\n", literal,
           copy, sizeof literal, sizeof copy);

    int *more = make_squares(3);
    if (more == NULL) {
        return 1;
    }
    printf("squares: %d %d %d\n", more[0], more[1], more[2]);
    free(more);
    printf("make_squares has run %d times\n", squares_made);
    return 0;
}
