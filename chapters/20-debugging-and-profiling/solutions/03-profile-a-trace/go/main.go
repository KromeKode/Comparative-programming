// Reads a trace of function calls and prints a flat profile: for each function, how
// many calls it got, its total time, and its self time. See ../README.md.
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

type function struct {
	calls, total, self int64
	running            int64 // how many of its calls are running now
	outermostStart     int64 // when the outermost of them started
}

type call struct {
	name      string
	start     int64
	inCallees int64 // time spent in the functions it called
}

func isTime(s string) bool {
	return len(s) > 0 && len(s) <= 12 && strings.Trim(s, "0123456789") == ""
}

func isName(s string) bool {
	for i := 0; i < len(s); i++ {
		if s[i] < '!' || s[i] > '~' {
			return false
		}
	}
	return true
}

func profile(scanner *bufio.Scanner) (map[string]*function, error) {
	functions := map[string]*function{}
	var stack []call
	var now int64
	for number := 1; scanner.Scan(); number++ {
		fields := strings.FieldsFunc(scanner.Text(), func(r rune) bool { return r == ' ' || r == '\t' })
		if len(fields) == 0 {
			continue
		}
		if len(fields) != 3 || !isTime(fields[0]) || !isName(fields[2]) ||
			(fields[1] != "enter" && fields[1] != "exit") {
			return nil, fmt.Errorf("line %d: expected TIME enter NAME or TIME exit NAME", number)
		}
		time, _ := strconv.ParseInt(fields[0], 10, 64)
		name := fields[2]
		if time < now {
			return nil, fmt.Errorf("line %d: time goes back from %d to %d", number, now, time)
		}
		now = time

		if fields[1] == "enter" {
			f := functions[name]
			if f == nil {
				f = &function{}
				functions[name] = f
			}
			f.calls++
			if f.running == 0 {
				f.outermostStart = time
			}
			f.running++
			stack = append(stack, call{name: name, start: time})
			continue
		}

		if len(stack) == 0 {
			return nil, fmt.Errorf("line %d: exit from %s, but nothing is running", number, name)
		}
		c := stack[len(stack)-1]
		if c.name != name {
			return nil, fmt.Errorf("line %d: exit from %s, but the innermost call is to %s",
				number, name, c.name)
		}
		stack = stack[:len(stack)-1]
		duration := time - c.start
		f := functions[name]
		f.self += duration - c.inCallees
		f.running--
		// A recursive call's time is already in the total of the call around it.
		if f.running == 0 {
			f.total += time - f.outermostStart
		}
		if len(stack) > 0 {
			stack[len(stack)-1].inCallees += duration
		}
	}
	if len(stack) > 0 {
		return nil, fmt.Errorf("end of trace: %s is still running", stack[len(stack)-1].name)
	}
	return functions, nil
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	scanner.Buffer(nil, 1<<20)
	functions, err := profile(scanner)
	if err != nil {
		fmt.Println("error:", err)
		return
	}
	if len(functions) == 0 {
		fmt.Println("no calls")
	}
	names := slices.SortedFunc(maps.Keys(functions), func(a, b string) int {
		return cmp.Or(cmp.Compare(functions[b].self, functions[a].self), cmp.Compare(a, b))
	})
	for _, name := range names {
		f := functions[name]
		fmt.Println(name, f.calls, f.total, f.self)
	}
}
