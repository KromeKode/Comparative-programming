package main

import "fmt"

func addOne(x int) int {
	return x + 1
}

func triple(x int) int {
	return x * 3
}

// func(int) int is the type of any function from int to int.
func applyTwice(f func(int) int, x int) int {
	return f(f(x))
}

func compose(outer, inner func(int) int) func(int) int {
	return func(x int) int {
		return outer(inner(x))
	}
}

func main() {
	fmt.Println("apply_twice(add_one, 5) =", applyTwice(addOne, 5))
	fmt.Println("apply_twice(triple, 5) =", applyTwice(triple, 5))
	square := func(x int) int { return x * x } // a function literal, with no name
	fmt.Println("apply_twice(square, 3) =", applyTwice(square, 3))
	addThenTriple := compose(triple, addOne)
	fmt.Println("compose(triple, add_one)(5) =", addThenTriple(5))
}
