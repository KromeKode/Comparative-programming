#include <stdio.h>
#include <string.h>

// An X-macro. The list is written once, as uses of a macro X that isn't
// defined yet. Each use of the list below defines X, expands the list, and
// undefines X again, so each one turns the same entries into different code.
#define STATUSES(X)                    \
    X(OK, 200, "OK")                   \
    X(MOVED, 301, "Moved Permanently") \
    X(NOT_FOUND, 404, "Not Found")     \
    X(TEAPOT, 418, "I'm a teapot")     \
    X(SERVER_ERROR, 500, "Internal Server Error")

// 1. An enum, with each name given its code.
enum status {
#define X(name, code, text) STATUS_##name = code,
    STATUSES(X)
#undef X
};

// 2. A count: 0 +1 +1 +1 +1 +1.
enum {
#define X(name, code, text) +1
    STATUS_COUNT = 0 STATUSES(X)
#undef X
};

// 3. A table with every entry's name as a string.
static const struct {
    enum status status;
    const char *name;
} all_statuses[] = {
#define X(name, code, text) {STATUS_##name, #name},
    STATUSES(X)
#undef X
};

// 4. A switch with a case for every entry, so none can be left out.
static const char *status_text(enum status status) {
    switch (status) {
#define X(name, code, text) \
    case STATUS_##name:     \
        return text;
        STATUSES(X)
#undef X
    }
    return NULL;
}

static void lookup_code(int code) {
    const char *text = status_text((enum status)code);
    printf("lookup %d: %s\n", code, text ? text : "unknown");
}

static void lookup_name(const char *name) {
    for (size_t i = 0; i < STATUS_COUNT; i++) {
        if (strcmp(all_statuses[i].name, name) == 0) {
            printf("lookup %s: %d\n", name, (int)all_statuses[i].status);
            return;
        }
    }
    printf("lookup %s: unknown\n", name);
}

int main(void) {
    printf("%d statuses\n", STATUS_COUNT);
    for (size_t i = 0; i < STATUS_COUNT; i++) {
        enum status status = all_statuses[i].status;
        printf("%d %s: %s\n", (int)status, all_statuses[i].name, status_text(status));
    }
    lookup_code(404);
    lookup_code(999);
    lookup_name("TEAPOT");
    lookup_name("BREWING");
    return 0;
}
