package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// Pointers give swap two slots of the caller's slice. A string is a small
// header (pointer and length), so swapping two moves no characters.
func swap(a, b *string) {
	*a, *b = *b, *a
}

// reverse reverses words[from..to] and returns how many swaps that took.
func reverse(words []string, from, to int) int {
	swaps := 0
	for ; from < to; from, to = from+1, to-1 {
		swap(&words[from], &words[to])
		swaps++
	}
	return swaps
}

func parsePosition(text string, count int) (int, bool) {
	for _, c := range text {
		if c < '0' || c > '9' {
			fmt.Printf("error: %q is not a position\n", text)
			return 0, false
		}
	}
	value, err := strconv.Atoi(text)
	if err != nil || value < 1 || value > count {
		fmt.Printf("error: position %s is out of range (1 to %d)\n", text, count)
		return 0, false
	}
	return value, true
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	var words []string
	if scanner.Scan() {
		words = strings.Fields(scanner.Text())
	}

	swaps := 0
	for scanner.Scan() {
		parts := strings.Fields(scanner.Text())
		if len(parts) == 0 {
			continue
		}
		command := parts[0]
		if command != "swap" && command != "reverse" {
			fmt.Printf("error: unknown command %q\n", command)
			continue
		}
		if len(parts) != 3 {
			fmt.Printf("error: %s needs 2 positions, got %d\n", command, len(parts)-1)
			continue
		}
		i, ok := parsePosition(parts[1], len(words))
		if !ok {
			continue
		}
		j, ok := parsePosition(parts[2], len(words))
		if !ok {
			continue
		}
		if command == "swap" {
			swap(&words[i-1], &words[j-1])
			swaps++
		} else {
			swaps += reverse(words, min(i, j)-1, max(i, j)-1)
		}
		fmt.Println(strings.Join(words, " "))
	}
	fmt.Printf("swaps: %d\n", swaps)
}
