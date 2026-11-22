#include "cache.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

static long long system_now(void *context) {
    (void)context;
    return (long long)time(NULL);
}

const struct cache_clock cache_system_clock = {system_now, NULL};

struct entry {
    char *key;
    char *value;
    long long expires;  // the time the entry stops being live
};

struct cache {
    long long ttl;
    struct cache_clock clock;
    struct entry *entries;
    size_t count;
    size_t capacity;
};

static long long clock_now(const struct cache *cache) {
    return cache->clock.now(cache->clock.context);
}

static bool is_live(const struct entry *entry, long long now) { return now < entry->expires; }

static char *copy(const char *s) {
    size_t size = strlen(s) + 1;
    char *p = malloc(size);
    if (p != NULL) memcpy(p, s, size);
    return p;
}

static struct entry *find(const struct cache *cache, const char *key) {
    for (size_t i = 0; i < cache->count; i++) {
        if (strcmp(cache->entries[i].key, key) == 0) return &cache->entries[i];
    }
    return NULL;
}

// Adds an entry for key with no value yet, or returns NULL if there is no memory.
static struct entry *add(struct cache *cache, const char *key) {
    if (cache->count == cache->capacity) {
        size_t capacity = cache->capacity == 0 ? 8 : 2 * cache->capacity;
        struct entry *entries = realloc(cache->entries, capacity * sizeof *entries);
        if (entries == NULL) return NULL;
        cache->entries = entries;
        cache->capacity = capacity;
    }
    char *copied = copy(key);
    if (copied == NULL) return NULL;
    struct entry *entry = &cache->entries[cache->count++];
    *entry = (struct entry){.key = copied};
    return entry;
}

// Frees the entries that have expired, and closes up the gaps they leave.
static void forget_expired(struct cache *cache, long long now) {
    size_t kept = 0;
    for (size_t i = 0; i < cache->count; i++) {
        struct entry *entry = &cache->entries[i];
        if (is_live(entry, now)) {
            cache->entries[kept++] = *entry;
        } else {
            free(entry->key);
            free(entry->value);
        }
    }
    cache->count = kept;
}

struct cache *cache_new(long long ttl, struct cache_clock clock) {
    struct cache *cache = malloc(sizeof *cache);
    if (cache != NULL) *cache = (struct cache){.ttl = ttl, .clock = clock};
    return cache;
}

void cache_free(struct cache *cache) {
    if (cache == NULL) return;
    for (size_t i = 0; i < cache->count; i++) {
        free(cache->entries[i].key);
        free(cache->entries[i].value);
    }
    free(cache->entries);
    free(cache);
}

bool cache_put(struct cache *cache, const char *key, const char *value) {
    long long now = clock_now(cache);
    forget_expired(cache, now);
    char *copied = copy(value);
    if (copied == NULL) return false;
    struct entry *entry = find(cache, key);
    if (entry == NULL) entry = add(cache, key);
    if (entry == NULL) {
        free(copied);
        return false;
    }
    free(entry->value);
    entry->value = copied;
    entry->expires = now + cache->ttl;
    return true;
}

const char *cache_get(const struct cache *cache, const char *key) {
    const struct entry *entry = find(cache, key);
    if (entry == NULL || !is_live(entry, clock_now(cache))) return NULL;
    return entry->value;
}

size_t cache_size(const struct cache *cache) {
    long long now = clock_now(cache);
    size_t live = 0;
    for (size_t i = 0; i < cache->count; i++) {
        if (is_live(&cache->entries[i], now)) live++;
    }
    return live;
}
