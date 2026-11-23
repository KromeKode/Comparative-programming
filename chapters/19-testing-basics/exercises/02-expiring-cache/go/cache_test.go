// Write tests for the cache here, as TestXxx functions. One is here to start with,
// on the computer's clock, which can't show what happens as time passes without the
// test waiting for it. The README lists what the cache promises.
package cache

import "testing"

func TestPutThenGet(t *testing.T) {
	c := New(60, SystemClock{})
	c.Put("a", "1")
	if got, ok := c.Get("a"); !ok || got != "1" {
		t.Errorf(`Get("a") = %q, %v, want "1", true`, got, ok)
	}
}
