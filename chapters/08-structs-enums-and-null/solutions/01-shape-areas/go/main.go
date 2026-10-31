package main

import (
	"bufio"
	"fmt"
	"math"
	"os"
	"strconv"
	"strings"
)

// A shape is one of circle, rectangle, or triangle: an interface that only they
// implement.
type shape interface{ area() float64 }

type circle struct{ radius float64 }

type rectangle struct{ width, height float64 }

type triangle struct{ base, height float64 }

func (c circle) area() float64    { return math.Pi * c.radius * c.radius }
func (r rectangle) area() float64 { return r.width * r.height }
func (t triangle) area() float64  { return t.base * t.height / 2 }

func parseSize(text string) (float64, error) {
	value, err := strconv.ParseFloat(text, 64)
	if err != nil {
		return 0, fmt.Errorf("%q is not a number", text)
	}
	if value < 0 {
		return 0, fmt.Errorf("%q is negative", text)
	}
	return value, nil
}

// parse turns a shape name and its sizes into a shape, or returns an error.
func parse(name string, texts []string) (shape, error) {
	needed := map[string]int{"circle": 1, "rectangle": 2, "triangle": 2}[name]
	if needed == 0 {
		return nil, fmt.Errorf("unknown shape %q", name)
	}
	if len(texts) != needed {
		plural := "s"
		if needed == 1 {
			plural = ""
		}
		return nil, fmt.Errorf("%s needs %d size%s, got %d", name, needed, plural, len(texts))
	}
	sizes := make([]float64, needed)
	for i, text := range texts {
		size, err := parseSize(text)
		if err != nil {
			return nil, err
		}
		sizes[i] = size
	}
	switch name {
	case "circle":
		return circle{sizes[0]}, nil
	case "rectangle":
		return rectangle{sizes[0], sizes[1]}, nil
	default:
		return triangle{sizes[0], sizes[1]}, nil
	}
}

func main() {
	total := 0.0
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		words := strings.Fields(scanner.Text())
		if len(words) == 0 {
			continue
		}
		s, err := parse(words[0], words[1:])
		if err != nil {
			fmt.Println("error:", err)
			continue
		}
		total += s.area()
		fmt.Printf("%s: %.2f\n", words[0], s.area())
	}
	fmt.Printf("total: %.2f\n", total)
}
