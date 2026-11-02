package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// Number[T] is a constraint that mentions T itself: its methods take and
// return the same type. eval asks for T Number[T], so an Int adds only Ints.
type Number[T any] interface {
	Add(T) T
	Sub(T) T
	Mul(T) T
	Div(T) (T, bool) // false when dividing by zero
	String() string
}

// parseWhole reads an optional minus sign and at least one digit.
func parseWhole(text string) (int64, bool) {
	digits := strings.TrimPrefix(text, "-")
	if digits == "" || strings.Trim(digits, "0123456789") != "" {
		return 0, false
	}
	n, err := strconv.ParseInt(text, 10, 64)
	return n, err == nil
}

type Int int64

func parseInt(text string) (Int, bool) {
	n, ok := parseWhole(text)
	return Int(n), ok
}

func (a Int) Add(b Int) Int  { return a + b }
func (a Int) Sub(b Int) Int  { return a - b }
func (a Int) Mul(b Int) Int  { return a * b }
func (a Int) String() string { return strconv.FormatInt(int64(a), 10) }

// Go's / rounds toward zero.
func (a Int) Div(b Int) (Int, bool) {
	if b == 0 {
		return 0, false
	}
	return a / b, true
}

// Frac is always in lowest terms, with a positive bottom.
type Frac struct{ top, bottom int64 }

func gcd(a, b int64) int64 {
	for b != 0 {
		a, b = b, a%b
	}
	if a < 0 {
		return -a
	}
	return a
}

func newFrac(top, bottom int64) Frac {
	if bottom < 0 {
		top, bottom = -top, -bottom
	}
	d := gcd(top, bottom)
	return Frac{top / d, bottom / d}
}

func parseFrac(text string) (Frac, bool) {
	topText, bottomText, found := strings.Cut(text, "/")
	if !found {
		bottomText = "1"
	}
	top, ok1 := parseWhole(topText)
	bottom, ok2 := parseWhole(bottomText)
	// "1/-2" and "1/0" are not fractions: the bottom must be above zero.
	if !ok1 || !ok2 || bottom <= 0 {
		return Frac{}, false
	}
	return newFrac(top, bottom), true
}

func (a Frac) Add(b Frac) Frac { return newFrac(a.top*b.bottom+b.top*a.bottom, a.bottom*b.bottom) }
func (a Frac) Sub(b Frac) Frac { return newFrac(a.top*b.bottom-b.top*a.bottom, a.bottom*b.bottom) }
func (a Frac) Mul(b Frac) Frac { return newFrac(a.top*b.top, a.bottom*b.bottom) }

func (a Frac) Div(b Frac) (Frac, bool) {
	if b.top == 0 {
		return Frac{}, false
	}
	return newFrac(a.top*b.bottom, a.bottom*b.top), true
}

func (a Frac) String() string {
	if a.bottom == 1 {
		return strconv.FormatInt(a.top, 10)
	}
	return fmt.Sprintf("%d/%d", a.top, a.bottom)
}

// Mod7 is always 0 to 6.
type Mod7 int64

// Go's % keeps the sign of the left side, so -1 % 7 is -1. Adding 7 fixes it.
func mod7(n int64) Mod7 { return Mod7((n%7 + 7) % 7) }

func parseMod7(text string) (Mod7, bool) {
	n, ok := parseWhole(text)
	return mod7(n), ok
}

func (a Mod7) Add(b Mod7) Mod7 { return mod7(int64(a + b)) }
func (a Mod7) Sub(b Mod7) Mod7 { return mod7(int64(a - b)) }
func (a Mod7) Mul(b Mod7) Mod7 { return mod7(int64(a * b)) }
func (a Mod7) String() string  { return strconv.FormatInt(int64(a), 10) }

// x to the power 5 is the inverse of x when 7 is prime: x * x^5 = x^6 = 1.
func (a Mod7) Div(b Mod7) (Mod7, bool) {
	if b == 0 {
		return 0, false
	}
	return a.Mul(b.Mul(b).Mul(b).Mul(b).Mul(b)), true
}

// eval runs the tokens on a stack of T. Go can't call a function on the type
// T itself, so the caller passes in parse.
func eval[T Number[T]](tokens []string, parse func(string) (T, bool)) (T, error) {
	var zero T
	var stack []T
	for _, token := range tokens {
		switch token {
		case "+", "-", "*", "/":
			if len(stack) < 2 {
				return zero, errors.New("not enough numbers")
			}
			left, right := stack[len(stack)-2], stack[len(stack)-1]
			stack = stack[:len(stack)-2]
			var result T
			switch token {
			case "+":
				result = left.Add(right)
			case "-":
				result = left.Sub(right)
			case "*":
				result = left.Mul(right)
			default:
				var ok bool
				if result, ok = left.Div(right); !ok {
					return zero, errors.New("division by zero")
				}
			}
			stack = append(stack, result)
		default:
			number, ok := parse(token)
			if !ok {
				return zero, fmt.Errorf("bad token \"%s\"", token)
			}
			stack = append(stack, number)
		}
	}
	if len(stack) != 1 {
		return zero, fmt.Errorf("expected one result, found %d", len(stack))
	}
	return stack[0], nil
}

// show turns eval's result into text, so the three cases below share one shape.
func show[T Number[T]](result T, err error) (string, error) {
	return result.String(), err
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		words := strings.Fields(scanner.Text())
		if len(words) == 0 {
			continue
		}
		var text string
		var err error
		switch words[0] {
		case "int":
			text, err = show(eval(words[1:], parseInt))
		case "frac":
			text, err = show(eval(words[1:], parseFrac))
		case "mod7":
			text, err = show(eval(words[1:], parseMod7))
		default:
			err = fmt.Errorf("unknown kind \"%s\"", words[0])
		}
		if err != nil {
			fmt.Println("error:", err)
		} else {
			fmt.Println(text)
		}
	}
}
