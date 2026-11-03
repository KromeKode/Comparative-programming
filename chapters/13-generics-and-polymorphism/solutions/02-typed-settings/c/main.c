#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 512
#define MAX_TEXT 256

// Text and tags are stored as fixed-size text: tags as words with one space
// between them.
struct text {
    char chars[MAX_TEXT];
};

// One setting. C has no generics, so the value is a void * that points at a
// typed variable below, and the functions that read, show, and compare it
// know its real type. Nothing checks that they agree with each other.
struct setting {
    const char *name;
    const char *needs;
    void *value;
    const void *def;
    size_t size;
    bool (*parse)(const char *text, void *out);
    void (*show)(const void *value, char *out, size_t size);
    bool (*equal)(const void *a, const void *b);
};

// The typed variables. summary reads them directly, with no casts.
static int volume = 5;
static const int volume_default = 5;
static bool muted = false;
static const bool muted_default = false;
static struct text name = {"guest"};
static const struct text name_default = {"guest"};
static struct text tags = {""};
static const struct text tags_default = {""};

static bool parse_int(const char *text, void *out) {
    const char *digits = text[0] == '-' ? text + 1 : text;
    if (*digits == '\0' || strspn(digits, "0123456789") != strlen(digits)) return false;
    errno = 0;
    long long value = strtoll(text, NULL, 10);
    if (errno != 0 || value < INT_MIN || value > INT_MAX) return false;
    *(int *)out = (int)value;
    return true;
}

static void show_int(const void *value, char *out, size_t size) {
    snprintf(out, size, "%d", *(const int *)value);
}

static bool equal_ints(const void *a, const void *b) { return *(const int *)a == *(const int *)b; }

static bool parse_bool(const char *text, void *out) {
    if (strcmp(text, "true") != 0 && strcmp(text, "false") != 0) return false;
    *(bool *)out = strcmp(text, "true") == 0;
    return true;
}

static void show_bool(const void *value, char *out, size_t size) {
    snprintf(out, size, "%s", *(const bool *)value ? "true" : "false");
}

static bool equal_bools(const void *a, const void *b) {
    return *(const bool *)a == *(const bool *)b;
}

static bool parse_text(const char *text, void *out) {
    snprintf(((struct text *)out)->chars, MAX_TEXT, "%s", text);
    return true;
}

static void show_text(const void *value, char *out, size_t size) {
    snprintf(out, size, "%s", ((const struct text *)value)->chars);
}

// memcmp would also compare the unused bytes after the end of the text.
static bool equal_texts(const void *a, const void *b) {
    return strcmp(((const struct text *)a)->chars, ((const struct text *)b)->chars) == 0;
}

// Each space between words becomes ", ".
static void show_tags(const void *value, char *out, size_t size) {
    char shown[2 * MAX_TEXT + 2];
    size_t used = 0;
    shown[used++] = '[';
    for (const char *c = ((const struct text *)value)->chars; *c; c++) {
        if (*c == ' ') shown[used++] = ',';
        shown[used++] = *c;
    }
    shown[used++] = ']';
    shown[used] = '\0';
    snprintf(out, size, "%s", shown);
}

static struct setting settings[] = {
    {"volume", "a whole number", &volume, &volume_default, sizeof volume, parse_int, show_int,
     equal_ints},
    {"muted", "true or false", &muted, &muted_default, sizeof muted, parse_bool, show_bool,
     equal_bools},
    {"name", "", &name, &name_default, sizeof name, parse_text, show_text, equal_texts},
    {"tags", "", &tags, &tags_default, sizeof tags, parse_text, show_tags, equal_texts},
};

#define SETTING_COUNT (sizeof settings / sizeof settings[0])

static struct setting *find(const char *wanted) {
    for (size_t i = 0; i < SETTING_COUNT; i++) {
        if (strcmp(settings[i].name, wanted) == 0) return &settings[i];
    }
    printf("error: no setting named \"%s\"\n", wanted);
    return NULL;
}

static void describe(const struct setting *setting, bool mark_default) {
    char shown[2 * MAX_TEXT + 2];
    setting->show(setting->value, shown, sizeof shown);
    bool is_default = mark_default && setting->equal(setting->value, setting->def);
    printf("%s = %s%s\n", setting->name, shown, is_default ? " (default)" : "");
}

int main(void) {
    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *words[MAX_LINE / 2];
        size_t count = 0;
        for (char *word = strtok(line, " \t\r\n"); word; word = strtok(NULL, " \t\r\n")) {
            words[count++] = word;
        }
        if (count == 0) continue;
        const char *command = words[0];
        if (strcmp(command, "list") == 0) {
            for (size_t i = 0; i < SETTING_COUNT; i++) describe(&settings[i], true);
        } else if (strcmp(command, "summary") == 0) {
            if (muted) {
                printf("%s has muted the sound\n", name.chars);
            } else {
                printf("%s hears the sound at %lld%%\n", name.chars, volume * 10LL);
            }
        } else {
            struct setting *setting = find(words[1]);
            if (setting == NULL) continue;
            if (strcmp(command, "set") == 0) {
                // Join the rest of the words with single spaces.
                char value[MAX_LINE] = "";
                for (size_t i = 2; i < count; i++) {
                    if (i > 2) strcat(value, " ");
                    strcat(value, words[i]);
                }
                if (!setting->parse(value, setting->value)) {
                    printf("error: %s needs %s, not \"%s\"\n", setting->name, setting->needs,
                           value);
                    continue;
                }
            } else if (strcmp(command, "reset") == 0) {
                memcpy(setting->value, setting->def, setting->size);
            }
            describe(setting, false);
        }
    }
    return 0;
}
