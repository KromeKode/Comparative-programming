package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strings"
)

const (
	maxOpen  = 2
	maxLines = 8
)

type File struct {
	lines  []string
	locked bool
}

type Disk struct {
	files     map[string]*File
	openCount int
}

type Handle struct {
	disk *Disk
	name string
}

func (d *Disk) linesUsed() int {
	total := 0
	for _, f := range d.files {
		total += len(f.lines)
	}
	return total
}

// Open opens a file. When it returns no error, the caller must call Close,
// which is what defer is for.
func (d *Disk) Open(name string, writing bool) (*Handle, error) {
	if d.openCount == maxOpen {
		return nil, errors.New("too many open files")
	}
	file, found := d.files[name]
	if writing {
		if found && file.locked {
			return nil, fmt.Errorf("%q is read-only", name)
		}
		if !found {
			file = &File{}
			d.files[name] = file
		}
		file.lines = nil
	} else if !found {
		return nil, fmt.Errorf("no file %q", name)
	}
	d.openCount++
	fmt.Printf("open %s\n", name)
	return &Handle{d, name}, nil
}

func (h *Handle) Close() {
	h.disk.openCount--
	fmt.Printf("close %s\n", h.name)
}

// Lines returns a copy of the file's lines.
func (h *Handle) Lines() []string {
	return append([]string(nil), h.disk.files[h.name].lines...)
}

func (h *Handle) Write(line string) error {
	if h.disk.linesUsed() == maxLines {
		return errors.New("disk full")
	}
	file := h.disk.files[h.name]
	file.lines = append(file.lines, line)
	return nil
}

func lines(count int) string {
	if count == 1 {
		return "1 line"
	}
	return fmt.Sprintf("%d lines", count)
}

func writeFile(d *Disk, name string, words []string) error {
	target, err := d.Open(name, true)
	if err != nil {
		return err
	}
	defer target.Close()
	for _, word := range words {
		if err := target.Write(word); err != nil {
			return err
		}
	}
	return nil
}

func catFile(d *Disk, name string) error {
	source, err := d.Open(name, false)
	if err != nil {
		return err
	}
	defer source.Close()
	for _, line := range source.Lines() {
		fmt.Printf("  %s\n", line)
	}
	return nil
}

// copyFile returns how many lines it copied. The deferred calls run as it
// returns, last deferred first.
func copyFile(d *Disk, from, to string) (int, error) {
	if from == to {
		return 0, fmt.Errorf("cannot copy %q onto itself", from)
	}
	source, err := d.Open(from, false)
	if err != nil {
		return 0, err
	}
	defer source.Close()
	target, err := d.Open(to, true)
	if err != nil {
		return 0, err
	}
	defer target.Close()
	copied := 0
	for _, line := range source.Lines() {
		if err := target.Write(line); err != nil {
			return copied, err
		}
		copied++
	}
	return copied, nil
}

func lockFile(d *Disk, name string) error {
	file, found := d.files[name]
	if !found {
		return fmt.Errorf("no file %q", name)
	}
	file.locked = true
	fmt.Printf("locked %s\n", name)
	return nil
}

func takes(command string, want, got int) error {
	if got == want {
		return nil
	}
	noun := "names"
	if want == 1 {
		noun = "name"
	}
	return fmt.Errorf("%s takes %d %s, got %d", command, want, noun, got)
}

// The messages are printed here, after the command returned and its deferred
// Close calls have run.
func run(d *Disk, command string, args []string) error {
	switch command {
	case "write":
		if len(args) == 0 {
			return errors.New("write needs a file name")
		}
		if err := writeFile(d, args[0], args[1:]); err != nil {
			return err
		}
		fmt.Printf("wrote %s\n", lines(len(args)-1))
	case "cat":
		if err := takes(command, 1, len(args)); err != nil {
			return err
		}
		return catFile(d, args[0])
	case "copy":
		if err := takes(command, 2, len(args)); err != nil {
			return err
		}
		copied, err := copyFile(d, args[0], args[1])
		if err != nil {
			return err
		}
		fmt.Printf("copied %s\n", lines(copied))
	case "lock":
		if err := takes(command, 1, len(args)); err != nil {
			return err
		}
		return lockFile(d, args[0])
	default:
		return fmt.Errorf("unknown command %q", command)
	}
	return nil
}

func main() {
	disk := &Disk{files: map[string]*File{}}
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		parts := strings.Fields(scanner.Text())
		if len(parts) == 0 {
			continue
		}
		if err := run(disk, parts[0], parts[1:]); err != nil {
			fmt.Printf("error: %v\n", err)
		}
	}
	fmt.Printf("files still open: %d\n", disk.openCount)
}
