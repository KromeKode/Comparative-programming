package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

func guessesText(count int) string {
	if count == 1 {
		return "1 guess"
	}
	return fmt.Sprintf("%d guesses", count)
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	if !scanner.Scan() {
		os.Exit(1)
	}
	secret, err := strconv.Atoi(strings.TrimSpace(scanner.Text()))
	if err != nil {
		fmt.Fprintln(os.Stderr, "the secret must be a whole number")
		os.Exit(1)
	}

	guesses := 0
	for scanner.Scan() {
		text := strings.TrimSpace(scanner.Text())
		if text == "" {
			continue // a blank line
		}
		guess, err := strconv.Atoi(text)
		if err != nil {
			fmt.Printf("%s: not a number\n", text)
			continue
		}
		guesses++
		switch {
		case guess < secret:
			fmt.Printf("%d: too low\n", guess)
		case guess > secret:
			fmt.Printf("%d: too high\n", guess)
		default:
			fmt.Printf("%d: correct in %s\n", guess, guessesText(guesses))
			return // a break here would only leave the switch, not the loop
		}
	}
	fmt.Printf("gave up after %s; the number was %d\n", guessesText(guesses), secret)
}
