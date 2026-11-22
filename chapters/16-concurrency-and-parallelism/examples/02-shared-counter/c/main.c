#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

#define THREADS 4
#define TIMES 100000

// count += 1 reads count, adds 1, and writes it back. Two threads that do it
// at once can both read the same value, and one of the additions is lost.
// That is a data race, and in C it is undefined behavior.

static long locked_count = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// A mutex lets one thread at a time run the code between lock and unlock.
static void *add_with_mutex(void *arg) {
    (void)arg;
    for (int i = 0; i < TIMES; i++) {
        pthread_mutex_lock(&lock);
        locked_count += 1;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

// An atomic variable can be read, changed, and written as one step.
static atomic_long atomic_count = 0;

static void *add_with_atomic(void *arg) {
    (void)arg;
    for (int i = 0; i < TIMES; i++) atomic_fetch_add(&atomic_count, 1);
    return NULL;
}

static void run(void *(*work)(void *)) {
    pthread_t threads[THREADS];
    for (int i = 0; i < THREADS; i++) pthread_create(&threads[i], NULL, work, NULL);
    for (int i = 0; i < THREADS; i++) pthread_join(threads[i], NULL);
}

int main(void) {
    printf("%d threads add 1, %d times each\n", THREADS, TIMES);
    run(add_with_mutex);
    printf("with a mutex: %ld\n", locked_count);
    run(add_with_atomic);
    printf("with an atomic: %ld\n", atomic_load(&atomic_count));
    return 0;
}
