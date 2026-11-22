#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

#define COUNT 1000
#define WORKERS 3
#define CAPACITY 16

// C has no channels, so here is one: a ring buffer, a mutex that guards it,
// and two condition variables, which let a thread sleep until another one
// says that something has changed.
struct channel {
    long items[CAPACITY];
    int head, size;
    bool closed;
    pthread_mutex_t lock;
    pthread_cond_t not_empty, not_full;
};

static void channel_init(struct channel *ch) {
    *ch = (struct channel){.head = 0};
    pthread_mutex_init(&ch->lock, NULL);
    pthread_cond_init(&ch->not_empty, NULL);
    pthread_cond_init(&ch->not_full, NULL);
}

static void channel_send(struct channel *ch, long item) {
    pthread_mutex_lock(&ch->lock);
    // pthread_cond_wait unlocks the mutex while it sleeps, and locks it again
    // before it returns. It can wake for no reason, so the test is a loop.
    while (ch->size == CAPACITY) pthread_cond_wait(&ch->not_full, &ch->lock);
    ch->items[(ch->head + ch->size) % CAPACITY] = item;
    ch->size++;
    pthread_cond_signal(&ch->not_empty);
    pthread_mutex_unlock(&ch->lock);
}

// Returns false once the channel is closed and empty.
static bool channel_receive(struct channel *ch, long *item) {
    pthread_mutex_lock(&ch->lock);
    while (ch->size == 0 && !ch->closed) pthread_cond_wait(&ch->not_empty, &ch->lock);
    bool got = ch->size > 0;
    if (got) {
        *item = ch->items[ch->head];
        ch->head = (ch->head + 1) % CAPACITY;
        ch->size--;
        pthread_cond_signal(&ch->not_full);
    }
    pthread_mutex_unlock(&ch->lock);
    return got;
}

static void channel_close(struct channel *ch) {
    pthread_mutex_lock(&ch->lock);
    ch->closed = true;
    pthread_cond_broadcast(&ch->not_empty);  // wake every waiting receiver
    pthread_mutex_unlock(&ch->lock);
}

static struct channel jobs, results;
static atomic_int workers_left = WORKERS;

static void *produce(void *arg) {
    (void)arg;
    for (long n = 1; n <= COUNT; n++) channel_send(&jobs, n);
    channel_close(&jobs);
    return NULL;
}

static void *work(void *arg) {
    (void)arg;
    long n;
    while (channel_receive(&jobs, &n)) channel_send(&results, n * n);
    // The last worker to finish closes the results.
    if (atomic_fetch_sub(&workers_left, 1) == 1) channel_close(&results);
    return NULL;
}

int main(void) {
    channel_init(&jobs);
    channel_init(&results);
    pthread_t producer, workers[WORKERS];
    pthread_create(&producer, NULL, produce, NULL);
    for (int i = 0; i < WORKERS; i++) pthread_create(&workers[i], NULL, work, NULL);

    long received = 0, sum = 0, square;
    while (channel_receive(&results, &square)) {
        received++;
        sum += square;
    }
    pthread_join(producer, NULL);
    for (int i = 0; i < WORKERS; i++) pthread_join(workers[i], NULL);
    printf("%d workers squared 1 to %d\n", WORKERS, COUNT);
    printf("results: %ld\n", received);
    printf("sum: %ld\n", sum);
    return 0;
}
