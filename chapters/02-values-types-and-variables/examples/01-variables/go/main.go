package main

import "fmt"

const maxPlayers = 4 // a compile-time constant
const pi = 3.14159

func main() {
	count := 0               // := declares a variable and infers its type (int)
	var birthYear int = 1815 // Go has no read-only variables, only constants
	ready := true

	count += 3

	fmt.Println("count =", count)
	fmt.Println("birth year =", birthYear)
	fmt.Println("pi =", pi)
	fmt.Println("max players =", maxPlayers)
	fmt.Println("ready =", ready)
}
