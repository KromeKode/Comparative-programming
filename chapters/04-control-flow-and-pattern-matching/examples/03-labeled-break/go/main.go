package main

import "fmt"

var grid = [3][3]int{{3, 8, 1}, {9, 4, 7}, {2, 6, 5}}

func main() {
targets:
	for _, target := range []int{7, 10} {
		checked := 0
		for r, row := range grid {
			for c, value := range row {
				checked++
				if value == target {
					fmt.Printf("%d: row %d, column %d (checked %d cells)\n", target, r, c, checked)
					continue targets // leaves both inner loops at once
				}
			}
		}
		fmt.Printf("%d: not found (checked %d cells)\n", target, checked)
	}
}
