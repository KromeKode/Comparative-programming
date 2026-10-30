package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func makeCounter(start, step int) func() int {
	next := start // captured by the closure, and kept alive by it
	return func() int {
		value := next
		next += step
		return value
	}
}

func main() {
	counters := map[string]func() int{}
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		fields := strings.Fields(scanner.Text())
		switch {
		case len(fields) == 4 && fields[0] == "new":
			start, errStart := strconv.Atoi(fields[2])
			step, errStep := strconv.Atoi(fields[3])
			if errStart != nil || errStep != nil {
				fmt.Fprintln(os.Stderr, "start and step must be whole numbers")
				os.Exit(1)
			}
			counters[fields[1]] = makeCounter(start, step)
		case len(fields) == 2 && fields[0] == "next":
			name := fields[1]
			if counter, ok := counters[name]; ok {
				fmt.Printf("%s: %d\n", name, counter())
			} else {
				fmt.Printf("%s: no such counter\n", name)
			}
		}
	}
}
