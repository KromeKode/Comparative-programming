package main

import (
	"errors"
	"fmt"
	"slices"
	"strings"
)

// Fraction is kept in lowest terms, with the sign on the numerator. Its fields
// start with a lower-case letter, so code outside this package can't see
// them. This example is all one package, so here that is only a promise.
type Fraction struct {
	numerator   int
	denominator int // always positive
}

func gcd(a, b int) int {
	for b != 0 {
		a, b = b, a%b
	}
	return max(a, -a)
}

// NewFraction is the constructor: Go has none built in, only this convention.
func NewFraction(numerator, denominator int) (Fraction, error) {
	if denominator == 0 {
		return Fraction{}, errors.New("denominator is zero")
	}
	if denominator < 0 {
		numerator, denominator = -numerator, -denominator
	}
	divisor := gcd(numerator, denominator)
	return Fraction{numerator / divisor, denominator / divisor}, nil
}

// mustFraction is for values the program knows are fine.
func mustFraction(numerator, denominator int) Fraction {
	f, err := NewFraction(numerator, denominator)
	if err != nil {
		panic(err)
	}
	return f
}

func (f Fraction) Add(other Fraction) Fraction {
	return mustFraction(f.numerator*other.denominator+other.numerator*f.denominator,
		f.denominator*other.denominator)
}

// Compare returns a negative number, zero, or a positive number, the shape
// that slices.SortFunc wants. Go has no operator overloading, so < can't be
// used on fractions.
func (f Fraction) Compare(other Fraction) int {
	return f.numerator*other.denominator - other.numerator*f.denominator
}

// String is what fmt prints for a Fraction.
func (f Fraction) String() string {
	if f.denominator == 1 {
		return fmt.Sprint(f.numerator)
	}
	return fmt.Sprintf("%d/%d", f.numerator, f.denominator)
}

func yesNo(value bool) string {
	if value {
		return "yes"
	}
	return "no"
}

func main() {
	half, third := mustFraction(1, 2), mustFraction(1, 3)
	fmt.Println("1/2 + 1/3 =", half.Add(third))
	fmt.Println("2/-4 is stored as", mustFraction(2, -4))
	fmt.Println("6/3 is stored as", mustFraction(6, 3))
	if _, err := NewFraction(1, 0); err != nil {
		fmt.Println("1/0: error:", err)
	}
	// == compares structs field by field, which is right in lowest terms.
	fmt.Println("1/2 == 2/4:", yesNo(half == mustFraction(2, 4)))
	fmt.Println("1/3 < 1/2:", yesNo(third.Compare(half) < 0))
	fractions := []Fraction{mustFraction(3, 4), third, mustFraction(-1, 2), mustFraction(2, 4)}
	// Fraction.Compare is a method used as a function of two fractions.
	slices.SortFunc(fractions, Fraction.Compare)
	text := make([]string, len(fractions))
	for i, f := range fractions {
		text[i] = f.String()
	}
	fmt.Println("sorted:", strings.Join(text, ", "))
}
