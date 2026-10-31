#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C cannot move anything. "Moving" ownership means copying a pointer and
// agreeing that the old place no longer owns the block. Setting the old
// pointer to NULL turns that agreement into something the code can check.

typedef struct {
    char *text; // owned, or NULL
} Message;

const char *show(const Message *m) {
    return m->text == NULL ? "(nothing)" : m->text;
}

// Moves the text out of from and into to, freeing whatever to held before.
void message_move(Message *to, Message *from) {
    if (to == from) {
        return;
    }
    free(to->text);
    to->text = from->text;
    from->text = NULL;
}

int main(void) {
    Message a = {malloc(6)};
    Message b = {NULL};
    if (a.text == NULL) {
        return 1;
    }
    strcpy(a.text, "hello");

    // b = a would copy the pointer and leave two owners, and freeing both
    // would free the block twice.
    message_move(&b, &a);
    printf("after message_move(&b, &a): a = %s, b = %s\n", show(&a), show(&b));

    // Each Message frees what it holds. a holds NULL, and free(NULL) does
    // nothing, so the block is freed exactly once.
    free(a.text);
    free(b.text);
    printf("freed both, and the block was freed once\n");
    return 0;
}
