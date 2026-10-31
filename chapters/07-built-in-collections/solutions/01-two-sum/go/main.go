package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// twoSum returns the pair with the smallest j, and for that j the smallest i.
func twoSum(target int64, numbers []int64) (int, int, bool) {
	// Each number seen so far, mapped to the first index where it appeared.
	firstIndex := map[int64]int{}
	for j, n := range numbers {
		if i, ok := firstIndex[target-n]; ok {
			return i, j, true
		}
		if _, ok := firstIndex[n]; !ok {
			firstIndex[n] = j
		}
	}
	return 0, 0, false
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	scanner.Buffer(make([]byte, 1<<20), 1<<26) // allow lines longer than 64 KB
	for scanner.Scan() {
		head, rest, ok := strings.Cut(scanner.Text(), ":")
		if !ok {
			continue // a blank line
		}
		target, err := strconv.ParseInt(strings.TrimSpace(head), 10, 64)
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		var numbers []int64
		for _, word := range strings.Fields(rest) {
			n, err := strconv.ParseInt(word, 10, 64)
			if err != nil {
				fmt.Fprintln(os.Stderr, err)
				os.Exit(1)
			}
			numbers = append(numbers, n)
		}
		if i, j, found := twoSum(target, numbers); found {
			fmt.Println(i, j)
		} else {
			fmt.Println("none")
		}
	}
}
