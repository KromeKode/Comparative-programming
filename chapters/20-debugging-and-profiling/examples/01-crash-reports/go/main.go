// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
package main

import (
	"fmt"
	"io"
	"os"
	"strings"
	"unicode"
)

func countLetters(text string) [26]int {
	var counts [26]int
	for _, c := range text {
		if unicode.IsLetter(c) {
			counts[c-'a']++
		}
	}
	return counts
}

func main() {
	text, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	counts := countLetters(string(text))
	for i, n := range counts {
		if n > 0 {
			fmt.Println(string(rune('a'+i)), n, strings.Repeat("#", n))
		}
	}
}
