package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// shiftLetter shifts an ASCII letter and leaves every other rune alone.
func shiftLetter(r rune, shift int) rune {
	switch {
	case r >= 'a' && r <= 'z':
		return 'a' + (r-'a'+rune(shift))%26
	case r >= 'A' && r <= 'Z':
		return 'A' + (r-'A'+rune(shift))%26
	}
	return r
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		number, text, _ := strings.Cut(line, " ") // split at the first space only
		shift, err := strconv.Atoi(number)
		if err != nil {
			fmt.Fprintln(os.Stderr, "not a number:", number)
			os.Exit(1)
		}
		shift = (shift%26 + 26) % 26 // Go's % keeps the sign of the left side
		// strings.Map applies the function to each rune and builds a new string.
		fmt.Println(strings.Map(func(r rune) rune { return shiftLetter(r, shift) }, text))
	}
}
