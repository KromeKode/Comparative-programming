# Exercise 4.3: Roman numerals

Each line of input holds either a whole number from 1 to 3999, or a valid Roman
numeral in capital letters for a number in that range. Convert it the other way:

```text
<number> -> <roman numeral>
<roman numeral> -> <number>
```

Skip blank lines. A line is a number if it is made of digits.

| Letter | I | V | X | L | C | D | M |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Value | 1 | 5 | 10 | 50 | 100 | 500 | 1000 |

Letters are written from largest to smallest and added up: `VIII` is 5 + 3 = 8. The
exception is the six subtractive pairs, where a smaller letter comes before a larger
one and is subtracted: `IV` (4), `IX` (9), `XL` (40), `XC` (90), `CD` (400), and
`CM` (900).

| Input | Output |
| --- | --- |
| `1994` | `1994 -> MCMXCIV` |
| `MCMXCIV` | `MCMXCIV -> 1994` |
| `3999` | `3999 -> MMMCMXCIX` |
| `LVIII` | `LVIII -> 58` |

```sh
python3 tools/check.py 04 --lang typescript
```

**Things to notice:**

- **Number to numeral** is a loop inside a loop. Walk a table of values from largest to
  smallest, including the six pairs (1000 M, 900 CM, 500 D, 400 CD, ..., 1 I), and
  append each symbol while it still fits.
- **Numeral to number** is one loop, and a `switch` or `match` to turn each letter into
  its value. Add each letter's value, but subtract it when the next letter is larger.
- **Looking one step ahead** needs a bounds check. C can skip it because a string ends
  with `'\0'`, which can map to 0. Rust can write the check and the comparison as one
  condition with a let chain:
  `if let Some(&next) = values.get(i + 1) && value < next`. Kotlin's
  `text.getOrNull(i + 1)` returns `null` past the end, which folds the check into the
  lookup.
