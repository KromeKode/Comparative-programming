// Files whose names end in _test.go are compiled only by go test. This one is
// in package leap itself, so it could use the package's unexported names too.
package leap

import "testing"

// go test runs every function named TestXxx that takes a *testing.T.
func TestDivisibleBy4(t *testing.T) {
	if !IsLeapYear(2024) {
		t.Error("IsLeapYear(2024) = false, want true")
	}
	if IsLeapYear(2023) {
		t.Error("IsLeapYear(2023) = true, want false")
	}
}

func TestCenturies(t *testing.T) {
	if IsLeapYear(1900) {
		t.Error("IsLeapYear(1900) = true, want false")
	}
	if !IsLeapYear(2000) {
		t.Error("IsLeapYear(2000) = false, want true")
	}
}
