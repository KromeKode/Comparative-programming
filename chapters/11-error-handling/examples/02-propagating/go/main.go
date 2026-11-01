package main

import (
	"errors"
	"fmt"
	"strconv"
	"strings"
)

// Pretend files, so the example does not touch the disk.
var files = map[string]string{"good.txt": "1\n2\n3", "bad.txt": "4\nfive\n6"}

// A sentinel error: callers compare against it with errors.Is.
var ErrNotFound = errors.New("no such file")

func readFile(name string) (string, error) {
	text, ok := files[name]
	if !ok {
		// %w wraps ErrNotFound, so errors.Is can still find it.
		return "", fmt.Errorf("%s: %w", name, ErrNotFound)
	}
	return text, nil
}

func parseNumber(text string) (int, error) {
	value, err := strconv.Atoi(text)
	if err != nil {
		return 0, fmt.Errorf("%q is not a number", text)
	}
	return value, nil
}

func sumFile(name string) (int, error) {
	text, err := readFile(name)
	if err != nil {
		return 0, err // pass the error up unchanged
	}
	total := 0
	for index, line := range strings.Split(text, "\n") {
		value, err := parseNumber(line)
		if err != nil {
			// Add context: which file and which line.
			return 0, fmt.Errorf("%s: line %d: %w", name, index+1, err)
		}
		total += value
	}
	return total, nil
}

// A missing file counts as 0, but any other error stops the sum.
func sumAll(names ...string) (int, error) {
	total := 0
	for _, name := range names {
		value, err := sumFile(name)
		if errors.Is(err, ErrNotFound) {
			continue
		}
		if err != nil {
			return 0, err
		}
		total += value
	}
	return total, nil
}

func show(label string, total int, err error) {
	if err != nil {
		fmt.Printf("  %s -> error: %v\n", label, err)
	} else {
		fmt.Printf("  %s -> %d\n", label, total)
	}
}

func main() {
	fmt.Println("one file at a time:")
	for _, name := range []string{"good.txt", "bad.txt", "missing.txt"} {
		total, err := sumFile(name)
		show(name, total, err)
	}
	fmt.Println("several files, where a missing file counts as 0:")
	total, err := sumAll("good.txt", "missing.txt")
	show("good.txt + missing.txt", total, err)
	total, err = sumAll("good.txt", "bad.txt")
	show("good.txt + bad.txt", total, err)
}
