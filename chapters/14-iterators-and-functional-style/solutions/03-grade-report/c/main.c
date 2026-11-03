#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define MAX_WORD 32
#define MAX_GRADES 4096

struct grade {
    char name[MAX_WORD], subject[MAX_WORD];
    int score;
};

struct student {
    const char *name;
    int total;
    bool passed;
};

// Sorting by subject, then name, puts each subject's grades next to each other
// with the names in order. Grouping is then a walk along the array.
static int by_subject(const void *a, const void *b) {
    const struct grade *x = a, *y = b;
    int order = strcmp(x->subject, y->subject);
    return order != 0 ? order : strcmp(x->name, y->name);
}

static int by_name(const void *a, const void *b) {
    const struct grade *x = a, *y = b;
    return strcmp(x->name, y->name);
}

static int by_total(const void *a, const void *b) {
    const struct student *x = a, *y = b;
    if (x->total != y->total) return y->total - x->total;  // highest first
    return strcmp(x->name, y->name);
}

static int by_student_name(const void *a, const void *b) {
    const struct student *x = a, *y = b;
    return strcmp(x->name, y->name);
}

static struct grade grades[MAX_GRADES];
static struct student students[MAX_GRADES];

static const char *subject_of(const struct grade *grade) { return grade->subject; }
static const char *name_of(const struct grade *grade) { return grade->name; }

// Returns the index just past the run of grades that share grades[start]'s key.
static size_t group_end(size_t start, size_t count, const char *(*key)(const struct grade *)) {
    size_t end = start;
    while (end < count && strcmp(key(&grades[end]), key(&grades[start])) == 0) end++;
    return end;
}

static void print_subjects(size_t count) {
    printf("subjects:\n");
    qsort(grades, count, sizeof grades[0], by_subject);
    for (size_t start = 0, end; start < count; start = end) {
        end = group_end(start, count, subject_of);
        int total = 0, best = 0;
        for (size_t i = start; i < end; i++) {
            total += grades[i].score;
            if (grades[i].score > best) best = grades[i].score;
        }
        printf("  %s: average %d, best %d (", grades[start].subject, total / (int)(end - start),
               best);
        const char *sep = "";
        for (size_t i = start; i < end; i++) {
            if (grades[i].score != best) continue;
            printf("%s%s", sep, grades[i].name);
            sep = ", ";
        }
        printf(")\n");
    }
}

static void print_students(size_t count) {
    qsort(grades, count, sizeof grades[0], by_name);
    size_t students_count = 0;
    for (size_t start = 0, end; start < count; start = end) {
        end = group_end(start, count, name_of);
        struct student *student = &students[students_count++];
        *student = (struct student){grades[start].name, 0, true};
        for (size_t i = start; i < end; i++) {
            student->total += grades[i].score;
            if (grades[i].score < 50) student->passed = false;
        }
    }
    qsort(students, students_count, sizeof students[0], by_total);
    printf("ranking:\n");
    for (size_t i = 0; i < students_count; i++) {
        printf("  %zu. %s %d\n", i + 1, students[i].name, students[i].total);
    }
    qsort(students, students_count, sizeof students[0], by_student_name);
    printf("passed everything:");
    bool any = false;
    for (size_t i = 0; i < students_count; i++) {
        if (!students[i].passed) continue;
        printf("%s%s", any ? ", " : " ", students[i].name);
        any = true;
    }
    printf("%s\n", any ? "" : " nobody");
}

int main(void) {
    char line[MAX_LINE];
    size_t count = 0;
    while (count < MAX_GRADES && fgets(line, sizeof line, stdin) != NULL) {
        struct grade *g = &grades[count];
        if (sscanf(line, "%31s %31s %d", g->name, g->subject, &g->score) == 3) count++;
    }
    print_subjects(count);
    print_students(count);
    return 0;
}
