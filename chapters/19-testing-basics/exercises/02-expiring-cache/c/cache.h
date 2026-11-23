#ifndef CACHE_H
#define CACHE_H

#include <stdbool.h>
#include <stddef.h>

// Something that tells the time, in whole seconds: now(context) reads it.
struct cache_clock {
    long long (*now)(void *context);
    void *context;
};

// The computer's clock, which counts the seconds since 1970.
extern const struct cache_clock cache_system_clock;

// A cache keeps each value for ttl seconds after it was put, and then forgets it.
struct cache;

// Returns a cache that reads the time from clock, or NULL if there is no memory.
struct cache *cache_new(long long ttl, struct cache_clock clock);

void cache_free(struct cache *cache);

// Stores a copy of value under key for the next ttl seconds, replacing any value
// that is there. Returns false if there is no memory.
bool cache_put(struct cache *cache, const char *key, const char *value);

// Returns the value under key, or NULL if there is none or it has expired. The
// string belongs to the cache, and lasts until the next cache_put or cache_free.
const char *cache_get(const struct cache *cache, const char *key);

// Returns how many entries haven't expired.
size_t cache_size(const struct cache *cache);

#endif
