// Exercise 18.3: version ranges. The highest version that each range allows.
//
// The versions and ranges are semver.c's business; this file reads the input and
// prints the answers.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semver.h"

static void out_of_memory(void) {
    fprintf(stderr, "out of memory\n");
    exit(1);
}

// All of standard input, with a '\0' after it. The versions point into it, so it
// stays until the program ends.
static char *read_all(void) {
    size_t length = 0, capacity = 4096;
    char *text = malloc(capacity);
    if (text == NULL) out_of_memory();
    size_t got;
    while ((got = fread(text + length, 1, capacity - length - 1, stdin)) > 0) {
        length += got;
        if (length + 1 == capacity) {
            capacity *= 2;
            text = realloc(text, capacity);
            if (text == NULL) out_of_memory();
        }
    }
    text[length] = '\0';
    return text;
}

static int is_space(char c) { return c == ' ' || (c >= '\t' && c <= '\r'); }

int main(void) {
    char *text = read_all();
    if (*text == '\0') {
        free(text);
        return 0;
    }
    // Each line ends at a '\n', which becomes a '\0'.
    char *first = text;
    char *rest = strchr(text, '\n');
    if (rest != NULL) *rest++ = '\0';

    Version *versions = NULL;
    size_t count = 0;
    for (char *word = first; *word != '\0';) {
        if (is_space(*word)) {
            word++;
            continue;
        }
        char *end = word;
        while (*end != '\0' && !is_space(*end)) end++;
        Version *grown = realloc(versions, (count + 1) * sizeof(Version));
        if (grown == NULL) out_of_memory();
        versions = grown;
        if (!version_parse(word, (size_t)(end - word), &versions[count])) {
            printf("error: line 1: %.*s isn't a version\n", (int)(end - word), word);
            free(versions);
            free(text);
            return 0;
        }
        count++;
        word = end;
    }
    if (count == 0) {
        printf("error: line 1 must list the versions\n");
        free(text);
        return 0;
    }

    for (int number = 2; rest != NULL && *rest != '\0'; number++) {
        char *line = rest;
        rest = strchr(rest, '\n');
        if (rest != NULL) *rest++ = '\0';
        // Trim the spaces at each end.
        while (is_space(*line)) line++;
        char *end = line + strlen(line);
        while (end > line && is_space(end[-1])) end--;
        *end = '\0';
        if (*line == '\0') continue;
        char *error;
        Range *range = range_parse(line, &error);
        if (range == NULL) {
            if (error == NULL) out_of_memory();
            printf("error: line %d: %s\n", number, error);
            free(error);
            continue;
        }
        const Version *best = range_highest(range, versions, count);
        printf("%s -> ", line);
        if (best != NULL) {
            version_print(best, stdout);
        } else {
            printf("none");
        }
        printf("\n");
        range_free(range);
    }
    free(versions);
    free(text);
    return 0;
}
