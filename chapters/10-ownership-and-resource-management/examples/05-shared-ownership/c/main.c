#include <stdio.h>
#include <stdlib.h>

// Reference counting by hand: each node counts its owners and frees itself
// when the last one lets go. C++'s shared_ptr and Rust's Rc do this for you.
typedef struct Node {
    const char *name;
    int owners;
    struct Node *next; // an owning link: it counts as an owner
} Node;

Node *node_new(const char *name) {
    Node *n = malloc(sizeof *n);
    if (n == NULL) {
        exit(1);
    }
    n->name = name;
    n->owners = 1;
    n->next = NULL;
    printf("  make %s\n", name);
    return n;
}

Node *node_retain(Node *n) {
    n->owners++;
    return n;
}

void node_release(Node *n) {
    if (n == NULL || --n->owners > 0) {
        return;
    }
    printf("  drop %s\n", n->name);
    node_release(n->next);
    free(n);
}

int main(void) {
    printf("two owners:\n");
    Node *doc = node_new("doc");
    Node *also = node_retain(doc);
    printf("  owners: %d\n", doc->owners);
    node_release(doc);
    printf("  one owner let go, owners: %d\n", also->owners);
    node_release(also);

    printf("a cycle of owning links:\n");
    Node *a = node_new("a");
    Node *b = node_new("b");
    a->next = node_retain(b);
    b->next = node_retain(a);
    Node *watch = a; // not an owner: just a way to look
    node_release(a);
    node_release(b);
    printf("  both owners let go, and a and b still count %d owner each\n",
           watch->owners);
    printf("  break the cycle by hand:\n");
    Node *next = watch->next;
    watch->next = NULL;
    node_release(next);
    return 0;
}
