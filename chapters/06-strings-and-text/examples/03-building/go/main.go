package main

import (
	"fmt"
	"strconv"
	"strings"
)

func main() {
	fruits := []string{"apple", "banana", "cherry"}
	fmt.Println("joined:", strings.Join(fruits, ", "))

	// Strings are immutable, so += copies. strings.Builder appends in place.
	var built strings.Builder
	for n := 1; n <= 5; n++ {
		if n > 1 {
			built.WriteByte('-')
		}
		built.WriteString(strconv.Itoa(n))
	}
	fmt.Println("built:", built.String())

	fmt.Printf("padded: [%-8s] [%8d]\n", "Ada", 90) // - means left-align
	fmt.Printf("zero-padded: %05d\n", 42)
	fmt.Printf("two decimals: %.2f\n", 3.14159)
}
