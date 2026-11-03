package main

import (
	"bufio"
	"cmp"
	"fmt"
	"maps"
	"os"
	"slices"
	"strconv"
	"strings"
)

type Grade struct {
	Name, Subject string
	Score         int
}

// Go has no group-by, so this is the usual loop into a map of slices.
func groupBy(grades []Grade, key func(Grade) string) map[string][]Grade {
	groups := map[string][]Grade{}
	for _, grade := range grades {
		groups[key(grade)] = append(groups[key(grade)], grade)
	}
	return groups
}

func total(grades []Grade) int {
	sum := 0
	for _, grade := range grades {
		sum += grade.Score
	}
	return sum
}

func report(grades []Grade) []string {
	lines := []string{"subjects:"}
	subjects := groupBy(grades, func(g Grade) string { return g.Subject })
	// A map's order is random, so the keys are sorted before use.
	for _, subject := range slices.Sorted(maps.Keys(subjects)) {
		group := subjects[subject]
		best := slices.MaxFunc(group, func(a, b Grade) int { return cmp.Compare(a.Score, b.Score) })
		var names []string
		for _, grade := range group {
			if grade.Score == best.Score {
				names = append(names, grade.Name)
			}
		}
		slices.Sort(names)
		average := total(group) / len(group)
		lines = append(lines, fmt.Sprintf("  %s: average %d, best %d (%s)",
			subject, average, best.Score, strings.Join(names, ", ")))
	}

	students := groupBy(grades, func(g Grade) string { return g.Name })
	names := slices.Collect(maps.Keys(students))
	// cmp.Or picks the first comparison that isn't a tie: total, then name.
	slices.SortFunc(names, func(a, b string) int {
		return cmp.Or(cmp.Compare(total(students[b]), total(students[a])), cmp.Compare(a, b))
	})
	lines = append(lines, "ranking:")
	for i, name := range names {
		lines = append(lines, fmt.Sprintf("  %d. %s %d", i+1, name, total(students[name])))
	}

	var passed []string
	for _, name := range slices.Sorted(maps.Keys(students)) {
		failed := slices.ContainsFunc(students[name], func(g Grade) bool { return g.Score < 50 })
		if !failed {
			passed = append(passed, name)
		}
	}
	if len(passed) == 0 {
		passed = []string{"nobody"}
	}
	return append(lines, "passed everything: "+strings.Join(passed, ", "))
}

func main() {
	var grades []Grade
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		if words := strings.Fields(scanner.Text()); len(words) > 0 {
			score, _ := strconv.Atoi(words[2])
			grades = append(grades, Grade{words[0], words[1], score})
		}
	}
	fmt.Println(strings.Join(report(grades), "\n"))
}
