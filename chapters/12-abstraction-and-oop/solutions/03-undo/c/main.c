#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 1000
#define MAX_TEXT 256
#define MAX_WORDS 128
#define MAX_LINE 1024

struct document {
    char lines[MAX_LINES][MAX_TEXT];
    int count;
};

// An edit that knows how to do itself and how to undo itself. Every command
// starts with a pointer to the functions for its kind.
struct command;
struct command_vtable {
    // Returns false, and writes why into error, if the edit cannot be done.
    bool (*apply)(struct command *self, struct document *doc, char *error,
                  size_t size);
    void (*undo)(struct command *self, struct document *doc);
    void (*describe)(const struct command *self, char *out, size_t size);
};

struct command {
    const struct command_vtable *vtable;
};

bool check_line(const struct document *doc, int number, char *error,
                size_t size) {
    if (number < 1 || number > doc->count) {
        snprintf(error, size, "no line %d", number);
        return false;
    }
    return true;
}

struct add_line {
    struct command base;
    char text[MAX_TEXT];
};

bool add_apply(struct command *self, struct document *doc, char *error,
               size_t size) {
    if (doc->count == MAX_LINES) {
        snprintf(error, size, "too many lines");
        return false;
    }
    memcpy(doc->lines[doc->count++], ((struct add_line *)self)->text, MAX_TEXT);
    return true;
}

void add_undo(struct command *self, struct document *doc) {
    (void)self;
    doc->count--;
}

void add_describe(const struct command *self, char *out, size_t size) {
    snprintf(out, size, "add \"%s\"", ((const struct add_line *)self)->text);
}

struct delete_line {
    struct command base;
    int number;
    char removed[MAX_TEXT];
};

bool delete_apply(struct command *self, struct document *doc, char *error,
                  size_t size) {
    struct delete_line *deletion = (struct delete_line *)self;
    if (!check_line(doc, deletion->number, error, size)) {
        return false;
    }
    int index = deletion->number - 1;
    memcpy(deletion->removed, doc->lines[index], MAX_TEXT);
    memmove(doc->lines[index], doc->lines[index + 1],
            (size_t)(doc->count - index - 1) * MAX_TEXT);
    doc->count--;
    return true;
}

void delete_undo(struct command *self, struct document *doc) {
    struct delete_line *deletion = (struct delete_line *)self;
    int index = deletion->number - 1;
    memmove(doc->lines[index + 1], doc->lines[index],
            (size_t)(doc->count - index) * MAX_TEXT);
    memcpy(doc->lines[index], deletion->removed, MAX_TEXT);
    doc->count++;
}

void delete_describe(const struct command *self, char *out, size_t size) {
    const struct delete_line *deletion = (const struct delete_line *)self;
    snprintf(out, size, "delete line %d", deletion->number);
}

struct replace_line {
    struct command base;
    int number;
    char text[MAX_TEXT];
    char old[MAX_TEXT];
};

bool replace_apply(struct command *self, struct document *doc, char *error,
                   size_t size) {
    struct replace_line *replace = (struct replace_line *)self;
    if (!check_line(doc, replace->number, error, size)) {
        return false;
    }
    memcpy(replace->old, doc->lines[replace->number - 1], MAX_TEXT);
    memcpy(doc->lines[replace->number - 1], replace->text, MAX_TEXT);
    return true;
}

void replace_undo(struct command *self, struct document *doc) {
    struct replace_line *replace = (struct replace_line *)self;
    memcpy(doc->lines[replace->number - 1], replace->old, MAX_TEXT);
}

void replace_describe(const struct command *self, char *out, size_t size) {
    const struct replace_line *replace = (const struct replace_line *)self;
    snprintf(out, size, "replace line %d with \"%s\"", replace->number,
             replace->text);
}

static const struct command_vtable add_vtable = {add_apply, add_undo,
                                                 add_describe};
static const struct command_vtable delete_vtable = {delete_apply, delete_undo,
                                                    delete_describe};
static const struct command_vtable replace_vtable = {
    replace_apply, replace_undo, replace_describe};

void *allocate(size_t size) {
    void *memory = calloc(1, size);
    if (memory == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return memory;
}

// Joins words with single spaces into text.
void join(char *text, char **words, int count) {
    text[0] = '\0';
    size_t used = 0;
    for (int i = 0; i < count; i++) {
        int written = snprintf(text + used, MAX_TEXT - used, "%s%s",
                               i > 0 ? " " : "", words[i]);
        if (written < 0 || used + (size_t)written >= MAX_TEXT) {
            break;
        }
        used += (size_t)written;
    }
}

// A stack of commands that grows as needed.
struct stack {
    struct command **items;
    int count, capacity;
};

void push(struct stack *stack, struct command *command) {
    if (stack->count == stack->capacity) {
        stack->capacity = stack->capacity == 0 ? 16 : stack->capacity * 2;
        struct command **items =
            realloc(stack->items, (size_t)stack->capacity * sizeof *items);
        if (items == NULL) {
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
        stack->items = items;
    }
    stack->items[stack->count++] = command;
}

void clear(struct stack *stack) {
    while (stack->count > 0) {
        free(stack->items[--stack->count]);
    }
}

struct editor {
    struct document doc;
    struct stack done, undone;
};

void run_command(struct editor *editor, struct command *command) {
    char error[64];
    if (!command->vtable->apply(command, &editor->doc, error, sizeof error)) {
        printf("error: %s\n", error); // nothing is recorded
        free(command);
        return;
    }
    push(&editor->done, command);
    clear(&editor->undone);
}

// Moves the newest command from one stack to the other. Undo calls undo on
// it, and redo calls apply.
void move(struct editor *editor, bool redo) {
    struct stack *from = redo ? &editor->undone : &editor->done;
    struct stack *to = redo ? &editor->done : &editor->undone;
    if (from->count == 0) {
        printf("error: nothing to %s\n", redo ? "redo" : "undo");
        return;
    }
    struct command *command = from->items[--from->count];
    char text[MAX_TEXT + 64];
    if (redo) {
        // This cannot fail: the document is as it was when apply first worked.
        command->vtable->apply(command, &editor->doc, text, sizeof text);
    } else {
        command->vtable->undo(command, &editor->doc);
    }
    push(to, command);
    command->vtable->describe(command, text, sizeof text);
    printf("%s: %s\n", redo ? "redo" : "undo", text);
}

int main(void) {
    static struct editor editor;
    char line[MAX_LINE];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *words[MAX_WORDS];
        int count = 0;
        for (char *word = strtok(line, " \t\r\n");
             word != NULL && count < MAX_WORDS;
             word = strtok(NULL, " \t\r\n")) {
            words[count++] = word;
        }
        if (count == 0) {
            continue;
        }
        if (strcmp(words[0], "add") == 0) {
            struct add_line *add = allocate(sizeof *add);
            add->base.vtable = &add_vtable;
            join(add->text, words + 1, count - 1);
            run_command(&editor, &add->base);
        } else if (strcmp(words[0], "delete") == 0 && count == 2) {
            struct delete_line *deletion = allocate(sizeof *deletion);
            deletion->base.vtable = &delete_vtable;
            deletion->number = atoi(words[1]);
            run_command(&editor, &deletion->base);
        } else if (strcmp(words[0], "replace") == 0 && count >= 2) {
            struct replace_line *replace = allocate(sizeof *replace);
            replace->base.vtable = &replace_vtable;
            replace->number = atoi(words[1]);
            join(replace->text, words + 2, count - 2);
            run_command(&editor, &replace->base);
        } else if (strcmp(words[0], "undo") == 0) {
            move(&editor, false);
        } else if (strcmp(words[0], "redo") == 0) {
            move(&editor, true);
        } else if (strcmp(words[0], "show") == 0) {
            if (editor.doc.count == 0) {
                printf("(empty)\n");
            }
            for (int i = 0; i < editor.doc.count; i++) {
                printf("%d: %s\n", i + 1, editor.doc.lines[i]);
            }
        }
    }
    clear(&editor.done);
    clear(&editor.undone);
    free(editor.done.items);
    free(editor.undone.items);
    return 0;
}
