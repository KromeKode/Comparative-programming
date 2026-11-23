// Reads one JSON text on stdin, strictly as RFC 8259 has it, and prints it compactly:
// no spaces, keys in the order they came, numbers as they were written, and strings
// written again with only the escapes they need. Or prints the first error's line and
// column, and exits with 1.
package main

import (
	"fmt"
	"io"
	"os"
	"strings"
)

const maxDepth = 1000

// quote returns s as a JSON string, with only the escapes it needs.
func quote(s string) string {
	var out strings.Builder
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
				fmt.Fprintf(&out, `\u%04x`, r)
			} else {
				out.WriteRune(r)
			}
		}
	}
	out.WriteByte('"')
	return out.String()
}

type jsonError struct {
	at      int // an index into the runes
	message string
}

func (e *jsonError) Error() string { return e.message }

type parser struct {
	text  []rune // runes, so that an index counts code points, as columns do
	i     int
	depth int
}

// end stands for the end of the text.
const end rune = -1

func (p *parser) peek() rune {
	if p.i < len(p.text) {
		return p.text[p.i]
	}
	return end
}

func (p *parser) fail(expected string) error {
	found := "the end of the input"
	if c := p.peek(); c != end {
		found = quote(string(c))
	}
	return &jsonError{p.i, "expected " + expected + ", found " + found}
}

func (p *parser) space() {
	for c := p.peek(); c == ' ' || c == '\t' || c == '\n' || c == '\r'; c = p.peek() {
		p.i++
	}
}

func (p *parser) startsWith(at int, word string) bool {
	return at+len(word) <= len(p.text) && string(p.text[at:at+len(word)]) == word
}

func (p *parser) value() (string, error) {
	switch c := p.peek(); {
	case c == '{' || c == '[':
		p.depth++
		if p.depth > maxDepth {
			return "", &jsonError{p.i, fmt.Sprintf("nesting deeper than %d", maxDepth)}
		}
		var out string
		var err error
		if c == '{' {
			out, err = p.object()
		} else {
			out, err = p.array()
		}
		p.depth--
		return out, err
	case c == '"':
		s, err := p.str()
		return quote(s), err
	case c == '-' || '0' <= c && c <= '9':
		return p.number()
	}
	for _, word := range []string{"true", "false", "null"} {
		if p.startsWith(p.i, word) {
			p.i += len(word)
			return word, nil
		}
	}
	return "", p.fail("a value")
}

func (p *parser) object() (string, error) {
	p.i++ // the {
	p.space()
	if p.peek() == '}' {
		p.i++
		return "{}", nil
	}
	var members []string
	keys := map[string]bool{}
	for {
		if p.peek() != '"' {
			if len(members) == 0 {
				return "", p.fail(`a string key or "}"`)
			}
			return "", p.fail("a string key")
		}
		at := p.i
		key, err := p.str()
		if err != nil {
			return "", err
		}
		if keys[key] {
			return "", &jsonError{at, "duplicate key " + quote(key)}
		}
		keys[key] = true
		p.space()
		if p.peek() != ':' {
			return "", p.fail(`":"`)
		}
		p.i++
		p.space()
		value, err := p.value()
		if err != nil {
			return "", err
		}
		members = append(members, quote(key)+":"+value)
		p.space()
		switch p.peek() {
		case '}':
			p.i++
			return "{" + strings.Join(members, ",") + "}", nil
		case ',':
			p.i++
			p.space()
		default:
			return "", p.fail(`"," or "}"`)
		}
	}
}

func (p *parser) array() (string, error) {
	p.i++ // the [
	p.space()
	if p.peek() == ']' {
		p.i++
		return "[]", nil
	}
	var items []string
	for {
		item, err := p.value()
		if err != nil {
			return "", err
		}
		items = append(items, item)
		p.space()
		switch p.peek() {
		case ']':
			p.i++
			return "[" + strings.Join(items, ",") + "]", nil
		case ',':
			p.i++
			p.space()
		default:
			return "", p.fail(`"," or "]"`)
		}
	}
}

func isDigit(c rune) bool { return '0' <= c && c <= '9' }

func (p *parser) digits() error {
	if !isDigit(p.peek()) {
		return p.fail("a digit")
	}
	for isDigit(p.peek()) {
		p.i++
	}
	return nil
}

func (p *parser) number() (string, error) {
	start := p.i
	if p.peek() == '-' {
		p.i++
	}
	if p.peek() == '0' {
		p.i++ // a 0 can't have digits after it
	} else if err := p.digits(); err != nil {
		return "", err
	}
	if p.peek() == '.' {
		p.i++
		if err := p.digits(); err != nil {
			return "", err
		}
	}
	if p.peek() == 'e' || p.peek() == 'E' {
		p.i++
		if p.peek() == '+' || p.peek() == '-' {
			p.i++
		}
		if err := p.digits(); err != nil {
			return "", err
		}
	}
	return string(p.text[start:p.i]), nil // as it was written
}

// hex4 returns the four hex digits at text[at:], and false if they aren't.
func (p *parser) hex4(at int) (rune, bool) {
	if at+4 > len(p.text) {
		return 0, false
	}
	var n rune
	for _, c := range p.text[at : at+4] {
		switch {
		case '0' <= c && c <= '9':
			n = n*16 + c - '0'
		case 'a' <= c && c <= 'f':
			n = n*16 + c - 'a' + 10
		case 'A' <= c && c <= 'F':
			n = n*16 + c - 'A' + 10
		default:
			return 0, false
		}
	}
	return n, true
}

var simple = map[rune]rune{
	'"': '"', '\\': '\\', '/': '/', 'b': '\b', 'f': '\f', 'n': '\n', 'r': '\r', 't': '\t',
}

// str returns the string at i, with its escapes undone.
func (p *parser) str() (string, error) {
	start := p.i
	p.i++ // the "
	var out strings.Builder
	for {
		c := p.peek()
		switch {
		case c == end:
			return "", &jsonError{start, "string is never closed"}
		case c == '"':
			p.i++
			return out.String(), nil
		case c < 0x20:
			return "", &jsonError{p.i, "control character in a string"}
		case c != '\\':
			out.WriteRune(c)
			p.i++
			continue
		}
		if p.i+1 == len(p.text) {
			return "", &jsonError{start, "string is never closed"}
		}
		escape := p.text[p.i+1]
		if r, ok := simple[escape]; ok {
			out.WriteRune(r)
			p.i += 2
			continue
		}
		code, ok := p.hex4(p.i + 2)
		if escape != 'u' || !ok {
			return "", &jsonError{p.i, "invalid escape"}
		}
		at := p.i
		p.i += 6
		if 0xDC00 <= code && code <= 0xDFFF {
			return "", &jsonError{at, "unpaired surrogate"}
		}
		if 0xD800 <= code && code <= 0xDBFF {
			// A character above U+FFFF is two escapes, a high surrogate and a low one.
			low, ok := p.hex4(p.i + 2)
			if !p.startsWith(p.i, `\u`) || !ok || low < 0xDC00 || low > 0xDFFF {
				return "", &jsonError{at, "unpaired surrogate"}
			}
			code = 0x10000 + (code-0xD800)<<10 + (low - 0xDC00)
			p.i += 6
		}
		out.WriteRune(code)
	}
}

func (p *parser) parse() (string, error) {
	p.space()
	out, err := p.value()
	if err != nil {
		return "", err
	}
	p.space()
	if p.i < len(p.text) {
		return "", p.fail("the end of the input")
	}
	return out, nil
}

func main() {
	data, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintln(os.Stderr, "jsoncheck:", err)
		os.Exit(1)
	}
	p := &parser{text: []rune(string(data))}
	out, err := p.parse()
	if err != nil {
		at := err.(*jsonError).at
		// Lines end at "\n", and columns count characters from 1.
		line, column := 1, 1
		for _, c := range p.text[:at] {
			if c == '\n' {
				line, column = line+1, 1
			} else {
				column++
			}
		}
		fmt.Fprintf(os.Stderr, "jsoncheck: line %d, column %d: %s\n", line, column, err)
		os.Exit(1)
	}
	fmt.Println(out)
}
