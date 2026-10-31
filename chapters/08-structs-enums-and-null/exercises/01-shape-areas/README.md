# Exercise 8.1: Shape areas

Each line of input names a shape and gives its sizes, separated by spaces or tabs:

| Line | Shape | Area |
| --- | --- | --- |
| `circle R` | A circle of radius R | π × R² |
| `rectangle W H` | A rectangle W wide and H high | W × H |
| `triangle B H` | A triangle with base B and height H | B × H ÷ 2 |

For each line, print the shape's name and its area with two digits after the decimal
point. After the last line, print the total of all the areas the same way. Skip blank
lines.

A line that is not a valid shape prints an error instead and adds nothing to the total.
Check in this order, and report only the first problem:

1. The name is not `circle`, `rectangle`, or `triangle` (names are lowercase, so
   `Circle` is unknown): `error: unknown shape "hexagon"`
2. The wrong number of sizes: `error: circle needs 1 size, got 2` or
   `error: rectangle needs 2 sizes, got 1`
3. A size that is not a number, checking from left to right:
   `error: "3cm" is not a number`
4. A size below zero: `error: "-2" is negative`

The tests use plain decimal numbers such as `2`, `1.5`, and `0.25`.

| Input | Output |
| --- | --- |
| `circle 1` | `circle: 3.14` |
| `rectangle 3 4` | `rectangle: 12.00` |
| `triangle 6 2` | `triangle: 6.00` |
| `hexagon 2` | `error: unknown shape "hexagon"` |
| `rectangle 2 3cm` | `error: "3cm" is not a number` |
| (end of input) | `total: 21.14` |

```sh
python3 tools/check.py 08 --lang rust
```

**Things to notice:**

- **Parse first, then compute.** Turn each line into a `Shape` value, a sum type with
  one variant per kind of shape (section 8.4), and write `area` as a single match or
  switch over it. `area` never sees text, and the parser never does arithmetic.
- **The compiler can check the match.** Add a fourth shape to your `Shape` type without
  touching `area`, and Rust, C++, TypeScript, Dart, and Kotlin refuse to compile until
  you handle it. C warns (an error here, with `-Werror`) when the match is a `switch`
  on an enum. Go, Python, and JavaScript say nothing.
- **Some number parsers read only the start.** JavaScript's `parseFloat("3cm")` is `3`,
  while `Number("3cm")` is `NaN`. C's `strtod` and C++'s `std::stod` and
  `std::from_chars` all stop at the first character that does not fit and tell you
  where they stopped, so check that they used the whole word.
- **An error is a sum type too.** A line gives either a shape or a message. Rust says so
  with `Result<Shape, String>`, Go returns a shape and an `error`, and the other
  languages throw an exception. Chapter 11 compares these.

**Stretch:** add `square S` (area S × S). Make the change in `Shape` first and let the
compiler, if yours checks, list the places that need a new case.
