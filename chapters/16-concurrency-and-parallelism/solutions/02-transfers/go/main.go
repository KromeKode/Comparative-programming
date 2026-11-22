package main

import (
	"bufio"
	"fmt"
	"os"
	"runtime"
	"strconv"
	"strings"
	"sync"
)

type transfer struct {
	source, target, amount int
}

// One lock for each account, so that transfers between different accounts
// can run at the same time. Each lock guards the balance next to it.
type account struct {
	sync.Mutex
	balance int
}

func plural(count int, noun string) string {
	if count == 1 {
		return fmt.Sprintf("%d %s", count, noun)
	}
	return fmt.Sprintf("%d %ss", count, noun)
}

// The whole numbers on a line, and false if a word isn't one.
func numbers(line string) ([]int, bool) {
	var result []int
	for _, word := range strings.Fields(line) {
		if strings.Trim(word, "0123456789") != "" {
			return nil, false
		}
		n, _ := strconv.Atoi(word)
		result = append(result, n)
	}
	return result, true
}

func move(accounts []account, t transfer) {
	if t.source == t.target {
		return // locking one Mutex twice waits forever
	}
	// Every worker takes the two locks in the same order, lower account
	// first, so no two workers can each hold the lock the other wants.
	low, high := min(t.source, t.target), max(t.source, t.target)
	accounts[low].Lock()
	defer accounts[low].Unlock()
	accounts[high].Lock()
	defer accounts[high].Unlock()
	sourceBalance := accounts[t.source].balance
	targetBalance := accounts[t.target].balance
	// Lets another goroutine run between the read and the write, as it could
	// while a real bank waits on its database.
	runtime.Gosched()
	accounts[t.source].balance = sourceBalance - t.amount
	accounts[t.target].balance = targetBalance + t.amount
}

func main() {
	var lines []string
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	var first, second []int
	ok := len(lines) > 0
	if ok {
		first, ok = numbers(lines[0])
	}
	if !ok || len(first) != 1 || first[0] < 1 || first[0] > 16 {
		fmt.Println("error: line 1 must be the number of workers, from 1 to 16")
		return
	}
	workers := first[0]
	ok = len(lines) > 1
	if ok {
		second, ok = numbers(lines[1])
	}
	if !ok || len(second) == 0 {
		fmt.Println("error: line 2 must be the starting balances")
		return
	}

	var transfers []transfer
	for i := 2; i < len(lines); i++ {
		words, ok := numbers(lines[i])
		switch {
		case ok && len(words) == 0:
		case !ok || len(words) != 3:
			fmt.Printf("error: line %d: a transfer is three numbers: from, to, and amount\n", i+1)
		case words[0] >= len(second):
			fmt.Printf("error: line %d: no account %d\n", i+1, words[0])
		case words[1] >= len(second):
			fmt.Printf("error: line %d: no account %d\n", i+1, words[1])
		default:
			transfers = append(transfers, transfer{words[0], words[1], words[2]})
		}
	}

	accounts := make([]account, len(second))
	for i, balance := range second {
		accounts[i].balance = balance
	}
	// Worker i applies transfers i, i + workers, i + 2 * workers, and so on.
	var wg sync.WaitGroup
	for i := range workers {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for j := i; j < len(transfers); j += workers {
				move(accounts, transfers[j])
			}
		}()
	}
	wg.Wait()

	fmt.Printf("%s, %s, %s\n", plural(workers, "worker"), plural(len(accounts), "account"),
		plural(len(transfers), "transfer"))
	for i := range workers {
		share, moved := 0, 0
		for j := i; j < len(transfers); j += workers {
			share++
			moved += transfers[j].amount
		}
		fmt.Printf("worker %d: %s, %d moved\n", i, plural(share, "transfer"), moved)
	}
	total := 0
	for i := range accounts {
		fmt.Printf("account %d: %d\n", i, accounts[i].balance)
		total += accounts[i].balance
	}
	fmt.Printf("total: %d\n", total)
}
