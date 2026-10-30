# Exercise 1.3: Sum lines

Read whole numbers from stdin, one per line, until the input ends. Print their sum.

- Numbers can be negative.
- Ignore blank lines, and spaces around a number.
- The last line may or may not end with a newline.
- Each number and the total fit in a signed 64-bit integer. **They do not always fit
  in 32 bits.** One test adds 3,000,000,000 + 3,000,000,000.
- No input at all prints `0`.

| Input | Output |
| --- | --- |
| `1`, `2`, `3` on three lines | `6` |
| `3000000000` twice | `6000000000` |

```sh
python3 tools/check.py 01 --lang c
```

**Things to notice:** in C, C++, Rust, Go, Dart, and Kotlin you have to pick an integer
size. Try `int` in C, `i32` in Rust, or `Int` with `toInt()` in Kotlin and watch the big
test fail. Python integers never
overflow. JavaScript numbers are 64-bit floats, exact for whole numbers up to 2^53.
Chapter 2 covers all of this in detail.
