// Keeps a to-do list in todo.json, in the current folder: adds tasks, lists them, marks
// them done, and removes them. ../README.md says what it should do.
package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/fs"
	"os"
	"regexp"
	"slices"
	"strconv"
	"strings"
	"syscall"
)

const file = "todo.json"
const version = 1
const usage = `usage: todo add TEXT...
       todo list
       todo done ID
       todo remove ID`

var digits = regexp.MustCompile(`^[0-9]+$`)

// Task is a task as the program keeps it. The tags give the fields' names in the
// file, and json writes the fields in this order.
type Task struct {
	ID   int    `json:"id"`
	Text string `json:"text"`
	Done bool   `json:"done"`
}

// list is what todo.json holds.
type list struct {
	Version int    `json:"version"`
	Tasks   []Task `json:"tasks"`
}

// fileTask and fileList are what todo.json holds, as read. Unmarshal leaves a field that
// the JSON doesn't have as it was, so that without the pointers, which stay nil, a
// missing "done" would be false.
type fileTask struct {
	ID   *int    `json:"id"`
	Text *string `json:"text"`
	Done *bool   `json:"done"`
}

type fileList struct {
	Version *int        `json:"version"`
	Tasks   *[]fileTask `json:"tasks"`
}

// problem is a problem with todo.json, which stops the program with status 2.
type problem string

func (p problem) Error() string { return string(p) }

// message is what strerror says for the errors the README names. Go's own messages
// for them start with a small letter, such as "open todo.json: is a directory".
func message(err error) string {
	switch {
	case errors.Is(err, fs.ErrNotExist):
		return "No such file or directory"
	case errors.Is(err, syscall.EISDIR):
		return "Is a directory"
	}
	return err.Error()
}

// readTasks returns the tasks in todo.json, or none if there's no todo.json.
func readTasks() ([]Task, error) {
	data, err := os.ReadFile(file)
	if errors.Is(err, fs.ErrNotExist) {
		return []Task{}, nil // empty, not nil: json writes a nil slice as null
	} else if err != nil {
		return nil, problem(message(err))
	}
	if !json.Valid(data) {
		return nil, problem("not valid JSON")
	}
	// A later version of todo may keep anything else in the file, so the version comes
	// first. Unmarshal fails unless the JSON is an object.
	var head map[string]any
	if json.Unmarshal(data, &head) == nil {
		if v, ok := head["version"].(float64); ok && v != version {
			return nil, problem(fmt.Sprintf("unknown version %v", v))
		}
	}
	notList := problem("not a to-do list")
	var read fileList
	decoder := json.NewDecoder(bytes.NewReader(data))
	decoder.DisallowUnknownFields() // a key that no field has is an error, not ignored
	if decoder.Decode(&read) != nil || read.Version == nil || read.Tasks == nil {
		return nil, notList
	}
	tasks := []Task{}
	seen := map[int]bool{}
	for _, task := range *read.Tasks {
		if task.ID == nil || task.Text == nil || task.Done == nil || *task.ID < 1 || seen[*task.ID] {
			return nil, notList
		}
		seen[*task.ID] = true
		tasks = append(tasks, Task{*task.ID, *task.Text, *task.Done})
	}
	return tasks, nil
}

// writeTasks writes the tasks to todo.json. It writes a new file and renames it over
// the old one, which replaces it in one step, so that a crash leaves the old list or
// the new one, and never half of one.
func writeTasks(tasks []Task) error {
	var text bytes.Buffer
	encoder := json.NewEncoder(&text)
	encoder.SetIndent("", "  ")
	encoder.SetEscapeHTML(false) // write & as it is, not as \u0026
	// Encode adds a "\n" after the JSON.
	if err := encoder.Encode(list{version, tasks}); err != nil {
		return err
	}
	temporary := file + ".tmp"
	out, err := os.Create(temporary)
	if err == nil {
		_, err = out.Write(text.Bytes())
		if err == nil {
			err = out.Sync() // onto the disk before the rename
		}
		if closeErr := out.Close(); err == nil {
			err = closeErr
		}
		if err == nil {
			err = os.Rename(temporary, file)
		}
		if err != nil {
			os.Remove(temporary)
		}
	}
	if err != nil {
		return problem(message(err))
	}
	return nil
}

// run carries out a command whose arguments have been checked, and returns the status.
func run(command string, args []string) (int, error) {
	tasks, err := readTasks()
	if err != nil {
		return 2, err
	}
	switch command {
	case "add":
		id := 1
		for _, task := range tasks {
			id = max(id, task.ID+1)
		}
		added := Task{id, strings.Join(args, " "), false}
		if err := writeTasks(append(tasks, added)); err != nil {
			return 2, err
		}
		fmt.Printf("added %d: %s\n", added.ID, added.Text)
		return 0, nil
	case "list":
		for _, task := range tasks {
			mark := " "
			if task.Done {
				mark = "x"
			}
			fmt.Printf("%d [%s] %s\n", task.ID, mark, task.Text)
		}
		return 0, nil
	}
	id, _ := strconv.Atoi(args[0]) // args[0] is digits
	i := slices.IndexFunc(tasks, func(task Task) bool { return task.ID == id })
	if i < 0 {
		fmt.Fprintf(os.Stderr, "todo: no task %s\n", args[0])
		return 1, nil
	}
	found, verb := tasks[i], "removed"
	if command == "done" {
		tasks[i].Done = true
		verb = "done"
	} else {
		tasks = slices.Delete(tasks, i, i+1)
	}
	if err := writeTasks(tasks); err != nil {
		return 2, err
	}
	fmt.Printf("%s %d: %s\n", verb, found.ID, found.Text)
	return 0, nil
}

func main() {
	command, args := "", os.Args[1:]
	if len(args) > 0 {
		command, args = args[0], args[1:]
	}
	// The command line is checked before todo.json is read.
	ok := (command == "add" && len(args) > 0) ||
		(command == "list" && len(args) == 0) ||
		((command == "done" || command == "remove") && len(args) == 1 && digits.MatchString(args[0]))
	if !ok {
		fmt.Fprintln(os.Stderr, usage)
		os.Exit(2)
	}
	status, err := run(command, args)
	if err != nil {
		fmt.Fprintf(os.Stderr, "todo: %s: %s\n", file, err)
	}
	os.Exit(status)
}
