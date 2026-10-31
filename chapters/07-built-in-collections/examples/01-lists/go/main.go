package main

import (
	"fmt"
	"slices"
	"strings"
)

// fmt.Println prints a slice as [3 1 4], so this helper adds the commas.
func show(items []int) string {
	parts := make([]string, len(items))
	for i, n := range items {
		parts[i] = fmt.Sprint(n)
	}
	return "[" + strings.Join(parts, ", ") + "]"
}

func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func main() {
	numbers := []int{3, 1, 4, 1, 5}
	fmt.Println("numbers:", show(numbers))
	fmt.Println("length:", len(numbers))
	fmt.Printf("first: %d, last: %d\n", numbers[0], numbers[len(numbers)-1])

	// append returns the slice, which may now point at a new, larger array.
	numbers = append(numbers, 9)
	fmt.Println("append 9:", show(numbers))
	numbers = slices.Insert(numbers, 1, 2)
	fmt.Println("insert 2 at index 1:", show(numbers))
	removed := numbers[0]
	numbers = slices.Delete(numbers, 0, 1)
	fmt.Printf("remove index 0 (was %d): %s\n", removed, show(numbers))

	fmt.Println("contains 4:", yesNo(slices.Contains(numbers, 4)))
	fmt.Println("index of 1:", slices.Index(numbers, 1))

	slices.Sort(numbers)
	fmt.Println("sorted:", show(numbers))
	fmt.Println("slice 1..4:", show(numbers[1:4])) // shares the array, no copy

	slices.Reverse(numbers)
	fmt.Println("reversed:", show(numbers))
}
