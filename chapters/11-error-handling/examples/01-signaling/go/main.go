package main

import (
	"errors"
	"fmt"
	"strconv"
)

func main() {
	inputs := []string{"42", "4x2", "", "99999999999999999999"}

	// Atoi returns two values: the number and an error. The error is nil when
	// it worked, and the caller is expected to check it right away.
	fmt.Println("strconv.Atoi:")
	for _, text := range inputs {
		value, err := strconv.Atoi(text)
		if err != nil {
			fmt.Printf("  %q -> error: %v\n", text, err)
			continue
		}
		fmt.Printf("  %q -> %d\n", text, value)
	}

	// The error wraps a sentinel value that says which kind it is.
	fmt.Println("errors.Is:")
	for _, text := range inputs[1:] {
		_, err := strconv.Atoi(text)
		syntax := errors.Is(err, strconv.ErrSyntax)
		tooBig := errors.Is(err, strconv.ErrRange)
		fmt.Printf("  %q -> syntax: %t, range: %t\n", text, syntax, tooBig)
	}
}
