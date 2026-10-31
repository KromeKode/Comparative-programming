package main

import (
	"cmp"
	"fmt"
	"io"
	"os"
	"slices"
	"strings"
	"unicode"
)

// A word is a run of ASCII letters and non-ASCII characters.
func isWordRune(r rune) bool {
	return r > unicode.MaxASCII || ('a' <= r && r <= 'z') || ('A' <= r && r <= 'Z')
}

// asciiLower lowercases A to Z only, leaving every other rune alone.
func asciiLower(r rune) rune {
	if 'A' <= r && r <= 'Z' {
		return r + ('a' - 'A')
	}
	return r
}

func main() {
	data, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	counts := map[string]int{}
	// FieldsFunc splits wherever the function returns true.
	words := strings.FieldsFunc(string(data), func(r rune) bool { return !isWordRune(r) })
	for _, word := range words {
		counts[strings.Map(asciiLower, word)]++
	}

	// Most frequent first; ties in byte order, which for UTF-8 is code point order.
	keys := make([]string, 0, len(counts))
	for word := range counts {
		keys = append(keys, word)
	}
	slices.SortFunc(keys, func(a, b string) int {
		return cmp.Or(cmp.Compare(counts[b], counts[a]), strings.Compare(a, b))
	})
	for _, word := range keys {
		fmt.Println(counts[word], word)
	}
}
