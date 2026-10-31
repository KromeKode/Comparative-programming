package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

// Clockwise order, so turning right adds 1 and turning left subtracts 1.
type direction int

const (
	north direction = iota
	east
	south
	west
	directionCount = 4
)

func (d direction) String() string {
	return [...]string{"north", "east", "south", "west"}[d]
}

func (d direction) turn(steps int) direction {
	// Adding directionCount first keeps the left operand of % from going negative.
	return (d + direction(steps) + directionCount) % directionCount
}

// step returns how far one step forward moves in x and y.
func (d direction) step() (int, int) {
	switch d {
	case east:
		return 1, 0
	case south:
		return 0, -1
	case west:
		return -1, 0
	default:
		return 0, 1
	}
}

// The zero value of a robot is at (0, 0), facing north, because north is 0.
type robot struct {
	x, y   int
	facing direction
}

func (r *robot) run(command rune) {
	switch command {
	case 'L':
		r.facing = r.facing.turn(-1)
	case 'R':
		r.facing = r.facing.turn(1)
	case 'F':
		dx, dy := r.facing.step()
		r.x += dx
		r.y += dy
	}
}

func isNotCommand(c rune) bool {
	return !strings.ContainsRune("LRF", c)
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := scanner.Text()
		if line == "" {
			continue
		}
		if bad := strings.IndexFunc(line, isNotCommand); bad >= 0 {
			fmt.Printf("error: unknown command '%c' at position %d\n", line[bad], bad+1)
			continue
		}
		var r robot
		for _, command := range line {
			r.run(command)
		}
		fmt.Println(r.x, r.y, r.facing)
	}
}
