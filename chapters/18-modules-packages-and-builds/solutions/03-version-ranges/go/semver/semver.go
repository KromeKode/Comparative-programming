// Package semver reads versions and ranges as npm writes them: 1.2.3-beta.1, ^1.2,
// ~1.2.3, >=1 <2, 1.x || 3. Only the names that start with a capital letter can be
// used from other packages.
package semver

import (
	"cmp"
	"fmt"
	"slices"
	"strconv"
	"strings"
)

// Version is a version such as 1.2.3, or 1.2.3-beta.1 with a pre-release.
type Version struct {
	Major, Minor, Patch int64
	Pre                 []string // the pre-release's identifiers, if it has one
}

func (v Version) String() string {
	text := fmt.Sprintf("%d.%d.%d", v.Major, v.Minor, v.Patch)
	if len(v.Pre) > 0 {
		text += "-" + strings.Join(v.Pre, ".")
	}
	return text
}

func (v Version) core() [3]int64 { return [3]int64{v.Major, v.Minor, v.Patch} }

func allDigits(text string) bool {
	return strings.Trim(text, "0123456789") == ""
}

// parseNumber reads a number in a version: 0, or up to 15 digits that don't start
// with 0.
func parseNumber(text string) (int64, bool) {
	if text == "" || len(text) > 15 || !allDigits(text) || (len(text) > 1 && text[0] == '0') {
		return 0, false
	}
	number, err := strconv.ParseInt(text, 10, 64)
	return number, err == nil
}

// parsePre reads a pre-release: identifiers separated by dots. Each is digits
// without a 0 in front, or letters, digits, and hyphens with at least one that
// isn't a digit.
func parsePre(text string) ([]string, bool) {
	identifiers := strings.Split(text, ".")
	for _, identifier := range identifiers {
		ok := identifier != "" && strings.Trim(identifier,
			"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-") == ""
		if !ok || (len(identifier) > 1 && identifier[0] == '0' && allDigits(identifier)) {
			return nil, false
		}
	}
	return identifiers, true
}

// Parse returns the version that text is, and false if it isn't one.
func Parse(text string) (Version, bool) {
	// The core is everything before the first '-', and the pre-release after it.
	core, pre, hasPre := strings.Cut(text, "-")
	parts := strings.Split(core, ".")
	if len(parts) != 3 {
		return Version{}, false
	}
	var v Version
	var ok [4]bool
	v.Major, ok[0] = parseNumber(parts[0])
	v.Minor, ok[1] = parseNumber(parts[1])
	v.Patch, ok[2] = parseNumber(parts[2])
	ok[3] = true
	if hasPre {
		v.Pre, ok[3] = parsePre(pre)
	}
	return v, ok == [4]bool{true, true, true, true}
}

// compareIdentifiers compares numbers as numbers, and puts them before other
// identifiers, which compare character by character. Numbers have no 0 in front,
// so a longer one is larger.
func compareIdentifiers(a, b string) int {
	aNumber, bNumber := allDigits(a), allDigits(b)
	switch {
	case aNumber && bNumber:
		return cmp.Or(cmp.Compare(len(a), len(b)), strings.Compare(a, b))
	case aNumber:
		return -1
	case bNumber:
		return 1
	}
	return strings.Compare(a, b)
}

// Compare returns a negative number if a comes before b, 0 if they are equal, and
// a positive number if after.
func Compare(a, b Version) int {
	order := cmp.Or(cmp.Compare(a.Major, b.Major), cmp.Compare(a.Minor, b.Minor),
		cmp.Compare(a.Patch, b.Patch))
	if order != 0 {
		return order
	}
	// A version without a pre-release comes after the same version with one.
	if len(a.Pre) == 0 || len(b.Pre) == 0 {
		return cmp.Compare(len(b.Pre), len(a.Pre))
	}
	return slices.CompareFunc(a.Pre, b.Pre, compareIdentifiers)
}

type op int

const (
	eq op = iota
	lt
	le
	gt
	ge
)

// simple is a simple comparator: an operator and a full version.
type simple struct {
	op    op
	bound Version
}

// Range is sets of simple comparators joined by ||. A version is in the range if
// it passes every comparator of some set. Its field is unexported, so other
// packages can make a Range only with ParseRange.
type Range struct {
	sets [][]simple
}

// expand returns the simple comparators that a comparator stands for, and false if
// it isn't one.
func expand(token string) ([]simple, bool) {
	kind := ""
	for _, prefix := range []string{"<=", ">=", "<", ">", "=", "^", "~"} {
		if rest, found := strings.CutPrefix(token, prefix); found {
			kind, token = prefix, rest
			break
		}
	}
	core, pre, hasPre := strings.Cut(token, "-")
	parts := strings.Split(core, ".")
	if len(parts) > 3 {
		return nil, false
	}
	// known counts the numbers, which must come before any wildcards.
	var nums [3]int64
	known := 0
	for i, part := range parts {
		if part == "x" || part == "X" || part == "*" {
			continue
		}
		number, ok := parseNumber(part)
		if !ok || known != i {
			return nil, false
		}
		nums[known] = number
		known++
	}
	low := Version{nums[0], nums[1], nums[2], nil}
	if hasPre {
		// A pre-release can follow only three numbers.
		identifiers, ok := parsePre(pre)
		if known < 3 || !ok {
			return nil, false
		}
		low.Pre = identifiers
	}
	// after returns the first version after all those whose parts up to level
	// match: the -0 pre-release of the next major, minor, or patch.
	after := func(level int) Version {
		switch level {
		case 0:
			return Version{nums[0] + 1, 0, 0, []string{"0"}}
		case 1:
			return Version{nums[0], nums[1] + 1, 0, []string{"0"}}
		}
		return Version{nums[0], nums[1], nums[2] + 1, []string{"0"}}
	}
	if known == 0 {
		// Any version: nothing is outside it, and nothing is above or below it.
		if kind == "<" || kind == ">" {
			return []simple{{lt, Version{0, 0, 0, []string{"0"}}}}, true
		}
		return nil, true
	}
	next := after(known - 1)
	full := known == 3
	switch {
	case (kind == "" || kind == "=") && full:
		return []simple{{eq, low}}, true
	case kind == "" || kind == "=":
		return []simple{{ge, low}, {lt, next}}, true
	case kind == ">" && full:
		return []simple{{gt, low}}, true
	case kind == ">":
		return []simple{{ge, Version{next.Major, next.Minor, next.Patch, nil}}}, true
	case kind == ">=":
		return []simple{{ge, low}}, true
	case kind == "<" && full:
		return []simple{{lt, low}}, true
	case kind == "<":
		return []simple{{lt, Version{low.Major, low.Minor, low.Patch, []string{"0"}}}}, true
	case kind == "<=" && full:
		return []simple{{le, low}}, true
	case kind == "<=":
		return []simple{{lt, next}}, true
	case kind == "~":
		return []simple{{ge, low}, {lt, after(min(known-1, 1))}}, true
	}
	// ^ allows changes that don't change the first part that isn't 0.
	first := known - 1
	for i := known - 1; i >= 0; i-- {
		if nums[i] != 0 {
			first = i
		}
	}
	return []simple{{ge, low}, {lt, after(first)}}, true
}

// ParseRange returns the range that text is, or an error saying why it isn't one.
func ParseRange(text string) (Range, error) {
	var r Range
	for _, side := range strings.Split(text, "||") {
		tokens := strings.Fields(side)
		if len(tokens) == 0 {
			return Range{}, fmt.Errorf("each side of || needs a comparator")
		}
		var simples []simple
		for _, token := range tokens {
			expanded, ok := expand(token)
			if !ok {
				return Range{}, fmt.Errorf("%s isn't a comparator", token)
			}
			simples = append(simples, expanded...)
		}
		r.sets = append(r.sets, simples)
	}
	return r, nil
}

func (s simple) holds(v Version) bool {
	order := Compare(v, s.bound)
	switch s.op {
	case eq:
		return order == 0
	case lt:
		return order < 0
	case le:
		return order <= 0
	case gt:
		return order > 0
	}
	return order >= 0
}

// Allows reports whether the range allows the version.
func (r Range) Allows(v Version) bool {
	for _, simples := range r.sets {
		passes, named := true, false
		for _, s := range simples {
			passes = passes && s.holds(v)
			named = named || (len(s.bound.Pre) > 0 && s.bound.core() == v.core())
		}
		// A pre-release is in only if a comparator of the set names a pre-release of
		// the same major, minor, and patch.
		if passes && (len(v.Pre) == 0 || named) {
			return true
		}
	}
	return false
}

// Highest returns the highest of versions that the range allows, and false if it
// allows none.
func (r Range) Highest(versions []Version) (Version, bool) {
	var best Version
	found := false
	for _, v := range versions {
		if r.Allows(v) && (!found || Compare(v, best) > 0) {
			best, found = v, true
		}
	}
	return best, found
}
