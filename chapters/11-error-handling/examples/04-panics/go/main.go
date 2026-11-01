package main

import "fmt"

type User struct {
	Name string
}

var users = map[int]*User{1: {Name: "ada"}}

// findUser returns nil when there is no such user.
func findUser(id int) *User {
	return users[id]
}

// attempt runs action and reports what it returned or what it panicked with.
func attempt(label string, action func() any) {
	defer func() {
		// recover stops the panic, and returns the value it was raised with.
		if value := recover(); value != nil {
			fmt.Printf("%s: panic: %v\n", label, value)
		}
	}()
	fmt.Printf("%s: %v\n", label, action())
}

func main() {
	numbers := []int{10, 20, 30}
	index, zero := 5, 0

	// Each of these is a bug, and each one panics.
	attempt("numbers[5]", func() any { return numbers[index] })
	attempt("10 / zero", func() any { return 10 / zero })
	attempt("findUser(7).Name", func() any { return findUser(7).Name })
	attempt("len(numbers) != 4", func() any {
		// Go has no assert. You check, and panic yourself.
		if len(numbers) != 4 {
			panic("expected 4 numbers")
		}
		return "passed"
	})
}
