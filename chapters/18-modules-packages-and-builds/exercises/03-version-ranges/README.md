# Exercise 18.3: Version ranges

A manifest such as `package.json` doesn't name one version of each dependency: it names
a range, such as `^1.2.0`, and the package manager picks the highest published version
in it. Given a list of versions and some ranges, written the way npm writes them, print
the highest version that each range allows.

The first line of input lists the versions, separated by spaces. Each line after it is a
range. For example:

```text
1.0.0 1.2.0 1.2.5 1.3.0 2.0.0-beta.1 2.0.0 2.1.0
^1.2.0
~1.2.0
>=2.0.0-beta.1 <2.0.0
1.x || >=2.1
<1.0.0
```

prints:

```text
^1.2.0 -> 1.3.0
~1.2.0 -> 1.2.5
>=2.0.0-beta.1 <2.0.0 -> 2.0.0-beta.1
1.x || >=2.1 -> 2.1.0
<1.0.0 -> none
```

Each range is printed with the spaces at its ends trimmed, then ` -> `, and then the
highest version it allows, or `none`. Blank lines are skipped, but they still count as
lines. These are the rules that npm's own library, `node-semver`, follows, with a few of
its extras left out.

**Versions.** A version is three numbers separated by dots, `MAJOR.MINOR.PATCH`, and an
optional pre-release after a `-`, as in `1.2.3-beta.1`. Each number is `0`, or up to 15
digits that don't start with `0`. A pre-release is identifiers separated by dots, and
each identifier is either digits that don't start with `0` (unless it is just `0`), or
letters, digits, and hyphens with at least one that isn't a digit. So `1.0.0-x-1.2` is a
version, and `1.02.0`, `1.0`, `1.0.0-`, and `1.0.0-01` aren't.

Versions are ordered as [Semantic Versioning](https://semver.org/) says:

- Compare the major numbers, then the minor numbers, then the patch numbers, as numbers.
- If those are all equal, a version with a pre-release comes before the one without:
  `1.0.0-rc.1` comes before `1.0.0`.
- Two pre-releases compare one identifier at a time. Identifiers that are digits compare
  as numbers, and come before other identifiers, which compare by their character codes.
  If one pre-release runs out of identifiers first, and they were equal until then, it
  comes first.

So these versions are in order: `1.0.0-alpha`, `1.0.0-alpha.1`, `1.0.0-alpha.beta`,
`1.0.0-beta`, `1.0.0-beta.2`, `1.0.0-beta.11`, `1.0.0-rc.1`, `1.0.0`.

**Ranges.** A range is one or more sets of comparators, separated by `||`. A set is one
or more comparators separated by spaces or tabs, and a version is in the set if it
passes all of them. A version is in the range if it is in any of its sets.

A comparator is an operator, `=`, `<`, `<=`, `>`, `>=`, `^`, or `~`, or none, and right
after it a partial version: one, two, or three parts separated by dots, each a number or
one of `x`, `X`, and `*`, which stand for any number. A wildcard can't come before a
number, so `1.x.x` is a partial version and `1.x.3` isn't. A pre-release can follow only
three numbers, as in `>=1.2.3-beta`. Each comparator stands for one or two simple ones,
which compare against a full version:

| Comparator | Stands for | Why |
| --- | --- | --- |
| `1.2.3` or `=1.2.3` | `=1.2.3` | Exactly that version |
| `1.2`, `1.2.x`, or `=1.2` | `>=1.2.0 <1.3.0-0` | Any 1.2 |
| `1`, `1.x`, or `1.x.x` | `>=1.0.0 <2.0.0-0` | Any 1 |
| `*`, `x`, `>=*`, and the like | nothing at all | Any version |
| `<*` or `>*` | `<0.0.0-0` | No version is below or above all of them |
| `>1.2` | `>=1.3.0` | Above every 1.2 |
| `>=1.2` | `>=1.2.0` | |
| `<1.2` | `<1.2.0-0` | Below every 1.2, even its pre-releases |
| `<=1.2` | `<1.3.0-0` | Up to the last 1.2 |
| `~1.2.3` or `~1.2` | `>=1.2.3 <1.3.0-0`, or from `1.2.0` | Changes to the patch number |
| `~1` | `>=1.0.0 <2.0.0-0` | Changes to the minor and patch numbers |
| `^1.2.3` | `>=1.2.3 <2.0.0-0` | Anything up to the next change of the first number that isn't 0 |
| `^0.2.3` | `>=0.2.3 <0.3.0-0` | |
| `^0.0.3` | `>=0.0.3 <0.0.4-0` | |
| `^0.0` or `^0.0.x` | `>=0.0.0 <0.1.0-0` | Of the numbers given, none isn't 0, so up to the next change of the last one |
| `^1.x` or `^1` | `>=1.0.0 <2.0.0-0` | |

A pre-release on the comparator stays on the lower end: `^1.2.3-beta.1` is
`>=1.2.3-beta.1 <2.0.0-0`. The `-0` on an upper end is the lowest pre-release there can
be, so `<2.0.0-0` keeps out `2.0.0-alpha` as well as `2.0.0`.

**Pre-releases.** A version with a pre-release is in a set only if it passes every
comparator, and the set has a comparator with a pre-release on the same
`MAJOR.MINOR.PATCH`, in the version the comparator was written with. So `>=1.0.0-beta`
allows `1.0.0-rc.1` but not `1.1.0-rc.1`, and `*` and `^1.0.0` allow no pre-release at
all. That way, someone who asked for `^1.2.0` doesn't get `1.3.0-beta`, which nobody has
promised works yet.

**Problems.** A problem with the first line prints one line, and nothing else. A problem
with a range prints one line for that range, and the program goes on to the next line.
It checks the sides of `||` from left to right, and the comparators in each from left to
right, and reports the first problem it finds.

| Problem | Output |
| --- | --- |
| A word on the first line isn't a version | `error: line 1: WORD isn't a version` |
| The first line has no words | `error: line 1 must list the versions` |
| A side of `\|\|` has no comparators, as in `1.0.0 \|\|` | `error: line N: each side of \|\| needs a comparator` |
| A word isn't a comparator | `error: line N: WORD isn't a comparator` |

Input with no lines at all prints nothing. npm allows a few things that this doesn't,
and that are all errors here: a `v` in front of a version, spaces between an operator
and its version (`>= 1.2.3`), hyphen ranges (`1.2.3 - 2.3.4`), `~>`, build metadata
after a `+`, and a pre-release after a wildcard.

**A module of its own.** Put the versions and ranges in a module called `semver`, and
leave the main program only reading the input and printing the answers. Keep what a
range is made of private to the module: the main program should be able to parse a range
and ask it questions, but not build one or look inside. The starter is only the main
program, so add the module's files next to it. In Go, the starter's `go.mod` names the
module `example.com/ranges`, so a package in `semver/semver.go` is imported as
`example.com/ranges/semver`. The checker builds every source file in the folder, and
follows the imports.

```sh
python3 tools/check.py 18 --lang python
```

**Things to notice:**

- **Checked against npm.** The tests' answers are what `node-semver` itself, version 7,
  gives with `semver.maxSatisfying`, and the nine solutions agree with it on 4,000
  random ranges against random lists of versions.
- **What each module keeps to itself.** In each solution the main program can parse
  versions and ranges, compare versions, and ask a range what it allows, but not see how
  a range is stored. C's `Range` is an opaque struct, C++'s keeps its sets and its
  helper types `private`, and Rust's and Go's have a field that isn't exported, so the
  only way to get a `Range` is to parse one. Dart's and Kotlin's have a private
  constructor, `Range._` in Dart and `Range private constructor` in Kotlin, with the
  parser inside the library or class. JavaScript and TypeScript keep the sets in a `#`
  field, and Python in `_sets`, by agreement.
- **Regular expressions or not.** Python, JavaScript, TypeScript, Dart, and Kotlin build
  their patterns from pieces, `NUMBER`, `IDENTIFIER`, and `PRE`, the way the semver
  specification writes its grammar. Rust has no regular expressions in its standard
  library, and C has none in its own, since `regex.h` is POSIX, not C. So the C and Rust
  solutions split on `-`, `.`, and `||` by hand, and the C++ and Go ones do the same,
  though `std::regex` and Go's `regexp` could have done it.
- **Big numbers.** Fifteen digits keeps every number, and the number after it, below
  2^53, so a 64-bit integer holds it, and so does a JavaScript number, exactly. Numeric
  identifiers in a pre-release can be any length, so the solutions compare them by
  length first and then digit by digit, which works because they can't start with 0.
- **Why `^0.2.3` stops at `0.3.0`.** Semantic Versioning says that anything may change
  before 1.0.0, so npm treats the first number that isn't 0 as the one that signals a
  breaking change. A library at 0.x that wants its users to get its fixes has to say
  which minor versions are safe, or reach 1.0.0.

**Stretch:** add hyphen ranges, which npm writes with spaces around the hyphen:
`1.2.3 - 2.3.4` is `>=1.2.3 <=2.3.4`. A partial version on the left fills in zeros, and
one on the right stands for the end of what it names, so `1.2 - 2.3` is
`>=1.2.0 <2.4.0-0`. Then check your answers against `node-semver`: `npm install semver`
and call `semver.maxSatisfying(versions, range)`.
