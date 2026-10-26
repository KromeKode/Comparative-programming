package main

import "fmt"

func main() {
	fmt.Print("up:")
	for i := range 5 { // Go 1.22 and later: range over an integer
		fmt.Print(" ", i)
	}
	fmt.Println()

	fmt.Print("down:")
	for i := 5; i >= 1; i-- {
		fmt.Print(" ", i)
	}
	fmt.Println()

	fmt.Print("by threes:")
	for i := 0; i < 10; i += 3 {
		fmt.Print(" ", i)
	}
	fmt.Println()

	fmt.Print("halving:")
	n := 100
	for n > 0 { // Go has no while: a for with only a condition does that job
		fmt.Print(" ", n)
		n /= 2
	}
	fmt.Println()

	colors := []string{"red", "green", "blue"}
	fmt.Print("colors:")
	for _, color := range colors {
		fmt.Print(" ", color)
	}
	fmt.Println()

	fmt.Print("numbered:")
	for i, color := range colors {
		fmt.Printf(" %d=%s", i+1, color)
	}
	fmt.Println()

	// No do-while either: loop forever and break at the end.
	m := 100
	for {
		m *= 2
		if m >= 50 {
			break
		}
	}
	fmt.Println("do-while:", m)
}
