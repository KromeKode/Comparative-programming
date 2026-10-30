# Exercise 4.2: Number guessing game

The classic first game: the program picks a secret number and the player guesses until
they find it. To make it testable, the secret comes from the input instead of a random
number generator.

The first line of input is the secret, a whole number. Every line after it is a guess.
For each line, after trimming spaces:

| The line is | Print | Counts as a guess? |
| --- | --- | --- |
| Blank | nothing | no |
| Not a whole number | `<text>: not a number` | no |
| Lower than the secret | `<guess>: too low` | yes |
| Higher than the secret | `<guess>: too high` | yes |
| The secret | `<guess>: correct in <n> guesses`, then stop reading | yes |

If the input runs out before a correct guess, print
`gave up after <n> guesses; the number was <secret>`. Write `1 guess`, not
`1 guesses`, in both messages.

Input:

```text
60
fifty
100
60
```

Output:

```text
fifty: not a number
100: too high
60: correct in 2 guesses
```

```sh
python3 tools/check.py 04 --lang rust
```

**Things to notice:**

- **Stopping early.** After the correct guess, ignore the rest of the input. That is a
  `break` out of the loop, or a `return`. In Go, a `break` inside a `switch` only
  leaves the `switch`, so it will not stop the loop.
- **"The loop ended without finding it."** Python's `for ... else` runs the `else`
  block only when the loop finishes without a `break`, which is exactly the "gave up"
  case. Other languages need a flag or an early `return`.
- **Comparing once.** Rust's `guess.cmp(&secret)` returns `Less`, `Greater`, or `Equal`,
  and a `match` must handle all three. C++20's `<=>` and the `compareTo` methods of Dart
  and Kotlin do the same job. In Kotlin, a `when` with no subject is often clearer:
  `guess < secret ->`, `guess > secret ->`, `else ->`.
- **What counts as a number differs.** The tests use only plain integers like `-4` and
  `100`, and plain non-numbers like `fifty` and `12abc`. Beyond that the parsers
  disagree: Python's `int("1_000")` is 1000, JavaScript's `Number("0x10")` is 16 and
  `Number("1e2")` is 100, and C's `strtol` stops at the first character it cannot
  use, so you must check that it used the whole line.

**Stretch:** make it a real game. Pick the secret at random from 1 to 100, read
guesses from the keyboard, and print a prompt before each one.
