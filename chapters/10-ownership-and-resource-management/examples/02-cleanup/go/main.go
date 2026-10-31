package main

import (
	"errors"
	"fmt"
)

// File is a pretend file that prints when it opens and closes. A real
// *os.File's Close also returns an error; see section 10.9.
type File struct {
	name string
}

func Open(name string, fails bool) (*File, error) {
	if fails {
		return nil, fmt.Errorf("cannot open %s", name)
	}
	fmt.Printf("  open %s\n", name)
	return &File{name}, nil
}

func (f *File) Close() {
	fmt.Printf("  close %s\n", f.name)
}

// Each defer runs when the function returns, last deferred first.
func runJob(outputFails, copyFails bool) error {
	input, err := Open("input", false)
	if err != nil {
		return err
	}
	defer input.Close()

	output, err := Open("output", outputFails)
	if err != nil {
		return err
	}
	defer output.Close()

	if copyFails {
		return errors.New("disk full")
	}
	fmt.Println("  copy input to output")
	return nil
}

func main() {
	jobs := []struct {
		title       string
		outputFails bool
		copyFails   bool
	}{
		{"everything works", false, false},
		{"opening output fails", true, false},
		{"copying fails", false, true},
	}
	for i, job := range jobs {
		number := i + 1
		fmt.Printf("job %d: %s\n", number, job.title)
		if err := runJob(job.outputFails, job.copyFails); err != nil {
			fmt.Printf("  job %d failed: %v\n", number, err)
		} else {
			fmt.Printf("  job %d done\n", number)
		}
	}
}
