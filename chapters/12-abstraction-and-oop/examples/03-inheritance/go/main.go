package main

import "fmt"

// A bag of items. AddAll is written in terms of Add.
type Bag struct {
	items []string
}

func (b *Bag) Add(item string) {
	b.items = append(b.items, item)
}

func (b *Bag) AddAll(items []string) {
	for _, item := range items {
		b.Add(item) // b is a *Bag, so this is always Bag's Add
	}
}

func (b *Bag) Size() int {
	return len(b.items)
}

// Embedding: CountingBag contains a Bag, and Bag's methods are promoted, so
// bag.Size() works without writing it out. This is composition, not
// inheritance: Bag's methods never call back into CountingBag.
type CountingBag struct {
	Bag
	count int
}

func (c *CountingBag) Add(item string) {
	c.count++
	c.Bag.Add(item)
}

func (c *CountingBag) AddAll(items []string) {
	c.count += len(items)
	c.Bag.AddAll(items)
}

// HalfCountingBag wraps only Add. Its AddAll is Bag's, which calls Bag's Add,
// so the items added that way are never counted.
type HalfCountingBag struct {
	Bag
	count int
}

func (c *HalfCountingBag) Add(item string) {
	c.count++
	c.Bag.Add(item)
}

func report(label string, size, count int) {
	fmt.Printf("%s: the bag holds %d items, the count says %d\n", label, size, count)
}

func main() {
	fruit := []string{"apple", "pear", "plum"}

	bag := &CountingBag{}
	bag.AddAll(fruit)
	bag.Add("fig")
	report("embedding", bag.Size(), bag.count)

	half := &HalfCountingBag{}
	half.AddAll(fruit)
	half.Add("fig")
	report("embedding, Add only", half.Size(), half.count)
}
