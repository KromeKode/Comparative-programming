package main

import "fmt"

func main() {
	// Integer division truncates toward zero, as in C.
	fmt.Println("7 / 2 =", 7/2)
	fmt.Println("-7 / 2 =", -7/2)
	// The remainder takes the sign of the left operand.
	fmt.Println("7 % 3 =", 7%3)
	fmt.Println("-7 % 3 =", -7%3)
	fmt.Println("7 % -3 =", 7%-3)
	// 7.0 is an untyped float constant, so the whole expression is a float.
	fmt.Println("7.0 / 2 =", 7.0/2)
}
