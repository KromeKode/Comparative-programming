// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

// dedup rewrites the file without its repeated lines, and returns how many there were.
func dedup(path string) (int, error) {
	data, err := os.ReadFile(path) // bytes, whether or not they are UTF-8
	if err != nil {
		return 0, err
	}
	lines := strings.Split(string(data), "\n")
	if lines[len(lines)-1] == "" { // the file ended with a newline, or was empty
		lines = lines[:len(lines)-1]
	}
	seen := make(map[string]bool)
	var kept []string
	for _, line := range lines {
		if !seen[line] {
			seen[line] = true
			kept = append(kept, line)
		}
	}
	removed := len(lines) - len(kept)
	if removed == 0 {
		return 0, nil
	}

	info, err := os.Stat(path)
	if err != nil {
		return 0, err
	}
	// A new file in the same folder, so that the rename can't cross to another disk.
	// CreateTemp puts a random number where the * is.
	file, err := os.CreateTemp(filepath.Dir(path), ".dedup-*")
	if err != nil {
		return 0, err
	}
	// Removes the new file if anything below fails. After the rename there is no file
	// by that name, and Remove fails, which doesn't matter.
	defer os.Remove(file.Name())
	_, err = file.WriteString(strings.Join(kept, "\n") + "\n")
	if err == nil {
		err = file.Chmod(info.Mode()) // CreateTemp makes it readable by its owner only
	}
	if err == nil {
		err = file.Sync() // onto the disk before the rename
	}
	// Close can fail too, for a write the system held back, so its error counts.
	if closeErr := file.Close(); err == nil {
		err = closeErr
	}
	if err != nil {
		return 0, err
	}
	return removed, os.Rename(file.Name(), path)
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "usage: dedup FILE...")
		os.Exit(2)
	}
	status := 0
	for _, path := range os.Args[1:] {
		removed, err := dedup(path)
		if err != nil {
			// The error has what failed and the path, such as
			// "open notes.txt: no such file or directory".
			fmt.Fprintln(os.Stderr, "dedup:", err)
			status = 1
			continue
		}
		plural := "s"
		if removed == 1 {
			plural = ""
		}
		fmt.Printf("%s: removed %d line%s\n", path, removed, plural)
	}
	os.Exit(status)
}
