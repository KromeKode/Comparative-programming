// Evaluates arithmetic with +, -, *, and parentheses: each argument, or each line of
// input if there are none. Section 20.5 runs it in a debugger, to watch the parser
// call itself. It expects well-formed expressions, and doesn't check.
package main

import (
	"bufio"
	"fmt"
	"os"
)

// A parser is a recursive descent parser, with one method for each rule of the
// grammar.
type parser struct {
	text string
	pos  int
}

// peek skips spaces, and gives the next byte, or 0 at the end.
func (p *parser) peek() byte {
	for p.pos < len(p.text) && p.text[p.pos] == ' ' {
		p.pos++
	}
	if p.pos < len(p.text) {
		return p.text[p.pos]
	}
	return 0
}

// expr = term { ("+" | "-") term }
func (p *parser) expr() int64 {
	value := p.term()
	for p.peek() == '+' || p.peek() == '-' {
		op := p.text[p.pos]
		p.pos++
		right := p.term()
		if op == '+' {
			value += right
		} else {
			value -= right
		}
	}
	return value
}

// term = factor { "*" factor }
func (p *parser) term() int64 {
	value := p.factor()
	for p.peek() == '*' {
		p.pos++
		value *= p.factor()
	}
	return value
}

// factor = number | "(" expr ")"
func (p *parser) factor() int64 {
	if p.peek() == '(' {
		p.pos++
		value := p.expr()
		p.peek()
		p.pos++ // the ")"
		return value
	}
	var value int64
	for p.pos < len(p.text) && '0' <= p.text[p.pos] && p.text[p.pos] <= '9' {
		value = value*10 + int64(p.text[p.pos]-'0')
		p.pos++
	}
	return value
}

func evaluate(text string) int64 {
	p := parser{text: text}
	return p.expr()
}

func main() {
	for _, text := range os.Args[1:] {
		fmt.Println(evaluate(text))
	}
	if len(os.Args) > 1 {
		return
	}
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		fmt.Println(evaluate(scanner.Text()))
	}
}
