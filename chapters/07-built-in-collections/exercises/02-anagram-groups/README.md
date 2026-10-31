# Exercise 7.2: Anagram groups

Two words are **anagrams** if they use the same letters the same number of times:
`listen` and `silent`, or `care`, `race`, and `acre`.

Read every word from the input. Words are runs of ASCII letters separated by spaces or
line breaks. Lowercase each word, then put the words into groups of anagrams, and print
one group per line with its words separated by spaces.

- Print the groups in the order their **first** word appeared.
- Inside a group, keep the words in the order they appeared, repeats included.
- A word with no anagram is a group of one.
- If there are no words, print nothing.

| Input | Output |
| --- | --- |
| `eat tea tan ate nat bat` | `eat tea ate`, then `tan nat`, then `bat` |
| `Listen Silent enlist Google` | `listen silent enlist`, then `google` |
| `aab abb ab ba aba` | `aab aba`, then `abb`, then `ab ba` |

```sh
python3 tools/check.py 07 --lang rust
```

**Things to notice:**

- **A key that all anagrams share.** Sort the letters of a word, and every anagram
  gives the same string: `eat`, `tea`, and `ate` all become `aet`. Use that as a map key
  and the list of words as the value. Counting letters also works: 26 counts, joined
  into a string or packed into a tuple, make a key that takes no sorting.
- **Which maps remember order.** Python's `dict`, JavaScript's `Map`, Dart's map
  literals, and Kotlin's `mutableMapOf` and `groupBy` keep keys in the order they were
  added, so printing the map's values in order is the whole answer. C++'s
  `unordered_map`, Rust's `HashMap`, and Go's maps do not. Their solutions map each key
  to a group number and keep the groups in a list. A sorted map (`std::map` or
  `BTreeMap`) is ordered too, but by key, which is the wrong order: the test
  `07-group-order` catches it.
- **The one-call versions.** Kotlin's `groupBy { key }` does the whole job. Python's
  `setdefault(key, []).append(word)`, Dart's `putIfAbsent(key, () => [])`, Kotlin's
  `getOrPut`, and Rust's `entry(key).or_insert_with(...)` each look up a group and
  create it if it is missing, in one step.
- **C groups by sorting.** With no map, the C solution sorts the words by key, so each
  group's words end up next to each other, and then sorts the groups by where their
  first word appeared.

**Stretch:** print only the groups with two or more words, largest group first, and
groups of the same size in order of first appearance. That needs a **stable** sort,
one that keeps equal items in their original order. Python's `sorted`, JavaScript's
`sort` (since ES2019), Kotlin's `sortedBy`, Rust's `sort`, C++'s `std::stable_sort`, and
Go's `slices.SortStableFunc` are stable. C's `qsort`, C++'s `std::sort`, Rust's
`sort_unstable`, Go's `slices.SortFunc`, and Dart's `sort` make no such promise.
