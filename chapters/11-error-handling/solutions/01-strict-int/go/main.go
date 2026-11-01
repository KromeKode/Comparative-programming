package main

import (
	"bufio"
	"errors"
	"fmt"
	"math"
	"os"
)

// Sentinel errors for the failures that carry no data.
var (
	ErrEmpty    = errors.New("empty input")
	ErrNoDigits = errors.New("no digits after the sign")
	ErrTooLarge = fmt.Errorf("too large, the maximum is %d", math.MaxInt32)
	ErrTooSmall = fmt.Errorf("too small, the minimum is %d", math.MinInt32)
)

// An error type for the failure that does carry data.
type InvalidCharacterError struct {
	Character byte
	Position  int
}

func (e *InvalidCharacterError) Error() string {
	return fmt.Sprintf("invalid character '%c' at position %d", e.Character, e.Position)
}

func parseInt(text string) (int32, error) {
	if text == "" {
		return 0, ErrEmpty
	}
	negative := text[0] == '-'
	start := 0
	if text[0] == '+' || text[0] == '-' {
		start = 1
	}
	if start == len(text) {
		return 0, ErrNoDigits
	}
	// Check every character first, so a bad one is reported even when the
	// digits before it are already too large.
	for i := start; i < len(text); i++ {
		if text[i] < '0' || text[i] > '9' {
			return 0, &InvalidCharacterError{Character: text[i], Position: i + 1}
		}
	}
	// Stop growing once the number is past every limit, so it cannot
	// overflow however many digits there are.
	const limit = math.MaxInt32 + 2
	var magnitude int64
	for i := start; i < len(text); i++ {
		magnitude = min(magnitude*10+int64(text[i]-'0'), limit)
	}
	if negative {
		if -magnitude < math.MinInt32 {
			return 0, ErrTooSmall
		}
		return int32(-magnitude), nil
	}
	if magnitude > math.MaxInt32 {
		return 0, ErrTooLarge
	}
	return int32(magnitude), nil
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := scanner.Text()
		value, err := parseInt(line)
		if err != nil {
			fmt.Printf("%q -> error: %v\n", line, err)
		} else {
			fmt.Printf("%q -> %d\n", line, value)
		}
	}
}
