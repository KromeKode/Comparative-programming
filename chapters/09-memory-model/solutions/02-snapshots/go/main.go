package main

import (
	"bufio"
	"fmt"
	"math"
	"os"
	"slices"
	"strconv"
	"strings"
	"unicode/utf8"
)

// Each row is a slice, and a slice header shares its array when copied.
type grid [][]rune

// clone copies every row. slices.Clone(g) alone would copy only the row
// headers, and the snapshot would share its rows with the grid.
func (g grid) clone() grid {
	out := make(grid, len(g))
	for i, row := range g {
		out[i] = slices.Clone(row)
	}
	return out
}

// parseNumber reads a number made of digits. One too big for an int becomes
// math.MaxInt, which is outside any grid.
func parseNumber(text string) (int, bool) {
	for _, c := range text {
		if c < '0' || c > '9' {
			fmt.Printf("error: %q is not a number\n", text)
			return 0, false
		}
	}
	value, err := strconv.Atoi(text)
	if err != nil {
		value = math.MaxInt
	}
	return value, true
}

func oneCharacter(text string) (rune, bool) {
	if utf8.RuneCountInString(text) != 1 {
		fmt.Printf("error: %q is not one character\n", text)
		return 0, false
	}
	c, _ := utf8.DecodeRuneInString(text)
	return c, true
}

func takes(command string, want, got int) bool {
	if got != want {
		plural := "s"
		if want == 1 {
			plural = ""
		}
		fmt.Printf("error: %s takes %d value%s, got %d\n", command, want, plural, got)
	}
	return got == want
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	if !scanner.Scan() {
		return
	}
	var width, height int
	fmt.Sscan(scanner.Text(), &width, &height)
	g := make(grid, height)
	for y := range g {
		g[y] = []rune(strings.Repeat(".", width))
	}
	var saved grid // nil until the first save

	for scanner.Scan() {
		parts := strings.Fields(scanner.Text())
		if len(parts) == 0 {
			continue
		}
		command, values := parts[0], parts[1:]
		switch command {
		case "paint":
			if !takes(command, 3, len(values)) {
				continue
			}
			x, ok := parseNumber(values[0])
			if !ok {
				continue
			}
			y, ok := parseNumber(values[1])
			if !ok {
				continue
			}
			if x < 1 || x > width || y < 1 || y > height {
				fmt.Printf("error: (%s, %s) is outside the %d by %d grid\n",
					values[0], values[1], width, height)
				continue
			}
			if c, ok := oneCharacter(values[2]); ok {
				g[y-1][x-1] = c
			}
		case "fill":
			if !takes(command, 1, len(values)) {
				continue
			}
			if c, ok := oneCharacter(values[0]); ok {
				for _, row := range g {
					for x := range row {
						row[x] = c
					}
				}
			}
		case "save":
			if takes(command, 0, len(values)) {
				saved = g.clone()
			}
		case "restore":
			if !takes(command, 0, len(values)) {
				continue
			}
			if saved == nil {
				fmt.Println("error: nothing saved")
			} else {
				g = saved.clone()
			}
		case "show":
			if takes(command, 0, len(values)) {
				for _, row := range g {
					fmt.Println(string(row))
				}
				fmt.Println()
			}
		default:
			fmt.Printf("error: unknown command %q\n", command)
		}
	}
}
