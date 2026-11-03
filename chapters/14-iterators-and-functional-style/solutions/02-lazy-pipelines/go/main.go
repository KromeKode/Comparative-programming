package main

import (
	"bufio"
	"fmt"
	"iter"
	"os"
	"strconv"
	"strings"
)

func isPrime(n int64) bool {
	if n < 2 {
		return false
	}
	for d := int64(2); d*d <= n; d++ {
		if n%d == 0 {
			return false
		}
	}
	return true
}

func number(word string) int64 {
	n, err := strconv.ParseInt(word, 10, 64)
	if err != nil {
		panic(err)
	}
	return n
}

// The sources are push iterators. The endless ones loop until yield returns
// false, which is how a later stage says it has seen enough.
func Naturals() iter.Seq[int64] {
	return func(yield func(int64) bool) {
		n := int64(1)
		for yield(n) {
			n++
		}
	}
}

func Fibonacci() iter.Seq[int64] {
	return func(yield func(int64) bool) {
		a, b := int64(0), int64(1)
		for yield(a) {
			a, b = b, a+b
		}
	}
}

func Range(low, high int64) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		for n := low; n <= high; n++ {
			if !yield(n) {
				return
			}
		}
	}
}

func Repeat(n int64) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		for yield(n) {
		}
	}
}

// The standard library has no adapters for iter.Seq yet, so these are the
// usual hand-written ones. Each returns a new lazy iterator.
func Map(items iter.Seq[int64], f func(int64) int64) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		for item := range items {
			if !yield(f(item)) {
				return
			}
		}
	}
}

func Filter(items iter.Seq[int64], keep func(int64) bool) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		for item := range items {
			if keep(item) && !yield(item) {
				return
			}
		}
	}
}

func TakeWhile(items iter.Seq[int64], keep func(int64) bool) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		for item := range items {
			if !keep(item) || !yield(item) {
				return
			}
		}
	}
}

func DropWhile(items iter.Seq[int64], drop func(int64) bool) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		dropping := true
		for item := range items {
			if dropping && drop(item) {
				continue
			}
			dropping = false
			if !yield(item) {
				return
			}
		}
	}
}

// Take stops right after the nth item. Waiting for one more would hang on
// a pipeline such as primes | filter below 10 | take 4.
func Take(items iter.Seq[int64], n int64) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		if n <= 0 {
			return
		}
		taken := int64(0)
		for item := range items {
			taken++
			if !yield(item) || taken == n {
				return
			}
		}
	}
}

func Skip(items iter.Seq[int64], n int64) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		i := int64(0)
		for item := range items {
			if i >= n && !yield(item) {
				return
			}
			i++
		}
	}
}

// Step keeps items 0, n, 2n, and so on.
func Step(items iter.Seq[int64], n int64) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		i := int64(0)
		for item := range items {
			if i%n == 0 && !yield(item) {
				return
			}
			i++
		}
	}
}

func source(words []string) iter.Seq[int64] {
	switch words[0] {
	case "naturals":
		return Naturals()
	case "primes":
		return Filter(Naturals(), isPrime)
	case "fibonacci":
		return Fibonacci()
	case "range":
		return Range(number(words[1]), number(words[2]))
	case "repeat":
		return Repeat(number(words[1]))
	}
	panic("unknown source " + words[0])
}

func predicate(words []string) func(int64) bool {
	switch words[0] {
	case "even":
		return func(x int64) bool { return x%2 == 0 }
	case "odd":
		return func(x int64) bool { return x%2 != 0 }
	case "prime":
		return isPrime
	case "above":
		n := number(words[1])
		return func(x int64) bool { return x > n }
	case "below":
		n := number(words[1])
		return func(x int64) bool { return x < n }
	}
	panic("unknown predicate " + words[0])
}

func function(words []string) func(int64) int64 {
	switch words[0] {
	case "square":
		return func(x int64) int64 { return x * x }
	case "negate":
		return func(x int64) int64 { return -x }
	case "add":
		n := number(words[1])
		return func(x int64) int64 { return x + n }
	case "mul":
		n := number(words[1])
		return func(x int64) int64 { return x * n }
	}
	panic("unknown function " + words[0])
}

func stage(items iter.Seq[int64], words []string) iter.Seq[int64] {
	switch words[0] {
	case "map":
		return Map(items, function(words[1:]))
	case "filter":
		return Filter(items, predicate(words[1:]))
	case "takewhile":
		return TakeWhile(items, predicate(words[1:]))
	case "dropwhile":
		return DropWhile(items, predicate(words[1:]))
	case "take":
		return Take(items, number(words[1]))
	case "skip":
		return Skip(items, number(words[1]))
	case "step":
		return Step(items, number(words[1]))
	}
	panic("unknown stage " + words[0])
}

func run(line string) string {
	var parts [][]string
	for _, part := range strings.Split(line, "|") {
		parts = append(parts, strings.Fields(part))
	}
	// A sum or count at the end is not a stage: it says what to do with the items.
	finish := "list"
	if last := parts[len(parts)-1]; len(last) == 1 && (last[0] == "sum" || last[0] == "count") {
		finish = last[0]
		parts = parts[:len(parts)-1]
	}
	items := source(parts[0])
	for _, words := range parts[1:] {
		items = stage(items, words)
	}
	// Nothing has run yet. Only the loop below pulls items through.
	var total, count int64
	var texts []string
	for item := range items {
		total += item
		count++
		texts = append(texts, strconv.FormatInt(item, 10))
	}
	switch {
	case finish == "sum":
		return strconv.FormatInt(total, 10)
	case finish == "count":
		return strconv.FormatInt(count, 10)
	case len(texts) == 0:
		return "(empty)"
	}
	return strings.Join(texts, " ")
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		if line := scanner.Text(); strings.TrimSpace(line) != "" {
			fmt.Println(run(line))
		}
	}
}
