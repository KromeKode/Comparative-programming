# Exercise 19.1: Test a word wrapper

This time the code is written, and the tests are the exercise. Each language's folder
has a function, `wrap`, that breaks text into lines, and a test file with one test in
it. Write the rest of the tests. The checker runs them on the code as given, where they
must all pass, and then on six copies of it, each with one bug put in, where they must
fail. A test suite that passes on a broken copy has missed that bug.

For example, with a width of 10:

```text
wrap("the quick brown fox jumps", 10)
```

gives three lines:

```text
the quick
brown fox
jumps
```

This is everything `wrap(text, width)` promises, and so everything your tests may rely
on:

- **Words.** A word is a run of characters that aren't spaces. Only the space character,
  `' '`, separates words, and the text is ASCII.
- **Lines.** The words go onto lines in order, with one space between two words on the
  same line, and no spaces at the start or end of a line, however many the text had.
- **Width.** Each line holds as many words as fit in `width` characters, counting the
  spaces between them, and the next word starts a new line. A line can be exactly
  `width` long.
- **Long words.** A word longer than `width` isn't split: it goes on a line of its own.
- **No words.** Text with no words, such as `""` or `"   "`, gives no lines at all.
- **Width.** `width` is at least 1. What `wrap` does with a width of 0 isn't promised,
  so don't test it.

These are the files and the function in each language, and how to run your tests
yourself:

| Language | Write the tests in | The function | Run the tests |
| --- | --- | --- | --- |
| C | `test_wrap.c` | `char *wrap(const char *text, size_t width)`, which returns the lines, each followed by `'\n'`, in one string from `malloc` that the caller frees, so no words is `""` | `cc -std=c17 -Wall -Wextra wrap.c test_wrap.c -o tests && ./tests` |
| C++ | `wrap_test.cpp` | `std::vector<std::string> wrap::wrap(std::string_view text, std::size_t width)` | `c++ -std=c++20 -Wall -Wextra wrap.cpp wrap_test.cpp -o tests && ./tests` |
| Rust | `src/tests.rs` | `pub fn wrap(text: &str, width: usize) -> Vec<String>` | `cargo test` |
| Go | `wrap_test.go` | `func Wrap(text string, width int) []string`, which returns `nil` for no words | `go test` |
| Python | `test_wrap.py` | `def wrap(text: str, width: int) -> list[str]` | `uv run pytest` |
| JavaScript | `wrap.test.js` | `export function wrap(text, width)`, which returns an array of strings | `node --test` |
| TypeScript | `wrap.test.ts` | `export function wrap(text: string, width: number): string[]` | `node --test`, and `tsc` to check the types |
| Dart | `test/wrap_test.dart` | `List<String> wrap(String text, int width)` | `dart test` |
| Kotlin | `src/test/kotlin/WrapTest.kt` | `fun wrap(text: String, width: Int): List<String>` | `gradle test` |

C and C++ have no test framework in their standard libraries, so their folders have a
small one, `check.h` and `check.hpp`, which section 19.3 describes. The others use their
language's own: Rust's `#[test]`, Go's `testing`, pytest, `node:test`, `package:test`,
and `kotlin.test` on JUnit 5. Python, Dart, and Kotlin download theirs the first time,
through uv, `dart pub`, and Gradle.

Don't change `wrap` itself: the checker puts each bug into the code as given, so it must
stay as it is. The bugs are in the `tests/` folder, as edits to each language's code,
with a line saying what each one gets wrong. Write your tests before you read them.
Then, if the checker says your tests missed one, read that one, and think about what
test would have caught it without knowing it was there.

```sh
python3 tools/check.py 19 --lang go
```

When every bug is caught, the checker prints
`passed on the code as given, caught 6 of 6 bugs`.

**Things to notice:**

- **Where the bugs are.** Most of the six are at an edge: a line exactly as long as the
  width, the last line, no words at all, a word longer than the width. Tests of ordinary
  text, two or three words that fit easily, catch hardly any of them. That is why tests
  are written for the edges first: each rule above has a boundary, and a test on each
  side of it.
- **One table, many cases.** Each case is a text, a width, and the lines it should give,
  so the solutions put them in a table: `@pytest.mark.parametrize`, a slice of structs
  in Go, an array of tuples in Rust, an array of arrays in JavaScript and TypeScript,
  and a list of records in Dart. Kotlin's solution uses one `@Test` for each rule
  instead, and C++'s one `TEST`, since `CHECK_EQ` prints both lists when they differ.
  Adding a case is one line.
- **What this checker does.** Putting bugs in on purpose to see whether the tests notice
  is called **mutation testing**. Tools do it for real projects, by making hundreds of
  small changes, such as `<=` to `<` or deleting a line, and reporting each change that
  no test noticed: cargo-mutants for Rust, mutmut for Python, Stryker for JavaScript and
  TypeScript, and PIT for the JVM. A change that survives is either a missing test, or
  code that doesn't matter.
- **Test what's promised.** `wrap` with a width of 0 does something, but nothing
  promises what, so a test of it would fail the day someone changed `wrap` in a way that
  broke nobody. Tests that pin down more than the promise make code hard to change.

**Stretch:** write a test that checks the rules on many texts, instead of a few chosen
ones: build random texts from a short list of words and runs of spaces, with a fixed
seed, and check that every line is at most `width` long unless it is one word, that no
line starts or ends with a space, and that the lines joined with spaces are the text's
words joined with spaces. Which of the six bugs does it catch on its own? This is
**property-based testing**, which Hypothesis (Python), proptest (Rust), fast-check
(JavaScript), and Kotest (Kotlin) do with shrinking: when a random case fails, they look
for the smallest one that still does.
