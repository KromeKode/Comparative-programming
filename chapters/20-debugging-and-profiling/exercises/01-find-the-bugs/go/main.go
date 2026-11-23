// Prints a report of each student's scores. ../README.md says what it should print.
package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// roundedAverage gives total / count, rounded to the nearest whole number, with
// halves up.
func roundedAverage(total, count int) int {
	return (2*total + count) / (2 * count)
}

func main() {
	var names []string // in the order they first appear
	scores := map[string][]int{}
	// The input is the file named on the command line, if there is one, so that a
	// debugger can keep the terminal to itself.
	input := os.Stdin
	if len(os.Args) > 1 {
		file, err := os.Open(os.Args[1])
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		defer file.Close()
		input = file
	}
	scanner := bufio.NewScanner(input)
	for scanner.Scan() {
		fields := strings.Fields(scanner.Text())
		if len(fields) == 0 {
			continue
		}
		name := fields[0]
		var marks []int
		for _, field := range fields[1:] {
			mark, err := strconv.Atoi(field)
			if err != nil {
				fmt.Fprintln(os.Stderr, err)
				os.Exit(1)
			}
			marks = append(marks, mark)
		}
		if _, ok := scores[name]; !ok {
			names = append(names, name)
		}
		scores[name] = marks
	}

	bestName, bestAverage := "-", -1
	classTotal, classCount := 0, 0
	total := 0
	for _, name := range names {
		marks := scores[name]
		for _, mark := range marks {
			total += mark
		}
		classTotal += total
		classCount += len(marks)
		average := roundedAverage(total, len(marks))
		fmt.Println(name, average)
		if average >= bestAverage {
			bestName, bestAverage = name, average
		}
	}

	if classCount == 0 {
		fmt.Println("class average: -")
	} else {
		fmt.Println("class average:", roundedAverage(classTotal, classCount))
	}
	fmt.Println("top:", bestName)
}
