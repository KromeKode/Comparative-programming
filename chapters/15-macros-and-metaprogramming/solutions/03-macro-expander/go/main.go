package main

import (
	"bufio"
	"errors"
	"fmt"
	"maps"
	"os"
	"slices"
	"strings"
)

type Macro struct {
	function bool // whether the macro takes parentheses, like SQUARE(x)
	params   []string
	body     string
}

var macros = map[string]Macro{}

var errBadDefine = errors.New("bad #define")

func isWordByte(c byte) bool {
	return 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || '0' <= c && c <= '9' || c == '_'
}

func wordEnd(text string, start int) int {
	for start < len(text) && isWordByte(text[start]) {
		start++
	}
	return start
}

func spacesEnd(text string, start int) int {
	for start < len(text) && text[start] == ' ' {
		start++
	}
	return start
}

func isName(word string) bool {
	return word != "" && !('0' <= word[0] && word[0] <= '9') && wordEnd(word, 0) == len(word)
}

// stringEnd returns the index just past the string that starts at
// text[start], a quote.
func stringEnd(text string, start int) int {
	close := strings.IndexByte(text[start+1:], '"')
	if close < 0 {
		return len(text)
	}
	return start + close + 2
}

// splitArgs splits the arguments of a call at its top-level commas, and
// trims them. It returns them with the index just past the ")".
func splitArgs(text string, open int, name string) ([]string, int, error) {
	var args []string
	depth := 0
	start, i := open+1, open+1
	for i < len(text) {
		switch c := text[i]; {
		case c == '"':
			i = stringEnd(text, i)
			continue
		case c == '(':
			depth++
		case c == ')' && depth > 0:
			depth--
		case c == ')':
			return append(args, strings.TrimSpace(text[start:i])), i + 1, nil
		case c == ',' && depth == 0:
			args = append(args, strings.TrimSpace(text[start:i]))
			start = i + 1
		}
		i++
	}
	return nil, 0, fmt.Errorf("unterminated call to %s", name)
}

// substitute returns the macro's body, with each parameter replaced by its
// argument.
func substitute(macro Macro, args []string, disabled map[string]bool) (string, error) {
	raw := map[string]string{}
	for p, param := range macro.params {
		raw[param] = args[p]
	}
	expanded := map[string]string{} // an argument is expanded only if it is used
	var out strings.Builder
	body := macro.body
	i := 0
	for i < len(body) {
		end := i + 1
		switch {
		case body[i] == '"':
			end = stringEnd(body, i)
		case body[i] == '#':
			// # and a parameter make a string of the argument as written.
			start := spacesEnd(body, i+1)
			stop := wordEnd(body, start)
			if arg, ok := raw[body[start:stop]]; ok {
				out.WriteString(`"` + arg + `"`)
				i = stop
				continue
			}
		case isWordByte(body[i]):
			end = wordEnd(body, i)
			word := body[i:end]
			if arg, ok := raw[word]; ok {
				if _, done := expanded[word]; !done {
					expansion, err := expand(arg, disabled)
					if err != nil {
						return "", err
					}
					expanded[word] = expansion
				}
				out.WriteString(expanded[word])
				i = end
				continue
			}
		}
		out.WriteString(body[i:end])
		i = end
	}
	return out.String(), nil
}

// expandCall expands a call to the macro called name, whose "(" is at
// text[open]. It returns the expansion and the index just past the ")".
func expandCall(name string, macro Macro, text string, open int,
	disabled, inner map[string]bool) (string, int, error) {
	args, after, err := splitArgs(text, open, name)
	if err != nil {
		return "", 0, err
	}
	if len(macro.params) == 0 && len(args) == 1 && args[0] == "" {
		args = nil
	}
	if count := len(macro.params); len(args) != count {
		noun := "arguments"
		if count == 1 {
			noun = "argument"
		}
		return "", 0, fmt.Errorf("%s takes %d %s, got %d", name, count, noun, len(args))
	}
	body, err := substitute(macro, args, disabled)
	if err != nil {
		return "", 0, err
	}
	expansion, err := expand(body, inner)
	return expansion, after, err
}

// expand expands every macro in text, except those in disabled.
func expand(text string, disabled map[string]bool) (string, error) {
	var out strings.Builder
	i := 0
	for i < len(text) {
		end := i + 1
		if text[i] == '"' {
			end = stringEnd(text, i)
		} else if isWordByte(text[i]) {
			end = wordEnd(text, i)
			word := text[i:end]
			if macro, ok := macros[word]; ok && !disabled[word] {
				// Inside its own expansion, a macro's name is left alone.
				inner := map[string]bool{word: true}
				maps.Copy(inner, disabled)
				if !macro.function {
					expansion, err := expand(macro.body, inner)
					if err != nil {
						return "", err
					}
					out.WriteString(expansion)
					i = end
					continue
				}
				paren := spacesEnd(text, end)
				if paren < len(text) && text[paren] == '(' {
					expansion, after, err := expandCall(word, macro, text, paren, disabled, inner)
					if err != nil {
						return "", err
					}
					out.WriteString(expansion)
					i = after
					continue
				}
			}
		}
		out.WriteString(text[i:end])
		i = end
	}
	return out.String(), nil
}

func define(rest string) error {
	end := wordEnd(rest, 0)
	name := rest[:end]
	if !isName(name) {
		return errBadDefine
	}
	afterOpen, function := strings.CutPrefix(rest[end:], "(")
	if !function {
		macros[name] = Macro{body: strings.TrimSpace(rest[end:])}
		return nil
	}
	inside, body, found := strings.Cut(afterOpen, ")")
	if !found {
		return errBadDefine
	}
	var params []string
	if strings.TrimSpace(inside) != "" {
		for _, param := range strings.Split(inside, ",") {
			param = strings.TrimSpace(param)
			if !isName(param) || slices.Contains(params, param) {
				return errBadDefine
			}
			params = append(params, param)
		}
	}
	macros[name] = Macro{function: true, params: params, body: strings.TrimSpace(body)}
	return nil
}

// run returns the line to print, and false for a directive, which prints
// nothing.
func run(line string) (string, bool, error) {
	directive, rest, _ := strings.Cut(strings.TrimSpace(line), " ")
	rest = strings.TrimSpace(rest)
	switch directive {
	case "#define":
		return "", false, define(rest)
	case "#undef":
		if !isName(rest) {
			return "", false, errors.New("bad #undef")
		}
		delete(macros, rest)
		return "", false, nil
	}
	text, err := expand(line, map[string]bool{})
	return text, true, err
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		text, show, err := run(scanner.Text())
		if err != nil {
			fmt.Println("error:", err)
		} else if show {
			fmt.Println(text)
		}
	}
}
