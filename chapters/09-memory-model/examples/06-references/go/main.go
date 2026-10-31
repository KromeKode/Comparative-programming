package main

import (
	"fmt"
	"slices"
)

type Point struct {
	X, Y int
}

// Returning the address of a local is fine in Go. The compiler sees that n
// outlives the call and puts it on the heap.
func newCounter() *int {
	n := 0
	return &n
}

func main() {
	// Arrays are values: assignment copies every item.
	a := [3]int{1, 2, 3}
	b := a
	b[0] = 100
	fmt.Printf("b := a copies an array: a = %v, b = %v\n", a, b)

	// A slice is a small header (pointer, length, capacity) over an array.
	// Copying the header shares the array.
	s := []int{1, 2, 3}
	t := s
	t[0] = 100
	fmt.Printf("t := s shares a slice's items: s = %v\n", s)

	// append writes into the shared array when there is room.
	base := make([]int, 3, 4) // length 3, room for 4
	x := append(base, 1)
	y := append(base, 2) // the same spare slot as x
	fmt.Printf("two appends into one spare slot: x = %v, y = %v\n", x, y)

	// Structs are values too.
	p := Point{1, 2}
	q := p
	q.X = 10
	fmt.Printf("q := p copies a struct: p = %v, q = %v\n", p, q)

	// A pointer shares on purpose. r.X is short for (*r).X.
	r := &p
	r.X = 10
	fmt.Printf("r := &p shares it: after r.X = 10, p = %v\n", p)

	// slices.Clone copies the outer slice only, so the rows are shared.
	grid := [][]int{{1, 2}, {3, 4}}
	shallow := slices.Clone(grid)
	shallow[0][0] = 9
	fmt.Printf("slices.Clone(grid) is shallow: grid = %v\n", grid)

	c := newCounter()
	*c++
	fmt.Printf("a pointer to a local outlives its function: *c = %d\n", *c)
}
