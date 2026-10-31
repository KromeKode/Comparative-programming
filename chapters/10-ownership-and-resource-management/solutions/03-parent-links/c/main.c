#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A folder owns its children, kept as a linked list through next. Its parent
// pointer owns nothing. Only the comments say which pointers own.
typedef struct Folder {
    char name[32];
    struct Folder *parent;      // not owned
    struct Folder *first_child; // owned
    struct Folder *next;        // owned by the same parent as this folder
} Folder;

int freed = 0; // how many folders folder_free has freed

Folder *folder_new(const char *name) {
    Folder *f = calloc(1, sizeof *f);
    if (f == NULL) {
        fputs("out of memory\n", stderr);
        exit(1);
    }
    snprintf(f->name, sizeof f->name, "%s", name);
    return f;
}

// Frees a folder and everything inside it.
void folder_free(Folder *f) {
    Folder *child = f->first_child;
    while (child != NULL) {
        Folder *next = child->next;
        folder_free(child);
        child = next;
    }
    free(f);
    freed++;
}

const char *plural(int count) {
    return count == 1 ? "folder" : "folders";
}

// Finds a folder by name, searching the whole tree from `from` down.
Folder *find(Folder *from, const char *name) {
    if (strcmp(from->name, name) == 0) {
        return from;
    }
    for (Folder *c = from->first_child; c != NULL; c = c->next) {
        Folder *found = find(c, name);
        if (found != NULL) {
            return found;
        }
    }
    return NULL;
}

// Prints the path by following parent pointers up to the root.
void print_path(const Folder *f) {
    if (f->parent == NULL) {
        printf("/");
        return;
    }
    if (f->parent->parent != NULL) {
        print_path(f->parent);
    }
    printf("/%s", f->name);
}

// True when f is inside ancestor, or is ancestor.
bool is_inside(const Folder *f, const Folder *ancestor) {
    for (; f != NULL; f = f->parent) {
        if (f == ancestor) {
            return true;
        }
    }
    return false;
}

// Takes f out of its parent's list. The caller owns it afterwards.
void detach(Folder *f) {
    Folder **link = &f->parent->first_child;
    while (*link != f) {
        link = &(*link)->next;
    }
    *link = f->next;
    f->next = NULL;
    f->parent = NULL;
}

// Adds f at the end of parent's list. The parent owns it afterwards.
void attach(Folder *f, Folder *parent) {
    Folder **link = &parent->first_child;
    while (*link != NULL) {
        link = &(*link)->next;
    }
    *link = f;
    f->parent = parent;
}

bool takes(const char *command, int want, int got) {
    if (got != want) {
        printf("error: %s takes %d %s, got %d\n", command, want,
               want == 1 ? "name" : "names", got);
    }
    return got == want;
}

Folder *lookup(Folder *root, const char *name) {
    Folder *found = find(root, name);
    if (found == NULL) {
        printf("error: no folder \"%s\"\n", name);
    }
    return found;
}

void run(Folder *root, char **parts, int count) {
    const char *command = parts[0];
    char **args = parts + 1;
    int got = count - 1;
    if (strcmp(command, "mkdir") == 0) {
        if (!takes(command, 2, got)) {
            return;
        }
        if (find(root, args[0]) != NULL) {
            printf("error: \"%s\" already exists\n", args[0]);
            return;
        }
        Folder *parent = lookup(root, args[1]);
        if (parent == NULL) {
            return;
        }
        Folder *f = folder_new(args[0]);
        attach(f, parent);
        printf("made ");
        print_path(f);
        printf("\n");
    } else if (strcmp(command, "path") == 0) {
        Folder *f = takes(command, 1, got) ? lookup(root, args[0]) : NULL;
        if (f != NULL) {
            print_path(f);
            printf("\n");
        }
    } else if (strcmp(command, "ls") == 0) {
        Folder *f = takes(command, 1, got) ? lookup(root, args[0]) : NULL;
        if (f == NULL) {
            return;
        }
        printf("%s:", f->name);
        for (Folder *c = f->first_child; c != NULL; c = c->next) {
            printf(" %s", c->name);
        }
        printf("%s\n", f->first_child == NULL ? " (empty)" : "");
    } else if (strcmp(command, "rm") == 0) {
        Folder *f = takes(command, 1, got) ? lookup(root, args[0]) : NULL;
        if (f == NULL) {
            return;
        }
        if (f == root) {
            printf("error: cannot remove /\n");
            return;
        }
        int before = freed;
        detach(f);
        folder_free(f);
        int count = freed - before;
        printf("removed %s, freed %d %s\n", args[0], count, plural(count));
    } else if (strcmp(command, "mv") == 0) {
        Folder *f = takes(command, 2, got) ? lookup(root, args[0]) : NULL;
        if (f == NULL) {
            return;
        }
        if (f == root) {
            printf("error: cannot move /\n");
            return;
        }
        Folder *parent = lookup(root, args[1]);
        if (parent == NULL) {
            return;
        }
        if (is_inside(parent, f)) {
            printf("error: cannot move %s into itself\n", args[0]);
            return;
        }
        detach(f);
        attach(f, parent);
        printf("moved %s to ", args[0]);
        print_path(f);
        printf("\n");
    } else {
        printf("error: unknown command \"%s\"\n", command);
    }
}

int main(void) {
    Folder *root = folder_new("/");
    char line[1000];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *parts[500];
        int count = 0;
        for (char *part = strtok(line, " \t\r\n"); part != NULL;
             part = strtok(NULL, " \t\r\n")) {
            parts[count++] = part;
        }
        if (count > 0) {
            run(root, parts, count);
        }
    }
    int before = freed;
    folder_free(root);
    int count = freed - before;
    printf("freed %d %s at exit\n", count, plural(count));
    return 0;
}
