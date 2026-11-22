package main

import (
	"cmp"
	"encoding/json"
	"fmt"
	"reflect"
)

// The text in backquotes after each field is a struct tag. The compiler
// keeps it with the type, and code that reads it with reflect, such as
// encoding/json, can use it as the program runs.
type Book struct {
	Title   string `json:"title"`
	Author  string `json:"author"`
	Year    int    `json:"year"`
	InPrint bool   `json:"in_print"`
}

func compareBool(a, b bool) int {
	switch {
	case a == b:
		return 0
	case a:
		return 1
	default:
		return -1
	}
}

// Go writes no ordering for structs, so this compares the fields in turn.
// cmp.Or returns the first result that isn't 0.
func compareBooks(a, b Book) int {
	return cmp.Or(
		cmp.Compare(a.Title, b.Title),
		cmp.Compare(a.Author, b.Author),
		cmp.Compare(a.Year, b.Year),
		compareBool(a.InPrint, b.InPrint),
	)
}

func main() {
	// reflect describes a type as the program runs: its fields, their
	// names, their types, and their tags.
	bookType := reflect.TypeFor[Book]()
	fmt.Print("fields:")
	for i := range bookType.NumField() {
		field := bookType.Field(i)
		fmt.Printf(" %s (%s, tag %q)", field.Name, field.Type, field.Tag.Get("json"))
	}
	fmt.Println()

	dune := Book{"Dune", "Frank Herbert", 1965, true}
	// %+v prints a struct with its field names, which fmt finds with reflect.
	fmt.Printf("dune: %+v\n", dune)

	copied := dune // assignment copies a struct
	fmt.Println("copied == dune:", copied == dune)

	reissue := dune
	reissue.Year = 1966
	fmt.Printf("reissue: %+v\n", reissue)
	fmt.Println("reissue == dune:", reissue == dune)
	fmt.Println("dune < reissue:", compareBooks(dune, reissue) < 0)

	// A struct whose fields can all be compared with == can be a map key.
	distinct := map[Book]bool{dune: true, copied: true, reissue: true}
	fmt.Println("distinct books:", len(distinct))

	text, _ := json.Marshal(dune)
	fmt.Println("json:", string(text))
}
