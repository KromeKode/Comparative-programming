// Write tests for Wrap here, as TestXxx functions. One is here to start with, and
// the README lists what Wrap promises.
package wrap

import (
	"slices"
	"testing"
)

func TestOneLine(t *testing.T) {
	if got := Wrap("one two", 10); !slices.Equal(got, []string{"one two"}) {
		t.Errorf(`Wrap("one two", 10) = %q, want ["one two"]`, got)
	}
}
