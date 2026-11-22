package main

import (
	"bufio"
	"container/heap"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// A step is one of these three types. The interface has no methods, so the
// loop tells them apart with a type switch.
type step interface{}

type printStep struct{ text string }
type sleepStep struct{ duration int }
type waitStep struct{ task int } // the number of the task to wait for

type task struct {
	name       string
	steps      []step
	next       int // the index of the step to run when it runs again
	done       bool
	waitingFor int
	waiters    []int // in the order they began waiting
}

// The first word of a line, and the rest of it.
func split(line string) (string, string) {
	command, rest, _ := strings.Cut(strings.TrimSpace(line), " ")
	return command, strings.TrimSpace(rest)
}

// Returns the tasks, and prints an error for each line with a problem.
func parse(lines []string) []*task {
	// A wait can name a task defined further down, so the names come first.
	numbers := map[string]int{}
	for _, line := range lines {
		if command, name := split(line); command == "task" {
			if _, seen := numbers[name]; !seen {
				numbers[name] = len(numbers)
			}
		}
	}
	var tasks []*task
	for i, line := range lines {
		command, rest := split(line)
		errorf := func(format string, args ...any) {
			fmt.Printf("error: line %d: "+format+"\n", append([]any{i + 1}, args...)...)
		}
		switch {
		case command == "":
		case command == "task":
			tasks = append(tasks, &task{name: rest})
		case len(tasks) == 0:
			errorf("a step before any task")
		case command == "print":
			current := tasks[len(tasks)-1]
			current.steps = append(current.steps, printStep{rest})
		case command == "sleep":
			duration, err := strconv.Atoi(rest)
			if err != nil || strings.Trim(rest, "0123456789") != "" {
				errorf("sleep takes a whole number")
				continue
			}
			current := tasks[len(tasks)-1]
			current.steps = append(current.steps, sleepStep{duration})
		case command == "wait":
			other, ok := numbers[rest]
			if !ok {
				errorf("no task named %s", rest)
				continue
			}
			current := tasks[len(tasks)-1]
			current.steps = append(current.steps, waitStep{other})
		default:
			errorf("unknown step: %s", command)
		}
	}
	return tasks
}

// The sleeping tasks, as a heap. container/heap keeps the heap in a slice,
// through these five methods, and Less puts the task to wake first on top.
type sleeper struct{ wake, order, task int }
type sleepers []sleeper

func (s sleepers) Len() int { return len(s) }
func (s sleepers) Less(i, j int) bool {
	if s[i].wake != s[j].wake {
		return s[i].wake < s[j].wake
	}
	return s[i].order < s[j].order
}
func (s sleepers) Swap(i, j int) { s[i], s[j] = s[j], s[i] }
func (s *sleepers) Push(x any)   { *s = append(*s, x.(sleeper)) }
func (s *sleepers) Pop() any {
	old := *s
	last := old[len(old)-1]
	*s = old[:len(old)-1]
	return last
}

type loop struct {
	tasks    []*task
	time     int
	ready    []int // the tasks that can run now, in order
	sleeping sleepers
	sleeps   int
}

// Runs a task until it sleeps, waits, or ends.
func (l *loop) runUntilStopped(id int) {
	t := l.tasks[id]
	for t.next < len(t.steps) {
		s := t.steps[t.next]
		t.next++
		switch s := s.(type) {
		case printStep:
			fmt.Printf("[%d] %s: %s\n", l.time, t.name, s.text)
		case sleepStep:
			if s.duration == 0 {
				l.ready = append(l.ready, id)
			} else {
				heap.Push(&l.sleeping, sleeper{l.time + s.duration, l.sleeps, id})
				l.sleeps++
			}
			return
		case waitStep:
			if other := l.tasks[s.task]; !other.done {
				t.waitingFor = s.task
				other.waiters = append(other.waiters, id)
				return
			} // a wait for a task that has finished carries on
		}
	}
	t.done = true
	fmt.Printf("[%d] %s: done\n", l.time, t.name)
	l.ready = append(l.ready, t.waiters...)
}

func (l *loop) run() {
	for {
		for len(l.ready) > 0 {
			id := l.ready[0]
			l.ready = l.ready[1:]
			l.runUntilStopped(id)
		}
		if len(l.sleeping) == 0 {
			break
		}
		// Nothing can run now, so the clock jumps to the next wake.
		l.time = l.sleeping[0].wake
		for len(l.sleeping) > 0 && l.sleeping[0].wake == l.time {
			l.ready = append(l.ready, heap.Pop(&l.sleeping).(sleeper).task)
		}
	}
	var stuck []string
	for _, t := range l.tasks {
		if !t.done {
			stuck = append(stuck, t.name+" waits for "+l.tasks[t.waitingFor].name)
		}
	}
	if len(stuck) == 0 {
		fmt.Printf("[%d] all done\n", l.time)
	} else {
		fmt.Printf("[%d] deadlock: %s\n", l.time, strings.Join(stuck, ", "))
	}
}

func main() {
	var lines []string
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	l := &loop{tasks: parse(lines)}
	for id := range l.tasks {
		l.ready = append(l.ready, id)
	}
	l.run()
}
