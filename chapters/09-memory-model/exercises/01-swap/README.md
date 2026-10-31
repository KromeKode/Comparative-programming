# Exercise 9.1: Swap

The first line of input is a row of words separated by spaces. Each line after it is a
command that rearranges the words:

| Command | Effect |
| --- | --- |
| `swap I J` | Exchange the words at positions I and J. I and J may be the same. |
| `reverse I J` | Reverse the words from position I to position J, both included. If I is greater than J, reverse from J to I. |

Positions count from 1. After each command, print the words separated by single spaces.
When the input ends, print `swaps: N`, where N is how many times your swap function
ran. Each `swap` runs it once, even when I and J are the same. `reverse` must do its
work with your swap function, swapping the outermost pair first and working inward, so
reversing K words runs it K / 2 times, rounded down.

If a command is wrong, print one of these errors, leave the words as they were, and
carry on. Check the positions from left to right and report the first problem.

| Problem | Error |
| --- | --- |
| Not `swap` or `reverse` | `error: unknown command "flip"` |
| Not exactly two positions | `error: swap needs 2 positions, got 1` |
| A position that is not all digits | `error: "-1" is not a position` |
| A position outside the words | `error: position 4 is out of range (1 to 3)` |

The out-of-range error repeats the position exactly as it was written, so `007` stays
`007`, and a position too big for any integer type is out of range too. Skip blank lines,
and allow any number of spaces between the parts of a line. If there is no input at
all, print `swaps: 0`. The tests use ASCII only, with at most 100 words on a line of
fewer than 1,000 characters.

| Input | Output |
| --- | --- |
| `the quick brown fox jumps` | |
| `swap 1 5` | `jumps quick brown fox the` |
| `swap 2 4` | `jumps fox brown quick the` |
| `reverse 1 5` | `the quick brown fox jumps` |
| | `swaps: 4` |

```sh
python3 tools/check.py 09 --lang c
```

**Things to notice:**

- **Write `swap` as its own function, and give it what your language allows.** C has to
  pass addresses, `void swap(char **a, char **b)`, called as `swap(&words[i],
  &words[j])`. Each word is a `char *`, so the swap exchanges two pointers and no
  characters move. C++ can take two references, `void swap(std::string &a, std::string
  &b)`. Go can take two pointers, `swap(&words[i], &words[j])`. Python, JavaScript,
  TypeScript, Dart, and Kotlin cannot reach the caller's variables at all, only the
  objects they refer to, so their swap takes the list and two indexes.
- **Rust refuses the obvious version.** `std::mem::swap(&mut words[i], &mut words[j])`
  borrows `words` mutably twice at once, and the compiler stops with `error[E0499]:
  cannot borrow 'words' as mutable more than once at a time`, because it cannot tell
  whether `i` and `j` are equal. Its help line suggests `split_at_mut`, which splits a
  slice into two halves that cannot overlap. Or take the slice and two indexes, as the
  built-in `slice::swap` does.
- **Swapping a word with itself must change nothing.** A swap through a temporary
  variable gets this right for free. The XOR trick from section 9.6 does not: used on
  one place twice, it leaves zero there.
- **In C, keep the first line's buffer alive.** `strtok` splits the line in place, so
  every word is a pointer into that buffer. Read the commands into a second buffer, or
  the next `fgets` writes over your words.

**Stretch:** add `rotate K`, which moves every word K places to the right and wraps
around the end. It can be done with three reverses: reverse the whole row, then the
first K words, then the rest.
