package wrap

import (
	"slices"
	"testing"
)

func TestWrap(t *testing.T) {
	tests := []struct {
		text  string
		width int
		want  []string
	}{
		// Lines as long as they can be, and a last line that is shorter.
		{"the quick brown fox jumps", 10, []string{"the quick", "brown fox", "jumps"}},
		// A line can be exactly the width, and no longer: "ab cd" is 5.
		{"the quick brown", 9, []string{"the quick", "brown"}},
		{"ab cd", 4, []string{"ab", "cd"}},
		{"ab cd", 5, []string{"ab cd"}},
		// One space between words, whatever the text had.
		{"  the   quick  ", 20, []string{"the quick"}},
		// No words, no lines.
		{"", 10, nil},
		{"   ", 10, nil},
		// A word longer than the width gets a line of its own, wherever it is.
		{"extraordinary", 5, []string{"extraordinary"}},
		{"an extraordinary day", 5, []string{"an", "extraordinary", "day"}},
		{"a b", 1, []string{"a", "b"}},
	}
	for _, tt := range tests {
		// slices.Equal counts a nil slice and an empty one as equal, and so does
		// this test: both are no lines.
		if got := Wrap(tt.text, tt.width); !slices.Equal(got, tt.want) {
			t.Errorf("Wrap(%q, %d) = %q, want %q", tt.text, tt.width, got, tt.want)
		}
	}
}
