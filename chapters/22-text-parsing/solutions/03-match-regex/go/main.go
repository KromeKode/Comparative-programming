// Prints each line of stdin that has a match for the pattern, as grep does, with a
// matcher of its own: the pattern becomes a tree, the tree becomes a program for a
// Thompson NFA, and the NFA reads each line once, keeping the set of states it could be
// in. So a line takes time in proportion to its length times the pattern's, whatever
// the pattern is.
package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strings"
)

// special is what a "\" can make a literal: the characters that mean something, inside
// brackets or out.
const special = `\.[]()*+?|^$-`

type patternError struct {
	at      int // an index into the pattern's runes
	message string
}

func (e *patternError) Error() string { return e.message }

// The tree. A character, a ".", and brackets are each a set of characters: "a" is the
// range a to a, and "." is the empty set, negated.
type node interface{ isNode() }

type chars struct {
	ranges  [][2]rune
	negated bool
}

type anchor struct{ kind rune } // '^' or '$'

type concat struct{ items []node }

type alternation struct{ options []node }

type repeat struct {
	item node
	kind rune // '*', '+', or '?'
}

func (chars) isNode()       {}
func (anchor) isNode()      {}
func (concat) isNode()      {}
func (alternation) isNode() {}
func (repeat) isNode()      {}

// A parser reads alternation = concat ("|" concat)*, concat = repeat*, and
// repeat = atom [*+?].
type parser struct {
	pattern []rune // runes, so that an index counts characters, as columns do
	i       int
}

// end stands for the end of the pattern.
const end rune = -1

func (p *parser) peek() rune {
	if p.i < len(p.pattern) {
		return p.pattern[p.i]
	}
	return end
}

func (p *parser) parse() (node, error) {
	n, err := p.alternation()
	if err != nil {
		return nil, err
	}
	if p.i < len(p.pattern) {
		// alternation stops early only at a ")".
		return nil, &patternError{p.i, `unmatched ")"`}
	}
	return n, nil
}

func (p *parser) alternation() (node, error) {
	var options []node
	for {
		n, err := p.concat()
		if err != nil {
			return nil, err
		}
		options = append(options, n)
		if p.peek() != '|' {
			break
		}
		p.i++
	}
	if len(options) == 1 {
		return options[0], nil
	}
	return alternation{options}, nil
}

func (p *parser) concat() (node, error) {
	var items []node
	for c := p.peek(); c != end && c != '|' && c != ')'; c = p.peek() {
		n, err := p.repeat()
		if err != nil {
			return nil, err
		}
		items = append(items, n)
	}
	return concat{items}, nil
}

func isRepeat(c rune) bool { return c == '*' || c == '+' || c == '?' }

func (p *parser) repeat() (node, error) {
	c := p.peek()
	// At the start, or after "(", "|", "^", "$", or another "*", "+", or "?".
	if isRepeat(c) {
		return nil, &patternError{p.i, "nothing to repeat"}
	}
	if c == '^' || c == '$' {
		p.i++
		return anchor{c}, nil
	}
	item, err := p.atom()
	if err != nil {
		return nil, err
	}
	if c := p.peek(); isRepeat(c) {
		p.i++
		return repeat{item, c}, nil
	}
	return item, nil
}

func (p *parser) atom() (node, error) {
	switch p.peek() {
	case '(':
		start := p.i
		p.i++
		n, err := p.alternation()
		if err != nil {
			return nil, err
		}
		if p.peek() != ')' {
			return nil, &patternError{start, `unclosed "("`}
		}
		p.i++
		return n, nil
	case '[':
		return p.brackets()
	case '.':
		p.i++
		return chars{nil, true}, nil
	}
	c, err := p.char()
	if err != nil {
		return nil, err
	}
	return chars{[][2]rune{{c, c}}, false}, nil
}

// char returns a character, or a "\" and the character it makes a literal.
func (p *parser) char() (rune, error) {
	c := p.pattern[p.i]
	if c != '\\' {
		p.i++
		return c, nil
	}
	if p.i+1 == len(p.pattern) {
		return 0, &patternError{p.i, "trailing backslash"}
	}
	c = p.pattern[p.i+1]
	if !strings.ContainsRune(special, c) {
		return 0, &patternError{p.i, "invalid escape"}
	}
	p.i += 2
	return c, nil
}

func (p *parser) brackets() (node, error) {
	start := p.i
	p.i++ // the [
	negated := p.peek() == '^'
	if negated {
		p.i++
	}
	if p.peek() == ']' {
		return nil, &patternError{start, "empty brackets"}
	}
	var ranges [][2]rune
	for p.peek() != ']' {
		if p.peek() == end {
			return nil, &patternError{start, `unclosed "["`}
		}
		at := p.i
		low, err := p.char()
		if err != nil {
			return nil, err
		}
		high := low
		// A "-" makes a range, unless it's last: "[a-]" is "a" and "-".
		if p.i+1 < len(p.pattern) && p.pattern[p.i] == '-' && p.pattern[p.i+1] != ']' {
			p.i++
			if high, err = p.char(); err != nil {
				return nil, err
			}
			if high < low {
				return nil, &patternError{at, "range out of order"}
			}
		}
		ranges = append(ranges, [2]rune{low, high})
	}
	p.i++ // the ]
	return chars{ranges, negated}, nil
}

// The program. opChars reads a character, if it's in the set, and goes on to the next
// instruction; opSplit goes on to both x and y; opJump goes on to x; opStart and opEnd
// go on if the position is the start or the end of the line; and opMatch is a match.
type op int

const (
	opChars op = iota
	opSplit
	opJump
	opStart
	opEnd
	opMatch
)

type instruction struct {
	op      op
	ranges  [][2]rune
	negated bool
	x, y    int
}

// compile appends n's instructions to the program. They refer to each other by index,
// since append can move them.
func compile(n node, program []instruction) []instruction {
	start := len(program)
	switch n := n.(type) {
	case chars:
		program = append(program, instruction{op: opChars, ranges: n.ranges, negated: n.negated})
	case anchor:
		if n.kind == '^' {
			program = append(program, instruction{op: opStart})
		} else {
			program = append(program, instruction{op: opEnd})
		}
	case concat:
		for _, item := range n.items {
			program = compile(item, program)
		}
	case alternation:
		// split L1, L2; L1: first; jump END; L2: split L3, L4; L3: second; ...
		var jumps []int
		for _, option := range n.options[:len(n.options)-1] {
			split := len(program)
			program = append(program, instruction{op: opSplit, x: split + 1})
			program = compile(option, program)
			jumps = append(jumps, len(program))
			program = append(program, instruction{op: opJump})
			program[split].y = len(program)
		}
		program = compile(n.options[len(n.options)-1], program)
		for _, jump := range jumps {
			program[jump].x = len(program)
		}
	case repeat:
		switch n.kind {
		case '*':
			// L1: split L2, L3; L2: item; jump L1; L3:
			program = append(program, instruction{op: opSplit, x: start + 1})
			program = compile(n.item, program)
			program = append(program, instruction{op: opJump, x: start})
			program[start].y = len(program)
		case '+':
			// L1: item; split L1, L2; L2:
			program = compile(n.item, program)
			program = append(program, instruction{op: opSplit, x: start, y: len(program) + 1})
		case '?':
			// split L1, L2; L1: item; L2:
			program = append(program, instruction{op: opSplit, x: start + 1})
			program = compile(n.item, program)
			program[start].y = len(program)
		}
	}
	return program
}

// follow returns the opChars instructions that pcs lead to without reading a
// character, at the position at, and true if one of them leads to opMatch.
func follow(program []instruction, pcs []int, at, length int) ([]int, bool) {
	var waiting []int
	seen := make([]bool, len(program))
	stack := append([]int(nil), pcs...)
	for len(stack) > 0 {
		pc := stack[len(stack)-1]
		stack = stack[:len(stack)-1]
		if seen[pc] {
			continue // so that a loop such as (a*)* doesn't go around forever
		}
		seen[pc] = true
		switch in := program[pc]; in.op {
		case opChars:
			waiting = append(waiting, pc)
		case opSplit:
			stack = append(stack, in.y, in.x)
		case opJump:
			stack = append(stack, in.x)
		case opStart:
			if at == 0 {
				stack = append(stack, pc+1)
			}
		case opEnd:
			if at == length {
				stack = append(stack, pc+1)
			}
		case opMatch:
			return nil, true
		}
	}
	return waiting, false
}

func (in instruction) accepts(c rune) bool {
	inside := false
	for _, r := range in.ranges {
		if r[0] <= c && c <= r[1] {
			inside = true
			break
		}
	}
	return inside != in.negated
}

// search reports whether a match starts anywhere in the line: instruction 0 starts at
// each position.
func search(program []instruction, line []rune) bool {
	var waiting []int
	for at := 0; at <= len(line); at++ {
		reached, matched := follow(program, append(waiting, 0), at, len(line))
		if matched {
			return true
		}
		waiting = nil
		if at < len(line) {
			for _, pc := range reached {
				if program[pc].accepts(line[at]) {
					waiting = append(waiting, pc+1)
				}
			}
		}
	}
	return false
}

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintln(os.Stderr, "usage: rematch PATTERN")
		os.Exit(2)
	}
	tree, err := (&parser{pattern: []rune(os.Args[1])}).parse()
	if err != nil {
		fmt.Fprintf(os.Stderr, "rematch: column %d: %s\n", err.(*patternError).at+1, err)
		os.Exit(2)
	}
	program := append(compile(tree, nil), instruction{op: opMatch})
	// All of it, not a bufio.Scanner, which stops at a line longer than 64 KiB.
	data, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintln(os.Stderr, "rematch:", err)
		os.Exit(2)
	}
	lines := strings.Split(string(data), "\n")
	if lines[len(lines)-1] == "" {
		lines = lines[:len(lines)-1] // what comes after the last "\n", or the empty input
	}
	out := bufio.NewWriter(os.Stdout)
	found := false
	for _, line := range lines {
		if search(program, []rune(line)) {
			out.WriteString(line + "\n")
			found = true
		}
	}
	out.Flush()
	if !found {
		os.Exit(1)
	}
}
