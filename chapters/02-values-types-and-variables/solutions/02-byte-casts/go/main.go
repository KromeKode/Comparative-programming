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
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		x, err := strconv.ParseInt(line, 10, 64)
		if err != nil {
			fmt.Fprintln(os.Stderr, "not a whole number:", line)
			os.Exit(1)
		}
		// Converting a variable to a smaller integer type keeps the low bits.
		fmt.Printf("%d -> u8 %d, i8 %d\n", x, uint8(x), int8(x))
	}
}
