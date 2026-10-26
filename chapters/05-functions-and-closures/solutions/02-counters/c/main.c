#include <stdio.h>
#include <string.h>

// C has no closures, so each counter's captured state lives in a struct.
struct Counter {
    char name[32];
    long long next;
    long long step;
};

static struct Counter counters[64];
static int counter_count = 0;

static struct Counter *find(const char *name) {
    for (int i = 0; i < counter_count; i++) {
        if (strcmp(counters[i].name, name) == 0) {
            return &counters[i];
        }
    }
    return NULL;
}

int main(void) {
    char line[128];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char name[32];
        long long start, step;
        if (sscanf(line, "new %31s %lld %lld", name, &start, &step) == 3) {
            struct Counter *counter = find(name);
            if (counter == NULL) {
                if (counter_count == 64) {
                    fprintf(stderr, "too many counters\n");
                    return 1;
                }
                counter = &counters[counter_count++];
                strcpy(counter->name, name);
            }
            counter->next = start;
            counter->step = step;
        } else if (sscanf(line, "next %31s", name) == 1) {
            struct Counter *counter = find(name);
            if (counter == NULL) {
                printf("%s: no such counter\n", name);
            } else {
                printf("%s: %lld\n", name, counter->next);
                counter->next += counter->step;
            }
        }
    }
    return 0;
}
