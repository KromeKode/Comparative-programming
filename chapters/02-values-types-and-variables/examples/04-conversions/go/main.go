package main

import (
	"fmt"
	"math"
	"strconv"
)

func main() {
	parsed, err := strconv.Atoi("42")
	if err != nil {
		panic(err)
	}
	fmt.Println(`"42" + 1 as a number:`, parsed+1)

	x := 3.99 // int(3.99) would not compile: Go refuses to truncate a constant
	fmt.Println("3.99 truncated:", int(x))
	fmt.Println("-3.99 truncated:", int(-x))
	fmt.Println("2.5 rounded:", math.Round(2.5)) // halfway rounds away from zero
	fmt.Println("-2.5 rounded:", math.Round(-2.5))

	seven, two := 7, 2
	fmt.Println("7 / 2 as integers:", seven/two)
	fmt.Println("7 / 2 as floats:", float64(seven)/float64(two)) // no automatic conversion
}
