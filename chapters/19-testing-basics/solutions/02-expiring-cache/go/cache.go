// Package cache keeps each value for a while after it was put, and then forgets it.
package cache

import "time"

// A Clock tells the time, in whole seconds.
type Clock interface {
	Now() int64
}

// SystemClock is the computer's clock, which counts the seconds since 1970.
type SystemClock struct{}

func (SystemClock) Now() int64 { return time.Now().Unix() }

type entry struct {
	value   string
	expires int64 // the time the entry stops being live
}

func (e entry) isLive(now int64) bool { return now < e.expires }

// A Cache keeps each value for ttl seconds after it was put.
type Cache struct {
	ttl     int64
	clock   Clock
	entries map[string]entry
}

// New returns an empty cache that reads the time from clock.
func New(ttl int64, clock Clock) *Cache {
	return &Cache{ttl: ttl, clock: clock, entries: map[string]entry{}}
}

// Put stores value under key for the next ttl seconds, replacing any value there.
func (c *Cache) Put(key, value string) {
	now := c.clock.Now()
	for k, e := range c.entries {
		if !e.isLive(now) {
			delete(c.entries, k)
		}
	}
	c.entries[key] = entry{value: value, expires: now + c.ttl}
}

// Get returns the value under key, and whether there is one that hasn't expired.
func (c *Cache) Get(key string) (string, bool) {
	e, ok := c.entries[key]
	if !ok || !e.isLive(c.clock.Now()) {
		return "", false
	}
	return e.value, true
}

// Len returns how many entries haven't expired.
func (c *Cache) Len() int {
	now := c.clock.Now()
	n := 0
	for _, e := range c.entries {
		if e.isLive(now) {
			n++
		}
	}
	return n
}
