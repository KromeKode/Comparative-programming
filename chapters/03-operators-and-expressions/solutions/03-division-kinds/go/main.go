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
		a, errA := strconv.Atoi(fields[0])
		b, errB := strconv.Atoi(fields[1])
		if errA != nil || errB != nil {
			fmt.Fprintln(os.Stderr, "not a pair of whole numbers:", scanner.Text())
			os.Exit(1)
		}
		// Go's / and % truncate toward zero.
		q, r := a/b, a%b
		// Floored division rounds down instead. The answers differ by one step when
		// the remainder is not zero and its sign differs from the divisor's.
		fq, fm := q, r
		if r != 0 && (r < 0) != (b < 0) {
			fq, fm = q-1, r+b
		}
		fmt.Printf("%d %d -> truncated %d %d, floored %d %d\n", a, b, q, r, fq, fm)
	}
}
