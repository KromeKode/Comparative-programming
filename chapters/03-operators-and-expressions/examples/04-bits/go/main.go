package main

import (
	"fmt"
	"math/bits"
)

const (
	Read  = 1 << iota // 001: iota counts 0, 1, 2 down the block
	Write             // 010
	Exec              // 100
)

func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func main() {
	perms := Read | Write // turn bits on with |
	fmt.Printf("read | write: %08b\n", perms)
	// & binds tighter than == in Go, so no parentheses are needed.
	fmt.Println("has write?", yesNo(perms&Write != 0))
	fmt.Println("has exec?", yesNo(perms&Exec != 0))
	perms &^= Write // Go's AND NOT operator clears bits
	fmt.Printf("clear write: %08b\n", perms)
	perms ^= Exec // flip a bit with ^
	fmt.Printf("toggle exec: %08b\n", perms)
	fmt.Println("5 << 2 =", 5<<2)
	fmt.Println("-16 >> 2 =", -16>>2)
	fmt.Println("set bits in 10110:", bits.OnesCount(0b10110))
}
