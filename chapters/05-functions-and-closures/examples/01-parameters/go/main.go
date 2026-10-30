package main

import "fmt"

// Go has no default or named arguments, so every caller passes every argument.
// Parameters of the same type can share one type name.
func greet(name, greeting, punctuation string) string {
	return greeting + ", " + name + punctuation
}

// ...int collects any number of int arguments into a slice named values.
func total(values ...int) int {
	sum := 0
	for _, value := range values {
		sum += value
	}
	return sum
}

func main() {
	fmt.Println(greet("Ada", "Hello", "!"))
	fmt.Println(greet("Grace", "Hi", "!"))
	fmt.Println(greet("Linus", "Hello", "?"))

	fmt.Println("total() =", total())
	fmt.Println("total(1, 2, 3) =", total(1, 2, 3))
	numbers := []int{4, 3, 2, 1}
	fmt.Println("total of [4, 3, 2, 1] =", total(numbers...)) // ... spreads a slice
}
