# Exercise 6.3: Word frequency

Read the whole input and count how often each word appears. A **word** is a run of
letters, where a letter is an ASCII letter (`A` to `Z`, `a` to `z`) or any non-ASCII
character. Everything else separates words: spaces, line breaks, digits, and
punctuation. Lowercase the ASCII letters before counting, so `The` and `the` are the
same word. Leave non-ASCII characters as they are.

Print one line per word, `<count> <word>`, most frequent first. When two words have the
same count, put them in order by code point, which for these tests is ordinary
alphabetical order. If the input has no words, print nothing.

Input:

```text
Zoë met José at the café.
José paid for the café.
```

Output:

```text
2 café
2 josé
2 the
1 at
1 for
1 met
1 paid
1 zoë
```

```sh
python3 tools/check.py 06 --lang go
```

**Things to notice:**

- **Reading everything at once.** This is the first exercise that needs the whole
  input before it can print anything. `read_to_string` in Rust, `io.ReadAll` in Go,
  `sys.stdin.read()` in Python, `readFileSync(0, "utf8")` in JavaScript and
  TypeScript, and `readText()` in Kotlin all read to the end. In C, read with `getchar`
  until `EOF`.
- **Splitting on a rule, not a character.** Rust's `split` takes a closure, Go has
  `strings.FieldsFunc`, and JavaScript and Dart can match the words directly with a
  regular expression. A regex character class needs the `u` flag in JavaScript to treat
  an emoji as one character. Chapter 21 covers regular expressions properly.
- **Counting needs a map.** `HashMap` in Rust, `map[string]int` in Go, `Counter` in
  Python, `Map` in JavaScript, and `groupingBy { it }.eachCount()` in Kotlin. Chapter 7
  compares maps across the languages. C has none, so the solution keeps a growing array
  of words and searches it.
- **Sorting by two keys.** Compare by count, largest first, and only when the counts are
  equal, compare the words. Go's `cmp.Or` and Rust's `then_with` say that directly.
- **Which order is "alphabetical"?** Comparing UTF-8 bytes, as C, C++, Rust, and Go do,
  gives code point order. Comparing UTF-16 code units, as JavaScript, Dart, and Kotlin
  do, gives the same order except for characters above U+FFFF, such as emoji, which
  sort before characters from U+E000 to U+FFFF. Neither is the order a dictionary
  uses; for that you need a collator, such as `Intl.Collator` in JavaScript.

**Stretch:** print only the 10 most common words, and ignore a list of common words
such as `the`, `a`, `and`, and `of`.
