#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Pretend files, so the example does not touch the disk.
static const struct {
    const char *name;
    const char *text;
} files[] = {{"good.txt", "1\n2\n3"}, {"bad.txt", "4\nfive\n6"}};

// Every function returns a status. On failure it also writes a message into
// the caller's error buffer.
enum status { OK, NOT_FOUND, NOT_A_NUMBER };

enum status read_file(const char *name, const char **text, char *error,
                      size_t size) {
    for (size_t i = 0; i < sizeof files / sizeof files[0]; i++) {
        if (strcmp(files[i].name, name) == 0) {
            *text = files[i].text;
            return OK;
        }
    }
    snprintf(error, size, "%s: no such file", name);
    return NOT_FOUND;
}

enum status parse_number(const char *text, long *value, char *error,
                         size_t size) {
    char *end;
    errno = 0;
    *value = strtol(text, &end, 10);
    if (end == text || *end != '\0' || errno == ERANGE) {
        snprintf(error, size, "\"%s\" is not a number", text);
        return NOT_A_NUMBER;
    }
    return OK;
}

enum status sum_file(const char *name, long *total, char *error,
                     size_t size) {
    const char *text;
    enum status status = read_file(name, &text, error, size);
    if (status != OK) {
        return status; // pass the error up unchanged
    }
    *total = 0;
    for (int line = 1;; line++) {
        size_t length = strcspn(text, "\n");
        char number[32];
        snprintf(number, sizeof number, "%.*s", (int)length, text);
        long value;
        char inner[64];
        status = parse_number(number, &value, inner, sizeof inner);
        if (status != OK) {
            // Add context: which file and which line.
            snprintf(error, size, "%s: line %d: %s", name, line, inner);
            return status;
        }
        *total += value;
        if (text[length] == '\0') {
            return OK;
        }
        text += length + 1;
    }
}

// A missing file counts as 0, but any other error stops the sum.
enum status sum_all(const char *names[], int count, long *total, char *error,
                    size_t size) {
    *total = 0;
    for (int i = 0; i < count; i++) {
        long value;
        enum status status = sum_file(names[i], &value, error, size);
        if (status == NOT_FOUND) {
            continue;
        }
        if (status != OK) {
            return status;
        }
        *total += value;
    }
    return OK;
}

void show(const char *label, enum status status, long total,
          const char *error) {
    if (status == OK) {
        printf("  %s -> %ld\n", label, total);
    } else {
        printf("  %s -> error: %s\n", label, error);
    }
}

int main(void) {
    char error[128];
    long total;

    printf("one file at a time:\n");
    const char *names[] = {"good.txt", "bad.txt", "missing.txt"};
    for (int i = 0; i < 3; i++) {
        enum status status = sum_file(names[i], &total, error, sizeof error);
        show(names[i], status, total, error);
    }

    printf("several files, where a missing file counts as 0:\n");
    const char *first[] = {"good.txt", "missing.txt"};
    enum status status = sum_all(first, 2, &total, error, sizeof error);
    show("good.txt + missing.txt", status, total, error);
    const char *second[] = {"good.txt", "bad.txt"};
    status = sum_all(second, 2, &total, error, sizeof error);
    show("good.txt + bad.txt", status, total, error);
    return 0;
}
