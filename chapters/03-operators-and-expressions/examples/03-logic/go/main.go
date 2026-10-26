package main

import (
	"cmp"
	"fmt"
)

var calls = 0 // counts how many times the right side ran

func bigRatio(x int) bool {
	calls++         // ++ is a statement in Go, never part of an expression
	return 10/x > 1 // dividing by zero here would panic
}

func displayName(name string) string {
	// Go has no ?: operator. cmp.Or returns its first argument that is not the zero value.
	return cmp.Or(name, "stranger")
}

func main() {
	for _, x := range []int{0, 5} {
		// && stops as soon as the left side is false, so bigRatio never sees 0.
		ok := x != 0 && bigRatio(x)
		answer := "no"
		if ok {
			answer = "yes"
		}
		fmt.Printf("x = %d: %s\n", x, answer)
	}
	fmt.Printf("right side ran %d of 2 times\n", calls)
	fmt.Printf("name %q: %s\n", "", displayName(""))
	fmt.Printf("name %q: %s\n", "Ada", displayName("Ada"))
}
