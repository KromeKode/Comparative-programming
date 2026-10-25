package main

import (
	"fmt"
	"os"
)

func main() {
	var n int
	if _, err := fmt.Scan(&n); err != nil {
		fmt.Fprintln(os.Stderr, "expected a whole number")
		os.Exit(1)
	}
	for i := 1; i <= n; i++ {
		switch {
		case i%15 == 0:
			fmt.Println("FizzBuzz")
		case i%3 == 0:
			fmt.Println("Fizz")
		case i%5 == 0:
			fmt.Println("Buzz")
		default:
			fmt.Println(i)
		}
	}
}
