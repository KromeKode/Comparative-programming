// Write tests for Parse and Format here, as TestXxx functions. One is here to start
// with, and the README lists what they promise.
package duration

import "testing"

func TestParse(t *testing.T) {
	if got, err := Parse("1h30m"); got != 5400 || err != nil {
		t.Errorf(`Parse("1h30m") = %d, %v, want 5400, nil`, got, err)
	}
}
