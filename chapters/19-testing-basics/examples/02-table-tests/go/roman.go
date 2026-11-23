// Package roman writes numbers in Roman numerals.
package roman

import (
	"errors"
	"fmt"
	"strings"
)

// ErrOutOfRange is the error for a number that has no Roman numeral.
var ErrOutOfRange = errors.New("out of range")

var numerals = []struct {
	value   int
	letters string
}{
	{1000, "M"},
	{900, "CM"},
	{500, "D"},
	{400, "CD"},
	{100, "C"},
	{90, "XC"},
	{50, "L"},
	{40, "XL"},
	{10, "X"},
	{9, "IX"},
	{5, "V"},
	{4, "IV"},
	{1, "I"},
}

// ToRoman returns n in Roman numerals, or an error that wraps ErrOutOfRange if n
// is outside 1 to 3999.
func ToRoman(n int) (string, error) {
	if n < 1 || n > 3999 {
		return "", fmt.Errorf("roman: %d is %w", n, ErrOutOfRange)
	}
	var numeral strings.Builder
	for _, step := range numerals {
		for ; n >= step.value; n -= step.value {
			numeral.WriteString(step.letters)
		}
	}
	return numeral.String(), nil
}
