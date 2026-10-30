package main

import "fmt"

func makeAdder(n int) func(int) int {
	return func(x int) int { return x + n }
}

// Go closures capture variables, not values: total lives on after makeAccumulator returns.
func makeAccumulator() func(int) int {
	total := 0
	return func(x int) int {
		total += x
		return total
	}
}

func main() {
	add5 := makeAdder(5)
	add10 := makeAdder(10)
	fmt.Println("add5(10) =", add5(10))
	fmt.Println("add10(10) =", add10(10))

	running := makeAccumulator()
	fmt.Println("running total:", running(3), running(5), running(2))

	var made []func() int
	for i := 0; i < 3; i++ {
		// Since Go 1.22 each pass gets a fresh i. Before, all three shared one i
		// and this printed 3 3 3.
		made = append(made, func() int { return i })
	}
	fmt.Print("made in a loop:")
	for _, f := range made {
		fmt.Print(" ", f())
	}
	fmt.Println()
}
