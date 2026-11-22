#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#define LIMIT 200000
#define WORKERS 4

static bool is_prime(long n) {
    if (n < 2) return false;
    for (long d = 2; d * d <= n; d++) {
        if (n % d == 0) return false;
    }
    return true;
}

// A thread runs a function that takes one void * and returns one, so the
// input and the result travel together in a struct.
struct job {
    long start, end;
    long count;
};

static void *count_primes(void *arg) {
    struct job *job = arg;
    job->count = 0;
    for (long n = job->start; n < job->end; n++) {
        if (is_prime(n)) job->count++;
    }
    return NULL;
}

int main(void) {
    pthread_t threads[WORKERS];
    struct job jobs[WORKERS];
    printf("counting primes below %d with %d workers\n", LIMIT, WORKERS);
    for (int i = 0; i < WORKERS; i++) {
        jobs[i] = (struct job){(long)LIMIT * i / WORKERS, (long)LIMIT * (i + 1) / WORKERS, 0};
        if (pthread_create(&threads[i], NULL, count_primes, &jobs[i]) != 0) {
            fprintf(stderr, "can't start thread %d\n", i);
            return 1;
        }
    }

    // pthread_join waits for a thread to finish. Each thread wrote only to its
    // own job, so after the joins, main can read them all.
    long total = 0;
    for (int i = 0; i < WORKERS; i++) {
        pthread_join(threads[i], NULL);
        printf("worker %d: [%ld, %ld) has %ld primes\n", i, jobs[i].start, jobs[i].end,
               jobs[i].count);
        total += jobs[i].count;
    }
    printf("total: %ld primes\n", total);
    return 0;
}
