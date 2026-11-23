// Prints how one JSON file differs from another: what was removed, what was added, and
// what changed. ../README.md says what it should print.
package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/fs"
	"maps"
	"os"
	"regexp"
	"slices"
	"strconv"
	"strings"
	"syscall"
)

const usage = "usage: jsondiff OLD NEW"

var identifier = regexp.MustCompile(`^[A-Za-z_][A-Za-z0-9_]*$`)

// message is what strerror says for the errors the README names. Go's own messages
// for them start with a small letter, such as "open old.json: no such file or directory".
func message(err error) string {
	switch {
	case errors.Is(err, fs.ErrNotExist):
		return "No such file or directory"
	case errors.Is(err, syscall.EISDIR):
		return "Is a directory"
	}
	return err.Error()
}

// writeString writes s as a JSON string. json.Marshal would write <, >, and & as \u003c,
// \u003e, and \u0026, and U+2028 and U+2029 as \u2028 and \u2029, so that the JSON is
// safe to put in a web page's <script>. The bytes of a character beyond ASCII are all
// 0x80 or more, so going byte by byte copies them as they are.
func writeString(b *strings.Builder, s string) {
	b.WriteByte('"')
	for i := 0; i < len(s); i++ {
		c := s[i]
		switch {
		case c == '"' || c == '\\':
			b.WriteByte('\\')
			b.WriteByte(c)
		case c == '\b':
			b.WriteString(`\b`)
		case c == '\f':
			b.WriteString(`\f`)
		case c == '\n':
			b.WriteString(`\n`)
		case c == '\r':
			b.WriteString(`\r`)
		case c == '\t':
			b.WriteString(`\t`)
		case c < 0x20:
			fmt.Fprintf(b, `\u%04x`, c)
		default:
			b.WriteByte(c)
		}
	}
	b.WriteByte('"')
}

// write writes value as compact JSON, with each object's keys in order.
func write(b *strings.Builder, value any) {
	switch v := value.(type) {
	case map[string]any:
		b.WriteByte('{')
		for i, key := range slices.Sorted(maps.Keys(v)) { // a map's order is random
			if i > 0 {
				b.WriteByte(',')
			}
			writeString(b, key)
			b.WriteByte(':')
			write(b, v[key])
		}
		b.WriteByte('}')
	case []any:
		b.WriteByte('[')
		for i, item := range v {
			if i > 0 {
				b.WriteByte(',')
			}
			write(b, item)
		}
		b.WriteByte(']')
	case string:
		writeString(b, v)
	case float64: // Unmarshal gives every number as a float64
		b.WriteString(strconv.FormatFloat(v, 'f', -1, 64))
	case bool:
		b.WriteString(strconv.FormatBool(v))
	case nil:
		b.WriteString("null")
	}
}

func compact(value any) string {
	var b strings.Builder
	write(&b, value)
	return b.String()
}

// step is the part of a path that leads to a key: .name, or ["first name"].
func step(key string) string {
	if identifier.MatchString(key) {
		return "." + key
	}
	return "[" + compact(key) + "]"
}

// diff adds a line to out for each difference between before and after.
func diff(path string, before, after any, out *[]string) {
	beforeMap, beforeIsMap := before.(map[string]any)
	afterMap, afterIsMap := after.(map[string]any)
	beforeList, beforeIsList := before.([]any)
	afterList, afterIsList := after.([]any)
	switch {
	case beforeIsMap && afterIsMap:
		keys := slices.Concat(slices.Collect(maps.Keys(beforeMap)), slices.Collect(maps.Keys(afterMap)))
		slices.Sort(keys)
		for _, key := range slices.Compact(keys) { // Compact drops the second of two the same
			here := path + step(key)
			beforeValue, inBefore := beforeMap[key]
			afterValue, inAfter := afterMap[key]
			switch {
			case !inAfter:
				*out = append(*out, fmt.Sprintf("- %s: %s", here, compact(beforeValue)))
			case !inBefore:
				*out = append(*out, fmt.Sprintf("+ %s: %s", here, compact(afterValue)))
			default:
				diff(here, beforeValue, afterValue, out)
			}
		}
	case beforeIsList && afterIsList:
		for i := range max(len(beforeList), len(afterList)) {
			here := fmt.Sprintf("%s[%d]", path, i)
			switch {
			case i >= len(afterList):
				*out = append(*out, fmt.Sprintf("- %s: %s", here, compact(beforeList[i])))
			case i >= len(beforeList):
				*out = append(*out, fmt.Sprintf("+ %s: %s", here, compact(afterList[i])))
			default:
				diff(here, beforeList[i], afterList[i], out)
			}
		}
	// != on two maps or two slices would panic, but those went the ways above. Two
	// values of different types are never equal.
	case before != after:
		*out = append(*out, fmt.Sprintf("~ %s: %s -> %s", path, compact(before), compact(after)))
	}
}

func main() {
	if len(os.Args) != 3 {
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(2)
	}
	var values []any
	for _, path := range os.Args[1:] {
		data, err := os.ReadFile(path)
		if err != nil {
			fmt.Fprintf(os.Stderr, "jsondiff: %s: %s\n", path, message(err))
			os.Exit(2)
		}
		var value any
		// Unmarshal checks that data is one JSON value and nothing more. A
		// json.Decoder reads one value and leaves whatever comes after it.
		if err := json.Unmarshal(data, &value); err != nil {
			fmt.Fprintf(os.Stderr, "jsondiff: %s: not valid JSON\n", path)
			os.Exit(2)
		}
		values = append(values, value)
	}
	var lines []string
	diff("$", values[0], values[1], &lines)
	for _, line := range lines {
		fmt.Println(line)
	}
	if len(lines) > 0 {
		os.Exit(1)
	}
}
