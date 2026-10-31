package main

import "fmt"

// Go has no sum types. The usual stand-in is an interface that only the variants
// implement; the unexported method keeps other packages from adding more.
type payment interface{ isPayment() }

type cash struct{}

type card struct{ last4 string }

type voucher struct {
	code  string
	cents int
}

func (cash) isPayment()    {}
func (card) isPayment()    {}
func (voucher) isPayment() {}

func money(cents int) string {
	return fmt.Sprintf("%d.%02d", cents/100, cents%100)
}

func describe(p payment, total int) string {
	// A type switch runs the case for the type p holds. Nothing checks that every
	// variant has a case, so default catches anything missed.
	switch p := p.(type) {
	case cash:
		return fmt.Sprintf("cash: pay %s in cash", money(total))
	case card:
		return fmt.Sprintf("card ending %s: charge %s to the card", p.last4, money(total))
	case voucher:
		if p.cents >= total {
			return fmt.Sprintf("voucher %s (%s): covers the order", p.code, money(p.cents))
		}
		left := money(total - p.cents)
		return fmt.Sprintf("voucher %s (%s): %s left to pay", p.code, money(p.cents), left)
	default:
		return "unknown payment"
	}
}

func main() {
	total := 1250
	payments := []payment{cash{}, card{"4242"}, voucher{"SPRING", 500}, voucher{"BIG", 2000}}
	fmt.Println("order total:", money(total))
	for _, p := range payments {
		fmt.Println(describe(p, total))
	}
}
