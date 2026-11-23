// Prints the lines that contain a pattern, from each file named on the command line, or
// from stdin. ../README.md says what it should print.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char USAGE[] = "usage: search [-icnv] PATTERN [FILE...]\n";
static const char STDIN_NAME[] = "(standard input)";

typedef struct {
    int ignore_case, count, numbers, invert;
} Options;

static int usage_error(const char *message, char letter) {
    fprintf(stderr, "search: %s", message);
    if (letter != '\0') fprintf(stderr, "%c", letter);
    fprintf(stderr, "\n%s", USAGE);
    return 2;
}

// Reads all of file. Returns NULL, with errno set, if it can't.
static char *read_all(FILE *file, size_t *size) {
    char *data = NULL;
    size_t length = 0, capacity = 0;
    for (;;) {
        if (length == capacity) {
            capacity = capacity == 0 ? 4096 : capacity * 2;
            char *bigger = realloc(data, capacity);
            if (bigger == NULL) break;  // errno is ENOMEM
            data = bigger;
        }
        size_t got = fread(data + length, 1, capacity - length, file);
        length += got;
        if (got == 0) break;  // the end, or an error, such as EISDIR for a folder
    }
    if (ferror(file) || length == capacity) {
        int error = errno;
        free(data);
        errno = error;
        return NULL;
    }
    *size = length;
    return data;
}

// A to Z made a to z, and every other byte as it was. tolower would depend on the
// locale, and could change bytes above 127.
static void lower(char *bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (bytes[i] >= 'A' && bytes[i] <= 'Z') bytes[i] = (char)(bytes[i] - 'A' + 'a');
    }
}

// Whether the pattern is somewhere in the bytes. strstr would stop at a '\0'.
static int contains(const char *bytes, size_t length, const char *pattern, size_t size) {
    for (size_t i = 0; i + size <= length; i++) {
        if (memcmp(bytes + i, pattern, size) == 0) return 1;
    }
    return 0;
}

// Writes the lines of data that are selected, and returns how many there are. With
// -i, data and pattern have been lowercased, and original has the bytes to print.
static long search(const char *original, const char *data, size_t size, const char *name,
                   const char *pattern, size_t pattern_size, Options options) {
    long count = 0, number = 0;
    for (size_t start = 0; start < size;) {
        const char *newline = memchr(data + start, '\n', size - start);
        size_t end = newline != NULL ? (size_t)(newline - data) : size;
        number++;
        int found = contains(data + start, end - start, pattern, pattern_size);
        if (found != options.invert) {
            count++;
            if (!options.count) {
                if (name != NULL) printf("%s:", name);
                if (options.numbers) printf("%ld:", number);
                fwrite(original + start, 1, end - start, stdout);
                putchar('\n');
            }
        }
        start = end + 1;
    }
    if (options.count) {
        if (name != NULL) printf("%s:", name);
        printf("%ld\n", count);
    }
    return count;
}

int main(int argc, char **argv) {
    Options options = {0};
    int i = 1;
    // Options come first; the first argument that isn't one is the pattern.
    for (; i < argc && argv[i][0] == '-' && argv[i][1] != '\0'; i++) {
        if (strcmp(argv[i], "--") == 0) {
            i++;
            break;
        }
        for (const char *letter = argv[i] + 1; *letter != '\0'; letter++) {
            switch (*letter) {
            case 'i': options.ignore_case = 1; break;
            case 'c': options.count = 1; break;
            case 'n': options.numbers = 1; break;
            case 'v': options.invert = 1; break;
            default: return usage_error("unknown option -", *letter);
            }
        }
    }
    if (i == argc) return usage_error("no pattern", '\0');
    char *pattern = argv[i++];  // argv's strings belong to the program, to change
    size_t pattern_size = strlen(pattern);
    if (options.ignore_case) lower(pattern, pattern_size);
    char *stdin_only[] = {"-"};
    char **files = i < argc ? argv + i : stdin_only;
    int file_count = i < argc ? argc - i : 1;

    long selected = 0;
    int status = 0;
    for (int f = 0; f < file_count; f++) {
        int is_stdin = strcmp(files[f], "-") == 0;
        FILE *file = is_stdin ? stdin : fopen(files[f], "rb");
        size_t size = 0;
        char *data = file != NULL ? read_all(file, &size) : NULL;
        int error = errno;
        if (file != NULL && !is_stdin) fclose(file);
        if (data == NULL) {
            fflush(stdout);  // what came before, before the error
            fprintf(stderr, "search: %s: %s\n", files[f], strerror(error));
            status = 2;
            continue;
        }
        char *text = data;  // the bytes to search in: a lowercase copy, with -i
        if (options.ignore_case && (text = malloc(size + 1)) != NULL) {
            memcpy(text, data, size);
            lower(text, size);
        }
        if (text == NULL) {
            fputs("search: out of memory\n", stderr);
            return 2;
        }
        const char *name = file_count == 1 ? NULL : is_stdin ? STDIN_NAME : files[f];
        selected += search(data, text, size, name, pattern, pattern_size, options);
        if (text != data) free(text);
        free(data);
    }
    if (status == 0 && selected == 0) status = 1;
    return status;
}
