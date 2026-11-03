package main

import (
	"bufio"
	"fmt"
	"iter"
	"os"
	"strconv"
	"strings"
)

// Each adapter takes any push iterator of numbers and returns another one.
// Ranging over the result pulls the numbers through one at a time.
func Windows(items iter.Seq[int64], k int) iter.Seq[[]int64] {
	return func(yield func([]int64) bool) {
		var window []int64
		for item := range items {
			window = append(window, item)
			if len(window) > k {
				window = window[1:] // the oldest item leaves
			}
			// The caller gets its own copy, since window keeps changing.
			if len(window) == k && !yield(append([]int64(nil), window...)) {
				return
			}
		}
	}
}

func Chunks(items iter.Seq[int64], k int) iter.Seq[[]int64] {
	return func(yield func([]int64) bool) {
		var chunk []int64
		for item := range items {
			chunk = append(chunk, item)
			if len(chunk) == k {
				if !yield(chunk) {
					return
				}
				chunk = nil
			}
		}
		if len(chunk) > 0 {
			yield(chunk)
		}
	}
}

func Runs(items iter.Seq[int64]) iter.Seq[[]int64] {
	return func(yield func([]int64) bool) {
		var run []int64
		for item := range items {
			if len(run) > 0 && item != run[0] {
				if !yield(run) {
					return
				}
				run = nil
			}
			run = append(run, item)
		}
		if len(run) > 0 {
			yield(run)
		}
	}
}

// Numbers reads the words as numbers, one at a time, as the caller asks.
func Numbers(words []string) iter.Seq[int64] {
	return func(yield func(int64) bool) {
		for _, word := range words {
			n, err := strconv.ParseInt(word, 10, 64)
			if err != nil {
				panic(err)
			}
			if !yield(n) {
				return
			}
		}
	}
}

func show(groups iter.Seq[[]int64]) string {
	var texts []string
	for group := range groups {
		items := make([]string, len(group))
		for i, item := range group {
			items[i] = strconv.FormatInt(item, 10)
		}
		texts = append(texts, "["+strings.Join(items, " ")+"]")
	}
	return strings.Join(texts, " ")
}

func answer(words []string) string {
	var text string
	if words[0] == "runs" {
		text = show(Runs(Numbers(words[1:])))
	} else {
		k, _ := strconv.Atoi(words[1])
		numbers := Numbers(words[2:])
		switch words[0] {
		case "windows":
			text = show(Windows(numbers, k))
		case "chunks":
			text = show(Chunks(numbers, k))
		default:
			var sums []string
			for window := range Windows(numbers, k) {
				var sum int64
				for _, item := range window {
					sum += item
				}
				sums = append(sums, strconv.FormatInt(sum, 10))
			}
			text = strings.Join(sums, " ")
		}
	}
	if text == "" {
		return "(none)"
	}
	return text
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		if words := strings.Fields(scanner.Text()); len(words) > 0 {
			fmt.Println(answer(words))
		}
	}
}
