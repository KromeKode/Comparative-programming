#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct score {
    const char *name;
    int points;
};

static const struct score scores[] = {{"ada", 91}, {"bob", 78}, {"cy", 85},
                                      {"dee", 62}, {"eve", 99}, {"finn", 54}};
enum { COUNT = sizeof scores / sizeof scores[0] };

static const char *yes_no(bool value) { return value ? "yes" : "no"; }

static int by_points_descending(const void *a, const void *b) {
    const struct score *x = a, *y = b;
    return (y->points > x->points) - (y->points < x->points);
}

// C has no map, filter, or reduce, so each question is a loop.
int main(void) {
    printf("passed:");
    const char *sep = " ";
    int total = 0;
    for (size_t i = 0; i < COUNT; i++) {
        if (scores[i].points < 70) continue;
        printf("%s%s", sep, scores[i].name);
        sep = ", ";
        total += scores[i].points;
    }
    printf("\ntotal of the passes: %d\n", total);

    const struct score *best = &scores[0];
    bool everyone = true, anyone = false;
    for (size_t i = 0; i < COUNT; i++) {
        if (scores[i].points > best->points) best = &scores[i];
        if (scores[i].points < 70) everyone = false;
        if (scores[i].points > 95) anyone = true;
    }
    printf("best: %s with %d\n", best->name, best->points);
    printf("everyone passed: %s\n", yes_no(everyone));
    printf("anyone above 95: %s\n", yes_no(anyone));

    struct score ranked[COUNT];
    memcpy(ranked, scores, sizeof scores);
    qsort(ranked, COUNT, sizeof ranked[0], by_points_descending);
    printf("top three:");
    for (size_t i = 0; i < 3; i++) {
        printf("%s %zu. %s %d", i ? "," : "", i + 1, ranked[i].name, ranked[i].points);
    }

    bool seen[26] = {false};
    for (size_t i = 0; i < COUNT; i++) {
        for (const char *c = scores[i].name; *c; c++) seen[*c - 'a'] = true;
    }
    printf("\nletters used:");
    for (int i = 0; i < 26; i++) {
        if (seen[i]) printf(" %c", 'a' + i);
    }
    printf("\n");
    return 0;
}
