package main

import (
	"fmt"
	"maps"
	"slices"
)

// Go randomizes the order of a range over a map on purpose, so sort the keys.
func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func printStock(label string, stock map[string]int) {
	fmt.Print(label, ":")
	for _, key := range slices.Sorted(maps.Keys(stock)) {
		fmt.Printf(" %s=%d", key, stock[key])
	}
	fmt.Println()
}

func main() {
	stock := map[string]int{"apple": 3, "banana": 5}
	printStock("stock", stock)

	stock["cherry"] = 7
	printStock("add cherry", stock)

	fmt.Println("banana:", stock["banana"])
	// A missing key gives the zero value. The second result says whether it was there.
	count, ok := stock["durian"]
	if !ok {
		fmt.Printf("durian: %d (missing)\n", count)
	}

	stock["apple"] += 2
	printStock("apple += 2", stock)
	delete(stock, "banana")
	printStock("remove banana", stock)

	_, has := stock["cherry"]
	fmt.Println("has cherry:", yesNo(has))
	fmt.Println("size:", len(stock))
}
