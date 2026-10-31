package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

// Every plugin has a name and changes a list of words.
type Plugin interface {
	Name() string
	Apply(words []string) []string
	Describe() string
}

// Go interfaces have no default methods. A plugin embeds named to get Name
// and the usual Describe, and can write its own Describe instead.
type named struct {
	name string
}

func (n named) Name() string     { return n.name }
func (n named) Describe() string { return n.name }

type Upper struct{ named }

func (Upper) Apply(words []string) []string {
	result := make([]string, len(words))
	for i, word := range words {
		result[i] = strings.ToUpper(word)
	}
	return result
}

type Lower struct{ named }

func (Lower) Apply(words []string) []string {
	result := make([]string, len(words))
	for i, word := range words {
		result[i] = strings.ToLower(word)
	}
	return result
}

type Reverse struct{ named }

func (Reverse) Apply(words []string) []string {
	result := make([]string, len(words))
	for i, word := range words {
		result[len(words)-1-i] = word
	}
	return result
}

type Censor struct {
	named
	word string
}

func (c Censor) Apply(words []string) []string {
	result := make([]string, len(words))
	for i, word := range words {
		result[i] = word
		if strings.EqualFold(word, c.word) {
			result[i] = strings.Repeat("*", len(word))
		}
	}
	return result
}

func (c Censor) Describe() string { return "censor(" + c.word + ")" }

type entry struct {
	name, usage, help string
	make              func(args []string) (Plugin, error)
}

// Registry maps a plugin's name to a way to make one. The rest of the program
// finds plugins only through here.
type Registry struct {
	entries []entry
}

func (r *Registry) Register(e entry) {
	r.entries = append(r.entries, e)
}

func (r *Registry) Create(name string, args []string) (Plugin, error) {
	for _, e := range r.entries {
		if e.name == name {
			return e.make(args)
		}
	}
	return nil, fmt.Errorf("no plugin named %s", name)
}

func noArguments(plugin Plugin) func([]string) (Plugin, error) {
	return func(args []string) (Plugin, error) {
		if len(args) > 0 {
			return nil, fmt.Errorf("%s takes no arguments", plugin.Name())
		}
		return plugin, nil
	}
}

func makeCensor(args []string) (Plugin, error) {
	if len(args) != 1 {
		return nil, fmt.Errorf("censor needs exactly one word")
	}
	return Censor{named{"censor"}, args[0]}, nil
}

func makeRegistry() *Registry {
	registry := &Registry{}
	registry.Register(entry{"upper", "upper", "make every letter uppercase",
		noArguments(Upper{named{"upper"}})})
	registry.Register(entry{"lower", "lower", "make every letter lowercase",
		noArguments(Lower{named{"lower"}})})
	registry.Register(entry{"reverse", "reverse", "reverse the order of the words",
		noArguments(Reverse{named{"reverse"}})})
	registry.Register(entry{"censor", "censor WORD", "hide WORD wherever it appears",
		makeCensor})
	return registry
}

func main() {
	registry := makeRegistry()
	var pipeline []Plugin
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		words := strings.Fields(scanner.Text())
		if len(words) == 0 {
			continue
		}
		switch {
		case len(words) == 1 && words[0] == "list":
			for _, e := range registry.entries {
				fmt.Printf("%s: %s\n", e.usage, e.help)
			}
		case len(words) >= 2 && words[0] == "add":
			plugin, err := registry.Create(words[1], words[2:])
			if err != nil {
				fmt.Println("error:", err)
				continue
			}
			pipeline = append(pipeline, plugin)
			fmt.Println("added", plugin.Describe())
		case len(words) == 1 && words[0] == "pipeline":
			steps := make([]string, len(pipeline))
			for i, plugin := range pipeline {
				steps[i] = plugin.Describe()
			}
			if len(steps) == 0 {
				fmt.Println("pipeline: (empty)")
			} else {
				fmt.Println("pipeline:", strings.Join(steps, " | "))
			}
		case words[0] == "run":
			words = words[1:]
			for _, plugin := range pipeline {
				words = plugin.Apply(words)
			}
			fmt.Println(strings.Join(words, " "))
		}
	}
}
