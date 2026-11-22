// Exercise 18.2: build order. The order to build modules in, and what to rebuild.
package main

import (
	"bufio"
	"container/heap"
	"fmt"
	"os"
	"regexp"
	"slices"
	"strings"
)

var namePattern = regexp.MustCompile(`^[A-Za-z0-9_./-]+$`)

type module struct {
	line    int
	imports []string // as written, with repeats dropped
}

type query struct {
	line int
	name string
}

// names is a min-heap of strings: container/heap needs these five methods.
type names []string

func (h names) Len() int           { return len(h) }
func (h names) Less(i, j int) bool { return h[i] < h[j] }
func (h names) Swap(i, j int)      { h[i], h[j] = h[j], h[i] }
func (h *names) Push(x any)        { *h = append(*h, x.(string)) }
func (h *names) Pop() any {
	old := *h
	last := old[len(old)-1]
	*h = old[:len(old)-1]
	return last
}

func main() {
	modules := map[string]*module{}
	var defined []string // the names in the order they were defined
	var queries []query
	scanner := bufio.NewScanner(os.Stdin)
	for number := 1; scanner.Scan(); number++ {
		line := scanner.Text()
		if name, rest, found := strings.Cut(line, ":"); found {
			name = strings.TrimSpace(name)
			imports := strings.Fields(rest)
			badName := func(s string) bool { return !namePattern.MatchString(s) }
			if badName(name) || slices.ContainsFunc(imports, badName) {
				fmt.Printf("error: line %d: a module is NAME: IMPORTS\n", number)
				return
			}
			if earlier, ok := modules[name]; ok {
				fmt.Printf("error: line %d: %s is already defined on line %d\n",
					number, name, earlier.line)
				return
			}
			m := &module{line: number}
			for _, imp := range imports {
				if !slices.Contains(m.imports, imp) {
					m.imports = append(m.imports, imp)
				}
			}
			modules[name] = m
			defined = append(defined, name)
			continue
		}
		words := strings.Fields(line)
		switch {
		case len(words) == 0:
		case len(words) == 2 && words[0] == "rebuild":
			queries = append(queries, query{number, words[1]})
		default:
			fmt.Printf("error: line %d: expected NAME: IMPORTS or rebuild NAME\n", number)
			return
		}
	}

	users := map[string][]string{}
	waiting := map[string]int{}
	for _, name := range defined {
		m := modules[name]
		for _, imp := range m.imports {
			if _, ok := modules[imp]; !ok {
				fmt.Printf("error: line %d: %s imports %s, which isn't defined\n",
					m.line, name, imp)
				return
			}
			users[imp] = append(users[imp], name)
		}
		waiting[name] = len(m.imports)
	}

	// Kahn's algorithm: a module is ready once everything it imports is built, and
	// of the ready modules, the first in alphabetical order goes next.
	ready := &names{}
	for _, name := range defined {
		if waiting[name] == 0 {
			heap.Push(ready, name)
		}
	}
	var order []string
	for ready.Len() > 0 {
		name := heap.Pop(ready).(string)
		order = append(order, name)
		for _, user := range users[name] {
			waiting[user]--
			if waiting[user] == 0 {
				heap.Push(ready, user)
			}
		}
	}

	if len(order) < len(modules) {
		// Every module left over imports another one left over, so following first
		// imports from the first module left over must come back to a module.
		var left []string
		for _, name := range defined {
			if waiting[name] > 0 {
				left = append(left, name)
			}
		}
		path := []string{slices.Min(left)}
		seen := map[string]int{path[0]: 0}
		for {
			imports := modules[path[len(path)-1]].imports
			isLeft := func(s string) bool { return waiting[s] > 0 }
			next := imports[slices.IndexFunc(imports, isLeft)]
			if at, ok := seen[next]; ok {
				cycle := append(path[at:], next)
				fmt.Println("error: import cycle: " + strings.Join(cycle, " -> "))
				return
			}
			seen[next] = len(path)
			path = append(path, next)
		}
	}

	text := strings.Join(order, ", ")
	if text == "" {
		text = "nothing to build"
	}
	fmt.Println("build order: " + text)
	for _, q := range queries {
		if _, ok := modules[q.name]; !ok {
			fmt.Printf("error: line %d: no module called %s\n", q.line, q.name)
			continue
		}
		// Everything that imports the changed module, directly or not, comes after
		// it in the build order, so one pass through the order finds them all.
		affected := map[string]bool{q.name: true}
		rebuilt := []string{q.name}
		for _, name := range order {
			isAffected := func(s string) bool { return affected[s] }
			if !affected[name] && slices.ContainsFunc(modules[name].imports, isAffected) {
				affected[name] = true
				rebuilt = append(rebuilt, name)
			}
		}
		fmt.Printf("rebuild %s: %s\n", q.name, strings.Join(rebuilt, ", "))
	}
}
