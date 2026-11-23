// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
#define _POSIX_C_SOURCE 200809L  // for mkstemp, fchmod, and fsync, from POSIX
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    const char *start;
    size_t length;  // without the '\n'
} Line;

// Reads a whole file. Returns NULL, with errno set, if it can't.
static char *read_file(const char *path, size_t *size) {
    FILE *file = fopen(path, "rb");  // "b" for bytes, which only matters on Windows
    if (file == NULL) return NULL;
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
        if (got == 0) break;  // the end of the file, or an error, such as EISDIR
    }
    int error = ferror(file) || length == capacity ? errno : 0;
    fclose(file);
    if (error != 0) {
        free(data);
        errno = error;
        return NULL;
    }
    *size = length;
    return data;
}

// Writes the lines, each with a '\n', to a new file next to path, with the same
// permissions, and renames it over path. Returns -1, with errno set, if it can't.
static int replace_file(const char *path, const Line *lines, size_t count) {
    struct stat info;
    if (stat(path, &info) != 0) return -1;
    // mkstemp makes a file with a fresh name: it puts letters where the Xs are.
    char *temporary = malloc(strlen(path) + sizeof ".XXXXXX");
    if (temporary == NULL) return -1;
    sprintf(temporary, "%s.XXXXXX", path);
    int fd = mkstemp(temporary);
    if (fd < 0) {
        free(temporary);
        return -1;
    }
    FILE *file = fdopen(fd, "wb");
    int ok = file != NULL;
    for (size_t i = 0; ok && i < count; i++) {
        ok = fwrite(lines[i].start, 1, lines[i].length, file) == lines[i].length &&
             fputc('\n', file) != EOF;
    }
    ok = ok && fflush(file) == 0;  // from the FILE's buffer to the system
    ok = ok && fsync(fd) == 0;     // onto the disk before the rename
    ok = ok && fchmod(fd, info.st_mode & 07777) == 0;  // mkstemp gave it 0600
    int error = ok ? 0 : errno;
    // fclose closes fd too; it can fail, for a write the system held back.
    if ((file != NULL ? fclose(file) : close(fd)) != 0 && error == 0) error = errno;
    if (error == 0 && rename(temporary, path) != 0) error = errno;
    if (error != 0) remove(temporary);
    free(temporary);
    errno = error;
    return error == 0 ? 0 : -1;
}

// Rewrites the file without its repeated lines, and returns how many there were, or
// -1, with errno set.
static long dedup(const char *path) {
    size_t size;
    char *data = read_file(path, &size);
    if (data == NULL) return -1;
    // Split at each '\n'; a last line without one is still a line.
    Line *lines = malloc((size + 1) * sizeof *lines);
    if (lines == NULL) {
        free(data);
        return -1;
    }
    size_t count = 0;
    for (size_t start = 0; start < size;) {
        const char *newline = memchr(data + start, '\n', size - start);
        size_t end = newline != NULL ? (size_t)(newline - data) : size;
        lines[count++] = (Line){data + start, end - start};
        start = end + 1;
    }
    // Keep each line that no line before it matches. Comparing each with all the kept
    // ones is slow for a big file; a hash table, as in chapter 25, would be faster.
    size_t kept = 0;
    for (size_t i = 0; i < count; i++) {
        int repeated = 0;
        for (size_t j = 0; j < kept && !repeated; j++) {
            repeated = lines[j].length == lines[i].length &&
                       memcmp(lines[j].start, lines[i].start, lines[i].length) == 0;
        }
        if (!repeated) lines[kept++] = lines[i];
    }
    long removed = (long)(count - kept);
    if (removed > 0 && replace_file(path, lines, kept) != 0) removed = -1;
    int error = errno;
    free(lines);
    free(data);
    errno = error;
    return removed;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs("usage: dedup FILE...\n", stderr);
        return 2;
    }
    int status = 0;
    for (int i = 1; i < argc; i++) {
        long removed = dedup(argv[i]);
        if (removed < 0) {
            // strerror turns errno into the system's message, such as
            // "No such file or directory".
            fprintf(stderr, "dedup: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        printf("%s: removed %ld line%s\n", argv[i], removed, removed == 1 ? "" : "s");
    }
    return status;
}
