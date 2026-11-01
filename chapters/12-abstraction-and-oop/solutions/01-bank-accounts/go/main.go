package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

const (
	overdraftLimit     = 100
	overdraftFee       = 5
	savingsWithdrawals = 2
)

// Account is what the rest of the program sees. It never asks which kind of
// account it has.
type Account interface {
	Name() string
	Balance() int64
	Deposit(amount int64)
	Withdraw(amount int64) error
	MonthEnd() string
}

// base holds what every account shares. Each kind embeds it, so Name,
// Balance, and Deposit are written once.
type base struct {
	name    string
	balance int64
}

func (b *base) Name() string         { return b.name }
func (b *base) Balance() int64       { return b.balance }
func (b *base) Deposit(amount int64) { b.balance += amount }

type Checking struct {
	base
}

func (c *Checking) Withdraw(amount int64) error {
	if c.balance-amount < -overdraftLimit {
		return fmt.Errorf("over the overdraft limit of %d", overdraftLimit)
	}
	c.balance -= amount
	return nil
}

func (c *Checking) MonthEnd() string {
	if c.balance < 0 {
		c.balance -= overdraftFee
		return fmt.Sprintf("fee %d", overdraftFee)
	}
	return "no fee"
}

type Savings struct {
	base
	withdrawals int
}

func (s *Savings) Withdraw(amount int64) error {
	if s.withdrawals == savingsWithdrawals {
		return fmt.Errorf("only %d withdrawals a month", savingsWithdrawals)
	}
	if amount > s.balance {
		return fmt.Errorf("insufficient funds")
	}
	s.balance -= amount
	s.withdrawals++
	return nil
}

func (s *Savings) MonthEnd() string {
	interest := s.balance / 100
	s.balance += interest
	s.withdrawals = 0
	return fmt.Sprintf("interest %d", interest)
}

var kinds = map[string]func(name string) Account{
	"checking": func(name string) Account { return &Checking{base{name: name}} },
	"savings":  func(name string) Account { return &Savings{base: base{name: name}} },
}

type Bank struct {
	accounts []Account // in the order they were opened
	byName   map[string]Account
}

func (b *Bank) find(name string) (Account, error) {
	account, ok := b.byName[name]
	if !ok {
		return nil, fmt.Errorf("no account named %s", name)
	}
	return account, nil
}

func (b *Bank) run(words []string) error {
	switch {
	case len(words) == 3 && words[0] == "open":
		kind, name := words[1], words[2]
		newAccount, ok := kinds[kind]
		if !ok {
			return fmt.Errorf("unknown account kind: %s", kind)
		}
		if _, taken := b.byName[name]; taken {
			return fmt.Errorf("%s already has an account", name)
		}
		account := newAccount(name)
		b.accounts = append(b.accounts, account)
		b.byName[name] = account
		fmt.Printf("%s: opened a %s account\n", name, kind)
	case len(words) == 3 && (words[0] == "deposit" || words[0] == "withdraw"):
		name := words[1]
		amount, err := strconv.ParseInt(words[2], 10, 64)
		if err != nil {
			return fmt.Errorf("not a whole number: %s", words[2])
		}
		account, err := b.find(name)
		if err != nil {
			return err
		}
		if words[0] == "deposit" {
			account.Deposit(amount)
			fmt.Printf("%s: deposited %d, balance %d\n", name, amount, account.Balance())
			return nil
		}
		if err := account.Withdraw(amount); err != nil {
			return fmt.Errorf("%s: %w", name, err)
		}
		fmt.Printf("%s: withdrew %d, balance %d\n", name, amount, account.Balance())
	case len(words) == 1 && words[0] == "month":
		for _, account := range b.accounts {
			change := account.MonthEnd()
			fmt.Printf("%s: %s, balance %d\n", account.Name(), change, account.Balance())
		}
	}
	return nil
}

func main() {
	bank := &Bank{byName: map[string]Account{}}
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		words := strings.Fields(scanner.Text())
		if err := bank.run(words); err != nil {
			fmt.Println("error:", err)
		}
	}
}
