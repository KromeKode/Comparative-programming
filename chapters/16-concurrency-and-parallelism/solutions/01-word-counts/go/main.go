package main

import (
	"bufio"
	"fmt"
	"os"
	"slices"
	"strconv"
	"strings"
	"sync"
)

func plural(count int, noun string) string {
	if count == 1 {
		return fmt.Sprintf("%d %s", count, noun)
	}
	return fmt.Sprintf("%d %ss", count, noun)
}

func isLetter(r rune) bool {
	return (r >= 'a' && r <= 'z') || (r >= 'A' && r <= 'Z')
}

// Runs on a goroutine of its own. The lines are a slice of main's, which
// nothing changes while it runs, and the map is its own.
func countWords(lines []string) map[string]int {
	counts := map[string]int{}
	for _, line := range lines {
		for _, word := range strings.FieldsFunc(line, func(r rune) bool { return !isLetter(r) }) {
			counts[strings.ToLower(word)]++
		}
	}
	return counts
}

func totalOf(counts map[string]int) int {
	total := 0
	for _, count := range counts {
		total += count
	}
	return total
}

func isNumber(word string) bool {
	return word != "" && strings.Trim(word, "0123456789") == ""
}

func main() {
	var lines []string
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	var first []string
	if len(lines) > 0 {
		first = strings.Fields(lines[0])
	}
	if len(first) != 2 || !isNumber(first[0]) || !isNumber(first[1]) {
		fmt.Println("error: the first line must be two numbers, workers and top")
		return
	}
	workers, _ := strconv.Atoi(first[0])
	top, _ := strconv.Atoi(first[1])
	if workers < 1 || workers > 16 {
		fmt.Println("error: workers must be from 1 to 16")
		return
	}

	// Each worker gets len(text) / workers lines, and the first few get one
	// more, so that every line is counted once.
	text := lines[1:]
	chunks := make([][]string, workers)
	start := 0
	for i := range workers {
		size := len(text) / workers
		if i < len(text)%workers {
			size++
		}
		chunks[i] = text[start : start+size]
		start += size
	}

	// Each goroutine writes only its own element of counts, so they need no
	// lock, and after Wait, main can read them all.
	counts := make([]map[string]int, workers)
	var wg sync.WaitGroup
	for i, chunk := range chunks {
		wg.Add(1)
		go func() {
			defer wg.Done()
			counts[i] = countWords(chunk)
		}()
	}
	wg.Wait()

	fmt.Printf("%s, %s\n", plural(workers, "worker"), plural(len(text), "line"))
	total := map[string]int{}
	for i, chunk := range chunks {
		fmt.Printf("worker %d: %s, %s\n", i, plural(len(chunk), "line"),
			plural(totalOf(counts[i]), "word"))
		for word, count := range counts[i] {
			total[word] += count
		}
	}
	fmt.Printf("%s, %d different\n", plural(totalOf(total), "word"), len(total))

	words := make([]string, 0, len(total))
	for word := range total {
		words = append(words, word)
	}
	slices.SortFunc(words, func(a, b string) int {
		if total[a] != total[b] {
			return total[b] - total[a]
		}
		return strings.Compare(a, b)
	})
	for _, word := range words[:min(top, len(words))] {
		fmt.Printf("%s: %d\n", word, total[word])
	}
}
