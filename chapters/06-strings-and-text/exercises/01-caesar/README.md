# Exercise 6.1: Caesar cipher

A Caesar cipher hides a message by moving every letter a fixed number of places along
the alphabet. With a shift of 3, `a` becomes `d`, `b` becomes `e`, and `z` wraps around
to `c`. Julius Caesar is said to have used a shift of 3. ROT13, a shift of 13, is still
used online to hide spoilers, because applying it twice gives back the original.

Each line of input holds a shift, one space, and some text. Shift every ASCII letter
(`A` to `Z` and `a` to `z`) by that many places, wrapping around and keeping its case.
Leave everything else exactly as it is: spaces, digits, punctuation, and every
non-ASCII character such as `ë` or an emoji. Print the result.

The shift is a whole number. It can be negative, which moves letters backward, and it
can be larger than 26. Ignore spaces at the ends of a line, and skip blank lines.

| Input | Output |
| --- | --- |
| `3 Hello, World!` | `Khoor, Zruog!` |
| `13 Hello, World!` | `Uryyb, Jbeyq!` |
| `-1 abc` | `zab` |
| `29 abc` | `def` |
| `1 Zoë and José` | `Apë boe Kpté` |

```sh
python3 tools/check.py 06 --lang c
```

**Things to notice:**

- **Turning a letter into a number and back.** Every language does it differently:
  `c - 'a'` and `'a' + n` in C, C++, and Kotlin (where a `Char` minus a `Char` is an
  `Int`); `c as u8` and `n as char` in Rust; runes in Go; `ord` and `chr` in Python;
  `charCodeAt` and `String.fromCharCode` in JavaScript; `codeUnitAt` and
  `String.fromCharCodes` in Dart.
- **Negative shifts meet chapter 3.** `-1 % 26` is `-1` in C, C++, Rust, Go,
  JavaScript, and Kotlin, which is not a letter position. Turn the shift into 0 to 25
  first with `((shift % 26) + 26) % 26`, `rem_euclid` in Rust, or `mod` in Kotlin.
  Python's and Dart's `%` already give 25.
- **Why non-ASCII text survives in every language.** UTF-8 encodes every non-ASCII
  character with bytes from 0x80 up, and UTF-16 encodes it with code units from 0x80
  up, so no part of `ë` or an emoji can be mistaken for a letter. That lets C walk the
  bytes, and JavaScript, Dart, and Kotlin walk UTF-16 code units, without breaking
  anything. Exercise 2 is where that shortcut stops working.
- **Splitting at the first space only.** The text can contain spaces, so split the line
  once: `split_once` in Rust, `strings.Cut` in Go, `partition` in Python,
  `substringBefore` and `substringAfter` in Kotlin, or `strtol`'s end pointer in C.

**Stretch:** a Caesar cipher is easy to break, because there are only 26 shifts. Write
a program that tries all 26 on a line and prints the one with the most common English
letters (`e`, `t`, `a`, `o`).
