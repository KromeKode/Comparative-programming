package main

import (
	"fmt"
	"slices"
	"strings"
)

func show(items []int) string {
	parts := make([]string, len(items))
	for i, n := range items {
		parts[i] = fmt.Sprint(n)
	}
	return "[" + strings.Join(parts, ", ") + "]"
}

// A slice is a small value (pointer, length, capacity) that refers to an array, so
// the function changes the caller's items.
func setFirst(items []int) {
	items[0] = -1
}

func main() {
	a := []int{1, 2, 3}

	b := a // copies the slice header, not the items
	b[0] = 99
	fmt.Println("b = a, then b[0] = 99: a is", show(a))

	c := slices.Clone(a)
	c[0] = 0
	fmt.Println("c = copy of a, then c[0] = 0: a is", show(a))

	setFirst(a)
	fmt.Println("set_first(a) sets a[0] = -1: a is", show(a))

	grid := [][]int{{0, 0}, {0, 0}}
	g := slices.Clone(grid) // copies the outer slice; the rows are still shared
	g[0][0] = 1
	fmt.Printf("g = copy of grid, then g[0][0] = 1: grid is [%s, %s]\n", show(grid[0]), show(grid[1]))
}
