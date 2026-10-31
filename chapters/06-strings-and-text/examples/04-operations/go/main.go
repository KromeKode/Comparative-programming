package main

import (
	"fmt"
	"slices"
	"strings"
)

func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func main() {
	text := "  The quick  brown fox  "

	trimmed := strings.TrimSpace(text)
	fmt.Printf("trimmed: [%s]\n", trimmed)

	// Fields splits on runs of whitespace; Split(s, " ") would give empty strings.
	fmt.Println("words:", strings.Join(strings.Fields(trimmed), "|"))

	fmt.Println("upper:", strings.ToUpper(trimmed))
	fmt.Println(`contains "quick":`, yesNo(strings.Contains(trimmed, "quick")))
	fmt.Println(`index of "brown":`, strings.Index(trimmed, "brown")) // -1 if missing
	fmt.Println("replaced:", strings.ReplaceAll(trimmed, "quick", "slow"))
	fmt.Println(`starts with "The":`, yesNo(strings.HasPrefix(trimmed, "The")))

	fruits := []string{"banana", "apple", "Cherry"}
	slices.Sort(fruits) // compares bytes
	fmt.Println("sorted:", strings.Join(fruits, " "))
	slices.SortFunc(fruits, func(a, b string) int {
		return strings.Compare(strings.ToLower(a), strings.ToLower(b))
	})
	fmt.Println("sorted ignoring case:", strings.Join(fruits, " "))
}
