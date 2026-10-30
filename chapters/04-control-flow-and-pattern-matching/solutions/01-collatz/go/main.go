package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		text := strings.TrimSpace(scanner.Text())
		if text == "" {
			continue
		}
		start, err := strconv.ParseUint(text, 10, 64)
		if err != nil {
			fmt.Fprintln(os.Stderr, "not a positive whole number:", text)
			os.Exit(1)
		}
		// The values climb far past 2^32 for some starts, so use 64 bits.
		n, peak, steps := start, start, 0
		for n != 1 {
			if n%2 == 0 {
				n /= 2
			} else {
				n = 3*n + 1
			}
			peak = max(peak, n) // max is built in since Go 1.21
			steps++
		}
		fmt.Printf("%d: steps %d, peak %d\n", start, steps, peak)
	}
}
