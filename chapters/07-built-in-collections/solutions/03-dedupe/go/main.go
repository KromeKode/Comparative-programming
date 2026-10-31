package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	scanner.Buffer(make([]byte, 1<<20), 1<<26) // allow lines longer than 64 KB
	for scanner.Scan() {
		seen := map[string]bool{}
		var kept []string
		for _, word := range strings.Fields(scanner.Text()) {
			if !seen[word] {
				seen[word] = true
				kept = append(kept, word)
			}
		}
		if len(kept) > 0 {
			fmt.Println(strings.Join(kept, " "))
		}
	}
}
