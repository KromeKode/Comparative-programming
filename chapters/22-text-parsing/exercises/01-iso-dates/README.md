# Exercise 22.1: Rewrite dates

Write `isodates`, which copies its input to its output with every date in it rewritten
in the one form that sorts the way dates do, `YYYY-MM-DD`:

```text
$ isodates < notes.txt
The library opened on 2026-11-23, a Monday.
The first loan was due back on 2026-12-07, and came back on 2026-12-09.
Next meeting: 2027-01-05 (or 2027-01-12).
```

where `notes.txt` was:

```text
The library opened on 23/11/2026, a Monday.
The first loan was due back on Dec 7, 2026, and came back on 9 December 2026.
Next meeting: January 5, 2027 (or 12 Jan 2027).
```

**Dates.** A date is in one of three forms, where `D` is a day of one or two digits, `M`
is a month of one or two digits, `MONTH` is a month's name, and `YYYY` is a year of four
digits:

| Form | Example |
| --- | --- |
| `D/M/YYYY`, day first | `23/11/2026`, `5/3/2026`, `05/03/2026` |
| `MONTH D, YYYY` | `Nov 23, 2026`, `November 23, 2026` |
| `D MONTH YYYY` | `23 Nov 2026`, `23 November 2026` |

A month's name is written in full, as in `September`, or with its first three letters,
as in `Sep`, and with a capital first letter and small ones after it. The spaces and the
comma are as in the table: one space, and a comma straight after the day.
`Sept 5, 2026`, `nov 23, 2026`, `Nov 23 2026`, `23/11/26`, and `23.11.2026` aren't
dates.

**Only real dates.** The day must be a day of that month in that year: `31/04/2026`
isn't a date, and nor are `0/5/2026` and `13/13/2026`. February has 29 days in a leap
year, which is a year divisible by 4, except that a year divisible by 100 must also be
divisible by 400: `29/02/2024` and `29/02/2000` are dates, and `29/02/2025` and
`29/02/1900` aren't. Any four digits make a year, so `1/1/0000` is `0000-01-01`. Text
that looks like a date but isn't one is left as it was.

**Only ASCII digits.** A digit is one of `0` to `9`. `٢٣/١١/٢٠٢٦`, in Arabic-Indic
digits, and `２３/11/2026`, with full-width ones, are left as they were.

**Not part of a word.** A date can't touch an ASCII letter, an ASCII digit, or `_` on
either side: `123/11/2026`, `23/11/20267`, `x23/11/2026`, and `onNov 23, 2026` aren't
dates, but `(23/11/2026)`, `23/11/2026.`, and `"Nov 23, 2026",` are. Any other character
can be next to a date, so in `é23/11/2026` and `🦊Nov 23, 2026🦊` the dates are rewritten.
When one date ends where another could start, as in `1/1/2026/2/2026`, the first is
rewritten and the rest is searched after it.

**Everything else** is copied as it is, byte for byte, including `\r\n` line ends and a
last line without a `\n`. The input is UTF-8. The exit status is 0.

The tests give each program its input on stdin, and check stdout:

```sh
python3 tools/check.py 22 --lang python
```

The Rust starter is a Cargo project, with the regex crate; the other eight languages
have regular expressions of their own.

**Things to notice:**

- **One regex, three forms.** A regex with an alternative for each form finds every
  candidate in one pass, and a replace that calls a function for each match can check
  the date and give back either the new text or the old. Python's `re.sub`, JavaScript's
  `replace`, Rust's `replace_all`, Dart's `replaceAllMapped`, and Kotlin's `replace` all
  take a function. Go's `ReplaceAllStringFunc` passes only the text that matched, with
  no groups. C++'s `regex_replace` and C's `regexec` have no function to call at all, so
  the text between the matches has to be copied by hand.
- **Which groups took part.** In a regex with three alternatives, the groups of the two
  that didn't match are empty, and each language says so in its own way: `None` in
  Python and Rust, `undefined` in JavaScript, `null` in Dart, `""` in Go and Kotlin's
  `groupValues`, `matched` false in C++, and an offset of `-1` in C.
- **`\b` and `\d` aren't the same everywhere.** In Python and Rust, both are Unicode's
  unless asked otherwise, so `\d` matches `٣` and `\b` takes `é` for a letter. In Go,
  JavaScript, Dart, and Java 21, they are ASCII's. C's POSIX regular expressions have
  neither. Test 4 has letters from outside ASCII next to dates, and test 5 has digits
  from outside ASCII in them.
- **First or longest.** Most of these engines take the first alternative that leads to a
  match, so `Jan|January` tries `Jan` first, and goes on to `January` when the rest of
  the regex can't match after `Jan`. POSIX takes the longest match instead, so in C the
  order of the alternatives doesn't matter.

**Stretch:** also take `2026-11-23` and `2026-1-5` (ISO's order, with one or two
digits), and leave `2026-02-30` as it is. Then make the program report each date that it
left alone because it isn't real, on stderr, with its line and column.
