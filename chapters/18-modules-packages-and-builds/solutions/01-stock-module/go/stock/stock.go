// Package stock keeps count of how many of each item there are.
package stock

import (
	"cmp"
	"slices"
)

// Stock holds counts of items. Its field starts with a lower-case letter, so it
// isn't exported: other packages can't read or change the counts except through
// the methods.
type Stock struct {
	counts map[string]int64
}

// Item is an item's name and how many of it there are. Both fields are exported.
type Item struct {
	Name  string
	Count int64
}

// New returns an empty Stock. The zero Stock{} would have a nil map, which can be
// read but not written, so other packages should start with New.
func New() *Stock {
	return &Stock{counts: map[string]int64{}}
}

// Add adds amount of item.
func (s *Stock) Add(item string, amount int64) {
	s.counts[item] += amount
}

// Take takes amount of item away, and returns true, if there are that many.
func (s *Stock) Take(item string, amount int64) bool {
	if s.counts[item] < amount {
		return false
	}
	s.counts[item] -= amount
	return true
}

// Count returns how many of item there are: 0 for an item never added.
func (s *Stock) Count(item string) int64 {
	return s.counts[item]
}

// Items returns the items there are any of, in alphabetical order. A map has no
// order of its own, so Items sorts them.
func (s *Stock) Items() []Item {
	var items []Item
	for name, count := range s.counts {
		if count > 0 {
			items = append(items, Item{name, count})
		}
	}
	slices.SortFunc(items, func(a, b Item) int { return cmp.Compare(a.Name, b.Name) })
	return items
}
