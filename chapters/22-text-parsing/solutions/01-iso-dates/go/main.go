// Copies stdin to stdout, with each date in it written as YYYY-MM-DD.
package main

import (
	"fmt"
	"io"
	"os"
	"regexp"
	"strconv"
	"strings"
)

var months = []string{
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December",
}

// In Go, \b looks at ASCII letters, digits, and _ alone, and \d is only [0-9].
var date = func() *regexp.Regexp {
	var names []string
	for _, name := range months {
		// Each month's name, or its first three letters: "Jan(?:uary)?", and so on.
		if len(name) > 3 {
			name = name[:3] + "(?:" + name[3:] + ")?"
		}
		names = append(names, name)
	}
	month := strings.Join(names, "|")
	d, m, y := "([0-9]{1,2})", "([0-9]{1,2})", "([0-9]{4})"
	return regexp.MustCompile(`\b(?:` + d + "/" + m + "/" + y + "|(" + month + ") " + d + ", " + y +
		"|" + d + " (" + month + ") " + y + `)\b`)
}()

func daysIn(month, year int) int {
	switch {
	case month == 2 && year%4 == 0 && (year%100 != 0 || year%400 == 0):
		return 29
	case month == 2:
		return 28
	case month == 4 || month == 6 || month == 9 || month == 11:
		return 30
	}
	return 31
}

// rewrite returns the date as YYYY-MM-DD, or text as it was if it isn't a real date.
// ReplaceAllStringFunc passes only the text that matched, so this matches it again for
// the groups.
func rewrite(text string) string {
	g := date.FindStringSubmatch(text)
	// Only one of the three forms matched, so the groups of the other two are "".
	var day, month, year int
	switch {
	case g[1] != "":
		day, month, year = atoi(g[1]), atoi(g[2]), atoi(g[3])
	case g[4] != "":
		day, month, year = atoi(g[5]), monthOf(g[4]), atoi(g[6])
	default:
		day, month, year = atoi(g[7]), monthOf(g[8]), atoi(g[9])
	}
	if month < 1 || month > 12 || day < 1 || day > daysIn(month, year) {
		return text // not a real date, so left as it was
	}
	return fmt.Sprintf("%04d-%02d-%02d", year, month, day)
}

// atoi can't fail here: the regex let through only a few ASCII digits.
func atoi(s string) int {
	n, _ := strconv.Atoi(s)
	return n
}

func monthOf(name string) int {
	for i, m := range months {
		if strings.HasPrefix(m, name) {
			return i + 1
		}
	}
	panic("not a month: " + name)
}

func main() {
	text, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintln(os.Stderr, "isodates:", err)
		os.Exit(1)
	}
	os.Stdout.WriteString(date.ReplaceAllStringFunc(string(text), rewrite))
}
