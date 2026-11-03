package main

import (
	"cmp"
	"fmt"
	"maps"
	"slices"
	"strings"
)

type Score struct {
	Name   string
	Points int
}

func yesNo(value bool) string {
	if value {
		return "yes"
	}
	return "no"
}

// Go's standard library has no map, filter, or reduce. Loops are the usual
// style, with the slices and maps packages for sorting, searching, and keys.
func main() {
	scores := []Score{{"ada", 91}, {"bob", 78}, {"cy", 85}, {"dee", 62}, {"eve", 99}, {"finn", 54}}

	var passed []string
	total := 0
	for _, score := range scores {
		if score.Points >= 70 {
			passed = append(passed, score.Name)
			total += score.Points
		}
	}
	fmt.Println("passed:", strings.Join(passed, ", "))
	fmt.Println("total of the passes:", total)

	byPoints := func(a, b Score) int { return cmp.Compare(a.Points, b.Points) }
	best := slices.MaxFunc(scores, byPoints)
	fmt.Printf("best: %s with %d\n", best.Name, best.Points)
	failed := slices.ContainsFunc(scores, func(s Score) bool { return s.Points < 70 })
	fmt.Println("everyone passed:", yesNo(!failed))
	high := slices.ContainsFunc(scores, func(s Score) bool { return s.Points > 95 })
	fmt.Println("anyone above 95:", yesNo(high))

	ranked := slices.Clone(scores)
	slices.SortFunc(ranked, func(a, b Score) int { return byPoints(b, a) })
	var top []string
	for i, score := range ranked[:3] {
		top = append(top, fmt.Sprintf("%d. %s %d", i+1, score.Name, score.Points))
	}
	fmt.Println("top three:", strings.Join(top, ", "))

	seen := map[string]bool{}
	for _, score := range scores {
		for _, letter := range strings.Split(score.Name, "") {
			seen[letter] = true
		}
	}
	// maps.Keys returns an iterator, and slices.Sorted collects it in order.
	fmt.Println("letters used:", strings.Join(slices.Sorted(maps.Keys(seen)), " "))
}
