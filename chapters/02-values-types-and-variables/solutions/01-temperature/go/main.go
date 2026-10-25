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
		value, err := strconv.ParseFloat(fields[0], 64)
		if err != nil {
			fmt.Fprintln(os.Stderr, "not a number:", fields[0])
			os.Exit(1)
		}
		if fields[1] == "C" {
			fmt.Printf("%.1f C = %.1f F\n", value, value*9/5+32)
		} else {
			fmt.Printf("%.1f F = %.1f C\n", value, (value-32)*5/9)
		}
	}
}
