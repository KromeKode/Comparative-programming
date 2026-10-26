# Exercise 3.1: Leap year

Each line of input holds a year from 1 to 9999. Print whether it is a leap year in the
Gregorian calendar:

```text
<year>: leap year
<year>: common year
```

A year is a leap year if it is divisible by 4, except that years divisible by 100 are
not, except that years divisible by 400 are. Skip blank lines.

| Input | Output |
| --- | --- |
| `2024` | `2024: leap year` |
| `2023` | `2023: common year` |
| `1900` | `1900: common year` |
| `2000` | `2000: leap year` |

```sh
python3 tools/check.py 03 --lang go
```

**Things to notice:** write the whole rule as one boolean expression, using `%`, `==`,
`&&`, and `||` (`and` and `or` in Python). Which parentheses are needed, and which
are only there to help the reader? In every one of the eight languages `&&` binds
tighter than `||`, so `a && b || c` means `(a && b) || c`.

Python's standard library already has this as `calendar.isleap(year)`. Write it
yourself first, then compare.
