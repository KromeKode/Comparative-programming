// Exercise 18.3: version ranges. The highest version that each range allows.
//
// The versions and ranges are package semver's business; this file reads the input
// and prints the answers.
package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"

	"example.com/ranges/semver"
)

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	if !scanner.Scan() {
		return
	}
	var versions []semver.Version
	for _, word := range strings.Fields(scanner.Text()) {
		version, ok := semver.Parse(word)
		if !ok {
			fmt.Printf("error: line 1: %s isn't a version\n", word)
			return
		}
		versions = append(versions, version)
	}
	if len(versions) == 0 {
		fmt.Println("error: line 1 must list the versions")
		return
	}
	for number := 2; scanner.Scan(); number++ {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		r, err := semver.ParseRange(line)
		if err != nil {
			fmt.Printf("error: line %d: %v\n", number, err)
			continue
		}
		if best, ok := r.Highest(versions); ok {
			fmt.Printf("%s -> %s\n", line, best)
		} else {
			fmt.Printf("%s -> none\n", line)
		}
	}
}
