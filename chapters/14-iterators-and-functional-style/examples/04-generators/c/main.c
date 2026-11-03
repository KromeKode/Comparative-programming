#include <stdbool.h>
#include <stdio.h>

// C has no generators, so these are written the other way round: they call
// visit with each value, and stop when visit returns false. context carries
// whatever state the visitor needs.
typedef bool (*visit_fn)(long value, void *context);

static void fibonacci(visit_fn visit, void *context) {
    long a = 0, b = 1;
    while (visit(a, context)) {
        long next = a + b;
        a = b;
        b = next;
    }
}

struct node {
    int value;
    const struct node *left, *right;
};

// Returns whether to keep going, so a false from visit stops every level of
// the recursion at once.
static bool in_order(const struct node *node, visit_fn visit, void *context) {
    return !node || (in_order(node->left, visit, context) && visit(node->value, context) &&
                     in_order(node->right, visit, context));
}

static bool print_some(long value, void *context) {
    int *left = context;  // how many more to print
    if (*left == 0) return false;
    printf(" %ld", value);
    (*left)--;
    return true;
}

static bool print_up_to(long value, void *context) {
    const long *limit = context;
    if (value > *limit) return false;
    printf(" %ld", value);
    return true;
}

int main(void) {
    const struct node one = {1, NULL, NULL}, three = {3, NULL, NULL};
    const struct node five = {5, NULL, NULL}, seven = {7, NULL, NULL};
    const struct node two = {2, &one, &three}, six = {6, &five, &seven};
    const struct node tree = {4, &two, &six};

    int left = 10;
    printf("fibonacci:");
    fibonacci(print_some, &left);

    left = 7;
    printf("\nin order:");
    in_order(&tree, print_some, &left);

    long limit = 3;  // print_up_to returns false at 4, so 5, 6, and 7 are never visited
    printf("\nup to 3:");
    in_order(&tree, print_up_to, &limit);
    printf("\n");
    return 0;
}
