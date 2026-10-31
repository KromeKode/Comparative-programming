package main

import (
	"fmt"
	"io"
	"os"
	"slices"
	"strings"
)

func main() {
	input, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	// A Go map has no order, so it maps each key to a group number, and the groups
	// live in a slice in the order they were created.
	groupOf := map[string]int{}
	var groups [][]string
	for _, word := range strings.Fields(string(input)) {
		word = strings.ToLower(word)
		letters := []byte(word)
		slices.Sort(letters) // anagrams have the same letters once sorted
		key := string(letters)
		index, ok := groupOf[key]
		if !ok {
			index = len(groups)
			groupOf[key] = index
			groups = append(groups, nil)
		}
		groups[index] = append(groups[index], word)
	}

	for _, group := range groups {
		fmt.Println(strings.Join(group, " "))
	}
}
