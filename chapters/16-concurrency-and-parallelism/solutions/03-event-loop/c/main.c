#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TASKS 100
#define MAX_STEPS 100

// A tagged union: kind says which member of the union holds the step.
struct step {
    enum { PRINT, SLEEP, WAIT } kind;
    union {
        const char *text;  // PRINT
        long duration;     // SLEEP
        int task;          // WAIT: the number of the task to wait for
    };
};

struct task {
    const char *name;
    struct step steps[MAX_STEPS];
    int count;
    int next;  // the index of the step to run when it runs again
    bool done;
    int waiting_for;
    int waiters[MAX_TASKS];  // in the order they began waiting
    int waiter_count;
};

static struct task tasks[MAX_TASKS];
static int task_count;

// The tasks that can run now, in order, in a ring. A task is in it at most
// once, so it never holds more than MAX_TASKS.
static int ready[MAX_TASKS];
static int ready_start, ready_count;

static void make_ready(int id) {
    ready[(ready_start + ready_count++) % MAX_TASKS] = id;
}

static int take_ready(void) {
    int id = ready[ready_start];
    ready_start = (ready_start + 1) % MAX_TASKS;
    ready_count--;
    return id;
}

// The sleeping tasks. C has no heap in its library, so this is a list that
// the loop searches for the task to wake first.
struct sleeper {
    long wake, order;
    int task;
};
static struct sleeper sleeping[MAX_TASKS];
static int sleeping_count;
static long sleeps, now;

static int first_to_wake(void) {
    int best = 0;
    for (int i = 1; i < sleeping_count; i++) {
        struct sleeper a = sleeping[i], b = sleeping[best];
        if (a.wake < b.wake || (a.wake == b.wake && a.order < b.order)) best = i;
    }
    return best;
}

// Runs a task until it sleeps, waits, or ends.
static void run_until_stopped(int id) {
    struct task *task = &tasks[id];
    while (task->next < task->count) {
        struct step step = task->steps[task->next++];
        switch (step.kind) {
        case PRINT:
            printf("[%ld] %s: %s\n", now, task->name, step.text);
            break;
        case SLEEP:
            if (step.duration == 0) {
                make_ready(id);
            } else {
                sleeping[sleeping_count++] = (struct sleeper){now + step.duration, sleeps++, id};
            }
            return;
        case WAIT:
            if (tasks[step.task].done) break;  // it has finished already, so carry on
            task->waiting_for = step.task;
            tasks[step.task].waiters[tasks[step.task].waiter_count++] = id;
            return;
        }
    }
    task->done = true;
    printf("[%ld] %s: done\n", now, task->name);
    for (int i = 0; i < task->waiter_count; i++) make_ready(task->waiters[i]);
}

static void run(void) {
    for (int id = 0; id < task_count; id++) make_ready(id);
    while (true) {
        while (ready_count > 0) run_until_stopped(take_ready());
        if (sleeping_count == 0) break;
        // Nothing can run now, so the clock jumps to the next wake.
        now = sleeping[first_to_wake()].wake;
        while (sleeping_count > 0 && sleeping[first_to_wake()].wake == now) {
            int i = first_to_wake();
            make_ready(sleeping[i].task);
            sleeping[i] = sleeping[--sleeping_count];
        }
    }
    bool stuck = false;
    for (int id = 0; id < task_count; id++) {
        if (tasks[id].done) continue;
        if (stuck) {
            printf(", ");
        } else {
            printf("[%ld] deadlock: ", now);
        }
        printf("%s waits for %s", tasks[id].name, tasks[tasks[id].waiting_for].name);
        stuck = true;
    }
    if (stuck) {
        printf("\n");
    } else {
        printf("[%ld] all done\n", now);
    }
}

static char *read_all(FILE *file) {
    size_t length = 0, capacity = 4096;
    char *text = malloc(capacity);
    size_t got;
    while (text != NULL && (got = fread(text + length, 1, capacity - length - 1, file)) > 0) {
        length += got;
        if (length + 1 == capacity) text = realloc(text, capacity *= 2);
    }
    if (text == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    text[length] = '\0';
    return text;
}

// Cuts a line in place into its first word and the rest, without the spaces
// around them.
static void split(char *line, char **command, char **rest) {
    while (*line == ' ') line++;
    char *end = line + strlen(line);
    while (end > line && end[-1] == ' ') end--;
    *end = '\0';
    *command = line;
    *rest = end;
    char *space = strchr(line, ' ');
    if (space != NULL) {
        *space = '\0';
        for (*rest = space + 1; **rest == ' ';) (*rest)++;
    }
}

static int find_task(char **names, int count, const char *name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(names[i], name) == 0) return i;
    }
    return -1;
}

static struct step *add_step(void) {
    struct task *task = &tasks[task_count - 1];
    if (task->count == MAX_STEPS) {
        fprintf(stderr, "more than %d steps in %s\n", MAX_STEPS, task->name);
        exit(1);
    }
    return &task->steps[task->count++];
}

int main(void) {
    char *input = read_all(stdin);
    size_t line_count = 0;
    for (char *at = input; *at != '\0'; at++) line_count += *at == '\n';
    char **commands = malloc((line_count + 1) * sizeof(char *));
    char **rests = malloc((line_count + 1) * sizeof(char *));
    char *names[MAX_TASKS];
    int name_count = 0;
    if (commands == NULL || rests == NULL) return 1;
    // Cuts the input into lines, and each line into its command and the rest.
    // A wait can name a task defined further down, so the names come first.
    line_count = 0;
    for (char *at = input; *at != '\0'; line_count++) {
        char *newline = strchr(at, '\n');
        if (newline != NULL) *newline = '\0';
        split(at, &commands[line_count], &rests[line_count]);
        if (strcmp(commands[line_count], "task") == 0) {
            if (name_count == MAX_TASKS) {
                fprintf(stderr, "more than %d tasks\n", MAX_TASKS);
                return 1;
            }
            names[name_count++] = rests[line_count];
        }
        at = newline != NULL ? newline + 1 : at + strlen(at);
    }

    for (size_t i = 0; i < line_count; i++) {
        char *command = commands[i], *rest = rests[i];
        if (*command == '\0') continue;
        if (strcmp(command, "task") == 0) {
            tasks[task_count++].name = rest;
        } else if (task_count == 0) {
            printf("error: line %zu: a step before any task\n", i + 1);
        } else if (strcmp(command, "print") == 0) {
            *add_step() = (struct step){.kind = PRINT, .text = rest};
        } else if (strcmp(command, "sleep") == 0) {
            if (*rest != '\0' && strspn(rest, "0123456789") == strlen(rest)) {
                *add_step() = (struct step){.kind = SLEEP, .duration = strtol(rest, NULL, 10)};
            } else {
                printf("error: line %zu: sleep takes a whole number\n", i + 1);
            }
        } else if (strcmp(command, "wait") == 0) {
            int other = find_task(names, name_count, rest);
            if (other >= 0) {
                *add_step() = (struct step){.kind = WAIT, .task = other};
            } else {
                printf("error: line %zu: no task named %s\n", i + 1, rest);
            }
        } else {
            printf("error: line %zu: unknown step: %s\n", i + 1, command);
        }
    }
    run();
    free(commands);
    free(rests);
    free(input);
    return 0;
}
