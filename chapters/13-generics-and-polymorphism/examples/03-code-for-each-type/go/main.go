package main

import (
	"fmt"
	"strconv"
	"strings"
)

type Point struct{ X, Y int }

// Go can't choose a function by a type parameter, and can't add methods to
// int or string. So each way of showing a type is a function value...
func showInt(n int) string       { return strconv.Itoa(n) }
func showBool(b bool) string     { return strconv.FormatBool(b) }
func showString(s string) string { return strconv.Quote(s) }
func showPoint(p Point) string   { return fmt.Sprintf("(%d, %d)", p.X, p.Y) }

// ...and listOf builds the function for a list out of the one for its items.
func listOf[T any](showItem func(T) string) func([]T) string {
	return func(items []T) string {
		texts := make([]string, len(items))
		for i, item := range items {
			texts[i] = showItem(item)
		}
		return "[" + strings.Join(texts, ", ") + "]"
	}
}

func main() {
	fmt.Println(showInt(42))
	fmt.Println(showString("hi"))
	fmt.Println(showBool(true))
	fmt.Println(listOf(showInt)([]int{1, 2, 3}))
	fmt.Println(listOf(listOf(showString))([][]string{{"a", "b"}, {}, {"c"}}))
	fmt.Println(showPoint(Point{1, 2}))
	fmt.Println(listOf(showPoint)([]Point{{1, 2}, {3, 4}}))
}
