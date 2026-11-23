// Keeps a to-do list in todo.json, in the current folder: adds tasks, lists them, marks
// them done, and removes them. ../README.md says what it should do.
#define _POSIX_C_SOURCE 200809L  // for fileno and fsync, from POSIX
#include <cJSON.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char file_name[] = "todo.json";
static const char usage[] =
    "usage: todo add TEXT...\n"
    "       todo list\n"
    "       todo done ID\n"
    "       todo remove ID\n";

typedef struct {
    long long id;
    const char *text;  // in the cJSON tree, or in the text that add makes
    bool done;
} Task;

typedef struct {
    Task *items;
    size_t count;
} Tasks;

static void out_of_memory(void) {
    fputs("todo: out of memory\n", stderr);
    exit(2);
}

// Reads a whole file into a string. Returns NULL, with errno set, if it can't.
static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) return NULL;
    char *text = NULL;
    size_t length = 0, capacity = 0;
    for (;;) {
        if (length + 1 >= capacity) {  // room for the '\0' at the end
            capacity = capacity == 0 ? 4096 : capacity * 2;
            char *bigger = realloc(text, capacity);
            if (bigger == NULL) break;
            text = bigger;
        }
        size_t got = fread(text + length, 1, capacity - length - 1, file);
        length += got;
        if (got == 0) break;  // the end, or an error, such as EISDIR for a folder
    }
    int error = ferror(file) || length + 1 >= capacity ? errno : 0;
    fclose(file);
    if (error != 0) {
        free(text);
        errno = error;
        return NULL;
    }
    text[length] = '\0';
    return text;
}

// Whether a node is a task: an object with an id, a whole number from 1 up, a text, and
// done, true or false, and nothing else. A missing key gives NULL, and cJSON_IsString
// and the others are false for NULL.
static bool is_task(const cJSON *item) {
    const cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");
    const cJSON *text = cJSON_GetObjectItemCaseSensitive(item, "text");
    const cJSON *done = cJSON_GetObjectItemCaseSensitive(item, "done");
    // cJSON keeps each number as a double, which holds every whole number to 2^53.
    return cJSON_IsObject(item) && cJSON_GetArraySize(item) == 3 && cJSON_IsNumber(id) &&
           id->valuedouble >= 1 && id->valuedouble <= 9007199254740991.0 &&
           id->valuedouble == (double)(long long)id->valuedouble && cJSON_IsString(text) &&
           cJSON_IsBool(done);
}

// Reads the tasks in todo.json, or none if there's no todo.json. Returns NULL, or what
// is wrong. The tasks' texts are in *json, which the caller deletes.
static const char *read_tasks(Tasks *tasks, cJSON **json) {
    static char problem[64];
    *tasks = (Tasks){NULL, 0};
    *json = NULL;
    char *text = read_file(file_name);
    if (text == NULL) return errno == ENOENT ? NULL : strerror(errno);
    // cJSON_Parse reads one value and ignores whatever comes after it. With 1 for
    // require_null_terminated, anything but white space after it is an error.
    *json = cJSON_ParseWithOpts(text, NULL, 1);
    free(text);
    if (*json == NULL) return "not valid JSON";
    // A later version of todo may keep anything else in the file, so the version comes
    // first. cJSON_GetObjectItemCaseSensitive gives NULL for a value that isn't an object.
    const cJSON *version = cJSON_GetObjectItemCaseSensitive(*json, "version");
    if (cJSON_IsNumber(version) && version->valuedouble != 1) {
        snprintf(problem, sizeof problem, "unknown version %g", version->valuedouble);
        return problem;
    }
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(*json, "tasks");
    if (!cJSON_IsObject(*json) || cJSON_GetArraySize(*json) != 2 || !cJSON_IsNumber(version) ||
        !cJSON_IsArray(list)) {
        return "not a to-do list";
    }
    size_t count = (size_t)cJSON_GetArraySize(list);
    tasks->items = malloc((count > 0 ? count : 1) * sizeof *tasks->items);
    if (tasks->items == NULL) out_of_memory();
    const cJSON *item;
    cJSON_ArrayForEach(item, list) {
        if (!is_task(item)) return "not a to-do list";
        Task task = {(long long)cJSON_GetObjectItemCaseSensitive(item, "id")->valuedouble,
                     cJSON_GetObjectItemCaseSensitive(item, "text")->valuestring,
                     cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(item, "done"))};
        for (size_t i = 0; i < tasks->count; i++) {
            if (tasks->items[i].id == task.id) return "not a to-do list";  // one id, twice
        }
        tasks->items[tasks->count++] = task;
    }
    return NULL;
}

// Writes text as a JSON string: in quotes, with " and \ and control characters escaped,
// and everything else, UTF-8 included, as it is.
static void write_string(FILE *out, const char *text) {
    fputc('"', out);
    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
        switch (*p) {
            case '"': fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\b': fputs("\\b", out); break;
            case '\f': fputs("\\f", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            default:
                if (*p < 0x20) fprintf(out, "\\u%04x", *p);
                else fputc(*p, out);
        }
    }
    fputc('"', out);
}

// Writes the tasks to todo.json, the way JSON.stringify(list, null, 2) does, and
// returns 0, or errno. It writes a new file and renames it over the old one, which
// replaces it in one step, so that a crash leaves the old list or the new one, and never
// half of one.
static int write_tasks(const Tasks *tasks) {
    static const char temporary[] = "todo.json.tmp";
    FILE *out = fopen(temporary, "wb");
    if (out == NULL) return errno;
    fputs("{\n  \"version\": 1,\n  \"tasks\": [", out);
    for (size_t i = 0; i < tasks->count; i++) {
        const Task *task = &tasks->items[i];
        fprintf(out, "%s\n    {\n      \"id\": %lld,\n      \"text\": ", i > 0 ? "," : "",
                task->id);
        write_string(out, task->text);
        fprintf(out, ",\n      \"done\": %s\n    }", task->done ? "true" : "false");
    }
    fputs(tasks->count > 0 ? "\n  ]\n}\n" : "]\n}\n", out);
    int error = ferror(out) ? errno : 0;
    if (error == 0 && fflush(out) != 0) error = errno;          // to the system
    if (error == 0 && fsync(fileno(out)) != 0) error = errno;  // onto the disk
    if (fclose(out) != 0 && error == 0) error = errno;
    if (error == 0 && rename(temporary, file_name) != 0) error = errno;
    if (error != 0) remove(temporary);
    return error;
}

// Carries out a command whose arguments have been checked, and returns the status.
static int run(const char *command, char **args, int count) {
    Tasks tasks;
    cJSON *json;
    const char *problem = read_tasks(&tasks, &json);
    char *text = NULL;  // the new task's, for add
    int status = 0;
    if (problem == NULL && strcmp(command, "add") == 0) {
        size_t length = 1;  // for the '\0' at the end
        for (int i = 0; i < count; i++) length += strlen(args[i]) + 1;  // and a space
        text = malloc(length);
        if (text == NULL) out_of_memory();
        text[0] = '\0';
        for (int i = 0; i < count; i++) {
            if (i > 0) strcat(text, " ");
            strcat(text, args[i]);
        }
        long long id = 1;
        for (size_t i = 0; i < tasks.count; i++) {
            if (tasks.items[i].id >= id) id = tasks.items[i].id + 1;
        }
        Task *bigger = realloc(tasks.items, (tasks.count + 1) * sizeof *tasks.items);
        if (bigger == NULL) out_of_memory();
        tasks.items = bigger;
        tasks.items[tasks.count++] = (Task){id, text, false};
        int error = write_tasks(&tasks);
        if (error != 0) problem = strerror(error);
        else printf("added %lld: %s\n", id, text);
    } else if (problem == NULL && strcmp(command, "list") == 0) {
        for (size_t i = 0; i < tasks.count; i++) {
            const Task *task = &tasks.items[i];
            printf("%lld [%c] %s\n", task->id, task->done ? 'x' : ' ', task->text);
        }
    } else if (problem == NULL) {
        // strtoll gives LLONG_MAX, and sets errno, for an ID too big for a long long,
        // which no task's id can be.
        errno = 0;
        long long id = strtoll(args[0], NULL, 10);
        size_t i = 0;
        while (i < tasks.count && (errno != 0 || tasks.items[i].id != id)) i++;
        if (i == tasks.count) {
            fprintf(stderr, "todo: no task %s\n", args[0]);
            status = 1;
        } else {
            Task task = tasks.items[i];
            if (strcmp(command, "done") == 0) {
                tasks.items[i].done = true;
            } else {
                memmove(&tasks.items[i], &tasks.items[i + 1],
                        (tasks.count - i - 1) * sizeof *tasks.items);
                tasks.count--;
            }
            int error = write_tasks(&tasks);
            if (error != 0) problem = strerror(error);
            else printf("%s %lld: %s\n", strcmp(command, "done") == 0 ? "done" : "removed",
                        task.id, task.text);
        }
    }
    if (problem != NULL) {
        fprintf(stderr, "todo: %s: %s\n", file_name, problem);
        status = 2;
    }
    free(text);
    free(tasks.items);
    cJSON_Delete(json);  // cJSON_Delete(NULL) does nothing
    return status;
}

int main(int argc, char *argv[]) {
    const char *command = argc > 1 ? argv[1] : "";
    char **args = argv + (argc > 1 ? 2 : 1);
    int count = argc > 1 ? argc - 2 : 0;
    bool is_id = count == 1 && args[0][0] != '\0' && strspn(args[0], "0123456789") == strlen(args[0]);
    // The command line is checked before todo.json is read.
    bool ok = (strcmp(command, "add") == 0 && count > 0) ||
              (strcmp(command, "list") == 0 && count == 0) ||
              ((strcmp(command, "done") == 0 || strcmp(command, "remove") == 0) && is_id);
    if (!ok) {
        fputs(usage, stderr);
        return 2;
    }
    return run(command, args, count);
}
