package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"slices"
	"strings"
)

type Config struct {
	Name    string
	Port    int
	Debug   bool
	Timeout int
}

// One error type per kind. Each one has an Error method, so each is an error.
type SyntaxError struct{ Line int }
type UnknownKeyError struct {
	Line int
	Key  string
}
type DuplicateKeyError struct {
	Line, First int
	Key         string
}
type BadValueError struct {
	Line int
	Key  string
	Err  error // what was wrong with the value
}
type MissingKeyError struct{ Key string }

func (e *SyntaxError) Error() string {
	return fmt.Sprintf("line %d: expected \"key = value\"", e.Line)
}

func (e *UnknownKeyError) Error() string {
	return fmt.Sprintf("line %d: unknown key %q", e.Line, e.Key)
}

func (e *DuplicateKeyError) Error() string {
	return fmt.Sprintf("line %d: %q is already set on line %d", e.Line, e.Key, e.First)
}

func (e *BadValueError) Error() string {
	return fmt.Sprintf("line %d: %s: %v", e.Line, e.Key, e.Err)
}

// Unwrap lets errors.Is and errors.As look inside at the value's problem.
func (e *BadValueError) Unwrap() error { return e.Err }

func (e *MissingKeyError) Error() string {
	return fmt.Sprintf("missing key %q", e.Key)
}

var kinds = []string{"syntax", "unknown key", "duplicate key", "bad value", "missing key"}

// kindOf returns the position of err's kind in kinds.
func kindOf(err error) int {
	switch err.(type) {
	case *SyntaxError:
		return 0
	case *UnknownKeyError:
		return 1
	case *DuplicateKeyError:
		return 2
	case *BadValueError:
		return 3
	default:
		return 4
	}
}

func isDigits(text string) bool {
	for _, c := range text {
		if c < '0' || c > '9' {
			return false
		}
	}
	return text != ""
}

func parseWhole(text string, low, high int) (int, error) {
	if !isDigits(text) {
		return 0, fmt.Errorf("%q is not a whole number", text)
	}
	value := 0
	for _, c := range text {
		// Stop growing past the limit, so a long number cannot overflow.
		value = min(value*10+int(c-'0'), high+1)
	}
	if value < low || value > high {
		return 0, fmt.Errorf("%s is not between %d and %d", text, low, high)
	}
	return value, nil
}

func parseBool(text string) (bool, error) {
	if text != "true" && text != "false" {
		return false, fmt.Errorf("%q is not true or false", text)
	}
	return text == "true", nil
}

// load returns the config, or every problem joined into one error.
func load(lines []string) (Config, error) {
	config := Config{Timeout: 30}
	firstLine := map[string]int{}
	var errs []error
	for index, line := range lines {
		number := index + 1
		line = strings.Trim(line, " ")
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}
		key, text, found := strings.Cut(line, "=")
		key, text = strings.Trim(key, " "), strings.Trim(text, " ")
		if !found || key == "" {
			errs = append(errs, &SyntaxError{Line: number})
			continue
		}
		if !slices.Contains([]string{"name", "port", "debug", "timeout"}, key) {
			errs = append(errs, &UnknownKeyError{Line: number, Key: key})
			continue
		}
		if first, ok := firstLine[key]; ok {
			errs = append(errs, &DuplicateKeyError{Line: number, Key: key, First: first})
			continue
		}
		firstLine[key] = number
		var err error
		switch key {
		case "name":
			if text == "" {
				err = errors.New("must not be empty")
			}
			config.Name = text
		case "port":
			config.Port, err = parseWhole(text, 1, 65535)
		case "debug":
			config.Debug, err = parseBool(text)
		case "timeout":
			config.Timeout, err = parseWhole(text, 1, 3600)
		}
		if err != nil {
			errs = append(errs, &BadValueError{Line: number, Key: key, Err: err})
		}
	}
	for _, key := range []string{"name", "port"} {
		if _, ok := firstLine[key]; !ok {
			errs = append(errs, &MissingKeyError{Key: key})
		}
	}
	// errors.Join returns nil when errs is empty.
	return config, errors.Join(errs...)
}

type file struct {
	name  string
	lines []string
}

func main() {
	var files []file
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := scanner.Text()
		if name, ok := strings.CutPrefix(line, "== "); ok {
			files = append(files, file{name: name})
		} else if len(files) > 0 {
			last := &files[len(files)-1]
			last.lines = append(last.lines, line)
		}
	}
	counts := make([]int, len(kinds))
	loaded := 0
	for _, f := range files {
		config, err := load(f.lines)
		if err != nil {
			// A joined error keeps its parts; Unwrap() []error gives them back.
			list := err.(interface{ Unwrap() []error }).Unwrap()
			plural := "s"
			if len(list) == 1 {
				plural = ""
			}
			fmt.Printf("%s: %d error%s\n", f.name, len(list), plural)
			for _, e := range list {
				fmt.Printf("  %v\n", e)
				counts[kindOf(e)]++
			}
			continue
		}
		loaded++
		fmt.Printf("%s: ok\n", f.name)
		fmt.Printf("  name = %s\n", config.Name)
		fmt.Printf("  port = %d\n", config.Port)
		fmt.Printf("  debug = %t\n", config.Debug)
		fmt.Printf("  timeout = %d\n", config.Timeout)
	}
	fmt.Printf("files loaded: %d of %d\n", loaded, len(files))
	parts := make([]string, len(kinds))
	for i, kind := range kinds {
		parts[i] = fmt.Sprintf("%s: %d", kind, counts[i])
	}
	fmt.Println(strings.Join(parts, ", "))
}
