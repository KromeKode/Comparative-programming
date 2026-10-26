# Exercise 3.2: Bit count

Each line of input holds a whole number `n` that fits in an unsigned 32-bit integer:
0 ≤ n ≤ 4,294,967,295. Print `n` in binary, how many of its bits are 1, and its width,
the number of binary digits it needs:

```text
<n>: <n in binary> (ones <count of 1 bits>, width <binary digits>)
```

Write the binary form without leading zeros. Zero is a special case: it prints as `0`,
but its width is 0, because it needs no digits at all. Skip blank lines.

| Input | Output |
| --- | --- |
| `10` | `10: 1010 (ones 2, width 4)` |
| `255` | `255: 11111111 (ones 8, width 8)` |
| `0` | `0: 0 (ones 0, width 0)` |
| `4294967295` | `4294967295: 11111111111111111111111111111111 (ones 32, width 32)` |

```sh
python3 tools/check.py 03 --lang rust
```

**Things to notice:**

- Most languages have all three as built-ins: `{:b}`, `count_ones`, and
  `leading_zeros` in Rust; `%b`, `bits.OnesCount32`, and `bits.Len32` in Go;
  `std::format("{:b}")`, `std::popcount`, and `std::bit_width` in C++20; `:b`,
  `bit_count`, and `bit_length` in Python. Look them up before writing loops.
- **C** has no binary format in C17's `printf` and no standard popcount before C23.
  Write the loops. A neat trick: `n & (n - 1)` clears the lowest 1 bit, so counting
  how many times you can do that before reaching 0 counts the ones.
- **JavaScript and TypeScript** convert to a *signed* 32-bit integer for every bitwise
  operator. Numbers from 2,147,483,648 up come out negative. Try your ones-counting
  loop with `>>` on 4294967295 and see what happens, then use `>>>`.
