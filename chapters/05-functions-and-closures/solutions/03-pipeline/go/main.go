package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// Each returned function literal captures its own n.
func makeStep(op string, n int) func(int) int {
	switch op {
	case "add":
		return func(x int) int { return x + n }
	case "sub":
		return func(x int) int { return x - n }
	case "mul":
		return func(x int) int { return x * n }
	case "min":
		return func(x int) int { return min(x, n) }
	case "max":
		return func(x int) int { return max(x, n) }
	}
	panic("unknown step: " + op)
}

func mustAtoi(text string) int {
	n, err := strconv.Atoi(text)
	if err != nil {
		panic("not a whole number: " + text)
	}
	return n
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		words := strings.Fields(scanner.Text())
		if len(words) == 0 {
			continue // a blank line
		}
		start := mustAtoi(words[0])
		var steps []func(int) int
		for i := 1; i+1 < len(words); i += 2 {
			steps = append(steps, makeStep(words[i], mustAtoi(words[i+1])))
		}
		value := start
		for _, step := range steps {
			value = step(value)
		}
		fmt.Printf("%d -> %d\n", start, value)
	}
}
