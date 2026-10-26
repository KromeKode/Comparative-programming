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
		fields := strings.Fields(scanner.Text())
		if len(fields) != 2 {
			continue // a blank line
		}
		a, errA := strconv.ParseInt(fields[0], 10, 64)
		b, errB := strconv.ParseInt(fields[1], 10, 64)
		if errA != nil || errB != nil {
			fmt.Fprintln(os.Stderr, "not a 64-bit whole number:", scanner.Text())
			os.Exit(1)
		}
		sum := a + b // Go defines overflow as wrapping, so we can add first and check after
		if (a > 0 && b > 0 && sum < 0) || (a < 0 && b < 0 && sum >= 0) {
			fmt.Println("overflow")
		} else {
			fmt.Println(sum)
		}
	}
}
