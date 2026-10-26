package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

var values = []int{1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1}
var symbols = []string{"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"}

func toRoman(n int) string {
	var out strings.Builder
	for i, value := range values {
		for n >= value { // take the biggest value that still fits
			out.WriteString(symbols[i])
			n -= value
		}
	}
	return out.String()
}

func letterValue(letter byte) int {
	switch letter {
	case 'I':
		return 1
	case 'V':
		return 5
	case 'X':
		return 10
	case 'L':
		return 50
	case 'C':
		return 100
	case 'D':
		return 500
	case 'M':
		return 1000
	}
	return 0
}

func fromRoman(text string) int {
	total := 0
	for i := range len(text) {
		value := letterValue(text[i])
		next := 0
		if i+1 < len(text) {
			next = letterValue(text[i+1])
		}
		// A smaller letter before a bigger one is subtracted: IV is 5 - 1.
		if value < next {
			total -= value
		} else {
			total += value
		}
	}
	return total
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		text := strings.TrimSpace(scanner.Text())
		if text == "" {
			continue
		}
		if n, err := strconv.Atoi(text); err == nil {
			fmt.Printf("%d -> %s\n", n, toRoman(n))
		} else {
			fmt.Printf("%s -> %d\n", text, fromRoman(text))
		}
	}
}
