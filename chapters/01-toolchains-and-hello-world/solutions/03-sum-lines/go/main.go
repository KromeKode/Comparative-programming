package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func main() {
	var total int64
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		value, err := strconv.ParseInt(line, 10, 64)
		if err != nil {
			fmt.Fprintln(os.Stderr, "not a whole number:", line)
			os.Exit(1)
		}
		total += value
	}
	fmt.Println(total)
}
