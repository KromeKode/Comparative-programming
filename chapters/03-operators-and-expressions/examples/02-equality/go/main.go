package main

import (
	"fmt"
	"math"
	"slices"
	"strings"
)

func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func main() {
	a := "hello"
	b := strings.Join([]string{"hel", "lo"}, "") // built separately, same characters
	fmt.Println("strings with ==:", yesNo(a == b))

	// Arrays have a fixed size, and == compares them element by element.
	xs := [3]int{1, 2, 3}
	ys := [3]int{1, 2, 3}
	fmt.Println("arrays with ==:", yesNo(xs == ys))

	// Slices cannot be compared with == at all: it is a compile error.
	s1 := []int{1, 2, 3}
	s2 := []int{1, 2, 3}
	fmt.Println("slices with slices.Equal:", yesNo(slices.Equal(s1, s2)))

	nan := math.NaN()
	fmt.Println("NaN == NaN:", yesNo(nan == nan))
}
