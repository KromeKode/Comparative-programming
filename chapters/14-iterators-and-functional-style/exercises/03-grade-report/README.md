# Exercise 14.3: Grade report

Each line of input is a student's name, a subject, and the student's score in it, from 0
to 100. Print a report in three parts.

```text
ada art 91                            subjects:
ada math 70                             art: average 84, best 91 (ada)
bob art 75                              math: average 82, best 88 (cy, dee)
bob music 64                            music: average 66, best 89 (ada)
cy art 86                             ranking:
cy math 88                              1. ada 250
dee math 88                             2. cy 240
dee music 45                            3. bob 139
cy music 66                             4. dee 133
ada music 89                          passed everything: ada, bob, cy
```

- **subjects:** a line for each subject, in alphabetical order, with the average score,
  rounded down, and the best score, followed by the names of everyone who got it, in
  alphabetical order.
- **ranking:** every student, numbered from 1, with the total of their scores, highest
  total first. Students with the same total are listed in alphabetical order, each with
  their own number.
- **passed everything:** the students whose every score is 50 or more, in alphabetical
  order, separated by commas, or `nobody`.

Names and subjects are lowercase words, and a student has at most one score in each
subject, but not every student takes every subject. The input has at least one line.
Skip blank lines, and allow any number of spaces between words. The tests use lines
under 256 characters and at most 1,000 lines.

Write the report as pipelines where your language has the tools: group, map, filter,
sort, and join, rather than loops that keep running totals. Each solution has a group-by
of its own, used once for the subjects and once for the students.

```sh
python3 tools/check.py 14 --lang rust
```

**Things to notice:**

- **Grouping.** Kotlin's `groupBy` and JavaScript's `Map.groupBy` (ES2024) take a
  function that gives each item's key, and return a map from each key to a list of its
  items. Dart's core library has no group-by, though `package:collection` has `groupBy`,
  so the solution fills a map, and `(groups[key] ??= []).add(grade)` makes each list the
  first time its key turns up. Rust's iterators have no group-by either, and its
  solution folds the grades into a `BTreeMap` with
  `entry(key).or_insert_with(Vec::new)`. Python's `itertools.groupby` only groups items
  that sit next to each other, so it needs sorted input, and the solution fills a
  `defaultdict(list)` instead. Go and C++ fill a map in a loop, and the C solution sorts
  the grades and walks along them, which is `itertools.groupby` done by hand.
- **Sorted keys.** A Rust `BTreeMap` and a C++ `std::map` keep their keys sorted, so the
  subjects and names come out in order. Kotlin's `groupBy` keeps the order in which keys
  first appear, and `toSortedMap` sorts them. Python's dictionaries, JavaScript's `Map`,
  and Dart's default `Map` keep that order too, so those solutions sort. A Go map has no
  order, and ranging over the same map twice can give two orders, so the Go solution
  ranges over `slices.Sorted(maps.Keys(subjects))`.
- **Sorting by two keys.** Highest total first, then by name, is one comparison in most
  languages. Python sorts by the tuple `(-total, name)`, and Rust by
  `(Reverse(total), name)`. Kotlin writes
  `compareByDescending { it.second }.thenBy { it.first }`, and Go passes `cmp.Or` two
  comparisons, of the totals with b first and then of the names, and `cmp.Or` returns
  the first one that isn't 0. JavaScript and TypeScript write
  `b.total - a.total || byText(a.name, b.name)`, since `||` skips a 0 too. C++ compares
  `std::tie(b.total, a.name) < std::tie(a.total, b.name)`, with a and b swapped for the
  total, and C writes a compare function for `qsort`.
- **"Every score" is `all`.** Python's `all`, Rust's and Kotlin's `all`, JavaScript's
  `every`, Dart's `every`, and C++'s `std::ranges::all_of` stop at the first score under
  50. Go has `slices.ContainsFunc`, which answers the opposite question: is any score
  under 50?

**Stretch:** add a fourth part, `best subjects:`, with a line for each student naming
the subject of their highest score, such as `ada: art`, and every subject on a tie, in
alphabetical order. How much of the code you already have can it reuse?
