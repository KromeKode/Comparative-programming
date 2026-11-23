// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
#define _POSIX_C_SOURCE 200809L  // for getline and strndup, from POSIX
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *key;  // the method, a space, and the route
    int requests, failed, slowest;
} Route;

// POSIX extended regular expressions have no \d and no named groups, so the groups are
// numbered: 1 is the date, 2 the time, 3 the method, 4 the path, 5 the status, and 6 the
// time taken. Group 0 is the whole match.
static const char LINE[] =
    "^([0-9]{4}-[0-9]{2}-[0-9]{2}) ([0-9]{2}:[0-9]{2}:[0-9]{2})"
    " (GET|POST|PUT|DELETE) (/[^ ]*)"
    " ([0-9]{3}) ([0-9]{1,9})ms$";
enum { GROUPS = 7, METHOD = 3, PATH = 4, STATUS = 5, MS = 6 };

static void *checked(void *p) {
    if (p == NULL) {
        fputs("logscan: out of memory\n", stderr);
        exit(1);
    }
    return p;
}

static regex_t compile(const char *pattern) {
    regex_t re;
    int err = regcomp(&re, pattern, REG_EXTENDED);
    if (err != 0) {
        char message[256];
        regerror(err, &re, message, sizeof message);
        fprintf(stderr, "logscan: %s: %s\n", pattern, message);
        exit(2);
    }
    return re;
}

// Returns "METHOD ROUTE", with each match of id in the path written "/:id". C has no
// replace, so this copies the text between the matches and "/:id" for each match.
static char *key_of(const char *line, const regmatch_t *m, const regex_t *id) {
    size_t method = (size_t)(m[METHOD].rm_eo - m[METHOD].rm_so);
    size_t path = (size_t)(m[PATH].rm_eo - m[PATH].rm_so);
    // "/:id" is at most twice as long as the "/" and digit or more that it replaces.
    char *key = checked(malloc(method + 1 + 2 * path + 1));
    char *out = key;
    memcpy(out, line + m[METHOD].rm_so, method);
    out += method;
    *out++ = ' ';
    // regexec reads up to a '\0', so the search needs the path on its own.
    char *rest = checked(strndup(line + m[PATH].rm_so, path));
    const char *at = rest;
    regmatch_t found;
    while (regexec(id, at, 1, &found, 0) == 0) {
        memcpy(out, at, (size_t)found.rm_so);
        out += found.rm_so;
        memcpy(out, "/:id", 4);
        out += 4;
        at += found.rm_eo;
    }
    strcpy(out, at);
    free(rest);
    return key;
}

int main(void) {
    regex_t line_re = compile(LINE), id = compile("/[0-9]+");
    // C has no map, so this is a list, searched from the start: fine for a few hundred
    // routes, and it keeps them in the order they came in.
    Route *routes = NULL;
    size_t count = 0, capacity = 0;
    int status = 0;
    char *line = NULL;
    size_t size = 0;
    ssize_t length;
    // getline keeps the '\n', and a '\r' before it.
    for (long number = 1; (length = getline(&line, &size, stdin)) != -1; number++) {
        if (length > 0 && line[length - 1] == '\n') line[--length] = '\0';
        if (length > 0 && line[length - 1] == '\r') line[--length] = '\0';
        regmatch_t m[GROUPS];
        // A '\0' in the line would end the search early, so it can't match.
        if (strlen(line) != (size_t)length || regexec(&line_re, line, GROUPS, m, 0) != 0) {
            fprintf(stderr, "logscan: line %ld: not a log line\n", number);
            status = 1;
            continue;
        }
        char *key = key_of(line, m, &id);
        size_t i = 0;
        while (i < count && strcmp(routes[i].key, key) != 0) i++;
        if (i < count) {
            free(key);
        } else {
            if (count == capacity) {
                capacity = capacity == 0 ? 16 : capacity * 2;
                routes = checked(realloc(routes, capacity * sizeof *routes));
            }
            routes[count++] = (Route){.key = key};
        }
        // The regex let through only digits, few enough to fit, and strtol stops at the
        // space or the "ms" after them.
        int code = (int)strtol(line + m[STATUS].rm_so, NULL, 10);
        int ms = (int)strtol(line + m[MS].rm_so, NULL, 10);
        routes[i].requests++;
        if (code >= 400) routes[i].failed++;
        if (ms > routes[i].slowest) routes[i].slowest = ms;
    }
    for (size_t i = 0; i < count; i++) {
        printf("%s: %d request%s, %d failed, slowest %dms\n", routes[i].key,
               routes[i].requests, routes[i].requests == 1 ? "" : "s", routes[i].failed,
               routes[i].slowest);
        free(routes[i].key);
    }
    free(routes);
    free(line);
    regfree(&line_re);
    regfree(&id);
    return status;
}
