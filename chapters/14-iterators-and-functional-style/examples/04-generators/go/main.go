package main

import (
	"fmt"
	"iter"
	"strconv"
	"strings"
)

// A Go iterator is already written like a generator: the function calls
// yield for each value, and yield returns false when the loop is done.
func Fibonacci() iter.Seq[int] {
	return func(yield func(int) bool) {
		a, b := 0, 1
		for yield(a) {
			a, b = b, a+b
		}
	}
}

type Node struct {
	Value       int
	Left, Right *Node
}

// walk reports whether to keep going, so a false from yield stops every
// level of the recursion at once.
func (n *Node) walk(yield func(int) bool) bool {
	return n == nil || n.Left.walk(yield) && yield(n.Value) && n.Right.walk(yield)
}

func (n *Node) InOrder() iter.Seq[int] {
	return func(yield func(int) bool) { n.walk(yield) }
}

func main() {
	tree := &Node{4, &Node{2, &Node{1, nil, nil}, &Node{3, nil, nil}},
		&Node{6, &Node{5, nil, nil}, &Node{7, nil, nil}}}

	var fibonacci []string
	for x := range Fibonacci() {
		if len(fibonacci) == 10 {
			break
		}
		fibonacci = append(fibonacci, strconv.Itoa(x))
	}
	fmt.Println("fibonacci:", strings.Join(fibonacci, " "))

	fmt.Print("in order:")
	for value := range tree.InOrder() {
		fmt.Print(" ", value)
	}

	// break makes yield return false, so 5, 6, and 7 are never visited.
	fmt.Print("\nup to 3:")
	for value := range tree.InOrder() {
		if value > 3 {
			break
		}
		fmt.Print(" ", value)
	}
	fmt.Println()
}
