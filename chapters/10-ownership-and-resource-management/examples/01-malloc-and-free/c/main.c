#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no idea who owns a block of memory. The program says it in names and
// comments, and every function that allocates is paired with one that frees.

typedef struct {
    char *name;  // owned: player_free frees it
    int *scores; // owned
    size_t count;
} Player;

// Returns a copy of text that the caller owns, or NULL if memory ran out.
// (strdup does this too, but it is not in C17.)
char *copy_string(const char *text) {
    size_t size = strlen(text) + 1;
    char *copy = malloc(size);
    if (copy != NULL) {
        memcpy(copy, text, size);
    }
    return copy;
}

// Returns a new player that the caller owns and must give to player_free, or
// NULL if memory ran out. If a later step fails, the earlier ones are undone.
Player *player_new(const char *name, size_t count) {
    Player *p = malloc(sizeof *p);
    if (p == NULL) {
        return NULL;
    }
    p->name = copy_string(name);
    if (p->name == NULL) {
        free(p);
        return NULL;
    }
    p->scores = calloc(count, sizeof *p->scores);
    if (p->scores == NULL) {
        free(p->name);
        free(p);
        return NULL;
    }
    p->count = count;
    return p;
}

// Frees p and everything it owns, in the reverse order of player_new.
// Like free, it does nothing when p is NULL.
void player_free(Player *p) {
    if (p == NULL) {
        return;
    }
    free(p->scores);
    free(p->name);
    free(p);
}

// Borrows p: reads it and keeps nothing.
int player_total(const Player *p) {
    int total = 0;
    for (size_t i = 0; i < p->count; i++) {
        total += p->scores[i];
    }
    return total;
}

// Takes ownership of name, which must come from malloc. The player frees it.
void player_rename(Player *p, char *name) {
    free(p->name);
    p->name = name;
}

// Returns a new string that the caller owns and must free.
char *greeting(const Player *p) {
    const char *hello = "Hello, ";
    size_t size = strlen(hello) + strlen(p->name) + 2;
    char *text = malloc(size);
    if (text != NULL) {
        snprintf(text, size, "%s%s!", hello, p->name);
    }
    return text;
}

int main(void) {
    Player *ada = player_new("Ada", 3);
    if (ada == NULL) {
        fputs("out of memory\n", stderr);
        return 1;
    }
    ada->scores[0] = 90;
    ada->scores[1] = 85;
    ada->scores[2] = 80;
    printf("%s has %zu scores, total %d\n", ada->name, ada->count,
           player_total(ada));

    // After the call the player owns the new name, so this function must not
    // free it or use it. Setting the pointer to NULL makes that hard to forget.
    char *name = copy_string("Ada Lovelace");
    if (name == NULL) {
        player_free(ada);
        return 1;
    }
    player_rename(ada, name);
    name = NULL;
    printf("renamed to %s\n", ada->name);

    // The caller owns what greeting returns.
    char *hello = greeting(ada);
    if (hello != NULL) {
        printf("%s\n", hello);
        free(hello);
    }

    // A pointer into the player is borrowed: it is good until the player is
    // freed, and the compiler does not check that.
    const char *borrowed = ada->name;
    printf("borrowed name: %s\n", borrowed);

    player_free(ada);
    ada = NULL;
    // borrowed now dangles. Reading it is undefined behavior, so don't.

    player_free(ada); // ada is NULL now, so this does nothing
    printf("the second player_free(ada) freed nothing, because ada is NULL\n");
    return 0;
}
