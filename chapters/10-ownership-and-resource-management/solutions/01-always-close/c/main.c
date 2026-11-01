#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_OPEN 2
#define MAX_LINES 8
#define MAX_FILES 20
#define NAME_SIZE 32

typedef struct {
    char name[NAME_SIZE];
    char lines[MAX_LINES][NAME_SIZE];
    int count;
    bool locked;
} File;

typedef struct {
    File files[MAX_FILES];
    int file_count;
    int open_count;
    char error[100]; // the message of the last error
} Disk;

typedef struct {
    Disk *disk;
    File *file;
} Handle;

// Formats an error message into the disk and returns it.
const char *fail(Disk *disk, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(disk->error, sizeof disk->error, format, args);
    va_end(args);
    return disk->error;
}

File *find(Disk *disk, const char *name) {
    for (int i = 0; i < disk->file_count; i++) {
        if (strcmp(disk->files[i].name, name) == 0) {
            return &disk->files[i];
        }
    }
    return NULL;
}

int lines_used(const Disk *disk) {
    int total = 0;
    for (int i = 0; i < disk->file_count; i++) {
        total += disk->files[i].count;
    }
    return total;
}

// Opens a file. Returns NULL on success, and then the caller must call
// handle_close exactly once. Returns an error message if nothing was opened.
const char *handle_open(Handle *h, Disk *disk, const char *name, bool writing) {
    if (disk->open_count == MAX_OPEN) {
        return fail(disk, "too many open files");
    }
    File *file = find(disk, name);
    if (writing) {
        if (file != NULL && file->locked) {
            return fail(disk, "\"%s\" is read-only", name);
        }
        if (file == NULL) {
            file = &disk->files[disk->file_count++];
            snprintf(file->name, sizeof file->name, "%s", name);
            file->locked = false;
        }
        file->count = 0;
    } else if (file == NULL) {
        return fail(disk, "no file \"%s\"", name);
    }
    disk->open_count++;
    printf("open %s\n", name);
    h->disk = disk;
    h->file = file;
    return NULL;
}

void handle_close(Handle *h) {
    h->disk->open_count--;
    printf("close %s\n", h->file->name);
}

const char *handle_write(Handle *h, const char *line) {
    if (lines_used(h->disk) == MAX_LINES) {
        return fail(h->disk, "disk full");
    }
    File *file = h->file;
    snprintf(file->lines[file->count++], NAME_SIZE, "%s", line);
    return NULL;
}

const char *lines(int count) {
    return count == 1 ? "line" : "lines";
}

// Each command returns NULL on success or an error message. Once a file is
// open, every way out goes through the labels at the bottom.
const char *write_file(Disk *disk, const char *name, char **words, int count) {
    Handle target;
    const char *error = handle_open(&target, disk, name, true);
    if (error != NULL) {
        return error;
    }
    for (int i = 0; i < count && error == NULL; i++) {
        error = handle_write(&target, words[i]);
    }
    handle_close(&target);
    if (error == NULL) {
        printf("wrote %d %s\n", count, lines(count));
    }
    return error;
}

const char *cat_file(Disk *disk, const char *name) {
    Handle source;
    const char *error = handle_open(&source, disk, name, false);
    if (error != NULL) {
        return error;
    }
    for (int i = 0; i < source.file->count; i++) {
        printf("  %s\n", source.file->lines[i]);
    }
    handle_close(&source);
    return NULL;
}

const char *copy_file(Disk *disk, const char *from, const char *to) {
    if (strcmp(from, to) == 0) {
        return fail(disk, "cannot copy \"%s\" onto itself", from);
    }
    Handle source;
    Handle target;
    int copied = 0;
    const char *error = handle_open(&source, disk, from, false);
    if (error != NULL) {
        return error;
    }
    error = handle_open(&target, disk, to, true);
    if (error != NULL) {
        goto close_source;
    }
    for (int i = 0; i < source.file->count; i++) {
        error = handle_write(&target, source.file->lines[i]);
        if (error != NULL) {
            goto close_target;
        }
        copied++;
    }

close_target:
    handle_close(&target);
close_source:
    handle_close(&source);
    if (error == NULL) {
        printf("copied %d %s\n", copied, lines(copied));
    }
    return error;
}

const char *lock_file(Disk *disk, const char *name) {
    File *file = find(disk, name);
    if (file == NULL) {
        return fail(disk, "no file \"%s\"", name);
    }
    file->locked = true;
    printf("locked %s\n", name);
    return NULL;
}

const char *takes(Disk *disk, const char *command, int want, int got) {
    if (got == want) {
        return NULL;
    }
    const char *noun = want == 1 ? "name" : "names";
    return fail(disk, "%s takes %d %s, got %d", command, want, noun, got);
}

const char *run(Disk *disk, char **parts, int count) {
    const char *command = parts[0];
    char **args = parts + 1;
    int got = count - 1;
    const char *error;
    if (strcmp(command, "write") == 0) {
        if (got == 0) {
            return fail(disk, "write needs a file name");
        }
        return write_file(disk, args[0], args + 1, got - 1);
    }
    if (strcmp(command, "cat") == 0) {
        error = takes(disk, command, 1, got);
        return error != NULL ? error : cat_file(disk, args[0]);
    }
    if (strcmp(command, "copy") == 0) {
        error = takes(disk, command, 2, got);
        return error != NULL ? error : copy_file(disk, args[0], args[1]);
    }
    if (strcmp(command, "lock") == 0) {
        error = takes(disk, command, 1, got);
        return error != NULL ? error : lock_file(disk, args[0]);
    }
    return fail(disk, "unknown command \"%s\"", command);
}

int main(void) {
    static Disk disk;
    char line[1000];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *parts[500];
        int count = 0;
        for (char *part = strtok(line, " \t\r\n"); part != NULL;
             part = strtok(NULL, " \t\r\n")) {
            parts[count++] = part;
        }
        if (count == 0) {
            continue;
        }
        const char *error = run(&disk, parts, count);
        if (error != NULL) {
            printf("error: %s\n", error);
        }
    }
    printf("files still open: %d\n", disk.open_count);
    return 0;
}
