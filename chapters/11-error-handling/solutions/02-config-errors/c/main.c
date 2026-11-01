#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINES 1000
#define MAX_TEXT 128
#define MAX_ERRORS 64

enum kind { SYNTAX, UNKNOWN_KEY, DUPLICATE_KEY, BAD_VALUE, MISSING_KEY };
#define KIND_COUNT 5
static const char *kind_names[KIND_COUNT] = {
    "syntax", "unknown key", "duplicate key", "bad value", "missing key"};

// What was wrong with a value, for a BAD_VALUE error.
enum problem { NO_PROBLEM, EMPTY_TEXT, NOT_WHOLE, OUT_OF_RANGE, NOT_BOOL };

// One struct for every kind. Each kind uses only some of the fields.
struct config_error {
    enum kind kind;
    int line;
    char key[MAX_TEXT];
    int first;            // DUPLICATE_KEY: the line that set the key
    enum problem problem; // BAD_VALUE
    char text[MAX_TEXT];  // BAD_VALUE: the value as written
    long low, high;       // OUT_OF_RANGE
};

struct config {
    char name[MAX_TEXT];
    long port;
    bool debug;
    long timeout;
};

struct errors {
    int count;
    struct config_error list[MAX_ERRORS];
};

static const char *keys[] = {"name", "port", "debug", "timeout"};
#define KEY_COUNT 4

void print_error(const struct config_error *e) {
    switch (e->kind) {
    case SYNTAX:
        printf("line %d: expected \"key = value\"\n", e->line);
        break;
    case UNKNOWN_KEY:
        printf("line %d: unknown key \"%s\"\n", e->line, e->key);
        break;
    case DUPLICATE_KEY:
        printf("line %d: \"%s\" is already set on line %d\n", e->line, e->key,
               e->first);
        break;
    case BAD_VALUE:
        printf("line %d: %s: ", e->line, e->key);
        switch (e->problem) {
        case NO_PROBLEM:
            break;
        case EMPTY_TEXT:
            printf("must not be empty\n");
            break;
        case NOT_WHOLE:
            printf("\"%s\" is not a whole number\n", e->text);
            break;
        case OUT_OF_RANGE:
            printf("%s is not between %ld and %ld\n", e->text, e->low,
                   e->high);
            break;
        case NOT_BOOL:
            printf("\"%s\" is not true or false\n", e->text);
            break;
        }
        break;
    case MISSING_KEY:
        printf("missing key \"%s\"\n", e->key);
        break;
    }
}

// Copy text without the spaces at either end.
void trim_copy(char *out, const char *start, const char *end) {
    while (start < end && *start == ' ') {
        start++;
    }
    while (end > start && end[-1] == ' ') {
        end--;
    }
    snprintf(out, MAX_TEXT, "%.*s", (int)(end - start), start);
}

// Each value parser returns NO_PROBLEM and sets *out, or returns a problem.
enum problem parse_whole(const char *text, long low, long high, long *out) {
    if (text[0] == '\0') {
        return NOT_WHOLE;
    }
    long value = 0;
    for (const char *c = text; *c != '\0'; c++) {
        if (*c < '0' || *c > '9') {
            return NOT_WHOLE;
        }
        // Stop growing past the limit, so a long number cannot overflow.
        value = value * 10 + (*c - '0');
        if (value > high) {
            value = high + 1;
        }
    }
    if (value < low || value > high) {
        return OUT_OF_RANGE;
    }
    *out = value;
    return NO_PROBLEM;
}

enum problem parse_bool(const char *text, bool *out) {
    if (strcmp(text, "true") != 0 && strcmp(text, "false") != 0) {
        return NOT_BOOL;
    }
    *out = strcmp(text, "true") == 0;
    return NO_PROBLEM;
}

struct config_error *add_error(struct errors *errors, enum kind kind,
                               int line, const char *key) {
    struct config_error *e = &errors->list[errors->count++];
    memset(e, 0, sizeof *e);
    e->kind = kind;
    e->line = line;
    snprintf(e->key, sizeof e->key, "%s", key);
    return e;
}

// Returns true and fills in *config, or false with every problem in *errors.
bool load(char lines[][MAX_TEXT], int count, struct config *config,
          struct errors *errors) {
    *config = (struct config){"", 0, false, 30};
    int first_line[KEY_COUNT] = {0};
    errors->count = 0;
    for (int i = 0; i < count && errors->count < MAX_ERRORS - 2; i++) {
        int number = i + 1;
        char whole[MAX_TEXT], key[MAX_TEXT], text[MAX_TEXT];
        const char *line = lines[i];
        trim_copy(whole, line, line + strlen(line));
        if (whole[0] == '\0' || whole[0] == '#') {
            continue;
        }
        const char *equals = strchr(whole, '=');
        if (equals == NULL) {
            add_error(errors, SYNTAX, number, "");
            continue;
        }
        trim_copy(key, whole, equals);
        trim_copy(text, equals + 1, whole + strlen(whole));
        if (key[0] == '\0') {
            add_error(errors, SYNTAX, number, "");
            continue;
        }
        int k = 0;
        while (k < KEY_COUNT && strcmp(keys[k], key) != 0) {
            k++;
        }
        if (k == KEY_COUNT) {
            add_error(errors, UNKNOWN_KEY, number, key);
            continue;
        }
        if (first_line[k] != 0) {
            add_error(errors, DUPLICATE_KEY, number, key)->first =
                first_line[k];
            continue;
        }
        first_line[k] = number;
        enum problem problem = NO_PROBLEM;
        long low = 1, high = k == 1 ? 65535 : 3600;
        if (k == 0) {
            problem = text[0] == '\0' ? EMPTY_TEXT : NO_PROBLEM;
            snprintf(config->name, sizeof config->name, "%s", text);
        } else if (k == 1) {
            problem = parse_whole(text, low, high, &config->port);
        } else if (k == 2) {
            problem = parse_bool(text, &config->debug);
        } else {
            problem = parse_whole(text, low, high, &config->timeout);
        }
        if (problem != NO_PROBLEM) {
            struct config_error *e = add_error(errors, BAD_VALUE, number, key);
            e->problem = problem;
            snprintf(e->text, sizeof e->text, "%s", text);
            e->low = low;
            e->high = high;
        }
    }
    for (int k = 0; k < 2; k++) {
        if (first_line[k] == 0) {
            add_error(errors, MISSING_KEY, 0, keys[k]);
        }
    }
    return errors->count == 0;
}

static char lines[MAX_LINES][MAX_TEXT];
static struct errors errors;

int main(void) {
    int total = 0;
    while (total < MAX_LINES && fgets(lines[total], MAX_TEXT, stdin) != NULL) {
        lines[total][strcspn(lines[total], "\n")] = '\0';
        total++;
    }
    int counts[KIND_COUNT] = {0};
    int files = 0, loaded = 0;
    int i = 0;
    while (i < total) {
        if (strncmp(lines[i], "== ", 3) != 0) {
            i++;
            continue;
        }
        const char *name = lines[i] + 3;
        int start = ++i;
        while (i < total && strncmp(lines[i], "== ", 3) != 0) {
            i++;
        }
        files++;
        struct config config;
        if (load(&lines[start], i - start, &config, &errors)) {
            loaded++;
            printf("%s: ok\n", name);
            printf("  name = %s\n", config.name);
            printf("  port = %ld\n", config.port);
            printf("  debug = %s\n", config.debug ? "true" : "false");
            printf("  timeout = %ld\n", config.timeout);
            continue;
        }
        printf("%s: %d error%s\n", name, errors.count,
               errors.count == 1 ? "" : "s");
        for (int e = 0; e < errors.count; e++) {
            printf("  ");
            print_error(&errors.list[e]);
            counts[errors.list[e].kind]++;
        }
    }
    printf("files loaded: %d of %d\n", loaded, files);
    for (int k = 0; k < KIND_COUNT; k++) {
        printf("%s: %d%s", kind_names[k], counts[k],
               k + 1 < KIND_COUNT ? ", " : "\n");
    }
    return 0;
}
