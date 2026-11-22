// Exercise 18.1: a stock module. This file is given: write stock/stock.go so that
// it builds. It reads the commands, and leaves keeping count to the package.
package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"

	"example.com/stockroom/stock"
)

// parseAmount reads an amount: a whole number from 1 to 999,999,999,999,999, which
// is digits, with any zeros in front, and at most 15 more after them.
func parseAmount(word string) (int64, bool) {
	digits := strings.TrimLeft(word, "0")
	if digits == "" || len(digits) > 15 || strings.Trim(word, "0123456789") != "" {
		return 0, false
	}
	amount, err := strconv.ParseInt(digits, 10, 64)
	return amount, err == nil
}

func main() {
	store := stock.New()
	scanner := bufio.NewScanner(os.Stdin)
	for number := 1; scanner.Scan(); number++ {
		words := strings.Fields(scanner.Text())
		if len(words) == 0 {
			continue
		}
		amount, ok := int64(0), false
		if len(words) == 3 {
			amount, ok = parseAmount(words[2])
		}
		switch {
		case ok && words[0] == "add":
			store.Add(words[1], amount)
			fmt.Printf("added %d %s, now %d\n", amount, words[1], store.Count(words[1]))
		case ok && words[0] == "take":
			if store.Take(words[1], amount) {
				fmt.Printf("took %d %s, now %d\n", amount, words[1], store.Count(words[1]))
			} else {
				fmt.Printf("error: line %d: only %d %s in stock\n",
					number, store.Count(words[1]), words[1])
			}
		case len(words) == 2 && words[0] == "count":
			fmt.Printf("%s: %d\n", words[1], store.Count(words[1]))
		case len(words) == 1 && words[0] == "list":
			var items []string
			for _, item := range store.Items() {
				items = append(items, fmt.Sprintf("%s %d", item.Name, item.Count))
			}
			text := strings.Join(items, ", ")
			if text == "" {
				text = "empty"
			}
			fmt.Println("stock: " + text)
		default:
			fmt.Printf("error: line %d: expected add, take, count, or list\n", number)
		}
	}
}
