// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
package main

import (
	"bufio"
	"fmt"
	"math"
	"os"
	"strconv"
	"strings"
)

type calcError struct {
	column  int // counted in characters, from 1
	message string
}

func (e *calcError) Error() string { return e.message }

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column.
type kind int

const (
	number   kind = iota
	operator      // an operator or a parenthesis
	end           // after the last token
)

type token struct {
	kind   kind
	text   string
	value  int64 // a number's value
	column int
}

func describe(t token) string {
	if t.kind == end {
		return "the end of the line"
	}
	return `"` + t.text + `"`
}

func isDigit(r rune) bool { return '0' <= r && r <= '9' }

func tokens(line string) ([]token, error) {
	// A []rune, so that an index counts code points, and i + 1 is the column.
	chars := []rune(line)
	var found []token
	for i := 0; i < len(chars); {
		c := chars[i]
		switch {
		case c == ' ' || c == '\t':
			i++
		case isDigit(c):
			start := i
			for i < len(chars) && isDigit(chars[i]) {
				i++
			}
			text := string(chars[start:i])
			// Only digits, so ParseInt fails only if the number doesn't fit.
			value, err := strconv.ParseInt(text, 10, 64)
			if err != nil {
				return nil, &calcError{start + 1, "number too large for 64 bits"}
			}
			found = append(found, token{number, text, value, start + 1})
		case strings.ContainsRune("+-*/%^()", c):
			found = append(found, token{operator, string(c), 0, i + 1})
			i++
		default:
			return nil, &calcError{i + 1, `unexpected "` + string(c) + `"`}
		}
	}
	return append(found, token{end, "", 0, len(chars) + 1}), nil
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. Go has no sum
// types: an expr is any type with an isExpr method, and these three are the only ones.
type expr interface{ isExpr() }

type numberExpr struct{ value int64 }

type negateExpr struct {
	operand expr
	column  int // where the operator is, for an error such as overflow
}

type binaryExpr struct {
	op          string
	left, right expr
	column      int
}

func (numberExpr) isExpr() {}
func (negateExpr) isExpr() {}
func (binaryExpr) isExpr() {}

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
var infix = map[string][2]int{
	"+": {1, 2}, "-": {1, 2}, "*": {3, 4}, "/": {3, 4}, "%": {3, 4}, "^": {8, 7},
}

// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const prefix = 5

type parser struct {
	tokens []token
	at     int
}

func (p *parser) next() token {
	t := p.tokens[p.at]
	if t.kind != end {
		p.at++
	}
	return t
}

// expression parses the longest expression whose operators hold at least minPower.
func (p *parser) expression(minPower int) (expr, error) {
	t := p.next()
	var left expr
	switch {
	case t.kind == number:
		left = numberExpr{t.value}
	case t.text == "-":
		operand, err := p.expression(prefix)
		if err != nil {
			return nil, err
		}
		left = negateExpr{operand, t.column}
	case t.text == "(":
		inside, err := p.expression(0)
		if err != nil {
			return nil, err
		}
		if close := p.next(); close.text != ")" {
			return nil, &calcError{close.column, `expected ")", found ` + describe(close)}
		}
		left = inside
	default:
		return nil, &calcError{t.column, `expected a number, "(", or "-", found ` + describe(t)}
	}
	for {
		t := p.tokens[p.at]
		powers, ok := infix[t.text]
		if t.kind != operator || !ok || powers[0] < minPower {
			return left, nil
		}
		p.next()
		right, err := p.expression(powers[1])
		if err != nil {
			return nil, err
		}
		left = binaryExpr{t.text, left, right, t.column}
	}
}

func parse(line string) (expr, error) {
	found, err := tokens(line)
	if err != nil {
		return nil, err
	}
	p := &parser{tokens: found}
	e, err := p.expression(0)
	if err != nil {
		return nil, err
	}
	if t := p.next(); t.kind != end {
		return nil, &calcError{t.column, "expected an operator, found " + describe(t)}
	}
	return e, nil
}

func show(e expr) string {
	switch e := e.(type) {
	case numberExpr:
		return strconv.FormatInt(e.value, 10)
	case negateExpr:
		return "(-" + show(e.operand) + ")"
	case binaryExpr:
		return "(" + show(e.left) + " " + e.op + " " + show(e.right) + ")"
	}
	panic("not an expr")
}

// Go's integers wrap around when they overflow, with no error, so each of these checks
// first that the answer fits. It returns false if it doesn't.
func apply(op string, a, b int64) (int64, bool) {
	switch op {
	case "+":
		return a + b, !(b > 0 && a > math.MaxInt64-b || b < 0 && a < math.MinInt64-b)
	case "-":
		return a - b, !(b < 0 && a > math.MaxInt64+b || b > 0 && a < math.MinInt64+b)
	case "*":
		if a == 0 || b == 0 {
			return 0, true
		}
		c := a * b
		return c, c/b == a && !(a == -1 && b == math.MinInt64) && !(b == -1 && a == math.MinInt64)
	case "/": // rounds toward zero
		return a / b, !(a == math.MinInt64 && b == -1)
	case "%": // has the sign of a, and math.MinInt64 % -1 is 0
		return a % b, true
	}
	// "^": squares the base for each bit of the exponent, and stops at the first overflow.
	result := int64(1)
	for ok := true; b > 0; b /= 2 {
		if b%2 == 1 {
			if result, ok = apply("*", result, a); !ok {
				return 0, false
			}
		}
		if b > 1 {
			if a, ok = apply("*", a, a); !ok {
				return 0, false
			}
		}
	}
	return result, true
}

func evaluate(e expr) (int64, error) {
	switch e := e.(type) {
	case numberExpr:
		return e.value, nil
	case negateExpr:
		value, err := evaluate(e.operand)
		if err != nil {
			return 0, err
		}
		if value == math.MinInt64 {
			return 0, &calcError{e.column, "overflow"}
		}
		return -value, nil
	case binaryExpr:
		a, err := evaluate(e.left)
		if err != nil {
			return 0, err
		}
		b, err := evaluate(e.right)
		if err != nil {
			return 0, err
		}
		switch {
		case (e.op == "/" || e.op == "%") && b == 0:
			return 0, &calcError{e.column, "division by zero"}
		case e.op == "^" && b < 0:
			return 0, &calcError{e.column, "negative exponent"}
		}
		value, ok := apply(e.op, a, b)
		if !ok {
			return 0, &calcError{e.column, "overflow"}
		}
		return value, nil
	}
	panic("not an expr")
}

func main() {
	status := 0
	scanner := bufio.NewScanner(os.Stdin)
	// A Scanner splits at "\n" and removes a "\r" before it.
	for n := 1; scanner.Scan(); n++ {
		line := scanner.Text()
		if strings.Trim(line, " \t") == "" {
			continue
		}
		e, err := parse(line)
		var value int64
		if err == nil {
			value, err = evaluate(e)
		}
		if err == nil {
			fmt.Printf("%s = %d\n", show(e), value)
			continue
		}
		column := err.(*calcError).column
		// The caret goes under the column. A tab before it stays a tab, so that the caret
		// lines up with the text whatever a tab's width is.
		pad := []rune(line)[:column-1]
		for i, c := range pad {
			if c != '\t' {
				pad[i] = ' '
			}
		}
		fmt.Fprintf(os.Stderr, "calc: line %d, column %d: %s\n", n, column, err)
		fmt.Fprintf(os.Stderr, "  %s\n  %s^\n", line, string(pad))
		status = 1
	}
	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "calc:", err)
		status = 1
	}
	os.Exit(status)
}
