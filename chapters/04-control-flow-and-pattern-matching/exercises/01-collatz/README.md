# Exercise 4.1: Collatz length

Start with a whole number `n`. If it is even, halve it. If it is odd, replace it with
`3n + 1`. Repeat until you reach 1. The Collatz conjecture says you always do, from
any start, and nobody has managed to prove it.

Each line of input holds a starting number from 1 to 1,000,000. Print how many steps
it takes to reach 1, and the largest value seen along the way, including the start:

```text
<start>: steps <count>, peak <largest value>
```

Skip blank lines.

| Input | Output |
| --- | --- |
| `1` | `1: steps 0, peak 1` |
| `6` | `6: steps 8, peak 16` |
| `27` | `27: steps 111, peak 9232` |
| `704511` | `704511: steps 242, peak 56991483520` |

For 6 the values are 6, 3, 10, 5, 16, 8, 4, 2, 1: eight steps, and the peak is 16.

```sh
python3 tools/check.py 04 --lang c
```

**Things to notice:**

- This is a `while` loop: you do not know in advance how many steps it will take.
- The last test climbs to 56,991,483,520, which does not fit in 32 bits. In C, C++,
  Rust, Go, and Kotlin, choose a 64-bit type for the running value. What does your C version
  print if you use `int`?
- JavaScript numbers are exact up to 2^53, so they are fine here. Dart's `int` is
  64 bits natively.
- Rust's and Kotlin's `if` is an expression, so the step can be one line:
  `n = if n % 2 == 0 { n / 2 } else { 3 * n + 1 };` in Rust, and
  `n = if (n % 2 == 0L) n / 2 else 3 * n + 1` in Kotlin.
