#include <stdio.h>
#include <string.h>

typedef struct {
    const char *name;
    const char *nickname; // NULL when the user has no nickname
} User;

// Returns a pointer to the user, or NULL if nobody has that name.
static const User *find(const User *users, size_t count, const char *name) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(users[i].name, name) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

int main(void) {
    const User users[] = {
        {"ada", "Countess"},
        {"alan", NULL},
        {"grace", "Amazing Grace"},
    };
    size_t count = sizeof users / sizeof users[0];

    // Every pointer that might be NULL has to be checked before it is used.
    for (size_t i = 0; i < count; i++) {
        const char *shown = users[i].nickname != NULL ? users[i].nickname : users[i].name;
        printf("%s is shown as %s\n", users[i].name, shown);
    }

    printf("nickname lengths:");
    for (size_t i = 0; i < count; i++) {
        printf(" %zu", users[i].nickname != NULL ? strlen(users[i].nickname) : 0);
    }
    printf("\n");

    const char *lookups[] = {"alan", "linus"};
    for (size_t i = 0; i < 2; i++) {
        const User *user = find(users, count, lookups[i]);
        printf("find \"%s\": %s\n", lookups[i], user != NULL ? user->name : "nobody");
    }

    const char *names[] = {"ada", "alan", "linus"};
    for (size_t i = 0; i < 3; i++) {
        const User *user = find(users, count, names[i]);
        const char *nickname = "none";
        if (user != NULL && user->nickname != NULL) {
            nickname = user->nickname;
        }
        printf("nickname of \"%s\": %s\n", names[i], nickname);
    }
    return 0;
}
