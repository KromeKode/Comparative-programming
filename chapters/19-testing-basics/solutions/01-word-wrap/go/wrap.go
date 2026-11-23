// Package wrap breaks text into lines.
package wrap

import "strings"

// Wrap wraps text into lines of at most width characters, breaking only at
// spaces.
func Wrap(text string, width int) []string {
	var lines []string
	line := ""
	for _, word := range strings.Split(text, " ") {
		if word == "" {
			continue
		}
		if line == "" {
			line = word
		} else if len(line)+1+len(word) <= width {
			line += " " + word
		} else {
			lines = append(lines, line)
			line = word
		}
	}
	if line != "" {
		lines = append(lines, line)
	}
	return lines
}
