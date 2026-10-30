# Exercise 5.1: Memoized Fibonacci

The Fibonacci numbers start with 0 and 1, and each one after that is the sum of the
two before it: 0, 1, 1, 2, 3, 5, 8, 13, 21, ... As a function:

```text
fib(0) = 0
fib(1) = 1
fib(n) = fib(n - 1) + fib(n - 2)
```

Each line of input holds a whole number `n` from 0 to 92. Print:

```text
fib(<n>) = <value>
```

Skip blank lines.

| Input | Output |
| --- | --- |
| `0` | `fib(0) = 0` |
| `10` | `fib(10) = 55` |
| `50` | `fib(50) = 12586269025` |
| `92` | `fib(92) = 7540113804746346429` |

Write `fib` as a recursive function that reads like the definition. Then make it fast
with a **memo**: a table of answers already worked out. Before working out `fib(n)`,
look it up in the table. After working it out, store it.

```sh
python3 tools/check.py 05 --lang javascript
```

**Things to notice:**

- **Why the memo matters.** Without it, `fib(n)` calls `fib(n - 1)` and `fib(n - 2)`,
  and each of those makes two more calls. `fib(40)` makes 331,160,281 calls, and each
  step up in `n` multiplies that by about 1.6. The test named
  `02-too-slow-without-a-memo` asks for `fib(60)`, which would take trillions of calls.
  With a memo, each `n` is worked out once.
- **Where the memo lives.** It has to outlast each call, so it cannot be a local
  variable inside `fib`. C can use a `static` array, and C++ a `static` local. Go can
  capture a map in a closure. Rust passes the map in as a `&mut` parameter. Python's
  `@functools.cache` keeps the table for you.
- **The answers need 64 bits.** `fib(92)` is 7,540,113,804,746,346,429, the largest
  Fibonacci number that fits in a signed 64-bit integer. Use `uint64_t` in C and C++,
  `u64` in Rust, and `uint64` in Go. Dart's `int` is 64 bits.
- **JavaScript numbers are not enough.** They stop holding every whole number exactly
  after 2^53, and `fib(79)` is the first Fibonacci number past it. Use `BigInt`:
  `BigInt(n)` turns a number into one, and `+` works between two of them.

**Stretch:** write `fib` as a loop that keeps only the last two values. Then try
writing `memoize(f)`, which takes any one-argument function and returns a version
with a memo. Wrap your plain recursive `fib` in it as `fast = memoize(fib)`. Why is
the first `fast(60)` still slow? (Python's `@cache` avoids this because the decorator
rebinds the name `fib` itself, so the recursive calls find the memoized version.)
