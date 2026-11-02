package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"slices"
	"strconv"
	"strings"
)

// Setting holds one value of type T, and knows how to read a T from text and
// how to show one. T is any, not comparable, because tags is a []string and
// slices can't be compared with ==. So each setting brings an equal function.
type Setting[T any] struct {
	name       string
	def, value T
	parse      func(string) (T, bool)
	show       func(T) string
	equal      func(a, b T) bool
	needs      string
}

func NewSetting[T any](name string, def T, parse func(string) (T, bool), show func(T) string,
	equal func(a, b T) bool, needs string) *Setting[T] {
	return &Setting[T]{name, def, def, parse, show, equal, needs}
}

func (s *Setting[T]) Name() string { return s.name }

func (s *Setting[T]) SetText(text string) error {
	value, ok := s.parse(text)
	if !ok {
		return fmt.Errorf("%s needs %s, not \"%s\"", s.name, s.needs, text)
	}
	s.value = value
	return nil
}

func (s *Setting[T]) Reset()           { s.value = s.def }
func (s *Setting[T]) Describe() string { return s.name + " = " + s.show(s.value) }
func (s *Setting[T]) IsDefault() bool  { return s.equal(s.value, s.def) }

// AnySetting is what the command loop needs, without knowing T. Every
// *Setting[T] has these methods, so one slice can hold all four.
type AnySetting interface {
	Name() string
	SetText(text string) error
	Reset()
	Describe() string
	IsDefault() bool
}

func same[T comparable](a, b T) bool { return a == b }

func parseInt(text string) (int, bool) {
	digits := strings.TrimPrefix(text, "-")
	if digits == "" || strings.Trim(digits, "0123456789") != "" {
		return 0, false
	}
	n, err := strconv.ParseInt(text, 10, 32) // 32 bits: a range error above that
	return int(n), err == nil
}

func parseBool(text string) (bool, bool) {
	switch text {
	case "true":
		return true, true
	case "false":
		return false, true
	}
	return false, false
}

func showTags(tags []string) string { return "[" + strings.Join(tags, ", ") + "]" }

// Typed fields: volume.value is an int and muted.value a bool, with no casts.
type Settings struct {
	volume *Setting[int]
	muted  *Setting[bool]
	name   *Setting[string]
	tags   *Setting[[]string]
	all    []AnySetting
}

func NewSettings() *Settings {
	identity := func(text string) string { return text }
	s := &Settings{
		volume: NewSetting("volume", 5, parseInt, strconv.Itoa, same[int], "a whole number"),
		muted:  NewSetting("muted", false, parseBool, strconv.FormatBool, same[bool], "true or false"),
		name: NewSetting("name", "guest", func(text string) (string, bool) { return text, true },
			identity, same[string], ""),
		tags: NewSetting("tags", []string{}, func(text string) ([]string, bool) {
			return strings.Fields(text), true
		}, showTags, slices.Equal[[]string], ""),
	}
	s.all = []AnySetting{s.volume, s.muted, s.name, s.tags}
	return s
}

func (s *Settings) Find(name string) (AnySetting, error) {
	for _, setting := range s.all {
		if setting.Name() == name {
			return setting, nil
		}
	}
	return nil, fmt.Errorf("no setting named \"%s\"", name)
}

func (s *Settings) Summary() string {
	if s.muted.value {
		return s.name.value + " has muted the sound"
	}
	return fmt.Sprintf("%s hears the sound at %d%%", s.name.value, s.volume.value*10)
}

func run(settings *Settings, words []string) error {
	if len(words) == 0 {
		return nil
	}
	switch words[0] {
	case "set", "get", "reset":
		setting, err := settings.Find(words[1])
		if err != nil {
			return err
		}
		if words[0] == "set" {
			if err := setting.SetText(strings.Join(words[2:], " ")); err != nil {
				return err
			}
		} else if words[0] == "reset" {
			setting.Reset()
		}
		fmt.Println(setting.Describe())
	case "list":
		for _, setting := range settings.all {
			mark := ""
			if setting.IsDefault() {
				mark = " (default)"
			}
			fmt.Println(setting.Describe() + mark)
		}
	case "summary":
		fmt.Println(settings.Summary())
	default:
		return errors.New("unknown command")
	}
	return nil
}

func main() {
	settings := NewSettings()
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		if err := run(settings, strings.Fields(scanner.Text())); err != nil {
			fmt.Println("error:", err)
		}
	}
}
