# Exercise 3.3: Division kinds

Each line of input holds two whole numbers `a` and `b`, with |a| < 2^31, |b| < 2^31,
and b ≠ 0. Print the quotient and remainder both ways that languages define integer
division:

```text
<a> <b> -> truncated <q> <r>, floored <q> <m>
```

- **Truncated** division rounds the quotient toward zero, as `/` and `%` do in C, C++,
  Rust, Go, and JavaScript. The remainder has the sign of `a`.
- **Floored** division rounds the quotient down, toward negative infinity, as `//` and
  `%` do in Python. The remainder has the sign of `b`.

In both kinds, `q * b + r == a`. Skip blank lines.

| Input | Output |
| --- | --- |
| `7 2` | `7 2 -> truncated 3 1, floored 3 1` |
| `-7 2` | `-7 2 -> truncated -3 -1, floored -4 1` |
| `7 -2` | `7 -2 -> truncated -3 1, floored -4 -1` |
| `-7 -2` | `-7 -2 -> truncated 3 -1, floored 3 -1` |

```sh
python3 tools/check.py 03 --lang dart
```

**Things to notice:**

- Each language gives you one kind for free. You have to build the other from it.
- **Rust's `div_euclid` and Dart's `%` are a third kind, Euclidean division**, where the
  remainder is never negative. It matches floored division when `b > 0` and differs
  when `b < 0`: try `7 -2`.
- **JavaScript** has no integer division at all. `Math.trunc(a / b)` and
  `Math.floor(a / b)` are safe here because the numbers are small. Watch for `-0`:
  `-6 % 3` is `-0`, which `console.log` prints as `-0` but a template string prints
  as `0`.
- **Why is −2^31 left out?** In C and C++, `INT_MIN / -1` is undefined behavior,
  because the answer, 2^31, does not fit in an `int`. On x86 it crashes the program.
  Rust panics, even in release builds, and Go defines the answer as `INT_MIN`.
