package main

import (
	"fmt"
	"unicode/utf16"
	"unicode/utf8"
)

func main() {
	// A Go string is a read-only slice of bytes, by convention UTF-8.
	s := "Zo\u00eb \U0001F44D\U0001F3FD"

	fmt.Println("text:", s)
	fmt.Printf("built-in length: %d (bytes)\n", len(s))
	fmt.Println("UTF-8 bytes:", len(s))
	fmt.Println("UTF-16 code units:", len(utf16.Encode([]rune(s))))
	fmt.Println("code points:", utf8.RuneCountInString(s)) // a rune is one code point
}
