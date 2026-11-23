// Prints the lines that contain a pattern, from each file named on the command line, or
// from stdin. ../README.md says what it should print.
package main

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"io"
	"io/fs"
	"os"
	"strconv"
	"strings"
	"syscall"
)

const usage = "usage: search [-icnv] PATTERN [FILE...]"
const stdinName = "(standard input)"

func usageError(message string) {
	fmt.Fprintf(os.Stderr, "search: %s\n%s\n", message, usage)
	os.Exit(2)
}

// lower returns a copy of b with A to Z made a to z, and every other byte as it was.
// bytes.ToLower would treat b as UTF-8, and change letters such as Ä too.
func lower(b []byte) []byte {
	out := make([]byte, len(b))
	for i, c := range b {
		if 'A' <= c && c <= 'Z' {
			c += 'a' - 'A'
		}
		out[i] = c
	}
	return out
}

// message is what strerror says for the errors the README names. Go's own messages
// for them start with a small letter, such as "open notes.txt: no such file or directory".
func message(err error) string {
	switch {
	case errors.Is(err, fs.ErrNotExist):
		return "No such file or directory"
	case errors.Is(err, syscall.EISDIR):
		return "Is a directory"
	}
	return err.Error()
}

// search writes the lines of data that are selected, and returns how many there are.
func search(out *bufio.Writer, data []byte, name string, pattern []byte, options string) int {
	lines := bytes.Split(data, []byte("\n"))
	if len(lines[len(lines)-1]) == 0 { // the data ended with a newline, or was empty
		lines = lines[:len(lines)-1]
	}
	has := func(option string) bool { return strings.Contains(options, option) }
	if has("i") {
		pattern = lower(pattern)
	}
	prefix := ""
	if name != "" {
		prefix = name + ":"
	}
	count := 0
	for i, line := range lines {
		text := line
		if has("i") {
			text = lower(line)
		}
		if bytes.Contains(text, pattern) == has("v") {
			continue
		}
		count++
		if !has("c") {
			out.WriteString(prefix)
			if has("n") {
				out.WriteString(strconv.Itoa(i+1) + ":")
			}
			out.Write(line)
			out.WriteByte('\n')
		}
	}
	if has("c") {
		fmt.Fprintf(out, "%s%d\n", prefix, count)
	}
	return count
}

func main() {
	args := os.Args[1:]
	options := ""
	// Options come first; the first argument that isn't one is the pattern.
	for len(args) > 0 && strings.HasPrefix(args[0], "-") && args[0] != "-" {
		arg := args[0]
		args = args[1:]
		if arg == "--" {
			break
		}
		for _, letter := range arg[1:] {
			if !strings.ContainsRune("icnv", letter) {
				usageError("unknown option -" + string(letter))
			}
			options += string(letter)
		}
	}
	if len(args) == 0 {
		usageError("no pattern")
	}
	pattern := []byte(args[0])
	files := args[1:]
	if len(files) == 0 {
		files = []string{"-"}
	}

	// os.Stdout has no buffer of its own: each write is a call to the system.
	out := bufio.NewWriter(os.Stdout)
	selected, status := 0, 0
	for _, file := range files {
		var data []byte
		var err error
		if file == "-" {
			data, err = io.ReadAll(os.Stdin)
		} else {
			data, err = os.ReadFile(file)
		}
		if err != nil {
			out.Flush() // what came before, before the error
			fmt.Fprintf(os.Stderr, "search: %s: %s\n", file, message(err))
			status = 2
			continue
		}
		name := ""
		if len(files) > 1 {
			name = file
			if file == "-" {
				name = stdinName
			}
		}
		selected += search(out, data, name, pattern, options)
	}
	if status == 0 && selected == 0 {
		status = 1
	}
	out.Flush() // before os.Exit, which ends the program at once
	os.Exit(status)
}
