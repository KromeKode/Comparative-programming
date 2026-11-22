package main

import (
	"bufio"
	"fmt"
	"os"
	"reflect"
	"strconv"
	"strings"
)

// The fields are written once, here. Each tag gives the column's name, and
// the code below finds the fields, their types, and their tags with reflect.
type Book struct {
	Title   string `column:"title"`
	Author  string `column:"author"`
	Year    int64  `column:"year"`
	Pages   int64  `column:"pages"`
	InPrint bool   `column:"in_print"`
}

var bookType = reflect.TypeFor[Book]()

// fieldIndex finds the field whose tag names column, or returns -1.
func fieldIndex(column string) int {
	for i := range bookType.NumField() {
		if bookType.Field(i).Tag.Get("column") == column {
			return i
		}
	}
	return -1
}

func isWhole(text string) bool {
	digits := strings.TrimPrefix(text, "-")
	return digits != "" && strings.Trim(digits, "0123456789") == ""
}

// set parses text into field, using the parser for the field's kind.
func set(field reflect.Value, text string) error {
	switch field.Kind() {
	case reflect.String:
		field.SetString(text)
	case reflect.Int64:
		n, err := strconv.ParseInt(text, 10, 64)
		if !isWhole(text) || err != nil {
			return fmt.Errorf("%q is not a whole number", text)
		}
		field.SetInt(n)
	case reflect.Bool:
		if text != "true" && text != "false" {
			return fmt.Errorf("%q is not true or false", text)
		}
		field.SetBool(text == "true")
	default:
		return fmt.Errorf("can't set a %s", field.Kind())
	}
	return nil
}

func describe(book Book) string {
	value := reflect.ValueOf(book)
	parts := make([]string, bookType.NumField())
	for i := range parts {
		shown := fmt.Sprint(value.Field(i).Interface())
		if value.Field(i).Kind() == reflect.String {
			shown = `"` + shown + `"`
		}
		parts[i] = bookType.Field(i).Tag.Get("column") + "=" + shown
	}
	return bookType.Name() + "(" + strings.Join(parts, ", ") + ")"
}

func split(line string) []string {
	parts := strings.Split(line, ",")
	for i, part := range parts {
		parts[i] = strings.TrimSpace(part)
	}
	return parts
}

func plural(count int, word string) string {
	if count == 1 {
		return fmt.Sprintf("%d %s", count, word)
	}
	return fmt.Sprintf("%d %ss", count, word)
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	var columns []int
	haveHeader := false
	books, bad, row := 0, 0, 0
	for scanner.Scan() {
		line := scanner.Text()
		if strings.TrimSpace(line) == "" {
			continue
		}
		parts := split(line)
		if !haveHeader {
			for i, name := range parts {
				index := fieldIndex(name)
				if index < 0 {
					fmt.Println("unknown column:", name)
					return
				}
				for _, earlier := range parts[:i] {
					if earlier == name {
						fmt.Println("repeated column:", name)
						return
					}
				}
				columns = append(columns, index)
			}
			haveHeader = true
			continue
		}
		row++
		if len(parts) != len(columns) {
			fmt.Printf("row %d: expected %d values, got %d\n", row, len(columns), len(parts))
			bad++
			continue
		}
		var book Book
		// Elem gives the Book that the pointer points at, which can be changed.
		fields := reflect.ValueOf(&book).Elem()
		var problem error
		for i, index := range columns {
			if err := set(fields.Field(index), parts[i]); err != nil {
				problem = fmt.Errorf("%s: %w", bookType.Field(index).Tag.Get("column"), err)
				break
			}
		}
		if problem != nil {
			fmt.Printf("row %d: %v\n", row, problem)
			bad++
			continue
		}
		fmt.Println(describe(book))
		books++
	}
	if haveHeader {
		fmt.Printf("%s, %s\n", plural(books, "book"), plural(bad, "bad row"))
	}
}
