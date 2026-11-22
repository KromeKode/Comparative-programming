package roman

import (
	"errors"
	"fmt"
	"testing"
)

// A table of cases, and a loop that runs each one as a subtest, named after its
// number, so a failure says which case it was, and `go test -run 'TestToRoman$/1994'`
// runs one of them.
func TestToRoman(t *testing.T) {
	tests := []struct {
		n    int
		want string
	}{
		{1, "I"},
		{4, "IV"},
		{9, "IX"},
		{14, "XIV"},
		{40, "XL"},
		{90, "XC"},
		{400, "CD"},
		{1994, "MCMXCIV"},
		{2024, "MMXXIV"},
		{3999, "MMMCMXCIX"},
	}
	for _, tt := range tests {
		t.Run(fmt.Sprint(tt.n), func(t *testing.T) {
			got, err := ToRoman(tt.n)
			if err != nil || got != tt.want {
				t.Errorf("ToRoman(%d) = %q, %v; want %q, nil", tt.n, got, err, tt.want)
			}
		})
	}
}

// The errors are part of what ToRoman promises, so they get tests too.
func TestToRomanOutOfRange(t *testing.T) {
	for _, n := range []int{0, 4000, -1} {
		if _, err := ToRoman(n); !errors.Is(err, ErrOutOfRange) {
			t.Errorf("ToRoman(%d) error = %v, want ErrOutOfRange", n, err)
		}
	}
}
