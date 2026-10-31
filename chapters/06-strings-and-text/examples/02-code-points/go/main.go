package main

import (
	"fmt"
	"strings"
)

func main() {
	s := "Zo\u00eb \U0001F44D\U0001F3FD"

	var bytes strings.Builder
	for i := 0; i < len(s); i++ { // indexing a string gives bytes
		fmt.Fprintf(&bytes, " %02X", s[i])
	}
	fmt.Println("UTF-8 bytes:" + bytes.String())

	var codePoints strings.Builder
	for _, r := range s { // ranging over a string decodes runes
		fmt.Fprintf(&codePoints, " U+%04X", r)
	}
	fmt.Println("code points:" + codePoints.String())

	// s[:3] would cut 'ë' in half. Convert to runes first.
	fmt.Println("first 3 code points:", string([]rune(s)[:3]))
}
