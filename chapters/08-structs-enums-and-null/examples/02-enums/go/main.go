package main

import "fmt"

// Go has no enum keyword. A named integer type and iota, which counts up from 0
// inside a const block, do the same job.
type light int

const (
	red light = iota
	green
	yellow
)

var lights = []light{red, green, yellow}

// String gives each value a name when fmt prints it.
func (l light) String() string {
	return [...]string{"red", "green", "yellow"}[l]
}

func (l light) next() light {
	return (l + 1) % light(len(lights))
}

func (l light) meaning() string {
	switch l {
	case red:
		return "stop"
	case green:
		return "go"
	case yellow:
		return "slow down"
	}
	return "?" // a light can hold any int, so Go needs a fallback
}

func (l light) seconds() int {
	return [...]int{30, 25, 5}[l]
}

func parseLight(text string) (light, bool) {
	for _, l := range lights {
		if l.String() == text {
			return l, true
		}
	}
	return 0, false
}

func showParse(text string) {
	if l, ok := parseLight(text); ok {
		fmt.Printf("parse %q: %v\n", text, l)
	} else {
		fmt.Printf("parse %q: not a light\n", text)
	}
}

func main() {
	fmt.Print("lights:")
	for _, l := range lights {
		fmt.Print(" ", l)
	}
	fmt.Println()
	fmt.Println("yellow is number", int(yellow))

	l := red
	for range lights {
		fmt.Printf("after %v: %v\n", l, l.next())
		l = l.next()
	}
	fmt.Printf("red means %s, green means %s, yellow means %s\n",
		red.meaning(), green.meaning(), yellow.meaning())

	total := 0
	for _, each := range lights {
		total += each.seconds()
	}
	fmt.Printf("one cycle: %d seconds\n", total)

	showParse("green")
	showParse("blue")
}
