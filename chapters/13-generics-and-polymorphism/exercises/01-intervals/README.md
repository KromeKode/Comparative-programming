# Exercise 13.1: Intervals

Each line of input is a kind, followed by one or more intervals. An interval is two
values joined by `..`, such as `3..8`, and it holds both ends and everything between
them. Merge the intervals that overlap, and print the kind followed by the merged
intervals, sorted by their low ends.

| Kind | Values | Order |
| --- | --- | --- |
| `int` | Whole numbers, such as `-3` or `42` | By size |
| `word` | Lowercase letters from `a` to `z` | Alphabetical: `ab` comes after `a` and before `b` |
| `version` | Three whole numbers joined by dots, such as `1.10.0` | Part by part, as numbers: `1.9.0` comes before `1.10.0` |

Two intervals overlap when they share at least one value. So `1..5` and `5..9` merge
into `1..9`, but `1..5` and `6..9` stay apart, even for ints.

| Input | Output |
| --- | --- |
| `int 1..5 3..8 10..12` | `int 1..8 10..12` |
| `int 10..20 9..9` | `int 9..9 10..20` |
| `word kiwi..pear apple..fig fig..grape` | `word apple..grape kiwi..pear` |
| `version 1.2.0..1.9.9 1.10.0..2.0.0 1.9.0..1.10.0` | `version 1.2.0..2.0.0` |
| `version 1.10.0..1.2.0` | `error: 1.10.0..1.2.0 is backwards` |
| `date 2024..2025` | `error: unknown kind "date"` |

The other errors are `error: bad interval "TEXT"`, when a word is not two values joined
by exactly one `..`, and `error: bad KIND "VALUE"`, such as `error: bad int "x"`, when a
value is not of the line's kind. Check the intervals from left to right, and the low end
before the high end, and print only the first error on a line.

A whole number is an optional minus sign and at least one digit, with no plus sign.
Print ints and the parts of versions without leading zeros, so `007` prints as `7`. Skip
blank lines, and allow any number of spaces between words. Every line has at least one
interval. The tests use ASCII only, numbers that fit in 32 bits, and lines under 256
characters.

Write an `Interval<T>` type and a `merge` function once, for any T that can be ordered,
and a parse function for each kind. All three kinds must go through the same `merge`.

```sh
python3 tools/check.py 13 --lang rust
```

**Things to notice:**

- **One merge, three orders.** In text order, `10` comes before `9` and `1.10.0` before
  `1.9.0`, and the tests check for both. Ints and words already have the right order in
  every language, but a version needs one that you write. Rust's
  `#[derive(PartialOrd, Ord)]` and C++'s
  `auto operator<=>(const Version&) const = default` compare the fields in the order
  they are declared, which is exactly right here, and so does Python's
  `@dataclass(order=True)`. Kotlin and Dart implement `Comparable<Version>`.
- **Go and TypeScript pass a compare function.** Go's `cmp.Ordered` covers only built-in
  types such as `int` and `string`, so `merge` takes a `compare func(a, b T) int`, as
  `slices.SortFunc` does. The three kinds pass `cmp.Compare[int]`, `strings.Compare`,
  and the method expression `Version.Compare`, which is a `func(Version, Version) int`.
  TypeScript has no constraint for "can be ordered", and `<` on two objects compares
  their text, where `"1.10.0" < "1.9.0"` is `true`. So TypeScript and JavaScript pass a
  compare function too.
- **Dart's bound.** `int` implements `Comparable<num>`, not `Comparable<int>`, so a
  bound of `T extends Comparable<T>` refuses `int` when the type is written out:
  `'int' doesn't conform to the bound 'Comparable<int>'`. The solution uses
  `T extends Comparable<Object>`, which `int`, `String`, and `Version` all meet.
- **The kind comes from the input, but T is fixed when the program is compiled.** Each
  branch of the `match` or `switch` calls `run` with a different parse function, and
  that fixes T for that branch. Rust and C++ make a separate copy of `run` for each
  kind. Python, TypeScript, and JavaScript keep three small functions in a map instead:
  each one fixes T inside, and all three have the same type, from a list of words to a
  line of text.
- **C passes a table of functions.** C has no generics, so the C solution keeps every
  value in one `union` and gives each kind a struct with three function pointers:
  `parse`, `compare`, and `print`. `qsort` passes its compare function only the two
  items, so the kind being sorted waits in a static variable. glibc's `qsort_r` and
  C11's optional `qsort_s` take one more argument for this, but neither is available
  everywhere.
- **Strict whole numbers.** `+3` is a bad int here, but Python's `int`, JavaScript's
  `Number`, Go's `strconv.Atoi`, Rust's `parse`, Kotlin's `toLongOrNull`, Dart's
  `int.tryParse`, and C's `strtoll` all accept it. Only C++'s `std::from_chars` refuses
  it. The other solutions check the form first, as in exercise 11.1.

**Stretch:** add a `date` kind, written as `2024-03-09`. How many lines did it take in
each language you tried, and did `merge` have to change?
