package main

import "fmt"

// Go functions can return several values. Naming them documents what they are.
func minMax(values []int) (lowest, highest int) {
	lowest, highest = values[0], values[0]
	for _, value := range values {
		lowest = min(lowest, value)
		highest = max(highest, value)
	}
	return lowest, highest
}

func divMod(a, b int) (int, int) {
	return a / b, a % b
}

func main() {
	lowest, highest := minMax([]int{3, -1, 4, 1, 5})
	fmt.Println("min and max of [3, -1, 4, 1, 5]:", lowest, highest)

	quotient, remainder := divMod(17, 5)
	fmt.Printf("17 divided by 5: %d remainder %d\n", quotient, remainder)

	first, second := "hello", "world"
	first, second = second, first // both right-hand values are read before assigning
	fmt.Println("swapped:", first, second)
}
