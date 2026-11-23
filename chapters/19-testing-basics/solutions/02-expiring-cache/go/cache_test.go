package cache

import "testing"

// A fakeClock says whatever time the test sets.
type fakeClock struct {
	time int64
}

func (c *fakeClock) Now() int64 { return c.time }

// t.Helper marks these as helpers, so a failure gives the line of the test that
// called them, and not their own.
func checkGet(t *testing.T, c *Cache, key, want string) {
	t.Helper()
	if got, ok := c.Get(key); !ok || got != want {
		t.Errorf("Get(%q) = %q, %v, want %q, true", key, got, ok, want)
	}
}

func checkMissing(t *testing.T, c *Cache, key string) {
	t.Helper()
	if got, ok := c.Get(key); ok {
		t.Errorf("Get(%q) = %q, true, want no value", key, got)
	}
}

func checkLen(t *testing.T, c *Cache, want int) {
	t.Helper()
	if got := c.Len(); got != want {
		t.Errorf("Len() = %d, want %d", got, want)
	}
}

func TestPutThenGet(t *testing.T) {
	c := New(10, &fakeClock{})
	c.Put("a", "1")
	checkGet(t, c, "a", "1")
	checkMissing(t, c, "b")
	checkLen(t, c, 1)
}

// An entry put at time t is there until t + ttl, and gone from then on.
func TestExpiresAfterTTL(t *testing.T) {
	clock := &fakeClock{time: 100}
	c := New(10, clock)
	c.Put("a", "1")
	clock.time = 109
	checkGet(t, c, "a", "1")
	clock.time = 110
	checkMissing(t, c, "a")
	checkLen(t, c, 0)
}

// Getting an entry doesn't make it last longer.
func TestGetDoesNotRefresh(t *testing.T) {
	clock := &fakeClock{}
	c := New(10, clock)
	c.Put("a", "1")
	clock.time = 5
	checkGet(t, c, "a", "1")
	clock.time = 10
	checkMissing(t, c, "a")
}

// Putting a key again replaces its value, and its ttl starts again.
func TestPutReplaces(t *testing.T) {
	clock := &fakeClock{}
	c := New(10, clock)
	c.Put("a", "1")
	clock.time = 5
	c.Put("a", "2")
	checkLen(t, c, 1)
	clock.time = 14
	checkGet(t, c, "a", "2")
	clock.time = 15
	checkMissing(t, c, "a")
}

// Each key has its own time: one expiring, or a put of another, leaves the rest.
func TestKeysAreSeparate(t *testing.T) {
	clock := &fakeClock{}
	c := New(10, clock)
	c.Put("a", "1")
	clock.time = 5
	c.Put("b", "2")
	checkGet(t, c, "a", "1")
	checkGet(t, c, "b", "2")
	checkLen(t, c, 2)
	clock.time = 10
	checkMissing(t, c, "a")
	checkGet(t, c, "b", "2")
	checkLen(t, c, 1)
}
