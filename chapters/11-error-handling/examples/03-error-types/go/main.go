package main

import (
	"errors"
	"fmt"
)

// A sentinel error: one fixed value that callers test with errors.Is.
var ErrFrozen = errors.New("account is frozen")

// An error type: it carries data, and callers find it with errors.As.
type InsufficientFundsError struct {
	Needed, Available int
}

func (e *InsufficientFundsError) Error() string {
	return fmt.Sprintf("insufficient funds: need %d, have %d", e.Needed, e.Available)
}

type Account struct {
	balance int
	frozen  bool
}

func (a *Account) Withdraw(amount int) error {
	if amount <= 0 {
		// A plain error: callers can only print it.
		return fmt.Errorf("invalid amount: %d", amount)
	}
	if a.frozen {
		return ErrFrozen
	}
	if amount > a.balance {
		return &InsufficientFundsError{Needed: amount, Available: a.balance}
	}
	a.balance -= amount
	return nil
}

func attempt(account *Account, amount int) {
	err := account.Withdraw(amount)
	if err == nil {
		fmt.Printf("withdraw %d: ok, %d left\n", amount, account.balance)
		return
	}
	fmt.Printf("withdraw %d: error: %v\n", amount, err)
	var short *InsufficientFundsError
	switch {
	case errors.As(err, &short):
		// errors.As found the error and set short to point at it.
		if err := account.Withdraw(short.Available); err != nil {
			panic(err)
		}
		fmt.Printf("  took %d instead, %d left\n", short.Available, account.balance)
	case errors.Is(err, ErrFrozen):
		fmt.Println("  call the bank to unfreeze it")
	}
}

func main() {
	account := &Account{balance: 100}
	fmt.Printf("start with %d\n", account.balance)
	for _, amount := range []int{30, -5, 100} {
		attempt(account, amount)
	}
	fmt.Println("freeze the account")
	account.frozen = true
	attempt(account, 10)
}
