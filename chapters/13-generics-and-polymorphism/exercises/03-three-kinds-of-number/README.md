# Exercise 13.3: Three kinds of number

Each line of input is a kind of number followed by a sum written in postfix notation,
also called reverse Polish notation: each operator comes after the two numbers it works
on, so `1 2 + 3 *` means (1 + 2) × 3. Work through the tokens from left to right with a
stack. Push each number. For an operator, pop the right-hand number, then the left-hand
one, and push the result. At the end, the stack must hold exactly one number, and that
is the answer to print.

| Kind | Numbers in the input | `/` | Printed as |
| --- | --- | --- | --- |
| `int` | Whole numbers | Divides and rounds toward zero, so `-7 2 /` is -3 | `-3` |
| `frac` | Whole numbers, or fractions such as `3/4` or `-1/2` | Exact | In lowest terms, with the sign on top and no `/1`: `5/6`, `-3/2`, `2` |
| `mod7` | Whole numbers, kept as their remainder after dividing by 7, from 0 to 6 | Multiplies by the divisor's inverse, the number that gives 1 when multiplied by it: `1 2 /` is 4, because 2 × 4 is 8, which is 1 more than 7 | `4` |

In `mod7`, the other operators wrap around too: `6 1 +` is 0, `3 4 -` is 6, `3 5 *` is
1, and `-1` is 6.

| Input | Output |
| --- | --- |
| `int 1 2 /` | `0` |
| `frac 1 2 /` | `1/2` |
| `mod7 1 2 /` | `4` |
| `frac 1 2 / 1 3 / +` | `5/6` |
| `int 1 2 + 3 *` | `9` |
| `int 1 0 /` | `error: division by zero` |
| `int 1 +` | `error: not enough numbers` |

The other errors are `error: unknown kind "KIND"`, `error: bad token "TOKEN"` when a
token is neither an operator nor a number of the line's kind, and
`error: expected one result, found N` when the stack does not end with exactly one
number. In `mod7`, dividing by 7 or any multiple of it is a division by zero. Stop at
the first error on a line.

A whole number is an optional minus sign and at least one digit, with no plus sign. A
fraction is a whole number, optionally followed by `/` and a whole number above zero,
with no sign and no spaces: `-6/4` means -3/2, but `1/-2` and `1/0` are bad tokens. Skip
blank lines, and allow any number of spaces between tokens. The tests use ASCII only,
and every number in them, including every numerator and denominator along the way, is at
most 1,000,000 in size. So the product of two of them fits in 64 bits, and fits exactly
in a JavaScript number.

Write `eval` once, generic over the kind of number, and a type for each kind with a way
to parse a token, the four operations, and a way to print. `eval` must never check which
kind it has.

```sh
python3 tools/check.py 13 --lang typescript
```

**Things to notice:**

- **A constraint that names the type itself.** `eval` needs `a + b` to take two numbers
  of the same kind and give back a third. A Rust trait says so with `Self`:
  `fn add(&self, other: &Self) -> Self`. Go writes
  `type Number[T any] interface { Add(T) T; ... }`, and `eval` asks for `T Number[T]`.
  Kotlin, TypeScript, and Dart write `N extends Arithmetic<N>` (`N : Arithmetic<N>` in
  Kotlin), C++'s concept asks for `{ a + b } -> std::same_as<N>`, and Python's
  `Protocol` uses `Self`.
- **Parsing has no number to call it on.** `eval` turns a token into an N before it has
  any N. Rust's trait has `fn parse(text: &str) -> Option<Self>`, with no `self`, and
  `eval` calls it as `N::parse(token)`. C++'s concept asks for a static member,
  `N::parse(text)`. Go, Kotlin, TypeScript, JavaScript, Dart, and Python can't call a
  function on a type parameter, so the caller passes the parser in. Go and Python pass a
  function, and Dart passes a static method as a value, `Int.parse`. Kotlin passes the
  class's companion object, `eval(tokens, Frac)`, and TypeScript and JavaScript pass the
  class itself, whose static `parse` is all `eval` asks for.
- **Two kinds of remainder.** `-1 % 7` is -1 in C, C++, Rust, Go, JavaScript,
  TypeScript, and Kotlin, but 6 in Python and Dart. Rust's `rem_euclid` and Kotlin's
  `mod` give the one that is never negative. The other solutions add 7 and take `%`
  again.
- **Two kinds of rounding.** `-7 2 /` is -3. Python's `//` rounds down and gives -4, so
  the Python `Int` divides the sizes and then fixes the sign. Dart uses `~/`, which
  rounds toward zero, and JavaScript and TypeScript use `Math.trunc(a / b)`. C, C++,
  Rust, Go, and Kotlin round toward zero already.
- **C passes the table by hand.** In C, a kind is a struct of function pointers,
  `parse`, `add`, `sub`, `mul`, `div`, and `print`, and `eval` takes a pointer to one.
  Rust and C++ do this job at compile time: they make a copy of `eval` for each kind,
  and each copy calls that kind's functions directly.

**Stretch:** add `mod11`. Then try `mod N`, where the line gives any prime N, as in
`mod 11 3 4 /`. Why does a modulus that arrives at run time not fit in a type parameter,
and where does it go instead?
