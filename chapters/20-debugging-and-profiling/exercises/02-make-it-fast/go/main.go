// Simulates a day of payments between bank accounts, and prints a summary. It gives
// the right answers, but slowly: ../README.md says what it prints, and how fast it
// has to be.
package main

import (
	"cmp"
	"fmt"
	"os"
	"slices"
)

const start = 1000 // what each account starts with

// A random is the Park-Miller generator, which every language's version uses, so that
// they all make up the same transfers from the same seed.
type random struct{ state int64 }

func (r *random) next() int64 {
	r.state = r.state * 48271 % 2147483647
	return r.state
}

type account struct {
	name    string
	balance int64
}

func accountName(i int64) string {
	return fmt.Sprintf("acct%06d", i)
}

// find gives the index of the account with this name.
func find(accounts []account, name string) int {
	for i, a := range accounts {
		if a.name == name {
			return i
		}
	}
	panic("no account called " + name)
}

func total(accounts []account) int64 {
	var sum int64
	for _, a := range accounts {
		sum += a.balance
	}
	return sum
}

func main() {
	var count, transfers, seed int64
	if _, err := fmt.Scan(&count, &transfers, &seed); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	accounts := make([]account, count)
	for i := range accounts {
		accounts[i] = account{accountName(int64(i)), start}
	}

	r := random{seed}
	var refused int64
	for range transfers {
		source := find(accounts, accountName(r.next()%count))
		target := find(accounts, accountName(r.next()%count))
		amount := r.next()%500 + 1
		if accounts[source].balance < amount {
			refused++
		} else {
			accounts[source].balance -= amount
			accounts[target].balance += amount
		}
		// Money is never made or lost, so the total stays the same.
		if total(accounts) != count*start {
			fmt.Fprintln(os.Stderr, "error: the total has changed")
			os.Exit(1)
		}
	}

	fmt.Printf("transfers: %d, refused: %d\n", transfers, refused)
	fmt.Printf("total: %d\n", total(accounts))
	slices.SortFunc(accounts, func(a, b account) int {
		return cmp.Or(cmp.Compare(b.balance, a.balance), cmp.Compare(a.name, b.name))
	})
	for _, a := range accounts[:min(5, len(accounts))] {
		fmt.Println(a.name, a.balance)
	}
}
