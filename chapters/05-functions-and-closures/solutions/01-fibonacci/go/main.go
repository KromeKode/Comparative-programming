package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func main() {
	memo := map[int]uint64{}
	// Declare fib first, so the function literal can refer to itself.
	var fib func(n int) uint64
	fib = func(n int) uint64 {
		if n < 2 {
			return uint64(n)
		}
		if value, ok := memo[n]; ok {
			return value
		}
		value := fib(n-1) + fib(n-2)
		memo[n] = value
		return value
	}

	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		text := strings.TrimSpace(scanner.Text())
		if text == "" {
			continue
		}
		n, err := strconv.Atoi(text)
		if err != nil {
			fmt.Fprintln(os.Stderr, "n must be a whole number:", text)
			os.Exit(1)
		}
		fmt.Printf("fib(%d) = %d\n", n, fib(n))
	}
}
