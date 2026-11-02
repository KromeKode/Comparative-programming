package main

import (
	"fmt"
	"slices"
	"strings"
)

type Animal interface{ Name() string }

type Cat struct{ name string }

func (c Cat) Name() string { return c.name }

type Dog struct{ name string }

func (d Dog) Name() string { return d.name }

func Names(animals []Animal) string {
	texts := make([]string, len(animals))
	for i, animal := range animals {
		texts[i] = animal.Name()
	}
	return strings.Join(texts, ", ")
}

// A []Cat is not a []Animal: a Cat is stored as a bare struct, an Animal as
// an interface value. Converting means copying, one item at a time.
func toAnimals(cats []Cat) []Animal {
	animals := make([]Animal, len(cats))
	for i, cat := range cats {
		animals[i] = cat
	}
	return animals
}

func main() {
	cats := []Cat{{"Tom"}, {"Kitty"}}
	animals := toAnimals(cats)
	fmt.Println("names:", Names(animals))

	shelter := append(slices.Clone(animals), Dog{"Rex"})
	fmt.Println("shelter:", Names(shelter))

	// A func(Animal, Animal) int is not a func(Cat, Cat) int either, so wrap it.
	byName := func(a, b Animal) int { return strings.Compare(a.Name(), b.Name()) }
	sorted := slices.Clone(cats)
	slices.SortFunc(sorted, func(a, b Cat) int { return byName(a, b) })
	fmt.Println("sorted by name:", Names(toAnimals(sorted)))
}
