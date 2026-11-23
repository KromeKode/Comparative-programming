// Prints a report of each student's scores. ../README.md says what it should print.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct student {
    char *name;
    int *scores;
    size_t count, capacity;
};

// The students, in the order they first appear.
struct students {
    struct student *items;
    size_t length, capacity;
};

static void *grow(void *items, size_t *capacity, size_t size) {
    *capacity = *capacity ? 2 * *capacity : 8;
    items = realloc(items, *capacity * size);
    if (!items) exit(1);
    return items;
}

static char *copy(const char *s) {
    char *result = malloc(strlen(s) + 1);
    if (!result) exit(1);
    return strcpy(result, s);
}

// Gives the student with this name, adding one with no scores if there is none.
static struct student *find(struct students *students, const char *name) {
    for (size_t i = 0; i < students->length; i++) {
        if (strcmp(students->items[i].name, name) == 0) return &students->items[i];
    }
    if (students->length == students->capacity) {
        students->items = grow(students->items, &students->capacity, sizeof *students->items);
    }
    struct student *student = &students->items[students->length++];
    *student = (struct student){copy(name), NULL, 0, 0};
    return student;
}

static void add_score(struct student *student, int score) {
    if (student->count == student->capacity) {
        student->scores = grow(student->scores, &student->capacity, sizeof *student->scores);
    }
    student->scores[student->count++] = score;
}

// Gives total / count, rounded to the nearest whole number, with halves up.
static long rounded_average(long total, long count) {
    return (2 * total + count) / (2 * count);
}

int main(int argc, char **argv) {
    // The input is the file named on the command line, if there is one, so that a
    // debugger can keep the terminal to itself.
    FILE *in = argc > 1 ? fopen(argv[1], "r") : stdin;
    if (!in) {
        perror(argv[1]);
        return 1;
    }
    struct students students = {0};
    char line[4096];
    while (fgets(line, sizeof line, in)) {
        const char *spaces = " \t\r\n";
        char *name = strtok(line, spaces);
        if (!name) continue;
        struct student *student = find(&students, name);
        for (char *field = strtok(NULL, spaces); field; field = strtok(NULL, spaces)) {
            add_score(student, atoi(field));
        }
    }

    const char *best_name = "-";
    long best_average = -1;
    long class_total = 0, class_count = 0;
    for (size_t i = 0; i < students.length; i++) {
        struct student *student = &students.items[i];
        long total = 0;
        for (size_t j = 0; j < student->count; j++) total += student->scores[j];
        class_total += total;
        class_count += (long)student->count;
        if (student->count == 0) {
            printf("%s -\n", student->name);
            continue;
        }
        long average = rounded_average(total, (long)student->count);
        printf("%s %ld\n", student->name, average);
        if (average > best_average) {
            best_name = student->name;
            best_average = average;
        }
    }

    if (class_count == 0) {
        printf("class average: -\n");
    } else {
        printf("class average: %ld\n", rounded_average(class_total, class_count));
    }
    printf("top: %s\n", best_name);

    for (size_t i = 0; i < students.length; i++) {
        free(students.items[i].name);
        free(students.items[i].scores);
    }
    free(students.items);
    return 0;
}
