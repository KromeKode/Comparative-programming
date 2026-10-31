# Exercise 11.1: A strict integer parser

Each line of input is a piece of text. Parse it as a 32-bit signed integer, from
-2147483648 to 2147483647, and print the line in quotes, then `->`, then either the
number or an error. The text must be an optional `+` or `-` followed by one or more
of the digits `0` to `9`, and nothing else: no spaces, no `_`, no `.`, and no `0x`.
Leading zeros are fine, so `007` is 7 and `-0` is 0.

| Problem | Error |
| --- | --- |
| The line is empty | `error: empty input` |
| A sign with nothing after it | `error: no digits after the sign` |
| A character that is not allowed | `error: invalid character 'a' at position 3` |
| More than 2147483647 | `error: too large, the maximum is 2147483647` |
| Less than -2147483648 | `error: too small, the minimum is -2147483648` |

Positions count characters from 1, the sign included. Report the first character that
is not allowed, even when the digits before it are already too large, so
`99999999999999999999x` is an invalid character and not too large. The tests use ASCII
only.

| Input | Output |
| --- | --- |
| `42` | `"42" -> 42` |
| `+007` | `"+007" -> 7` |
| (an empty line) | `"" -> error: empty input` |
| `12a4` | `"12a4" -> error: invalid character 'a' at position 3` |
| `-2147483649` | `"-2147483649" -> error: too small, the minimum is -2147483648` |

Write the parser as a function that takes the text and returns the number or an error.
The function must not print anything, and must not call your language's own number
parser: you saw in Exercise 8.2 that those accept more than this one may. Give the
error a type with one case for each row of the first table, and turn it into a message
in one place, outside the parser.

```sh
python3 tools/check.py 11 --lang rust
```

**Things to notice:**

- **Pick the error type your language expects.** The reference solutions use an enum
  with `Display` in Rust, sentinel errors plus an `InvalidCharacterError` struct in Go,
  a struct holding an error enum in C, one exception class with a `kind()` in C++,
  exception classes in Python, JavaScript, Dart, and Kotlin, and a union of tagged
  objects returned as a result in TypeScript. Only the invalid character case carries
  data, so only it needs fields.
- **The number can overflow while you build it.** `4294967296` is 2^32, so a 32-bit
  total wraps around to 0 and looks valid. `18446744073709551658` is 2^64 + 42, so it
  fools a 64-bit total the same way. Build the total in a 64-bit integer and stop it
  growing once it is past the limits, or check before each step. Python's integers never
  overflow, and JavaScript's numbers stay exact up to 2^53, which is far past the
  limits, but only if you stop adding digits once the total is too large.
- **The two limits are not mirror images.** -2147483648 fits but 2147483648 does not.
  If you build the positive magnitude and negate it at the end, compare it with
  2147483648 for a negative number and 2147483647 for a positive one.
- **Make the compiler check the cases.** A `switch` or `match` over the error kinds that
  leaves one out is a compile error in Rust, Dart, Kotlin, and TypeScript, and a
  warning with `-Wall` in C and C++. Put the messages in such a `switch` and the
  compiler tells you when you add a kind and forget its message.

**Stretch:** add a second argument, the base, from 2 to 36, so that `ff` in base 16 is
255. Which error needs a new case, and which messages need to change?
