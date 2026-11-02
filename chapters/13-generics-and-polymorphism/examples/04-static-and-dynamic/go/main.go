package main

import (
	"fmt"
	"strings"
)

type Animal interface {
	Name() string
	Sound() string
}

type Cat struct{ name string }

func (c Cat) Name() string  { return c.name }
func (c Cat) Sound() string { return "meow" }
func (c Cat) Purr() string  { return "prrr" }

type Dog struct{ name string }

func (d Dog) Name() string  { return d.name }
func (d Dog) Sound() string { return "woof" }

// A generic function takes a []Cat or a []Dog as it is. All the animals in
// one call must be the same type.
func Chorus[A Animal](animals []A) string {
	lines := make([]string, len(animals))
	for i, animal := range animals {
		lines[i] = animal.Name() + " says " + animal.Sound()
	}
	return strings.Join(lines, ", ")
}

// An interface value holds any Animal, and each call is looked up in the
// value's method table. The animals can be any mix.
func ChorusAll(animals []Animal) string {
	lines := make([]string, len(animals))
	for i, animal := range animals {
		lines[i] = animal.Name() + " says " + animal.Sound()
	}
	return strings.Join(lines, ", ")
}

// A generic function hands back the caller's own type, so the caller can
// still use what only a Cat has.
func LongestName[A Animal](animals []A) A {
	best := animals[0]
	for _, animal := range animals {
		if len(animal.Name()) > len(best.Name()) {
			best = animal
		}
	}
	return best
}

func main() {
	cats := []Cat{{"Tom"}, {"Kitty"}}
	dogs := []Dog{{"Rex"}}
	fmt.Println("cats:", Chorus(cats))
	fmt.Println("dogs:", Chorus(dogs))

	everyone := []Animal{cats[0], dogs[0], cats[1]}
	fmt.Println("everyone:", ChorusAll(everyone))

	cat := LongestName(cats)
	fmt.Printf("longest name: %s, who purrs %s\n", cat.Name(), cat.Purr())
}
