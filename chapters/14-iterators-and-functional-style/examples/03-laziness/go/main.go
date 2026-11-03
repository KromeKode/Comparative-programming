package main

import (
	"fmt"
	"iter"
	"slices"
)

// Both functions say when they run, so the output shows the order of the work.
func square(n int) int {
	fmt.Println("  square", n)
	return n * n
}

func even(x int) bool {
	fmt.Println("  even?", x)
	return x%2 == 0
}

// Go's iter package has no Map, Filter, or Take, but each is a few lines:
// a push iterator that wraps another one.
func Map[T, U any](seq iter.Seq[T], f func(T) U) iter.Seq[U] {
	return func(yield func(U) bool) {
		for item := range seq {
			if !yield(f(item)) {
				return
			}
		}
	}
}

func Filter[T any](seq iter.Seq[T], keep func(T) bool) iter.Seq[T] {
	return func(yield func(T) bool) {
		for item := range seq {
			if keep(item) && !yield(item) {
				return
			}
		}
	}
}

func Take[T any](seq iter.Seq[T], n int) iter.Seq[T] {
	return func(yield func(T) bool) {
		if n <= 0 {
			return
		}
		count := 0
		for item := range seq {
			if !yield(item) {
				return
			}
			count++
			if count == n {
				return // leaving the loop tells seq to stop
			}
		}
	}
}

// Naturals counts up forever. It stops only when its caller does.
func Naturals() iter.Seq[int] {
	return func(yield func(int) bool) {
		for n := 1; yield(n); n++ {
		}
	}
}

func main() {
	numbers := []int{1, 2, 3, 4, 5}

	// Eager: each loop finishes the whole slice before the next one starts.
	fmt.Println("eager:")
	var squares, evens []int
	for _, n := range numbers {
		squares = append(squares, square(n))
	}
	for _, x := range squares {
		if even(x) {
			evens = append(evens, x)
		}
	}
	fmt.Println("  first two:", evens[0], evens[1])

	// Lazy: the adapters only describe the work, and slices.Collect pulls
	// each number through all of them before it asks for the next.
	fmt.Println("lazy:")
	firstTwo := slices.Collect(Take(Filter(Map(slices.Values(numbers), square), even), 2))
	fmt.Println("  first two:", firstTwo[0], firstTwo[1])

	bigEven := func(x int) bool { return x%2 == 0 && x > 50 }
	squared := Map(Naturals(), func(n int) int { return n * n })
	fmt.Print("endless:")
	for x := range Take(Filter(squared, bigEven), 3) {
		fmt.Print(" ", x)
	}
	fmt.Println()
}
