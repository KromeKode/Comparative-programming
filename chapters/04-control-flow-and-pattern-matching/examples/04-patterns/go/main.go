package main

import "fmt"

// Go has no pattern matching. A switch with no value tests conditions in order.
func classify(x, y int) string {
	switch {
	case x == 0 && y == 0:
		return "origin"
	case y == 0:
		return fmt.Sprintf("on the x axis at x = %d", x)
	case x == 0:
		return fmt.Sprintf("on the y axis at y = %d", y)
	case x == y:
		return fmt.Sprintf("on the diagonal at %d", x)
	default:
		return fmt.Sprintf("x = %d, y = %d", x, y)
	}
}

func main() {
	points := [][2]int{{0, 0}, {3, 0}, {0, -2}, {4, 4}, {-1, 5}}
	for _, p := range points {
		fmt.Printf("(%d, %d): %s\n", p[0], p[1], classify(p[0], p[1]))
	}
}
