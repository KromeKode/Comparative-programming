package main

import "fmt"

// A struct type. Assigning a struct copies every field.
type point struct {
	x, y int
}

// A method: the receiver (p point) comes before the name.
func (p point) stepsFromOrigin() int {
	return abs(p.x) + abs(p.y)
}

// String makes fmt print a point as (x, y).
func (p point) String() string {
	return fmt.Sprintf("(%d, %d)", p.x, p.y)
}

func abs(n int) int {
	if n < 0 {
		return -n
	}
	return n
}

func yesNo(b bool) string {
	if b {
		return "yes"
	}
	return "no"
}

func main() {
	p := point{x: 1, y: 2}
	fmt.Println("p =", p)
	fmt.Println("p.x + p.y =", p.x+p.y)

	q := p // a copy, so changing q leaves p alone
	q.x = 5
	fmt.Printf("q = p with x = 5: q = %v, p = %v\n", q, p)

	r := p
	r.y = 9
	fmt.Printf("r = p, then r.y = 9: r = %v, p = %v\n", r, p)

	// Structs compare with == when all their fields can.
	a := point{3, -4}
	b := point{3, -4}
	fmt.Printf("a = %v, b = %v, a == b: %s\n", a, b, yesNo(a == b))
	fmt.Println("steps from (0, 0) to a:", a.stepsFromOrigin())
}
