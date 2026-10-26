package main

import (
	"fmt"
	"math"
)

func main() {
	// Variables, not constants: Go evaluates constant expressions exactly,
	// so the constant expression 0.1 + 0.2 really is 0.3.
	a, b := 0.1, 0.2
	sum := a + b

	fmt.Println("0.1 + 0.2 =", sum)
	fmt.Println("equal to 0.3?", yesNo(sum == 0.3))
	fmt.Println("close to 0.3?", yesNo(math.Abs(sum-0.3) < 1e-9))
}

func yesNo(b bool) string {
	if b {
		return "yes"
	}
	return "no"
}
