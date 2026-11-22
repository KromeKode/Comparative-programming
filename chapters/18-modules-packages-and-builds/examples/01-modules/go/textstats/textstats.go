// Package textstats counts the words in a text. A package is a folder: every
// .go file in it starts with "package textstats", and they all share one set of
// names. A name that starts with a capital letter is exported, and can be used
// by other packages. Any other name is private to the package.
package textstats

import "strings"

const Version = "1.0"

// words is unexported: only the files of this package can call it. It splits
// the text at every character that isn't an ASCII letter.
func words(text string) []string {
	return strings.FieldsFunc(text, func(r rune) bool {
		return !('a' <= r && r <= 'z' || 'A' <= r && r <= 'Z')
	})
}

// WordCount is the number of words in text. A word is a run of ASCII letters.
func WordCount(text string) int {
	return len(words(text))
}

// LongestWord is the first of the longest words in text.
func LongestWord(text string) string {
	best := ""
	for _, word := range words(text) {
		if len(word) > len(best) {
			best = word
		}
	}
	return best
}
