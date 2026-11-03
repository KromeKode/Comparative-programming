package main

import (
	"bufio"
	"cmp"
	"fmt"
	"os"
	"slices"
	"strconv"
	"strings"
)

type Version struct{ Major, Minor, Patch int }

func (v Version) String() string {
	return fmt.Sprintf("%d.%d.%d", v.Major, v.Minor, v.Patch)
}

// Compare orders versions part by part, as numbers. cmp.Or returns the first
// result that is not 0.
func (v Version) Compare(other Version) int {
	return cmp.Or(
		cmp.Compare(v.Major, other.Major),
		cmp.Compare(v.Minor, other.Minor),
		cmp.Compare(v.Patch, other.Patch),
	)
}

type Interval[T any] struct{ Low, High T }

func (i Interval[T]) String() string {
	return fmt.Sprintf("%v..%v", i.Low, i.High)
}

// merge takes a compare function, as slices.SortFunc does, because
// cmp.Ordered covers only built-in types like int and string, not Version.
func merge[T any](intervals []Interval[T], compare func(a, b T) int) []Interval[T] {
	slices.SortFunc(intervals, func(a, b Interval[T]) int { return compare(a.Low, b.Low) })
	var merged []Interval[T]
	for _, interval := range intervals {
		last := len(merged) - 1
		if last >= 0 && compare(interval.Low, merged[last].High) <= 0 {
			if compare(interval.High, merged[last].High) > 0 {
				merged[last].High = interval.High
			}
		} else {
			merged = append(merged, interval)
		}
	}
	return merged
}

func isDigits(text string) bool {
	return text != "" && strings.Trim(text, "0123456789") == ""
}

func parseInt(text string) (int, bool) {
	// strconv.Atoi would also take "+5", so check the form first.
	if !isDigits(strings.TrimPrefix(text, "-")) {
		return 0, false
	}
	n, err := strconv.Atoi(text)
	return n, err == nil
}

func parseWord(text string) (string, bool) {
	return text, text != "" && strings.Trim(text, "abcdefghijklmnopqrstuvwxyz") == ""
}

func parseVersion(text string) (Version, bool) {
	parts := strings.Split(text, ".")
	if len(parts) != 3 {
		return Version{}, false
	}
	var numbers [3]int
	for i, part := range parts {
		n, err := strconv.Atoi(part)
		if !isDigits(part) || err != nil {
			return Version{}, false
		}
		numbers[i] = n
	}
	return Version{numbers[0], numbers[1], numbers[2]}, true
}

// run parses every interval on a line, merges them, and formats the result.
func run[T any](kind string, parse func(string) (T, bool), compare func(a, b T) int,
	words []string) (string, error) {
	var intervals []Interval[T]
	for _, word := range words {
		ends := strings.Split(word, "..")
		if len(ends) != 2 {
			return "", fmt.Errorf("bad interval \"%s\"", word)
		}
		low, ok := parse(ends[0])
		if !ok {
			return "", fmt.Errorf("bad %s \"%s\"", kind, ends[0])
		}
		high, ok := parse(ends[1])
		if !ok {
			return "", fmt.Errorf("bad %s \"%s\"", kind, ends[1])
		}
		if compare(high, low) < 0 {
			return "", fmt.Errorf("%s is backwards", word)
		}
		intervals = append(intervals, Interval[T]{low, high})
	}
	parts := []string{kind}
	for _, interval := range merge(intervals, compare) {
		parts = append(parts, interval.String())
	}
	return strings.Join(parts, " "), nil
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		words := strings.Fields(scanner.Text())
		if len(words) == 0 {
			continue
		}
		kind, rest := words[0], words[1:]
		var text string
		var err error
		switch kind {
		case "int":
			text, err = run(kind, parseInt, cmp.Compare[int], rest)
		case "word":
			text, err = run(kind, parseWord, strings.Compare, rest)
		case "version":
			// Version.Compare is a method expression: a func(Version, Version) int.
			text, err = run(kind, parseVersion, Version.Compare, rest)
		default:
			err = fmt.Errorf("unknown kind \"%s\"", kind)
		}
		if err != nil {
			fmt.Println("error:", err)
		} else {
			fmt.Println(text)
		}
	}
}
