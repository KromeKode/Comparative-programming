// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. encoding/json fills in
// structs, by the names in their fields' tags, and checks each field's type.
package main

import (
	"encoding/json"
	"fmt"
	"os"
)

// A field that is missing from the JSON keeps its zero value, so a book with no year
// would have year 0. Pointers tell the two apart: they stay nil when it is missing.
type bookJSON struct {
	Title  *string  `json:"title"`
	Author *string  `json:"author"`
	Year   *int     `json:"year"`
	Tags   []string `json:"tags"` // missing or null: a nil slice, which is no tags
}

type libraryJSON struct {
	Books []bookJSON `json:"books"`
}

type book struct {
	title, author string
	year          int
	tags          []string
}

// The report's fields are written in this order, under these names.
type report struct {
	Count    int                 `json:"count"`
	Oldest   *string             `json:"oldest"`   // nil is written as null
	Tags     map[string][]string `json:"tags"`     // written in order of key
	Untagged []string            `json:"untagged"` // a nil slice would be null, not []
}

func readBooks(path string) ([]book, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var library libraryJSON
	if err := json.Unmarshal(data, &library); err != nil {
		return nil, fmt.Errorf("%s: %w", path, err)
	}
	if library.Books == nil {
		return nil, fmt.Errorf("%s: expected an object with a list of books", path)
	}
	books := make([]book, 0, len(library.Books))
	for _, b := range library.Books {
		if b.Title == nil || b.Author == nil || b.Year == nil {
			return nil, fmt.Errorf("%s: each book needs a title, an author, and a year", path)
		}
		books = append(books, book{*b.Title, *b.Author, *b.Year, b.Tags})
	}
	return books, nil
}

func makeReport(books []book) report {
	r := report{Count: len(books), Tags: map[string][]string{}, Untagged: []string{}}
	oldest := -1 // the first of the oldest, when two have the same year
	for i, b := range books {
		if oldest < 0 || b.year < books[oldest].year {
			oldest = i
		}
		for _, tag := range b.tags {
			r.Tags[tag] = append(r.Tags[tag], b.title)
		}
		if len(b.tags) == 0 {
			r.Untagged = append(r.Untagged, b.title)
		}
	}
	if oldest >= 0 {
		description := books[oldest].title + ", by " + books[oldest].author
		r.Oldest = &description
	}
	return r
}

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintln(os.Stderr, "usage: books FILE")
		os.Exit(2)
	}
	books, err := readBooks(os.Args[1])
	if err != nil {
		fmt.Fprintln(os.Stderr, "books:", err)
		os.Exit(1)
	}
	encoder := json.NewEncoder(os.Stdout)
	encoder.SetIndent("", "  ")
	// Without this, & < and > are written as & < and >, so that the
	// JSON is safe to put in a web page.
	encoder.SetEscapeHTML(false)
	if err := encoder.Encode(makeReport(books)); err != nil {
		fmt.Fprintln(os.Stderr, "books:", err)
		os.Exit(1)
	}
}
