# Exercise 6.2: Unicode-safe palindromes

A **palindrome** reads the same forwards and backwards: `racecar`, or
`A man, a plan, a canal: Panama` once you ignore case, spaces, and punctuation.

For each line of input, decide whether it is a palindrome under these rules:

1. Drop every ASCII character that is not a letter or a digit, such as spaces,
   commas, and apostrophes.
2. Lowercase the ASCII letters `A` to `Z`.
3. Keep every non-ASCII character exactly as it is, and treat each **code point** as
   one character. `é` and `🙂` each count as one character, however many bytes or code
   units they take.

Then compare what is left with its reverse. Print the line, with spaces at its ends
removed, followed by `: palindrome` or `: not a palindrome`. Skip blank lines. A line
with nothing left after step 1, such as `!!!`, counts as a palindrome: there is nothing
to disagree.

| Input | Output |
| --- | --- |
| `racecar` | `racecar: palindrome` |
| `A man, a plan, a canal: Panama` | `A man, a plan, a canal: Panama: palindrome` |
| `été` | `été: palindrome` |
| `Zoë` | `Zoë: not a palindrome` |
| `🙂x🙂` | `🙂x🙂: palindrome` |

```sh
python3 tools/check.py 06 --lang javascript
```

**Things to notice:**

- **Reversing the wrong units gives the wrong answer.** In UTF-8, `été` is the bytes
  `C3 A9 74 C3 A9`. Reversed, that is `A9 C3 74 A9 C3`, which is not the same, so a
  byte-by-byte check in C says `été` is not a palindrome. In JavaScript, Dart, and
  Kotlin, `🙂` is two UTF-16 code units, and reversing them swaps the two halves, so
  `🙂x🙂` fails the same way. The tests `03-accents` and `04-emoji` catch both mistakes.
- **Get code points first.** `chars()` in Rust, `[]rune` in Go, a plain `str` in
  Python, `[...text]` in JavaScript, `runes` in Dart, and `codePoints()` in Kotlin.
  C and C++ have to decode the UTF-8 themselves; `decode_utf8` from example
  02-code-points does it.
- **Only ASCII gets lowercased.** Rust's `to_ascii_lowercase`, and the `isascii` or
  `> 0x7F` checks in the other solutions, keep the rule exact. Full Unicode case folding is
  harder than it looks, and C's `tolower` only knows ASCII unless you set a locale.
- **Code points are still not characters.** `é` can also be written as `e` followed by
  U+0301 COMBINING ACUTE ACCENT. Reverse that by code points and the accent lands on
  the wrong letter. The tests avoid it, and section 6.1 explains it.

**Stretch:** check palindromes with two indexes that walk toward each other, skipping
characters that step 1 drops, instead of building a filtered copy. It uses no extra
memory. Which languages make walking backwards through code points easy?
