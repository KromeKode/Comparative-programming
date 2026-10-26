# Exercise 2.3: Checked add

Each line of input holds two whole numbers separated by a space. Both fit in a signed
64-bit integer: from −9,223,372,036,854,775,808 to 9,223,372,036,854,775,807. Print
their sum if it also fits in a signed 64-bit integer, and `overflow` if it does not.
Skip blank lines.

| Input | Output |
| --- | --- |
| `1 2` | `3` |
| `9223372036854775807 1` | `overflow` |
| `-9223372036854775808 9223372036854775807` | `-1` |
| `9007199254740993 0` | `9007199254740993` |

```sh
python3 tools/check.py 02 --lang javascript
```

**Things to notice:** every language needs a different trick.

- **C and C++** must check *before* adding, because the overflowing addition itself is
  undefined behavior.
- **Rust** has `checked_add`, which returns `None` on overflow.
- **Go and Dart** wrap around, so you can add first and then check the signs.
- **Python** never overflows, so you compare the exact sum with the 64-bit limits.
- **JavaScript and TypeScript** cannot even read these numbers with `Number`: the
  last test, 9,007,199,254,740,993, is past 2^53 and would come back as
  9,007,199,254,740,992. Use `BigInt`.
