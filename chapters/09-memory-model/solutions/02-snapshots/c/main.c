#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_SIDE 50
#define SPACES " \t\r\n"

// The cells are an array inside the struct, so `saved = grid` copies every
// cell. Nothing is shared, and there is nothing to free.
typedef struct {
    int width;
    int height;
    char cells[MAX_SIDE][MAX_SIDE];
} Grid;

// Reads a number made of digits into *out. A number too big for an unsigned
// long becomes ULONG_MAX, which is outside any grid.
static bool parse_number(const char *text, unsigned long *out) {
    for (const char *c = text; *c != '\0'; c++) {
        if (!isdigit((unsigned char)*c)) {
            printf("error: \"%s\" is not a number\n", text);
            return false;
        }
    }
    errno = 0;
    *out = strtoul(text, NULL, 10);
    if (errno == ERANGE) {
        *out = ULONG_MAX;
    }
    return true;
}

static bool one_character(const char *text) {
    if (strlen(text) != 1) {
        printf("error: \"%s\" is not one character\n", text);
        return false;
    }
    return true;
}

// Checks that a command got the right number of values.
static bool takes(const char *command, int want, int got) {
    if (got != want) {
        printf("error: %s takes %d value%s, got %d\n", command, want, want == 1 ? "" : "s",
               got);
        return false;
    }
    return true;
}

static void fill(Grid *grid, char c) {
    memset(grid->cells, c, sizeof grid->cells);
}

static void show(const Grid *grid) {
    for (int y = 0; y < grid->height; y++) {
        printf("%.*s\n", grid->width, grid->cells[y]);
    }
    printf("\n");
}

int main(void) {
    char line[MAX_LINE];
    Grid grid;
    if (fgets(line, sizeof line, stdin) == NULL) {
        return 0;
    }
    if (sscanf(line, "%d %d", &grid.width, &grid.height) != 2) {
        return 1;
    }
    fill(&grid, '.');

    Grid saved;
    bool has_saved = false;
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *parts[4];
        int n = 0;
        for (char *t = strtok(line, SPACES); t != NULL; t = strtok(NULL, SPACES)) {
            if (n < 4) {
                parts[n] = t;
            }
            n++;
        }
        if (n == 0) {
            continue;
        }
        const char *command = parts[0];
        int values = n - 1;
        if (strcmp(command, "paint") == 0) {
            unsigned long x, y;
            if (!takes(command, 3, values) || !parse_number(parts[1], &x) ||
                !parse_number(parts[2], &y)) {
                continue;
            }
            if (x < 1 || x > (unsigned long)grid.width || y < 1 ||
                y > (unsigned long)grid.height) {
                printf("error: (%s, %s) is outside the %d by %d grid\n", parts[1], parts[2],
                       grid.width, grid.height);
                continue;
            }
            if (one_character(parts[3])) {
                grid.cells[y - 1][x - 1] = parts[3][0];
            }
        } else if (strcmp(command, "fill") == 0) {
            if (takes(command, 1, values) && one_character(parts[1])) {
                fill(&grid, parts[1][0]);
            }
        } else if (strcmp(command, "save") == 0) {
            if (takes(command, 0, values)) {
                saved = grid; // copies the whole struct, cells and all
                has_saved = true;
            }
        } else if (strcmp(command, "restore") == 0) {
            if (!takes(command, 0, values)) {
                continue;
            }
            if (has_saved) {
                grid = saved;
            } else {
                printf("error: nothing saved\n");
            }
        } else if (strcmp(command, "show") == 0) {
            if (takes(command, 0, values)) {
                show(&grid);
            }
        } else {
            printf("error: unknown command \"%s\"\n", command);
        }
    }
    return 0;
}
