package main

import (
	"fmt"
	"maps"
	"slices"
)

// Go has no set type. A map whose values are all true does the job.
type set map[int]bool

func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func printSet(label string, items set) {
	fmt.Print(label, ":")
	for _, n := range slices.Sorted(maps.Keys(items)) {
		fmt.Print(" ", n)
	}
	fmt.Println()
}

// A struct with comparable fields can be a map key, so it works like a tuple.
type point struct{ x, y int }

func main() {
	a := set{1: true, 2: true, 3: true, 4: true}
	b := set{3: true, 4: true, 5: true}
	printSet("a", a)
	printSet("b", b)

	union, intersection, difference := set{}, set{}, set{}
	for n := range a {
		union[n] = true
		if b[n] { // a missing key gives false
			intersection[n] = true
		} else {
			difference[n] = true
		}
	}
	for n := range b {
		union[n] = true
	}
	printSet("union", union)
	printSet("intersection", intersection)
	printSet("a minus b", difference)
	fmt.Println("3 in a:", yesNo(a[3]))

	unique := set{}
	for _, n := range []int{3, 1, 3, 2, 1} {
		unique[n] = true
	}
	printSet("unique of 3 1 3 2 1, sorted", unique)

	path := []point{{0, 0}, {1, 0}, {0, 0}, {1, 1}}
	visited := map[point]bool{}
	for _, p := range path {
		visited[p] = true
	}
	fmt.Printf("visited %d points, %d distinct\n", len(path), len(visited))
	fmt.Println("(1, 0) visited:", yesNo(visited[point{1, 0}]))
}
