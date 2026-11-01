#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_WORDS 128
#define MAX_PLUGINS 64
#define MAX_TEXT 128

// A plugin changes the words in place. Every plugin starts with this struct.
struct plugin;
struct plugin_vtable {
    const char *name;
    void (*apply)(const struct plugin *self, char **words, int count);
    // Writes the plugin's description. NULL means "use the name".
    void (*describe)(const struct plugin *self, char *out, size_t size);
};

struct plugin {
    const struct plugin_vtable *vtable;
};

void describe(const struct plugin *plugin, char *out, size_t size) {
    if (plugin->vtable->describe != NULL) {
        plugin->vtable->describe(plugin, out, size);
    } else {
        snprintf(out, size, "%s", plugin->vtable->name);
    }
}

void upper_apply(const struct plugin *self, char **words, int count) {
    (void)self;
    for (int i = 0; i < count; i++) {
        for (char *c = words[i]; *c != '\0'; c++) {
            *c = (char)toupper((unsigned char)*c);
        }
    }
}

void lower_apply(const struct plugin *self, char **words, int count) {
    (void)self;
    for (int i = 0; i < count; i++) {
        for (char *c = words[i]; *c != '\0'; c++) {
            *c = (char)tolower((unsigned char)*c);
        }
    }
}

void reverse_apply(const struct plugin *self, char **words, int count) {
    (void)self;
    for (int i = 0, j = count - 1; i < j; i++, j--) {
        char *word = words[i];
        words[i] = words[j];
        words[j] = word;
    }
}

struct censor {
    struct plugin base;
    char word[MAX_TEXT];
};

int same_ignoring_case(const char *a, const char *b) {
    for (; *a != '\0' && *b != '\0'; a++, b++) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
    }
    return *a == *b;
}

void censor_apply(const struct plugin *self, char **words, int count) {
    const struct censor *censor = (const struct censor *)self;
    for (int i = 0; i < count; i++) {
        if (same_ignoring_case(words[i], censor->word)) {
            memset(words[i], '*', strlen(words[i]));
        }
    }
}

void censor_describe(const struct plugin *self, char *out, size_t size) {
    snprintf(out, size, "censor(%s)", ((const struct censor *)self)->word);
}

static const struct plugin_vtable upper_vtable = {"upper", upper_apply, NULL};
static const struct plugin_vtable lower_vtable = {"lower", lower_apply, NULL};
static const struct plugin_vtable reverse_vtable = {"reverse", reverse_apply,
                                                    NULL};
static const struct plugin_vtable censor_vtable = {"censor", censor_apply,
                                                   censor_describe};

// A registry entry says how to make one kind of plugin. make returns NULL
// and writes why into error if the arguments are wrong.
struct entry {
    const char *usage;
    const char *help;
    const struct plugin_vtable *vtable;
    struct plugin *(*make)(const struct entry *entry, char **args, int count,
                           char *error, size_t size);
};

void *allocate(size_t size) {
    void *memory = calloc(1, size);
    if (memory == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return memory;
}

struct plugin *make_simple(const struct entry *entry, char **args, int count,
                           char *error, size_t size) {
    (void)args;
    if (count > 0) {
        snprintf(error, size, "%s takes no arguments", entry->vtable->name);
        return NULL;
    }
    struct plugin *plugin = allocate(sizeof *plugin);
    plugin->vtable = entry->vtable;
    return plugin;
}

struct plugin *make_censor(const struct entry *entry, char **args, int count,
                           char *error, size_t size) {
    if (count != 1) {
        snprintf(error, size, "censor needs exactly one word");
        return NULL;
    }
    struct censor *censor = allocate(sizeof *censor);
    censor->base.vtable = entry->vtable;
    snprintf(censor->word, sizeof censor->word, "%s", args[0]);
    return &censor->base;
}

// The rest of the program finds plugins only through this table.
static const struct entry registry[] = {
    {"upper", "make every letter uppercase", &upper_vtable, make_simple},
    {"lower", "make every letter lowercase", &lower_vtable, make_simple},
    {"reverse", "reverse the order of the words", &reverse_vtable,
     make_simple},
    {"censor WORD", "hide WORD wherever it appears", &censor_vtable,
     make_censor},
};
#define REGISTRY_SIZE (int)(sizeof registry / sizeof registry[0])

struct plugin *create(const char *name, char **args, int count, char *error,
                      size_t size) {
    for (int i = 0; i < REGISTRY_SIZE; i++) {
        if (strcmp(registry[i].vtable->name, name) == 0) {
            return registry[i].make(&registry[i], args, count, error, size);
        }
    }
    snprintf(error, size, "no plugin named %s", name);
    return NULL;
}

int main(void) {
    struct plugin *pipeline[MAX_PLUGINS];
    int plugins = 0;
    char line[MAX_LINE];
    char text[MAX_TEXT + 16];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *words[MAX_WORDS];
        int count = 0;
        for (char *word = strtok(line, " \t\r\n");
             word != NULL && count < MAX_WORDS;
             word = strtok(NULL, " \t\r\n")) {
            words[count++] = word;
        }
        if (count == 0) {
            continue;
        }
        if (count == 1 && strcmp(words[0], "list") == 0) {
            for (int i = 0; i < REGISTRY_SIZE; i++) {
                printf("%s: %s\n", registry[i].usage, registry[i].help);
            }
        } else if (count >= 2 && strcmp(words[0], "add") == 0) {
            struct plugin *plugin =
                create(words[1], words + 2, count - 2, text, sizeof text);
            if (plugin == NULL) {
                printf("error: %s\n", text);
            } else if (plugins == MAX_PLUGINS) {
                printf("error: too many plugins\n");
                free(plugin);
            } else {
                pipeline[plugins++] = plugin;
                describe(plugin, text, sizeof text);
                printf("added %s\n", text);
            }
        } else if (count == 1 && strcmp(words[0], "pipeline") == 0) {
            printf("pipeline: %s", plugins == 0 ? "(empty)" : "");
            for (int i = 0; i < plugins; i++) {
                describe(pipeline[i], text, sizeof text);
                printf("%s%s", i > 0 ? " | " : "", text);
            }
            printf("\n");
        } else if (strcmp(words[0], "run") == 0) {
            for (int i = 0; i < plugins; i++) {
                pipeline[i]->vtable->apply(pipeline[i], words + 1, count - 1);
            }
            for (int i = 1; i < count; i++) {
                printf("%s%s", i > 1 ? " " : "", words[i]);
            }
            printf("\n");
        }
    }
    for (int i = 0; i < plugins; i++) {
        free(pipeline[i]);
    }
    return 0;
}
