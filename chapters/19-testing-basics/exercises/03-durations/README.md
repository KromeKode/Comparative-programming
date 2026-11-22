# Exercise 19.3: Test a duration parser

Each language's folder has two functions: one that reads a duration such as `1h30m` and
gives the number of seconds in it, and one that writes a number of seconds as a
duration. Write the tests for both. As in exercises 19.1 and 19.2, the checker runs them
on the code as given, where they must all pass, and then on copies of it with one bug
each, seven this time, where they must fail. Half of what a parser promises is what it
refuses, so expect half of your tests to be tests that it does.

For example:

```text
parse("1h30m")    5400
parse("90m")      5400
parse("30m1h")    an error: the units are out of order
format(5400)      "1h30m"
format(0)         "0s"
```

This is everything the two functions promise, and so everything your tests may rely on:

- **Parts.** A duration is one or more parts, each a whole number, written with the
  digits 0 to 9, and then a unit: `d` for days of 86400 seconds, `h` for hours, `m` for
  minutes, and `s` for seconds. Nothing else may be in it: no spaces, no signs, and no
  other letters.
- **Order.** The units come largest first, and each at most once, so `1d2h3m4s` and
  `2h4s` are durations, and `30m1h` and `1h1h` aren't.
- **Numbers.** A number can be 0, and it can be more than the next unit up holds: `0s`
  is 0 seconds, `1h0m` is 3600, and `90m` is 5400.
- **Parsing.** Parsing gives the duration's number of seconds, and an error for text
  that isn't a duration, which includes empty text, and text that ends with a number and
  no unit. Tests may check that there is an error, and of what type, but not what its
  message says.
- **Formatting.** Formatting a number of seconds, which is at least 0, gives the
  shortest duration that parses to it: each part is as large as it can be, the parts
  that would be 0 are left out, and 0 itself is `0s`. So 5400 is `1h30m`, 3600 is `1h`,
  and 86401 is `1d1s`.
- **Size.** Tests may use durations of up to a year, 31536000 seconds. What happens with
  one too long to count isn't promised.

These are the files and the functions in each language, and how to run your tests
yourself:

| Language | Write the tests in | The functions | Run the tests |
| --- | --- | --- | --- |
| C | `test_duration.c` | `bool parse_duration(const char *text, long long *seconds)`, which returns `false` for an error and leaves `*seconds` alone, and `bool format_duration(long long seconds, char *out, size_t size)`, which returns `false` if `out` is too small | `cc -std=c17 -Wall -Wextra duration.c test_duration.c -o tests && ./tests` |
| C++ | `duration_test.cpp` | `long long duration::parse_duration(std::string_view text)`, which throws `std::invalid_argument` for an error, and `std::string duration::format_duration(long long seconds)` | `c++ -std=c++20 -Wall -Wextra duration.cpp duration_test.cpp -o tests && ./tests` |
| Rust | `src/tests.rs` | `pub fn parse_duration(text: &str) -> Result<u64, ParseDurationError>` and `pub fn format_duration(seconds: u64) -> String` | `cargo test` |
| Go | `duration_test.go` | `func Parse(text string) (int64, error)` and `func Format(s int64) string` | `go test` |
| Python | `test_duration.py` | `def parse_duration(text: str) -> int`, which raises `ValueError` for an error, and `def format_duration(seconds: int) -> str` | `uv run pytest` |
| JavaScript | `duration.test.js` | `export function parseDuration(text)`, which throws a `SyntaxError` for an error, as `JSON.parse` does, and `export function formatDuration(seconds)` | `node --test` |
| TypeScript | `duration.test.ts` | `export function parseDuration(text: string): number`, which throws a `SyntaxError`, and `export function formatDuration(seconds: number): string` | `node --test`, and `tsc` to check the types |
| Dart | `test/duration_test.dart` | `int parseDuration(String text)`, which throws a `FormatException` for an error, as `int.parse` does, and `String formatDuration(int seconds)` | `dart test` |
| Kotlin | `src/test/kotlin/DurationTest.kt` | `fun parseDuration(text: String): Long`, which throws an `IllegalArgumentException` for an error, and `fun formatDuration(seconds: Long): String` | `gradle test` |

The bugs are in the `tests/` folder, as in exercise 19.1. Write your tests before you
read them, and if the checker says your tests missed one, read that one and think about
what test would have caught it without knowing it was there.

```sh
python3 tools/check.py 19 --lang go
```

When every bug is caught, the checker prints
`passed on the code as given, caught 7 of 7 bugs` for this exercise.

**Things to notice:**

- **Test what it refuses.** Most of the seven bugs make the parser accept text it should
  refuse, and a test suite of good durations passes on every one of them. Each rule
  above says what is allowed, and so, on its other side, what isn't, and both sides need
  a test. The solutions put the bad inputs in a table of their own, with a comment on
  each saying which rule it breaks.
- **Saying that something fails.** Each language's test framework has its own way to say
  that a call should fail, and each checks the type of the error:
  `pytest.raises(ValueError)`, `assert.throws(..., SyntaxError)`,
  `throwsFormatException` in Dart, `assertFailsWith<IllegalArgumentException>` in
  Kotlin, and `CHECK_THROWS` in C++. Where the error is a value, the test checks the
  value: `is_err()` in Rust, `err != nil` in Go, and `false` in C, where the test can
  also check that `*seconds` was left alone. None of them check the message, which isn't
  promised, and is the part of an error most likely to change.
- **Round trips.** Parsing what was formatted should give back the number it started
  from, for every number, which a loop can check for a hundred thousand of them without
  anyone writing down an expected value. That is a strong test, but not a complete one:
  it catches what makes the two functions disagree, and so it can't catch a formatter
  that writes a duration the long way, which the parser still reads.

**Stretch:** fuzz the parser. A **fuzzer** makes up inputs, millions of them, and runs a
test on each, keeping the inputs that reach new code and changing them to make more. Go
has one built in: write a `func FuzzParse(f *testing.F)` that adds a few durations with
`f.Add`, and then checks, in `f.Fuzz`, that whenever `Parse` accepts a text,
`Parse(Format(seconds))` gives the same number back. Run it with
`go test -fuzz=FuzzParse -fuzztime=30s`. Then try it on a copy of the code with
`Parse`'s check for durations too long to count taken out, once with `99999999999999d`
added to the inputs, and once without: how long does the fuzzer take to find what the
check was for? Rust has cargo-fuzz, Python has Atheris, and the JVM has Jazzer, which
work the same way.
