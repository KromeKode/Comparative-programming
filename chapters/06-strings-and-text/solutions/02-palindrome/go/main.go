package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
	"unicode"
)

// isPalindrome works on runes, so a multi-byte character stays in one piece.
func isPalindrome(text string) bool {
	var kept []rune
	for _, r := range text {
		switch {
		case r > unicode.MaxASCII:
			kept = append(kept, r)
		case unicode.IsLetter(r) || unicode.IsDigit(r):
			kept = append(kept, unicode.ToLower(r))
		}
	}
	for i, j := 0, len(kept)-1; i < j; i, j = i+1, j-1 {
		if kept[i] != kept[j] {
			return false
		}
	}
	return true
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		text := strings.TrimSpace(scanner.Text())
		if text == "" {
			continue
		}
		verdict := "not a palindrome"
		if isPalindrome(text) {
			verdict = "palindrome"
		}
		fmt.Printf("%s: %s\n", text, verdict)
	}
}
