# Exercise 2.1: Temperature converter

Each line of input holds a temperature: a number, a space, and a unit, either `C`
(Celsius) or `F` (Fahrenheit). Convert it to the other unit and print one line:

```text
<input with 1 decimal> <unit> = <result with 1 decimal> <other unit>
```

Use F = C × 9 / 5 + 32 and C = (F − 32) × 5 / 9. Skip blank lines. The number can be
negative and can have a decimal point.

| Input | Output |
| --- | --- |
| `100 C` | `100.0 C = 212.0 F` |
| `98.6 F` | `98.6 F = 37.0 C` |
| `0 F` | `0.0 F = -17.8 C` |

```sh
python3 tools/check.py 02 --lang python
```

**Things to notice:** every language has a different way to print exactly one decimal
place: `%.1f` in C and Go, `std::fixed` with `std::setprecision(1)` in C++, `{:.1}` in
Rust, `:.1f` in Python f-strings, `toFixed(1)` in JavaScript, `toStringAsFixed(1)` in
Dart, and `String.format(Locale.ROOT, "%.1f", x)` in Kotlin. Which ones change the
number into a string, and which only change how it is printed? Why does Kotlin need
`Locale.ROOT`?
