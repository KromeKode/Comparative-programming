// Exercise 18.2: build order. The order to build modules in, and what to rebuild.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPACE " \t\r\n\v\f"

typedef struct {
    char *name;
    int line;
    char **import_names;  // as written, with repeats dropped
    int *imports;         // the same, as indexes into modules
    int import_count;
    int *users;           // the modules that import this one
    int user_count;
    int waiting;          // imports not built yet
    int position;         // where it comes in the build order
} Module;

typedef struct {
    int line;
    char *name;
} Query;

static Module *modules;
static int module_count;
static Query *queries;
static int query_count;

// Makes room for one more element after count of them, or stops the program if
// there is no memory.
static void *grow(void *array, int count, size_t size) {
    void *bigger = realloc(array, (size_t)(count + 1) * size);
    if (bigger == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return bigger;
}

static char *copy(const char *text) {
    size_t size = strlen(text) + 1;
    char *result = grow(NULL, 0, size);
    memcpy(result, text, size);
    return result;
}

// A whole line of any length, without its '\n', or NULL at the end of the input.
static char *read_line(void) {
    size_t length = 0, capacity = 128;
    char *line = grow(NULL, 0, capacity);
    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        if (length + 1 == capacity) {
            capacity *= 2;
            line = grow(line, 0, capacity);
        }
        line[length++] = (char)c;
    }
    if (c == EOF && length == 0) {
        free(line);
        return NULL;
    }
    line[length] = '\0';
    return line;
}

static bool is_name(const char *text) {
    const char *allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_./-";
    return *text != '\0' && strspn(text, allowed) == strlen(text);
}

static void free_module(Module *module) {
    free(module->name);
    for (int i = 0; i < module->import_count; i++) free(module->import_names[i]);
    free(module->import_names);
    free(module->imports);
    free(module->users);
}

// The program stops at the first problem it finds, from several places, so main
// registers this with atexit to run however the program ends.
static void free_all(void) {
    for (int i = 0; i < module_count; i++) free_module(&modules[i]);
    free(modules);
    for (int i = 0; i < query_count; i++) free(queries[i].name);
    free(queries);
}

static int find(const char *name) {
    for (int i = 0; i < module_count; i++) {
        if (strcmp(modules[i].name, name) == 0) return i;
    }
    return -1;
}

// Reads a line that has a colon in it, which defines a module. Returns false, after
// printing why, if it can't.
static bool define(char *line, int number) {
    char *colon = strchr(line, ':');
    *colon = '\0';
    char *name = strtok(line, SPACE);
    if (name == NULL || strtok(NULL, SPACE) != NULL || !is_name(name)) {
        printf("error: line %d: a module is NAME: IMPORTS\n", number);
        return false;
    }
    Module module = {.name = copy(name), .line = number};
    for (char *word = strtok(colon + 1, SPACE); word != NULL; word = strtok(NULL, SPACE)) {
        if (!is_name(word)) {
            printf("error: line %d: a module is NAME: IMPORTS\n", number);
            free_module(&module);
            return false;
        }
        bool repeat = false;
        for (int i = 0; i < module.import_count; i++) {
            if (strcmp(module.import_names[i], word) == 0) repeat = true;
        }
        if (repeat) continue;
        module.import_names = grow(module.import_names, module.import_count, sizeof(char *));
        module.import_names[module.import_count++] = copy(word);
    }
    int earlier = find(module.name);
    if (earlier >= 0) {
        printf("error: line %d: %s is already defined on line %d\n", number, module.name,
               modules[earlier].line);
        free_module(&module);
        return false;
    }
    modules = grow(modules, module_count, sizeof(Module));
    modules[module_count++] = module;
    return true;
}

static bool read_input(void) {
    char *line;
    for (int number = 1; (line = read_line()) != NULL; number++) {
        bool ok = true;
        if (strchr(line, ':') != NULL) {
            ok = define(line, number);
        } else {
            char *first = strtok(line, SPACE);
            char *second = first ? strtok(NULL, SPACE) : NULL;
            if (first != NULL && strcmp(first, "rebuild") == 0 && second != NULL &&
                strtok(NULL, SPACE) == NULL) {
                queries = grow(queries, query_count, sizeof(Query));
                queries[query_count++] = (Query){number, copy(second)};
            } else if (first != NULL) {
                printf("error: line %d: expected NAME: IMPORTS or rebuild NAME\n", number);
                ok = false;
            }
        }
        free(line);
        if (!ok) return false;
    }
    return true;
}

// Finds each import's module, and the users of each module.
static bool link_imports(void) {
    for (int i = 0; i < module_count; i++) {
        Module *module = &modules[i];
        module->imports = grow(NULL, module->import_count, sizeof(int));
        for (int j = 0; j < module->import_count; j++) {
            int k = find(module->import_names[j]);
            if (k < 0) {
                printf("error: line %d: %s imports %s, which isn't defined\n", module->line,
                       module->name, module->import_names[j]);
                return false;
            }
            module->imports[j] = k;
            modules[k].users = grow(modules[k].users, modules[k].user_count, sizeof(int));
            modules[k].users[modules[k].user_count++] = i;
        }
        module->waiting = module->import_count;
        module->position = -1;
    }
    return true;
}

// Kahn's algorithm: a module is ready once everything it imports is built, and of
// the ready modules, the first in alphabetical order goes next. This one looks
// through them all for the first, which is simple and fast enough for a few
// thousand modules; a heap would do it faster.
static int *build_order(int *built) {
    int *order = grow(NULL, module_count, sizeof(int));
    *built = 0;
    while (true) {
        int next = -1;
        for (int i = 0; i < module_count; i++) {
            if (modules[i].position < 0 && modules[i].waiting == 0 &&
                (next < 0 || strcmp(modules[i].name, modules[next].name) < 0)) {
                next = i;
            }
        }
        if (next < 0) return order;
        modules[next].position = *built;
        order[(*built)++] = next;
        for (int j = 0; j < modules[next].user_count; j++) {
            modules[modules[next].users[j]].waiting--;
        }
    }
}

// Every module left over imports another one left over, so following first imports
// from the first module left over must come back to a module.
static void print_cycle(void) {
    int start = -1;
    for (int i = 0; i < module_count; i++) {
        if (modules[i].waiting > 0 &&
            (start < 0 || strcmp(modules[i].name, modules[start].name) < 0)) {
            start = i;
        }
    }
    int *path = grow(NULL, module_count, sizeof(int));
    int *seen = grow(NULL, module_count, sizeof(int));  // where each is on the path
    for (int i = 0; i < module_count; i++) seen[i] = -1;
    int length = 0;
    int at = start;
    while (seen[at] < 0) {
        seen[at] = length;
        path[length++] = at;
        int next = -1;
        for (int j = 0; j < modules[at].import_count && next < 0; j++) {
            if (modules[modules[at].imports[j]].waiting > 0) next = modules[at].imports[j];
        }
        at = next;
    }
    printf("error: import cycle: ");
    for (int i = seen[at]; i < length; i++) printf("%s -> ", modules[path[i]].name);
    printf("%s\n", modules[at].name);
    free(path);
    free(seen);
}

static void print_rebuild(const Query *query, const int *order, int built) {
    int changed = find(query->name);
    if (changed < 0) {
        printf("error: line %d: no module called %s\n", query->line, query->name);
        return;
    }
    // Everything that imports the changed module, directly or not, comes after it
    // in the build order, so one pass through the order from there finds them all.
    bool *affected = calloc((size_t)module_count, sizeof(bool));
    if (affected == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    affected[changed] = true;
    printf("rebuild %s: %s", query->name, query->name);
    for (int i = modules[changed].position + 1; i < built; i++) {
        const Module *module = &modules[order[i]];
        for (int j = 0; j < module->import_count; j++) {
            if (affected[module->imports[j]]) affected[order[i]] = true;
        }
        if (affected[order[i]]) printf(", %s", module->name);
    }
    printf("\n");
    free(affected);
}

int main(void) {
    atexit(free_all);
    if (!read_input() || !link_imports()) return 0;
    int built;
    int *order = build_order(&built);
    if (built < module_count) {
        print_cycle();
        free(order);
        return 0;
    }
    printf("build order: ");
    for (int i = 0; i < built; i++) printf(i ? ", %s" : "%s", modules[order[i]].name);
    printf("%s\n", built ? "" : "nothing to build");
    for (int i = 0; i < query_count; i++) print_rebuild(&queries[i], order, built);
    free(order);
    return 0;
}
