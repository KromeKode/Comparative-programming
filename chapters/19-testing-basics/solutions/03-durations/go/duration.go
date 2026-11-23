// Package duration reads and writes durations such as 1h30m.
package duration

import (
	"fmt"
	"math"
	"strconv"
	"strings"
)

const units = "dhms" // largest first, which is the order they come in

var seconds = [...]int64{86400, 3600, 60, 1} // in each of the units

// Parse returns the number of seconds that text says, such as 5400 for "1h30m".
func Parse(text string) (int64, error) {
	if text == "" {
		return 0, fmt.Errorf("a duration can't be empty")
	}
	var total int64
	nextUnit := 0 // each part's unit comes after the one before it in units
	i := 0
	for i < len(text) {
		start := i
		for i < len(text) && '0' <= text[i] && text[i] <= '9' {
			i++
		}
		if i == start {
			return 0, fmt.Errorf("%q has a part with no number", text)
		}
		if i == len(text) {
			return 0, fmt.Errorf("%q ends with a number that has no unit", text)
		}
		number, err := strconv.ParseInt(text[start:i], 10, 64)
		if err != nil {
			return 0, fmt.Errorf("%q has a number too big to count", text)
		}
		unit := strings.IndexByte(units, text[i])
		if unit == -1 {
			return 0, fmt.Errorf("%q isn't a unit", text[i])
		}
		if unit < nextUnit {
			return 0, fmt.Errorf("%q has its units out of order, or one twice", text)
		}
		if number > (math.MaxInt64-total)/seconds[unit] {
			return 0, fmt.Errorf("%q is too long to count", text)
		}
		total += number * seconds[unit]
		nextUnit = unit + 1
		i++
	}
	return total, nil
}

// Format returns s seconds as a duration, such as "1h30m" for 5400, leaving out the
// units that are 0.
func Format(s int64) string {
	if s == 0 {
		return "0s"
	}
	var b strings.Builder
	for unit, size := range seconds {
		amount := s / size
		s %= size
		if amount > 0 {
			fmt.Fprintf(&b, "%d%c", amount, units[unit])
		}
	}
	return b.String()
}
