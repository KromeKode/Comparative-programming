// Counts the words in its input, and prints the ten most common. It is slow on a long
// text, and section 20.7 runs it under a profiler to find out why.
//
// With -cpuprofile=FILE, it writes a CPU profile to FILE, for go tool pprof.
package main

import (
	"cmp"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"runtime/pprof"
	"slices"
	"strings"
)

type count struct {
	word string
	n    int
}

// splitWords gives the words of the text: runs of the letters a to z, in lowercase.
func splitWords(text string) []string {
	return strings.FieldsFunc(strings.ToLower(text), func(c rune) bool {
		return c < 'a' || c > 'z'
	})
}

// find gives the index of the word's count, or -1 if it has none.
func find(counts []count, word string) int {
	for i, c := range counts {
		if c.word == word {
			return i
		}
	}
	return -1
}

func countWords(words []string) []count {
	var counts []count
	for _, word := range words {
		if i := find(counts, word); i >= 0 {
			counts[i].n++
		} else {
			counts = append(counts, count{word, 1})
		}
	}
	return counts
}

func main() {
	cpuprofile := flag.String("cpuprofile", "", "write a CPU profile to this file")
	flag.Parse()
	if *cpuprofile != "" {
		f, err := os.Create(*cpuprofile)
		if err != nil {
			log.Fatal(err)
		}
		defer f.Close()
		if err := pprof.StartCPUProfile(f); err != nil {
			log.Fatal(err)
		}
		defer pprof.StopCPUProfile()
	}

	text, err := io.ReadAll(os.Stdin)
	if err != nil {
		log.Fatal(err)
	}
	counts := countWords(splitWords(string(text)))
	slices.SortFunc(counts, func(a, b count) int {
		return cmp.Or(cmp.Compare(b.n, a.n), cmp.Compare(a.word, b.word))
	})
	for _, c := range counts[:min(10, len(counts))] {
		fmt.Println(c.n, c.word)
	}
}
