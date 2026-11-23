// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strings"
)

type state int

// Go has no enums: these are numbered constants of their own type, and iota counts them.
const (
	recordStart state = iota // nothing of this record read yet
	fieldStart               // just after a comma
	unquoted                 // in a field that doesn't start with a quote
	quoted                   // in a field that does
	quote                    // just after a quote in a quoted field: its end, or the first of ""
	cr                       // just after a "\r" outside quotes, which needs a "\n" next
)

// end stands for the end of the text, so that the end goes through the machine as well.
const end rune = -1

type csvError struct {
	line, column int
	message      string
}

func (e *csvError) Error() string {
	return fmt.Sprintf("line %d, column %d: %s", e.line, e.column, e.message)
}

func fields(n int) string {
	if n == 1 {
		return "1 field"
	}
	return fmt.Sprintf("%d fields", n)
}

// parse calls emit with each record in text, and returns the first error.
func parse(text string, emit func(record []string)) error {
	s := recordStart
	var record []string
	var field strings.Builder
	width := 0           // how many fields the first record has, once it has ended
	line, column := 1, 0 // where char is: columns count characters, from 1
	var start [2]int     // where the quote that began a quoted field is, or the "\r"
	recordLine := 1
	// []rune decodes the UTF-8, so that "🦊" is one character.
	for _, char := range append([]rune(text), end) {
		column++
		if s == recordStart {
			recordLine = line
		}
		ended := false // whether char ends a record
		switch {
		case s == quoted && char == end:
			return &csvError{start[0], start[1], "quoted field is never closed"}
		case s == quoted && char == '"':
			s = quote
		case s == quoted:
			field.WriteRune(char)
		case s == quote && char == '"':
			field.WriteByte('"')
			s = quoted
		case s == cr && char == '\n':
			ended = true
		case s == cr:
			return &csvError{start[0], start[1], "a carriage return without a line feed after it"}
		case s == recordStart && char == end:
		case (s == recordStart || s == fieldStart) && char == '"':
			start = [2]int{line, column}
			s = quoted
		case s == unquoted && char == '"':
			return &csvError{line, column, "a quote in an unquoted field"}
		// From here on, s is recordStart, fieldStart, unquoted, or quote.
		case char == ',':
			record = append(record, field.String())
			field.Reset()
			s = fieldStart
		case char == '\n' || char == end:
			ended = true
		case char == '\r':
			start = [2]int{line, column}
			s = cr
		case s == quote:
			return &csvError{line, column, "expected a comma or a line end after a closing quote"}
		default:
			field.WriteRune(char)
			s = unquoted
		}
		if ended {
			record = append(record, field.String())
			field.Reset()
			if width == 0 {
				width = len(record)
			} else if len(record) != width {
				message := fmt.Sprintf("record has %s, but the first has %d",
					fields(len(record)), width)
				return &csvError{recordLine, 1, message}
			}
			emit(record)
			record = nil
			s = recordStart
		}
		if char == '\n' {
			line, column = line+1, 0
		}
	}
	return nil
}

// quoteJSON writes s as a JSON string, as JavaScript's JSON.stringify does. json.Marshal
// would also write <, >, &, U+2028, and U+2029 as escapes.
func quoteJSON(out *bufio.Writer, s string) {
	out.WriteByte('"')
	for _, r := range s {
		switch r {
		case '"':
			out.WriteString(`\"`)
		case '\\':
			out.WriteString(`\\`)
		case '\b':
			out.WriteString(`\b`)
		case '\f':
			out.WriteString(`\f`)
		case '\n':
			out.WriteString(`\n`)
		case '\r':
			out.WriteString(`\r`)
		case '\t':
			out.WriteString(`\t`)
		default:
			if r < 0x20 {
				fmt.Fprintf(out, `\u%04x`, r)
			} else {
				out.WriteRune(r)
			}
		}
	}
	out.WriteByte('"')
}

func main() {
	data, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintln(os.Stderr, "csv2json:", err)
		os.Exit(1)
	}
	// os.Stdout has no buffer of its own, so this gives it one, to write in big pieces.
	out := bufio.NewWriter(os.Stdout)
	err = parse(string(data), func(record []string) {
		out.WriteByte('[')
		for i, field := range record {
			if i > 0 {
				out.WriteByte(',')
			}
			quoteJSON(out, field)
		}
		out.WriteString("]\n")
	})
	out.Flush()
	if err != nil {
		fmt.Fprintln(os.Stderr, "csv2json:", err)
		os.Exit(1)
	}
}
