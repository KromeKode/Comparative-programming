# Exercise 7.3: Remove repeats, keep the order

For each line of input, print its words with every repeat removed: keep the **first**
time each word appears, drop the later ones, and leave the kept words in their original
order. Words are separated by spaces or tabs. Compare words exactly, so `Apple` and
`apple` are different words. Each line is handled on its own. Skip blank lines.

| Input | Output |
| --- | --- |
| `3 1 3 2 1` | `3 1 2` |
| `the cat and the hat sat on the mat` | `the cat and hat sat on mat` |
| `Apple apple APPLE apple Apple` | `Apple apple APPLE` |
| `café cafe café 🙂 🙂 naïve` | `café cafe 🙂 naïve` |

```sh
python3 tools/check.py 07 --lang kotlin
```

**Things to notice:**

- **A set answers "have I seen this?" in one step.** Walk the words, and print each one
  the set does not hold yet, then add it. Checking a list instead (`if word not in
  kept`) looks at every kept word each time. On a line of 100,000 words with about
  43,000 different ones, that took 18 seconds in Python on the machine that tested this
  book; the set version took 0.04 seconds.
- **Some languages do it in one call.** Their default set or map remembers insertion
  order, so turning the words into one keeps exactly the first occurrences:
  `[...new Set(words)]` in JavaScript, `words.toSet()` in Dart, `words.distinct()` in
  Kotlin, and `dict.fromkeys(words)` in Python. Python's `set(words)` would lose the
  order, because a Python `set` does not keep it, though a `dict` does.
- **Let `insert` tell you.** Adding to a set can report whether the item was new:
  `insert` returns a `bool` in Rust and `.second` of a pair in C++, and Dart's `add` and
  Kotlin's `add` return `true` for a new item. That saves a separate lookup.
- **Non-ASCII words need no special care.** Comparing whole words compares their bytes
  or code units, which is exact for `café` and `🙂`, as long as both copies of a word
  use the same encoding and normalization (section 6.7).
- **C builds its own set.** The C solution is a small hash set of strings: FNV-1a hashes
  each word to a slot, and a full slot sends it to the next one.

**Stretch:** remove repeats across the whole input instead of line by line, so that a
word printed on line 1 never appears again. What changes, and what has to live outside
the loop over lines?
