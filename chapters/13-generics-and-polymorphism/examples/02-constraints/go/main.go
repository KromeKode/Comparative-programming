package main

import (
	"cmp"
	"fmt"
	"strings"
)

// A constraint can list types. ~int64 means int64 and every type whose
// underlying type is int64, such as Cents below.
type Number interface {
	~int | ~int64 | ~float64
}

// Cents keeps whole cents, so adding never rounds. Its underlying type is
// int64, so + already works on it.
type Cents int64

func (c Cents) String() string {
	return fmt.Sprintf("$%d.%02d", c/100, c%100)
}

// var sum T starts at the zero value, which for every Number is 0.
func Total[T Number](items []T) T {
	var sum T
	for _, item := range items {
		sum += item
	}
	return sum
}

// cmp.Ordered covers ~int, ~string, ~float64, and the rest, so Cents meets
// it too. fmt.Sprint finds the String method when the program runs.
func DescribeRange[T cmp.Ordered](items []T) string {
	low, high := items[0], items[0]
	for _, item := range items {
		low = min(low, item)
		high = max(high, item)
	}
	return fmt.Sprint(low, " to ", high)
}

func list[T any](items []T) string {
	texts := make([]string, len(items))
	for i, item := range items {
		texts[i] = fmt.Sprint(item)
	}
	return strings.Join(texts, ", ")
}

func main() {
	numbers := []int{1, 2, 3}
	halves := []float64{0.5, 0.25}
	prices := []Cents{150, 225}
	fmt.Printf("total of %s: %v\n", list(numbers), Total(numbers))
	fmt.Printf("total of %s: %v\n", list(halves), Total(halves))
	fmt.Printf("total of %s: %v\n", list(prices), Total(prices))

	digits := []int{3, 9, 1}
	fruit := []string{"pear", "fig", "apple"}
	costs := []Cents{200, 99}
	fmt.Printf("range of %s: %s\n", list(digits), DescribeRange(digits))
	fmt.Printf("range of %s: %s\n", list(fruit), DescribeRange(fruit))
	fmt.Printf("range of %s: %s\n", list(costs), DescribeRange(costs))
}
