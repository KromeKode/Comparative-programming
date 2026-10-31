package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"slices"
	"strconv"
	"strings"
)

type Document struct {
	lines []string
}

func (d *Document) checkLine(number int) error {
	if number < 1 || number > len(d.lines) {
		return fmt.Errorf("no line %d", number)
	}
	return nil
}

// Command is an edit that knows how to do itself and how to undo itself.
type Command interface {
	Apply(doc *Document) error
	Undo(doc *Document)
	Describe() string
}

type Add struct {
	text string
}

func (a *Add) Apply(doc *Document) error {
	doc.lines = append(doc.lines, a.text)
	return nil
}

func (a *Add) Undo(doc *Document) {
	doc.lines = doc.lines[:len(doc.lines)-1]
}

func (a *Add) Describe() string {
	return fmt.Sprintf(`add "%s"`, a.text)
}

type Delete struct {
	number  int
	removed string
}

func (d *Delete) Apply(doc *Document) error {
	if err := doc.checkLine(d.number); err != nil {
		return err
	}
	d.removed = doc.lines[d.number-1]
	doc.lines = slices.Delete(doc.lines, d.number-1, d.number)
	return nil
}

func (d *Delete) Undo(doc *Document) {
	doc.lines = slices.Insert(doc.lines, d.number-1, d.removed)
}

func (d *Delete) Describe() string {
	return fmt.Sprintf("delete line %d", d.number)
}

type Replace struct {
	number    int
	text, old string
}

func (r *Replace) Apply(doc *Document) error {
	if err := doc.checkLine(r.number); err != nil {
		return err
	}
	r.old = doc.lines[r.number-1]
	doc.lines[r.number-1] = r.text
	return nil
}

func (r *Replace) Undo(doc *Document) {
	doc.lines[r.number-1] = r.old
}

func (r *Replace) Describe() string {
	return fmt.Sprintf(`replace line %d with "%s"`, r.number, r.text)
}

type Editor struct {
	doc    Document
	done   []Command
	undone []Command
}

func (e *Editor) Run(command Command) error {
	if err := command.Apply(&e.doc); err != nil {
		return err // nothing is recorded
	}
	e.done = append(e.done, command)
	e.undone = nil
	return nil
}

func (e *Editor) Undo() error {
	if len(e.done) == 0 {
		return errors.New("nothing to undo")
	}
	command := e.done[len(e.done)-1]
	e.done = e.done[:len(e.done)-1]
	command.Undo(&e.doc)
	e.undone = append(e.undone, command)
	fmt.Println("undo:", command.Describe())
	return nil
}

func (e *Editor) Redo() error {
	if len(e.undone) == 0 {
		return errors.New("nothing to redo")
	}
	command := e.undone[len(e.undone)-1]
	e.undone = e.undone[:len(e.undone)-1]
	if err := command.Apply(&e.doc); err != nil {
		return err
	}
	e.done = append(e.done, command)
	fmt.Println("redo:", command.Describe())
	return nil
}

func (e *Editor) Show() {
	if len(e.doc.lines) == 0 {
		fmt.Println("(empty)")
	}
	for i, line := range e.doc.lines {
		fmt.Printf("%d: %s\n", i+1, line)
	}
}

func run(editor *Editor, words []string) error {
	if len(words) == 0 {
		return nil
	}
	command, args := words[0], words[1:]
	switch command {
	case "add":
		return editor.Run(&Add{text: strings.Join(args, " ")})
	case "delete", "replace":
		if len(args) == 0 {
			return nil
		}
		number, err := strconv.Atoi(args[0])
		if err != nil {
			return fmt.Errorf("not a line number: %s", args[0])
		}
		if command == "delete" {
			return editor.Run(&Delete{number: number})
		}
		return editor.Run(&Replace{number: number, text: strings.Join(args[1:], " ")})
	case "undo":
		return editor.Undo()
	case "redo":
		return editor.Redo()
	case "show":
		editor.Show()
	}
	return nil
}

func main() {
	editor := &Editor{}
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		if err := run(editor, strings.Fields(scanner.Text())); err != nil {
			fmt.Println("error:", err)
		}
	}
}
