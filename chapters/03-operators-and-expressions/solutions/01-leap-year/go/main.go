package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func isLeap(year int) bool {
	return year%4 == 0 && (year%100 != 0 || year%400 == 0)
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		text := strings.TrimSpace(scanner.Text())
		if text == "" {
			continue
		}
		year, err := strconv.Atoi(text)
		if err != nil {
			fmt.Fprintln(os.Stderr, "not a year:", text)
			os.Exit(1)
		}
		kind := "common year"
		if isLeap(year) {
			kind = "leap year"
		}
		fmt.Printf("%d: %s\n", year, kind)
	}
}
