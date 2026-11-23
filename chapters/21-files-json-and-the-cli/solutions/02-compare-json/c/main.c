// Prints how one JSON file differs from another: what was removed, what was added, and
// what changed. ../README.md says what it should print.
#include <cJSON.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char usage[] = "usage: jsondiff OLD NEW";

static void out_of_memory(void) {
    fputs("jsondiff: out of memory\n", stderr);
    exit(2);
}

// Reads all of file, with a '\0' after it. Returns NULL, with errno set, if it can't.
static char *read_all(FILE *file) {
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
    data[length] = '\0';  // the loop ends with length below capacity, so there's room
    return data;
}

// A string that grows as text is added to it, with a '\0' at the end.
typedef struct {
    char *data;
    size_t length, capacity;
} Text;

static void add(Text *text, const char *bytes, size_t count) {
    if (text->length + count + 1 > text->capacity) {
        size_t capacity = 2 * (text->length + count + 1);
        char *bigger = realloc(text->data, capacity);
        if (bigger == NULL) out_of_memory();
        text->data = bigger;
        text->capacity = capacity;
    }
    memcpy(text->data + text->length, bytes, count);
    text->length += count;
    text->data[text->length] = '\0';
}

static void add_text(Text *text, const char *s) { add(text, s, strlen(s)); }

// Cuts text back to its first length bytes.
static void cut(Text *text, size_t length) {
    text->length = length;
    text->data[length] = '\0';
}

// Adds s as a JSON string. The bytes of a character beyond ASCII are all 0x80 or more,
// so going byte by byte copies them as they are.
static void add_string(Text *text, const char *s) {
    add_text(text, "\"");
    for (; *s != '\0'; s++) {
        unsigned char c = (unsigned char)*s;
        char escape[8];
        switch (c) {
            case '"': add_text(text, "\\\""); break;
            case '\\': add_text(text, "\\\\"); break;
            case '\b': add_text(text, "\\b"); break;
            case '\f': add_text(text, "\\f"); break;
            case '\n': add_text(text, "\\n"); break;
            case '\r': add_text(text, "\\r"); break;
            case '\t': add_text(text, "\\t"); break;
            default:
                if (c < 0x20) {
                    snprintf(escape, sizeof escape, "\\u%04x", c);
                    add_text(text, escape);
                } else {
                    add(text, s, 1);
                }
        }
    }
    add_text(text, "\"");
}

static int compare_keys(const void *a, const void *b) {
    const cJSON *const *x = a;
    const cJSON *const *y = b;
    return strcmp((*x)->string, (*y)->string);  // strcmp compares bytes as unsigned
}

// The members of an object, in order of their keys, in an array to free. cJSON keeps
// them in a linked list, in the order of the file; each one's key is in its string.
static const cJSON **sorted_members(const cJSON *object, size_t *count) {
    size_t n = (size_t)cJSON_GetArraySize(object);
    const cJSON **members = malloc((n > 0 ? n : 1) * sizeof *members);
    if (members == NULL) out_of_memory();
    size_t i = 0;
    for (const cJSON *member = object->child; member != NULL; member = member->next) {
        members[i++] = member;
    }
    qsort(members, n, sizeof *members, compare_keys);
    *count = n;
    return members;
}

// Adds value as compact JSON, with each object's keys in order.
static void add_value(Text *text, const cJSON *value) {
    if (cJSON_IsObject(value)) {
        size_t count;
        const cJSON **members = sorted_members(value, &count);
        add_text(text, "{");
        for (size_t i = 0; i < count; i++) {
            if (i > 0) add_text(text, ",");
            add_string(text, members[i]->string);
            add_text(text, ":");
            add_value(text, members[i]);
        }
        add_text(text, "}");
        free(members);
    } else if (cJSON_IsArray(value)) {
        add_text(text, "[");
        for (const cJSON *item = value->child; item != NULL; item = item->next) {
            if (item != value->child) add_text(text, ",");
            add_value(text, item);
        }
        add_text(text, "]");
    } else if (cJSON_IsString(value)) {
        add_string(text, value->valuestring);
    } else if (cJSON_IsNumber(value)) {
        // cJSON keeps a number as a double, which holds each whole number up to 2^53
        // exactly. cJSON_Print writes 15 digits when they're close enough, so that
        // 9007199254740991 comes out as 9.00719925474099e+15.
        char number[32];
        snprintf(number, sizeof number, "%.0f", value->valuedouble);
        add_text(text, number);
    } else {
        add_text(text, cJSON_IsTrue(value) ? "true" : cJSON_IsFalse(value) ? "false" : "null");
    }
}

// Prints one difference: a sign, the path, and the value, or for a change both values.
// Returns 1, to count it.
static int report(char sign, const Text *path, const cJSON *value, const cJSON *other) {
    Text line = {0};
    add(&line, &sign, 1);
    add_text(&line, " ");
    add_text(&line, path->data);
    add_text(&line, ": ");
    add_value(&line, value);
    if (other != NULL) {
        add_text(&line, " -> ");
        add_value(&line, other);
    }
    puts(line.data);  // puts adds a '\n'
    free(line.data);
    return 1;
}

// Whether two values, which aren't two objects or two arrays, are the same. The low
// byte of type says what kind of value it is; true and false are two kinds.
static bool same(const cJSON *a, const cJSON *b) {
    if ((a->type & 0xFF) != (b->type & 0xFF)) return false;
    if (cJSON_IsNumber(a)) return a->valuedouble == b->valuedouble;
    if (cJSON_IsString(a)) return strcmp(a->valuestring, b->valuestring) == 0;
    return true;  // two trues, two falses, or two nulls
}

// Whether key is a name such as first_name: a letter or _, then letters, digits, or _.
// isalpha would depend on the locale, and could say yes to bytes above 127.
static bool is_identifier(const char *key) {
    for (const char *c = key; *c != '\0'; c++) {
        bool letter = (*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || *c == '_';
        bool digit = *c >= '0' && *c <= '9';
        if (!letter && !(digit && c != key)) return false;
    }
    return key[0] != '\0';
}

// Adds the part of a path that leads to a key: .name, or ["first name"].
static void add_step(Text *path, const char *key) {
    if (is_identifier(key)) {
        add_text(path, ".");
        add_text(path, key);
    } else {
        add_text(path, "[");
        add_string(path, key);
        add_text(path, "]");
    }
}

// Prints a line for each difference between before and after, whose path is path, and
// returns how many there are.
static long diff(Text *path, const cJSON *before, const cJSON *after) {
    long count = 0;
    size_t length = path->length;  // to cut the path back to after each step
    if (cJSON_IsObject(before) && cJSON_IsObject(after)) {
        size_t nb, na;
        const cJSON **b = sorted_members(before, &nb);
        const cJSON **a = sorted_members(after, &na);
        // Both lists are in order of their keys, so taking the smaller of the two keys
        // at i and j, as a merge sort does, visits every key in order.
        size_t i = 0, j = 0;
        while (i < nb || j < na) {
            int order = i == nb ? 1 : j == na ? -1 : strcmp(b[i]->string, a[j]->string);
            add_step(path, order <= 0 ? b[i]->string : a[j]->string);
            if (order < 0) {
                count += report('-', path, b[i++], NULL);
            } else if (order > 0) {
                count += report('+', path, a[j++], NULL);
            } else {
                count += diff(path, b[i++], a[j++]);
            }
            cut(path, length);
        }
        free(b);
        free(a);
    } else if (cJSON_IsArray(before) && cJSON_IsArray(after)) {
        const cJSON *b = before->child, *a = after->child;
        for (size_t i = 0; b != NULL || a != NULL; i++) {
            char index[32];
            snprintf(index, sizeof index, "[%zu]", i);
            add_text(path, index);
            if (a == NULL) {
                count += report('-', path, b, NULL);
            } else if (b == NULL) {
                count += report('+', path, a, NULL);
            } else {
                count += diff(path, b, a);
            }
            cut(path, length);
            if (b != NULL) b = b->next;
            if (a != NULL) a = a->next;
        }
    } else if (!same(before, after)) {
        count += report('~', path, before, after);
    }
    return count;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "%s\n", usage);
        return 2;
    }
    cJSON *values[2] = {NULL, NULL};
    int status = 0;
    for (int i = 0; i < 2 && status == 0; i++) {
        const char *name = argv[i + 1];
        FILE *file = fopen(name, "rb");
        char *text = file != NULL ? read_all(file) : NULL;
        int error = errno;
        if (file != NULL) fclose(file);
        if (text == NULL) {
            fprintf(stderr, "jsondiff: %s: %s\n", name, strerror(error));
            status = 2;
            break;
        }
        // cJSON_Parse reads one value and ignores whatever comes after it. With 1 for
        // require_null_terminated, anything but white space after it is an error.
        values[i] = cJSON_ParseWithOpts(text, NULL, 1);
        free(text);
        if (values[i] == NULL) {
            fprintf(stderr, "jsondiff: %s: not valid JSON\n", name);
            status = 2;
        }
    }
    if (status == 0) {
        Text path = {0};
        add_text(&path, "$");
        status = diff(&path, values[0], values[1]) > 0 ? 1 : 0;
        free(path.data);
    }
    cJSON_Delete(values[0]);  // cJSON_Delete(NULL) does nothing
    cJSON_Delete(values[1]);
    return status;
}
