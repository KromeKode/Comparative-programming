package main

import (
	"fmt"
	"iter"
)

// A Ring keeps the last three items pushed into it, oldest first.
type Ring struct {
	items      [3]int
	start, len int
}

func (r *Ring) Push(item int) {
	r.items[(r.start+r.len)%3] = item
	if r.len < 3 {
		r.len++
	} else {
		r.start = (r.start + 1) % 3 // the oldest item is overwritten
	}
}

// All returns a push iterator (Go 1.23): a function that calls yield with
// each item, and stops early if yield returns false. A for loop can range
// over it, and the loop body becomes yield.
func (r *Ring) All() iter.Seq[int] {
	return func(yield func(int) bool) {
		for i := range r.len {
			if !yield(r.items[(r.start+i)%3]) {
				return
			}
		}
	}
}

func main() {
	var ring Ring
	for i := 1; i <= 5; i++ {
		ring.Push(i)
	}

	fmt.Print("loop:")
	for item := range ring.All() {
		fmt.Print(" ", item)
	}
	fmt.Print("\nagain:")
	for item := range ring.All() { // each call to All starts again
		fmt.Print(" ", item)
	}
	fmt.Println()

	// iter.Pull turns a push iterator into next and stop functions.
	next, stop := iter.Pull(ring.All())
	fmt.Print("by hand:")
	sep := " "
	for item, ok := next(); ok; item, ok = next() {
		fmt.Print(sep, item)
		sep = ", then "
	}
	fmt.Println(", then nothing")
	stop()

	// A range loop always starts a push iterator from the beginning, so
	// carrying on after a next means calling next.
	next, stop = iter.Pull(ring.All())
	defer stop()
	first, _ := next()
	fmt.Printf("resumed: took %d, then the loop got", first)
	for item, ok := next(); ok; item, ok = next() {
		fmt.Print(" ", item)
	}
	fmt.Println()
}
