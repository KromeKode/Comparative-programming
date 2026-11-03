package main

import (
	"cmp"
	"fmt"
)

// A generic type. T is a type parameter, and the constraint any lets it be
// any type at all.
type Stack[T any] struct {
	items []T
}

func (s *Stack[T]) Push(item T) {
	s.items = append(s.items, item)
}

// Pop returns the zero value of T and false when the stack is empty.
func (s *Stack[T]) Pop() (T, bool) {
	var zero T
	if len(s.items) == 0 {
		return zero, false
	}
	top := s.items[len(s.items)-1]
	s.items = s.items[:len(s.items)-1]
	return top, true
}

func (s *Stack[T]) Peek() (T, bool) {
	var zero T
	if len(s.items) == 0 {
		return zero, false
	}
	return s.items[len(s.items)-1], true
}

func (s *Stack[T]) Len() int {
	return len(s.items)
}

// A generic function. cmp.Ordered allows the types that < and > work on:
// integers, floats, and strings.
func Largest[T cmp.Ordered](items []T) (T, bool) {
	var zero T
	if len(items) == 0 {
		return zero, false
	}
	best := items[0]
	for _, item := range items {
		if item > best {
			best = item
		}
	}
	return best, true
}

// Two type parameters. Swap returns a Pair with them the other way round.
type Pair[A, B any] struct {
	First  A
	Second B
}

func (p Pair[A, B]) Swap() Pair[B, A] {
	return Pair[B, A]{p.Second, p.First}
}

func (p Pair[A, B]) String() string {
	return fmt.Sprintf("(%v, %v)", p.First, p.Second)
}

func show[T any](value T, ok bool) string {
	if !ok {
		return "none"
	}
	return fmt.Sprint(value)
}

func report[T any](name string, stack *Stack[T]) {
	popped := show(stack.Pop())
	top := show(stack.Peek())
	fmt.Printf("%s: popped %s, top is %s, %d left\n", name, popped, top, stack.Len())
}

func main() {
	var numbers Stack[int]
	numbers.Push(3)
	numbers.Push(1)
	numbers.Push(4)
	report("numbers", &numbers)

	var words Stack[string]
	words.Push("pear")
	words.Push("fig")
	report("words", &words)

	var empty Stack[float64]
	report("empty", &empty)

	fmt.Println("largest number:", show(Largest([]int{3, 1, 4, 1, 5})))
	fmt.Println("largest word:", show(Largest([]string{"pear", "fig", "apple"})))
	fmt.Println("largest of nothing:", show(Largest([]int{})))

	pair := Pair[int, string]{7, "seven"}
	fmt.Println(pair, "swapped is", pair.Swap())
}
