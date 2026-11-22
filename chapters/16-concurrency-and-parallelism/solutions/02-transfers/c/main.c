#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORKERS 16

static void *checked(void *memory) {
    if (memory == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return memory;
}

static char *read_all(FILE *file) {
    size_t length = 0, capacity = 4096;
    char *text = checked(malloc(capacity));
    size_t got;
    while ((got = fread(text + length, 1, capacity - length - 1, file)) > 0) {
        length += got;
        if (length + 1 == capacity) text = checked(realloc(text, capacity *= 2));
    }
    text[length] = '\0';
    return text;
}

// Splits text into lines in place, and returns where each one starts.
static char **split_lines(char *text, size_t *count) {
    size_t newlines = 0;
    for (char *at = text; *at != '\0'; at++) newlines += *at == '\n';
    char **lines = checked(malloc((newlines + 1) * sizeof(char *)));
    *count = 0;
    for (char *at = text; *at != '\0';) {
        lines[(*count)++] = at;
        char *newline = strchr(at, '\n');
        if (newline == NULL) break;
        *newline = '\0';
        at = newline + 1;
    }
    return lines;
}

// Splits a line into whole numbers, in place. Returns how many there are, or
// -1 if a word isn't a whole number.
static long numbers(char *line, long *out, long max) {
    long count = 0;
    for (char *word = strtok(line, " \t"); word != NULL; word = strtok(NULL, " \t")) {
        if (strspn(word, "0123456789") != strlen(word) || count == max) return -1;
        out[count++] = strtol(word, NULL, 10);
    }
    return count;
}

struct transfer {
    long source, target, amount;
};

// One lock for each account, so that transfers between different accounts
// can run at the same time. Each lock guards the balance with its number.
struct bank {
    long *balances;
    pthread_mutex_t *locks;
};

static void transfer(struct bank *bank, struct transfer t) {
    if (t.source == t.target) return;  // locking one mutex twice waits forever
    // Every worker takes the two locks in the same order, lower account
    // first, so no two workers can each hold the lock the other wants.
    long first = t.source < t.target ? t.source : t.target;
    long second = t.source < t.target ? t.target : t.source;
    pthread_mutex_lock(&bank->locks[first]);
    pthread_mutex_lock(&bank->locks[second]);
    long source_balance = bank->balances[t.source];
    long target_balance = bank->balances[t.target];
    // Lets another thread run between the read and the write, as it could
    // while a real bank waits on its database.
    sched_yield();
    bank->balances[t.source] = source_balance - t.amount;
    bank->balances[t.target] = target_balance + t.amount;
    pthread_mutex_unlock(&bank->locks[second]);
    pthread_mutex_unlock(&bank->locks[first]);
}

// Worker i applies transfers i, i + step, i + 2 * step, and so on.
struct job {
    struct bank *bank;
    struct transfer *transfers;
    size_t count, start, step;
};

static void *work(void *arg) {
    struct job *job = arg;
    for (size_t i = job->start; i < job->count; i += job->step) {
        transfer(job->bank, job->transfers[i]);
    }
    return NULL;
}

static const char *plural(long count) { return count == 1 ? "" : "s"; }

int main(void) {
    char *input = read_all(stdin), empty[] = "";
    size_t line_count;
    char **lines = split_lines(input, &line_count);

    long workers;
    if (numbers(line_count > 0 ? lines[0] : empty, &workers, 1) != 1 || workers < 1 ||
        workers > MAX_WORKERS) {
        printf("error: line 1 must be the number of workers, from 1 to %d\n", MAX_WORKERS);
        free(lines);
        free(input);
        return 0;
    }
    char *second = line_count > 1 ? lines[1] : empty;
    long *balances = checked(malloc((strlen(second) / 2 + 1) * sizeof(long)));
    long accounts = numbers(second, balances, (long)strlen(second) / 2 + 1);
    if (accounts < 1) {
        printf("error: line 2 must be the starting balances\n");
        free(balances);
        free(lines);
        free(input);
        return 0;
    }

    struct transfer *transfers = checked(malloc((line_count + 1) * sizeof(struct transfer)));
    size_t count = 0;
    for (size_t i = 2; i < line_count; i++) {
        long words[3];
        long n = numbers(lines[i], words, 3);
        if (n == 0) continue;
        if (n != 3) {
            printf("error: line %zu: a transfer is three numbers: from, to, and amount\n", i + 1);
        } else if (words[0] >= accounts || words[1] >= accounts) {
            printf("error: line %zu: no account %ld\n", i + 1,
                   words[0] >= accounts ? words[0] : words[1]);
        } else {
            transfers[count++] = (struct transfer){words[0], words[1], words[2]};
        }
    }

    struct bank bank = {balances, checked(malloc(accounts * sizeof(pthread_mutex_t)))};
    for (long i = 0; i < accounts; i++) pthread_mutex_init(&bank.locks[i], NULL);
    pthread_t threads[MAX_WORKERS];
    struct job jobs[MAX_WORKERS];
    for (long i = 0; i < workers; i++) {
        jobs[i] = (struct job){&bank, transfers, count, (size_t)i, (size_t)workers};
        if (pthread_create(&threads[i], NULL, work, &jobs[i]) != 0) {
            fprintf(stderr, "can't start thread %ld\n", i);
            return 1;
        }
    }
    for (long i = 0; i < workers; i++) pthread_join(threads[i], NULL);

    printf("%ld worker%s, %ld account%s, %zu transfer%s\n", workers, plural(workers), accounts,
           plural(accounts), count, plural((long)count));
    for (long i = 0; i < workers; i++) {
        long share = 0, moved = 0;
        for (size_t j = i; j < count; j += workers) {
            share++;
            moved += transfers[j].amount;
        }
        printf("worker %ld: %ld transfer%s, %ld moved\n", i, share, plural(share), moved);
    }
    long total = 0;
    for (long i = 0; i < accounts; i++) {
        printf("account %ld: %ld\n", i, balances[i]);
        total += balances[i];
        pthread_mutex_destroy(&bank.locks[i]);
    }
    printf("total: %ld\n", total);
    free(bank.locks);
    free(transfers);
    free(balances);
    free(lines);
    free(input);
    return 0;
}
