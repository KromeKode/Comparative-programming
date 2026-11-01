#include <stddef.h>
#include <stdio.h>

struct user {
    int id;
    const char *name;
};

static const struct user users[] = {{1, "ada"}};

const struct user *find_user(int id) {
    for (size_t i = 0; i < sizeof users / sizeof users[0]; i++) {
        if (users[i].id == id) {
            return &users[i];
        }
    }
    return NULL;
}

int main(void) {
    int numbers[] = {10, 20, 30};
    size_t length = sizeof numbers / sizeof numbers[0];
    size_t index = 5;
    int zero = 0;

    // C checks none of these at run time. Each one is undefined behavior, so
    // the program has to test before it acts.
    if (index < length) {
        printf("numbers[5]: %d\n", numbers[index]);
    } else {
        printf("numbers[5]: skipped, because 5 is not less than %zu\n",
               length);
    }

    if (zero != 0) {
        printf("10 / zero: %d\n", 10 / zero);
    } else {
        printf("10 / zero: skipped, because zero is 0\n");
    }

    const struct user *user = find_user(7);
    if (user != NULL) {
        printf("find_user(7)->name: %s\n", user->name);
    } else {
        printf("find_user(7)->name: skipped, because find_user returned "
               "NULL\n");
    }
    return 0;
}
