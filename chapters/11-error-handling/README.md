# Chapter 11: Error handling

Programs meet text that is not a number, files that are not there, and servers that do
not answer. This chapter is about how a function tells its caller that it failed, how
that news travels up to code that can do something about it, and what the caller does
then. The nine languages use four mechanisms. C returns a code. C++, Python, JavaScript,
TypeScript, Dart, and Kotlin throw exceptions. Rust returns a `Result` value, and Go
returns an `error` next to the result. Underneath the differences they agree on one
idea: a bug in the program is not an error to handle, and each language has a separate
way to stop when one happens.

**Languages:** all 9 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| The four ways a function can report failure, and what each one costs | Every chapter from here on |
| Passing errors up with `?`, `if err != nil`, and exceptions, adding context on the way | Files, JSON, and the CLI (chapter 20), web servers (chapters 35 to 38) |
| Error types that callers can match on, and errors that carry their cause | Text parsing (chapter 21), REST CRUD API (chapter 37) |
| Which failures are bugs, and what panics, assertions, and crashes are for | Testing basics (chapter 18), debugging and profiling (chapter 19) |
| Collecting many errors at once, and retrying the ones that may go away | Transactions and data access (chapter 34), HTTP from sockets (chapter 35) |

---

## 11.1 The concept: failure is part of the result

A function that can fail has two kinds of result: the value it was asked for, or the
reason it could not produce one. `parse("42")` gives 42. `parse("4x2")` cannot give a
number, and its caller needs to know that, and usually why. Every language has to answer
three questions. How does the function report the failure? How does the failure travel
up to code that can deal with it? And can the compiler make sure nobody ignores it?

There are four answers, roughly in the order they were invented:

- **Error codes.** The function returns a special value, such as -1, `NULL`, or a status
  code, and puts the details somewhere else: in an out parameter, or in C's `errno`. It
  is cheap and simple, but the caller can ignore it, and the special value must be one
  that a real result can never be.
- **Exceptions.** The function **throws** an object that describes the failure. The
  object travels up through every caller, skipping the rest of each one, until it
  reaches a `catch` block that matches it. The functions in between write nothing. The
  price is that a function's signature does not say what it can throw, in any of the six
  languages here that use exceptions.
- **Result values.** The function returns one value that is either the success or the
  error: Rust's `Result<T, E>` is `Ok(value)` or `Err(error)`. The caller cannot get the
  value out without saying what happens to the error, and `?` passes the error up in one
  character.
- **Two return values.** The function returns the result and an error side by side, and
  the error is `nil` when it worked. This is Go: `value, err := strconv.Atoi(text)`,
  then `if err != nil`. The error is an ordinary value, and passing it up is an ordinary
  `return`.

```text
 Error code: C                        Exception: C++, Python, JS, TS, Dart, Kotlin
 if (parse(text, &n) != 0) {          try {
     return -1;                           n = parse(text);
 }                                        use(n);
 use(n);                              } catch (const ParseError &e) {
                                          ...
                                      }

 Result: Rust                         Two return values: Go
 let n = parse(text)?;                n, err := parse(text)
 use(n);                              if err != nil {
                                          return err
                                      }
                                      use(n)
```

Separate from all four is a different kind of failure: a **bug**. An index past the end
of an array, a division by zero, or a null where the code promised there would be none
means the program itself is wrong. No caller can sensibly recover from that, because no
caller expected it. Most of the languages stop the program, or at least the current
task, with a **panic**, an **abort**, or an exception from a family you are not meant to
catch.

---

## 11.2 What each language gives you

| Language | Reporting a failure | Checked by the compiler? | Cost | Bugs | Idiomatic style |
| --- | --- | --- | --- | --- | --- |
| C | Return codes, `errno`, out parameters | No; GCC and Clang have `warn_unused_result`, and C23 has `[[nodiscard]]` | A comparison | Undefined behavior; `assert` aborts | Return 0 or -1, or a status enum, with details in `errno` or an out parameter |
| C++ | Exceptions; `std::optional` for "no value"; `std::expected` in C++23 | No; `[[nodiscard]]` warns about an ignored return value | Close to nothing until thrown; throwing is slow | Undefined behavior; `assert`; `.at()` throws | Exceptions for failures, `std::optional` for a missing value |
| Rust | `Result<T, E>`, and `Option<T>` for "no value" | Yes: an ignored `Result` is a warning, and the value can't be reached without handling `Err` | An ordinary return value | `panic!`, which exits with code 101 | An error enum, returned with `?` |
| Go | An `error` as the last return value | Partly: `value := f()` fails to compile when `f` returns two values, but a bare `f()` drops both | An ordinary return value | `panic`, which `recover` can stop | `if err != nil { return fmt.Errorf("context: %w", err) }` |
| Python | Exceptions | No | A `try` costs almost nothing since 3.11; raising is slower | The same exceptions: `IndexError`, `ZeroDivisionError` | Try the call and catch the failure |
| JavaScript | Exceptions, and rejected promises | No | Creating an `Error` records a stack trace | Often none: `undefined`, `NaN`, or `Infinity`; sometimes `TypeError` | `throw` an `Error` subclass |
| TypeScript | As JavaScript | No: a function can't declare what it throws, and a caught value has the type `unknown` | As JavaScript | As JavaScript, but the type checker stops many `undefined`s | As JavaScript, or a tagged union returned as a result |
| Dart | Exceptions that implement `Exception` | No | Throwing records a stack trace | Subclasses of `Error`: `RangeError`, `TypeError` | Throw an `Exception`; let an `Error` crash |
| Kotlin | Exceptions, all unchecked; nullable types for "no value" | No: Kotlin left out Java's checked exceptions | Creating an exception records a stack trace | `IndexOutOfBoundsException`, `NullPointerException`, and others | `?:` for a missing value; exceptions or sealed classes for failures |

Read the third column first. In Rust and Go, a function that can fail says so in its
signature, and Rust's compiler insists that you deal with it. A C function's signature
shows a status code, but not which values mean failure. In the six languages with
exceptions, the only way to know what a function throws is to read its documentation, or
its code. The "Bugs" column is where they agree most. Each language has a way to stop
the program when it finds a bug, and only C and C++ leave the common ones, such as a bad
index, undefined.

---

## 11.3 Reporting failure

Here is one task in each language's standard style: turn text into an integer. Each
program tries the same four inputs: a good number, one with a letter in the middle, an
empty string, and one too big for 32 bits.

**C**

```c
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// C reports failure through the return value, an out parameter, or the global
// errno. strtol uses all three, so checking it takes three tests.
// Returns true and sets *out on success, or false and sets *error.
bool parse_long(const char *text, long *out, const char **error) {
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (end == text) {
        *error = "no digits";
        return false;
    }
    if (errno == ERANGE) {
        *error = "out of range";
        return false;
    }
    if (*end != '\0') {
        *error = "extra characters after the number";
        return false;
    }
    *out = value;
    return true;
}

int main(void) {
    const char *inputs[] = {"42", "4x2", "", "99999999999999999999"};
    printf("parse_long, built on strtol:\n");
    for (int i = 0; i < 4; i++) {
        long value;
        const char *error;
        if (parse_long(inputs[i], &value, &error)) {
            printf("  \"%s\" -> %ld\n", inputs[i], value);
        } else {
            printf("  \"%s\" -> error: %s\n", inputs[i], error);
        }
    }

    // atoi has no way to say it failed: it returns 0, or what it read so far.
    printf("atoi:\n");
    for (int i = 0; i < 3; i++) {
        printf("  \"%s\" -> %d\n", inputs[i], atoi(inputs[i]));
    }
    return 0;
}
```

**C++**

```cpp
#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

int main() {
    std::vector<std::string> inputs{"42", "4x2", "", "99999999999999999999"};

    // std::stoi throws an exception when it cannot parse. It stops at the first
    // character that is not a digit, so it also reports how much it used.
    std::cout << "std::stoi:\n";
    for (const std::string &text : inputs) {
        std::cout << "  \"" << text << "\" -> ";
        try {
            std::size_t used = 0;
            int value = std::stoi(text, &used);
            std::cout << value;
            if (used != text.size()) {
                std::cout << " (it used " << used << " of " << text.size()
                          << " characters)";
            }
            std::cout << '\n';
        } catch (const std::invalid_argument &e) {
            std::cout << "std::invalid_argument: " << e.what() << '\n';
        } catch (const std::out_of_range &e) {
            std::cout << "std::out_of_range: " << e.what() << '\n';
        }
    }

    // std::from_chars returns an error code instead, and never throws.
    std::cout << "std::from_chars:\n";
    for (const std::string &text : inputs) {
        int value = 0;
        const char *end = text.data() + text.size();
        auto [stop, error] = std::from_chars(text.data(), end, value);
        std::cout << "  \"" << text << "\" -> ";
        if (error != std::errc()) {
            std::cout << "error: " << std::make_error_code(error).message()
                      << '\n';
        } else if (stop != end) {
            std::cout << value << ", with characters left over\n";
        } else {
            std::cout << value << '\n';
        }
    }
}
```

**Rust**

```rust
fn main() {
    let inputs = ["42", "4x2", "", "99999999999999999999"];

    // parse returns a Result: Ok with the number, or Err with the reason. The
    // compiler will not let you use the number without dealing with the Err.
    println!("parse::<i32>:");
    for text in inputs {
        match text.parse::<i32>() {
            Ok(value) => println!("  {text:?} -> Ok({value})"),
            Err(error) => println!("  {text:?} -> Err({:?}): {error}", error.kind()),
        }
    }

    // When you don't need the reason, turn the Result into an Option or a
    // default.
    println!("ok and unwrap_or:");
    for text in inputs {
        let maybe = text.parse::<i32>().ok();
        let or_zero = text.parse::<i32>().unwrap_or(0);
        println!("  {text:?} -> {maybe:?}, or {or_zero}");
    }
}
```

**Go**

```go
package main

import (
	"errors"
	"fmt"
	"strconv"
)

func main() {
	inputs := []string{"42", "4x2", "", "99999999999999999999"}

	// Atoi returns two values: the number and an error. The error is nil when
	// it worked, and the caller is expected to check it right away.
	fmt.Println("strconv.Atoi:")
	for _, text := range inputs {
		value, err := strconv.Atoi(text)
		if err != nil {
			fmt.Printf("  %q -> error: %v\n", text, err)
			continue
		}
		fmt.Printf("  %q -> %d\n", text, value)
	}

	// The error wraps a sentinel value that says which kind it is.
	fmt.Println("errors.Is:")
	for _, text := range inputs[1:] {
		_, err := strconv.Atoi(text)
		syntax := errors.Is(err, strconv.ErrSyntax)
		tooBig := errors.Is(err, strconv.ErrRange)
		fmt.Printf("  %q -> syntax: %t, range: %t\n", text, syntax, tooBig)
	}
}
```

**Python**

```python
def main() -> None:
    inputs = ["42", "4x2", "", "99999999999999999999"]

    # int raises an exception when it cannot parse. Python's ints have no
    # size limit, so the last one is fine.
    print("int:")
    for text in inputs:
        try:
            print(f"  {text!r} -> {int(text)}")
        except ValueError as error:
            print(f"  {text!r} -> ValueError: {error}")

    # Asking first is the other style. It needs a second pass over the text,
    # and isdigit misses signs, so Python code usually just tries.
    print("str.isdigit:")
    for text in inputs:
        print(f"  {text!r} -> {text.isdigit()}")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const inputs = ["42", "4x2", "", "99999999999999999999"];

// Number never throws. It returns NaN, "not a number", when it cannot parse,
// and the empty string becomes 0.
console.log("Number:");
for (const text of inputs) {
  console.log(`  ${JSON.stringify(text)} -> ${Number(text)}`);
}

// parseInt stops at the first character that is not a digit.
console.log("parseInt:");
for (const text of inputs) {
  console.log(`  ${JSON.stringify(text)} -> ${parseInt(text, 10)}`);
}

// A strict parser has to be written by hand, and it throws.
function parseInteger(text) {
  if (!/^[+-]?\d+$/.test(text)) {
    throw new SyntaxError(`not an integer: ${JSON.stringify(text)}`);
  }
  const value = Number(text);
  if (!Number.isSafeInteger(value)) {
    throw new RangeError(`too large to be exact: ${text}`);
  }
  return value;
}

console.log("parseInteger:");
for (const text of inputs) {
  try {
    console.log(`  ${JSON.stringify(text)} -> ${parseInteger(text)}`);
  } catch (error) {
    console.log(`  ${JSON.stringify(text)} -> ${error.name}: ${error.message}`);
  }
}
```

**TypeScript**

```ts
const inputs = ["42", "4x2", "", "99999999999999999999"];

// Number never throws. It returns NaN, "not a number", when it cannot parse,
// and the empty string becomes 0.
console.log("Number:");
for (const text of inputs) {
  console.log(`  ${JSON.stringify(text)} -> ${Number(text)}`);
}

// parseInt stops at the first character that is not a digit.
console.log("parseInt:");
for (const text of inputs) {
  console.log(`  ${JSON.stringify(text)} -> ${parseInt(text, 10)}`);
}

// A strict parser has to be written by hand, and it throws. Nothing in its
// type says so: TypeScript has no way to declare what a function throws.
function parseInteger(text: string): number {
  if (!/^[+-]?\d+$/.test(text)) {
    throw new SyntaxError(`not an integer: ${JSON.stringify(text)}`);
  }
  const value = Number(text);
  if (!Number.isSafeInteger(value)) {
    throw new RangeError(`too large to be exact: ${text}`);
  }
  return value;
}

console.log("parseInteger:");
for (const text of inputs) {
  try {
    console.log(`  ${JSON.stringify(text)} -> ${parseInteger(text)}`);
  } catch (error) {
    // A catch clause gets `unknown`, so check what was thrown before using it.
    if (!(error instanceof Error)) throw error;
    console.log(`  ${JSON.stringify(text)} -> ${error.name}: ${error.message}`);
  }
}
```

**Dart**

```dart
void main() {
  final inputs = ['42', '4x2', '', '99999999999999999999'];

  // int.parse throws a FormatException when it cannot parse.
  print('int.parse:');
  for (final text in inputs) {
    try {
      print('  "$text" -> ${int.parse(text)}');
    } on FormatException catch (error) {
      print('  "$text" -> FormatException: ${error.message}');
    }
  }

  // int.tryParse returns null instead, and the type int? makes you check.
  print('int.tryParse:');
  for (final text in inputs) {
    final value = int.tryParse(text);
    print('  "$text" -> ${value ?? 'null, so use a default: 0'}');
  }
}
```

**Kotlin**

```kotlin
fun main() {
    val inputs = listOf("42", "4x2", "", "99999999999999999999")

    // toInt throws a NumberFormatException when it cannot parse.
    println("toInt:")
    for (text in inputs) {
        try {
            println("  \"$text\" -> ${text.toInt()}")
        } catch (error: NumberFormatException) {
            println("  \"$text\" -> NumberFormatException: ${error.message}")
        }
    }

    // toIntOrNull returns null instead, and the type Int? makes you check.
    println("toIntOrNull:")
    for (text in inputs) {
        val value = text.toIntOrNull()
        println("  \"$text\" -> ${value ?: "null, so use a default: 0"}")
    }
}
```

Each language prints something different:

*C:*

```text
parse_long, built on strtol:
  "42" -> 42
  "4x2" -> error: extra characters after the number
  "" -> error: no digits
  "99999999999999999999" -> error: out of range
atoi:
  "42" -> 42
  "4x2" -> 4
  "" -> 0
```

*C++:*

```text
std::stoi:
  "42" -> 42
  "4x2" -> 4 (it used 1 of 3 characters)
  "" -> std::invalid_argument: stoi
  "99999999999999999999" -> std::out_of_range: stoi
std::from_chars:
  "42" -> 42
  "4x2" -> 4, with characters left over
  "" -> error: Invalid argument
  "99999999999999999999" -> error: Numerical result out of range
```

*Rust:*

```text
parse::<i32>:
  "42" -> Ok(42)
  "4x2" -> Err(InvalidDigit): invalid digit found in string
  "" -> Err(Empty): cannot parse integer from empty string
  "99999999999999999999" -> Err(PosOverflow): number too large to fit in target type
ok and unwrap_or:
  "42" -> Some(42), or 42
  "4x2" -> None, or 0
  "" -> None, or 0
  "99999999999999999999" -> None, or 0
```

*Go:*

```text
strconv.Atoi:
  "42" -> 42
  "4x2" -> error: strconv.Atoi: parsing "4x2": invalid syntax
  "" -> error: strconv.Atoi: parsing "": invalid syntax
  "99999999999999999999" -> error: strconv.Atoi: parsing "99999999999999999999": value out of range
errors.Is:
  "4x2" -> syntax: true, range: false
  "" -> syntax: true, range: false
  "99999999999999999999" -> syntax: false, range: true
```

*Python:*

```text
int:
  '42' -> 42
  '4x2' -> ValueError: invalid literal for int() with base 10: '4x2'
  '' -> ValueError: invalid literal for int() with base 10: ''
  '99999999999999999999' -> 99999999999999999999
str.isdigit:
  '42' -> True
  '4x2' -> False
  '' -> False
  '99999999999999999999' -> True
```

*JavaScript and TypeScript:*

```text
Number:
  "42" -> 42
  "4x2" -> NaN
  "" -> 0
  "99999999999999999999" -> 100000000000000000000
parseInt:
  "42" -> 42
  "4x2" -> 4
  "" -> NaN
  "99999999999999999999" -> 100000000000000000000
parseInteger:
  "42" -> 42
  "4x2" -> SyntaxError: not an integer: "4x2"
  "" -> SyntaxError: not an integer: ""
  "99999999999999999999" -> RangeError: too large to be exact: 99999999999999999999
```

*Dart:*

```text
int.parse:
  "42" -> 42
  "4x2" -> FormatException: Invalid radix-10 number
  "" -> FormatException: Invalid number
  "99999999999999999999" -> FormatException: Positive input exceeds the limit of integer
int.tryParse:
  "42" -> 42
  "4x2" -> null, so use a default: 0
  "" -> null, so use a default: 0
  "99999999999999999999" -> null, so use a default: 0
```

*Kotlin:*

```text
toInt:
  "42" -> 42
  "4x2" -> NumberFormatException: For input string: "4x2"
  "" -> NumberFormatException: For input string: ""
  "99999999999999999999" -> NumberFormatException: For input string: "99999999999999999999"
toIntOrNull:
  "42" -> 42
  "4x2" -> null, so use a default: 0
  "" -> null, so use a default: 0
  "99999999999999999999" -> null, so use a default: 0
```

**C: three signals at once.** `strtol` returns the number, sets `end` to where it
stopped reading, and sets `errno` to `ERANGE` when the number does not fit. A full check
needs all three: no digits at all (`end == text`), too big (`errno == ERANGE`, after
setting `errno` to 0 first, because nothing else clears it), and something left over
(`*end != '\0'`). `parse_long` wraps them in the usual C shape: return `true` or
`false`, and hand back the value or the error through pointers. `atoi` has no way to
report failure. It returns 0 for an empty string, which is also what it returns for
`"0"`, and 4 for `"4x2"`.

**C++: an exception or an error code, your choice.** `std::stoi` throws
`std::invalid_argument` when there are no digits and `std::out_of_range` when the number
is too big. With GCC's library, `what()` is only `"stoi"`, so the type tells you more
than the message. Like `strtol`, it stops at the first character that is not a digit, so
`"4x2"` gives 4 unless you compare `used` with the length. `std::from_chars`, from
C++17, never throws. It returns a struct holding where it stopped and a `std::errc`
code, which `std::make_error_code` turns into a message.

**Rust: a `Result` you have to open.** `parse` returns `Result<i32, ParseIntError>`. The
`i32` is inside, and the only ways to get it out say what happens on `Err`: a `match`,
`?`, `.ok()` to turn the `Result` into an `Option`, or `.unwrap_or(0)` for a default.
`error.kind()` returns an `IntErrorKind` enum, so code can tell an empty string from an
overflow without reading the message.

**Go: the error is the second result.** `Atoi` returns an `int` and an `error`. The
error's message names the function and the input. Inside it is one of two **sentinel
errors**, fixed values that the package exports, and `errors.Is(err, strconv.ErrSyntax)`
looks inside to find it. Go's `int` is 64 bits on 64-bit systems, and the last input is
too big even for that.

**Python: an exception, and a number with no limit.** `int` raises `ValueError` with a
message that quotes the input. Python's integers have no size limit, so the 20-digit
number parses fine, and a range check is up to you. The second loop shows the other
style: checking first. It needs a second pass over the text, and it gets the edge cases
wrong. `"-5".isdigit()` is `False`, and `"²".isdigit()` is `True`, but `int("²")` raises
`ValueError`. So Python code usually just calls `int` and catches the exception.
Python's documentation calls that style EAFP, "easier to ask for forgiveness than
permission".

**JavaScript and TypeScript: no error at all.** `Number("4x2")` is `NaN`, `Number("")`
is 0, and the 20-digit number becomes the nearest double. `parseInt("4x2")` is 4.
Neither one throws, so a strict parser has to be written by hand. `parseInteger` checks
the text with a regular expression, refuses numbers that `Number.isSafeInteger` says are
past 2^53, and throws a `SyntaxError` or a `RangeError`. The TypeScript version differs
in two places. A `catch` clause gives the error the type `unknown`, because JavaScript
can throw anything, even a string, so it checks `instanceof Error` before reading
`.message`. And nothing in `parseInteger`'s type says that it throws.

**Dart and Kotlin: one function that throws and one that returns null.** Dart's
`int.parse` throws a `FormatException`, and `int.tryParse` returns `null`. Its type is
`int?`, so the compiler makes you deal with the `null` before you can use the number,
here with `??` and a default. Kotlin's pair is `toInt`, which throws
`NumberFormatException` with Java's message, and `toIntOrNull`, which returns `Int?`,
with `?:` for the default. Both languages offer the pair for most parsing, and the null
version is the one to use when bad input is expected.

---

## 11.4 Passing errors up

Most code that meets an error cannot fix it. It passes the error up to a caller that
can, and adds what it knows on the way. In this example, `sum_file` reads a pretend file
and adds up the numbers on its lines, using `read_file` and `parse_number`. `sum_all`
adds up several files. It counts a missing file as 0, but a bad number stops it. The
error from `parse_number` knows only the text, so `sum_file` adds the file name and the
line number.

**C**

```c
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Pretend files, so the example does not touch the disk.
static const struct {
    const char *name;
    const char *text;
} files[] = {{"good.txt", "1\n2\n3"}, {"bad.txt", "4\nfive\n6"}};

// Every function returns a status. On failure it also writes a message into
// the caller's error buffer.
enum status { OK, NOT_FOUND, NOT_A_NUMBER };

enum status read_file(const char *name, const char **text, char *error,
                      size_t size) {
    for (size_t i = 0; i < sizeof files / sizeof files[0]; i++) {
        if (strcmp(files[i].name, name) == 0) {
            *text = files[i].text;
            return OK;
        }
    }
    snprintf(error, size, "%s: no such file", name);
    return NOT_FOUND;
}

enum status parse_number(const char *text, long *value, char *error,
                         size_t size) {
    char *end;
    errno = 0;
    *value = strtol(text, &end, 10);
    if (end == text || *end != '\0' || errno == ERANGE) {
        snprintf(error, size, "\"%s\" is not a number", text);
        return NOT_A_NUMBER;
    }
    return OK;
}

enum status sum_file(const char *name, long *total, char *error,
                     size_t size) {
    const char *text;
    enum status status = read_file(name, &text, error, size);
    if (status != OK) {
        return status; // pass the error up unchanged
    }
    *total = 0;
    for (int line = 1;; line++) {
        size_t length = strcspn(text, "\n");
        char number[32];
        snprintf(number, sizeof number, "%.*s", (int)length, text);
        long value;
        char inner[64];
        status = parse_number(number, &value, inner, sizeof inner);
        if (status != OK) {
            // Add context: which file and which line.
            snprintf(error, size, "%s: line %d: %s", name, line, inner);
            return status;
        }
        *total += value;
        if (text[length] == '\0') {
            return OK;
        }
        text += length + 1;
    }
}

// A missing file counts as 0, but any other error stops the sum.
enum status sum_all(const char *names[], int count, long *total, char *error,
                    size_t size) {
    *total = 0;
    for (int i = 0; i < count; i++) {
        long value;
        enum status status = sum_file(names[i], &value, error, size);
        if (status == NOT_FOUND) {
            continue;
        }
        if (status != OK) {
            return status;
        }
        *total += value;
    }
    return OK;
}

void show(const char *label, enum status status, long total,
          const char *error) {
    if (status == OK) {
        printf("  %s -> %ld\n", label, total);
    } else {
        printf("  %s -> error: %s\n", label, error);
    }
}

int main(void) {
    char error[128];
    long total;

    printf("one file at a time:\n");
    const char *names[] = {"good.txt", "bad.txt", "missing.txt"};
    for (int i = 0; i < 3; i++) {
        enum status status = sum_file(names[i], &total, error, sizeof error);
        show(names[i], status, total, error);
    }

    printf("several files, where a missing file counts as 0:\n");
    const char *first[] = {"good.txt", "missing.txt"};
    enum status status = sum_all(first, 2, &total, error, sizeof error);
    show("good.txt + missing.txt", status, total, error);
    const char *second[] = {"good.txt", "bad.txt"};
    status = sum_all(second, 2, &total, error, sizeof error);
    show("good.txt + bad.txt", status, total, error);
    return 0;
}
```

**C++**

```cpp
#include <charconv>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Pretend files, so the example does not touch the disk.
const std::map<std::string, std::string> files{{"good.txt", "1\n2\n3"},
                                               {"bad.txt", "4\nfive\n6"}};

// One exception class per kind of failure, so callers can catch just one.
struct NotFound : std::runtime_error {
    using std::runtime_error::runtime_error;
};
struct NotANumber : std::runtime_error {
    using std::runtime_error::runtime_error;
};

const std::string &read_file(const std::string &name) {
    auto found = files.find(name);
    if (found == files.end()) {
        throw NotFound(name + ": no such file");
    }
    return found->second;
}

long parse_number(const std::string &text) {
    long value = 0;
    const char *end = text.data() + text.size();
    auto [stop, error] = std::from_chars(text.data(), end, value);
    if (error != std::errc() || stop != end) {
        throw NotANumber("\"" + text + "\" is not a number");
    }
    return value;
}

long sum_file(const std::string &name) {
    // A NotFound from read_file passes straight through: no code needed.
    std::istringstream lines(read_file(name));
    long total = 0;
    std::string text;
    for (int line = 1; std::getline(lines, text); line++) {
        try {
            total += parse_number(text);
        } catch (const NotANumber &) {
            // Throw a new exception that carries the current one inside it.
            std::throw_with_nested(
                std::runtime_error(name + ": line " + std::to_string(line)));
        }
    }
    return total;
}

// A missing file counts as 0, but any other error stops the sum.
long sum_all(const std::vector<std::string> &names) {
    long total = 0;
    for (const std::string &name : names) {
        try {
            total += sum_file(name);
        } catch (const NotFound &) {
        }
    }
    return total;
}

// Join an exception's message with the messages of the ones nested inside it.
std::string describe(const std::exception &error) {
    std::string text = error.what();
    try {
        std::rethrow_if_nested(error);
    } catch (const std::exception &inner) {
        text += ": " + describe(inner);
    }
    return text;
}

template <typename Function>
void show(const std::string &label, Function function) {
    std::cout << "  " << label << " -> ";
    try {
        long total = function();
        std::cout << total << '\n';
    } catch (const std::exception &error) {
        std::cout << "error: " << describe(error) << '\n';
    }
}

int main() {
    std::cout << "one file at a time:\n";
    for (std::string name : {"good.txt", "bad.txt", "missing.txt"}) {
        show(name, [&] { return sum_file(name); });
    }
    std::cout << "several files, where a missing file counts as 0:\n";
    show("good.txt + missing.txt",
         [] { return sum_all({"good.txt", "missing.txt"}); });
    show("good.txt + bad.txt", [] { return sum_all({"good.txt", "bad.txt"}); });
}
```

**Rust**

```rust
use std::error::Error;
use std::fmt;

// Pretend files, so the example does not touch the disk.
const FILES: [(&str, &str); 2] = [("good.txt", "1\n2\n3"), ("bad.txt", "4\nfive\n6")];

#[derive(Debug)]
struct NotFound(String);

impl fmt::Display for NotFound {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}: no such file", self.0)
    }
}

impl Error for NotFound {}

#[derive(Debug)]
struct NotANumber(String);

impl fmt::Display for NotANumber {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "\"{}\" is not a number", self.0)
    }
}

impl Error for NotANumber {}

// Everything sum_file can fail with, as one enum.
#[derive(Debug)]
enum SumError {
    NotFound(NotFound),
    BadLine {
        file: String,
        line: usize,
        source: NotANumber,
    },
}

impl fmt::Display for SumError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            SumError::NotFound(error) => write!(f, "{error}"),
            SumError::BadLine { file, line, .. } => write!(f, "{file}: line {line}"),
        }
    }
}

impl Error for SumError {
    // The error that caused this one, if there is one.
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            SumError::NotFound(_) => None,
            SumError::BadLine { source, .. } => Some(source),
        }
    }
}

// This lets ? turn a NotFound into a SumError on its own.
impl From<NotFound> for SumError {
    fn from(error: NotFound) -> Self {
        SumError::NotFound(error)
    }
}

fn read_file(name: &str) -> Result<&'static str, NotFound> {
    FILES
        .iter()
        .find(|(file, _)| *file == name)
        .map(|(_, text)| *text)
        .ok_or_else(|| NotFound(name.to_string()))
}

fn parse_number(text: &str) -> Result<i64, NotANumber> {
    text.parse().map_err(|_| NotANumber(text.to_string()))
}

fn sum_file(name: &str) -> Result<i64, SumError> {
    let text = read_file(name)?; // on Err, convert it and return it
    let mut total = 0;
    for (index, text) in text.lines().enumerate() {
        total += parse_number(text).map_err(|source| SumError::BadLine {
            file: name.to_string(),
            line: index + 1,
            source,
        })?;
    }
    Ok(total)
}

// A missing file counts as 0, but any other error stops the sum.
fn sum_all(names: &[&str]) -> Result<i64, SumError> {
    let mut total = 0;
    for name in names {
        match sum_file(name) {
            Ok(value) => total += value,
            Err(SumError::NotFound(_)) => {}
            Err(other) => return Err(other),
        }
    }
    Ok(total)
}

// Join an error's message with the messages of its sources.
fn describe(error: &dyn Error) -> String {
    let mut text = error.to_string();
    let mut cause = error.source();
    while let Some(inner) = cause {
        text += ": ";
        text += &inner.to_string();
        cause = inner.source();
    }
    text
}

fn show(label: &str, result: Result<i64, SumError>) {
    match result {
        Ok(total) => println!("  {label} -> {total}"),
        Err(error) => println!("  {label} -> error: {}", describe(&error)),
    }
}

fn main() {
    println!("one file at a time:");
    for name in ["good.txt", "bad.txt", "missing.txt"] {
        show(name, sum_file(name));
    }
    println!("several files, where a missing file counts as 0:");
    for names in [["good.txt", "missing.txt"], ["good.txt", "bad.txt"]] {
        show(&names.join(" + "), sum_all(&names));
    }
}
```

**Go**

```go
package main

import (
	"errors"
	"fmt"
	"strconv"
	"strings"
)

// Pretend files, so the example does not touch the disk.
var files = map[string]string{"good.txt": "1\n2\n3", "bad.txt": "4\nfive\n6"}

// A sentinel error: callers compare against it with errors.Is.
var ErrNotFound = errors.New("no such file")

func readFile(name string) (string, error) {
	text, ok := files[name]
	if !ok {
		// %w wraps ErrNotFound, so errors.Is can still find it.
		return "", fmt.Errorf("%s: %w", name, ErrNotFound)
	}
	return text, nil
}

func parseNumber(text string) (int, error) {
	value, err := strconv.Atoi(text)
	if err != nil {
		return 0, fmt.Errorf("%q is not a number", text)
	}
	return value, nil
}

func sumFile(name string) (int, error) {
	text, err := readFile(name)
	if err != nil {
		return 0, err // pass the error up unchanged
	}
	total := 0
	for index, line := range strings.Split(text, "\n") {
		value, err := parseNumber(line)
		if err != nil {
			// Add context: which file and which line.
			return 0, fmt.Errorf("%s: line %d: %w", name, index+1, err)
		}
		total += value
	}
	return total, nil
}

// A missing file counts as 0, but any other error stops the sum.
func sumAll(names ...string) (int, error) {
	total := 0
	for _, name := range names {
		value, err := sumFile(name)
		if errors.Is(err, ErrNotFound) {
			continue
		}
		if err != nil {
			return 0, err
		}
		total += value
	}
	return total, nil
}

func show(label string, total int, err error) {
	if err != nil {
		fmt.Printf("  %s -> error: %v\n", label, err)
	} else {
		fmt.Printf("  %s -> %d\n", label, total)
	}
}

func main() {
	fmt.Println("one file at a time:")
	for _, name := range []string{"good.txt", "bad.txt", "missing.txt"} {
		total, err := sumFile(name)
		show(name, total, err)
	}
	fmt.Println("several files, where a missing file counts as 0:")
	total, err := sumAll("good.txt", "missing.txt")
	show("good.txt + missing.txt", total, err)
	total, err = sumAll("good.txt", "bad.txt")
	show("good.txt + bad.txt", total, err)
}
```

**Python**

```python
# Pretend files, so the example does not touch the disk.
FILES = {"good.txt": "1\n2\n3", "bad.txt": "4\nfive\n6"}


# One exception class per kind of failure, so callers can catch just one.
class NotFound(Exception):
    pass


class NotANumber(Exception):
    pass


class BadLine(Exception):
    pass


def read_file(name: str) -> str:
    if name not in FILES:
        raise NotFound(f"{name}: no such file")
    return FILES[name]


def parse_number(text: str) -> int:
    try:
        return int(text)
    except ValueError:
        # Replace int's ValueError with our own; "from None" drops it.
        raise NotANumber(f'"{text}" is not a number') from None


def sum_file(name: str) -> int:
    text = read_file(name)  # a NotFound passes straight through
    total = 0
    for line, number in enumerate(text.splitlines(), start=1):
        try:
            total += parse_number(number)
        except NotANumber as error:
            # Raise a new exception and keep the old one as its __cause__.
            raise BadLine(f"{name}: line {line}") from error
    return total


def sum_all(names: list[str]) -> int:
    """A missing file counts as 0, but any other error stops the sum."""
    total = 0
    for name in names:
        try:
            total += sum_file(name)
        except NotFound:
            pass
    return total


def describe(error: BaseException) -> str:
    """Join an exception's message with the messages of its causes."""
    parts = []
    cause: BaseException | None = error
    while cause is not None:
        parts.append(str(cause))
        cause = cause.__cause__
    return ": ".join(parts)


def show(label, function, *args):
    try:
        print(f"  {label} -> {function(*args)}")
    except Exception as error:
        print(f"  {label} -> error: {describe(error)}")


print("one file at a time:")
for name in ["good.txt", "bad.txt", "missing.txt"]:
    show(name, sum_file, name)
print("several files, where a missing file counts as 0:")
show("good.txt + missing.txt", sum_all, ["good.txt", "missing.txt"])
show("good.txt + bad.txt", sum_all, ["good.txt", "bad.txt"])
```

**JavaScript**

```js
// Pretend files, so the example does not touch the disk.
const files = new Map([
  ["good.txt", "1\n2\n3"],
  ["bad.txt", "4\nfive\n6"],
]);

// One error class per kind of failure, so callers can tell them apart.
class NotFound extends Error {}
class NotANumber extends Error {}

function readFile(name) {
  const text = files.get(name);
  if (text === undefined) {
    throw new NotFound(`${name}: no such file`);
  }
  return text;
}

function parseNumber(text) {
  if (!/^-?[0-9]+$/.test(text)) {
    throw new NotANumber(`"${text}" is not a number`);
  }
  return Number(text);
}

function sumFile(name) {
  const text = readFile(name); // a NotFound passes straight through
  let total = 0;
  for (const [index, line] of text.split("\n").entries()) {
    try {
      total += parseNumber(line);
    } catch (error) {
      // Throw a new error and keep the old one as its cause.
      throw new Error(`${name}: line ${index + 1}`, { cause: error });
    }
  }
  return total;
}

// A missing file counts as 0, but any other error stops the sum.
function sumAll(names) {
  let total = 0;
  for (const name of names) {
    try {
      total += sumFile(name);
    } catch (error) {
      if (!(error instanceof NotFound)) {
        throw error;
      }
    }
  }
  return total;
}

// Join an error's message with the messages of its causes.
function describe(error) {
  const parts = [];
  for (let cause = error; cause !== undefined; cause = cause.cause) {
    parts.push(cause.message);
  }
  return parts.join(": ");
}

function show(label, run) {
  try {
    console.log(`  ${label} -> ${run()}`);
  } catch (error) {
    console.log(`  ${label} -> error: ${describe(error)}`);
  }
}

console.log("one file at a time:");
for (const name of ["good.txt", "bad.txt", "missing.txt"]) {
  show(name, () => sumFile(name));
}
console.log("several files, where a missing file counts as 0:");
show("good.txt + missing.txt", () => sumAll(["good.txt", "missing.txt"]));
show("good.txt + bad.txt", () => sumAll(["good.txt", "bad.txt"]));
```

**TypeScript**

```ts
// Pretend files, so the example does not touch the disk.
const files = new Map<string, string>([
  ["good.txt", "1\n2\n3"],
  ["bad.txt", "4\nfive\n6"],
]);

// One error class per kind of failure, so callers can tell them apart.
class NotFound extends Error {}
class NotANumber extends Error {}

function readFile(name: string): string {
  const text = files.get(name);
  if (text === undefined) {
    throw new NotFound(`${name}: no such file`);
  }
  return text;
}

function parseNumber(text: string): number {
  if (!/^-?[0-9]+$/.test(text)) {
    throw new NotANumber(`"${text}" is not a number`);
  }
  return Number(text);
}

function sumFile(name: string): number {
  const text = readFile(name); // a NotFound passes straight through
  let total = 0;
  for (const [index, line] of text.split("\n").entries()) {
    try {
      total += parseNumber(line);
    } catch (error) {
      // Throw a new error and keep the old one as its cause.
      throw new Error(`${name}: line ${index + 1}`, { cause: error });
    }
  }
  return total;
}

// A missing file counts as 0, but any other error stops the sum.
function sumAll(names: string[]): number {
  let total = 0;
  for (const name of names) {
    try {
      total += sumFile(name);
    } catch (error) {
      if (!(error instanceof NotFound)) {
        throw error;
      }
    }
  }
  return total;
}

// Join an error's message with the messages of its causes. Anything can be
// thrown, so a cause has the type unknown until we check it.
function describe(error: unknown): string {
  const parts: string[] = [];
  let cause = error;
  while (cause instanceof Error) {
    parts.push(cause.message);
    cause = cause.cause;
  }
  return parts.join(": ");
}

function show(label: string, run: () => number): void {
  try {
    console.log(`  ${label} -> ${run()}`);
  } catch (error) {
    console.log(`  ${label} -> error: ${describe(error)}`);
  }
}

console.log("one file at a time:");
for (const name of ["good.txt", "bad.txt", "missing.txt"]) {
  show(name, () => sumFile(name));
}
console.log("several files, where a missing file counts as 0:");
show("good.txt + missing.txt", () => sumAll(["good.txt", "missing.txt"]));
show("good.txt + bad.txt", () => sumAll(["good.txt", "bad.txt"]));
```

**Dart**

```dart
// Pretend files, so the example does not touch the disk.
const files = {'good.txt': '1\n2\n3', 'bad.txt': '4\nfive\n6'};

// One exception class per kind of failure, so callers can catch just one.
class NotFound implements Exception {
  final String message;
  NotFound(this.message);
  @override
  String toString() => message;
}

class NotANumber implements Exception {
  final String message;
  NotANumber(this.message);
  @override
  String toString() => message;
}

// Dart exceptions have no built-in cause, so this class keeps its own.
class BadLine implements Exception {
  final String message;
  final Object cause;
  BadLine(this.message, this.cause);
  @override
  String toString() => message;
}

String readFile(String name) =>
    files[name] ?? (throw NotFound('$name: no such file'));

int parseNumber(String text) =>
    int.tryParse(text) ?? (throw NotANumber('"$text" is not a number'));

int sumFile(String name) {
  final text = readFile(name); // a NotFound passes straight through
  var total = 0;
  final lines = text.split('\n');
  for (var index = 0; index < lines.length; index++) {
    try {
      total += parseNumber(lines[index]);
    } on NotANumber catch (error) {
      throw BadLine('$name: line ${index + 1}', error);
    }
  }
  return total;
}

// A missing file counts as 0, but any other error stops the sum.
int sumAll(List<String> names) {
  var total = 0;
  for (final name in names) {
    try {
      total += sumFile(name);
    } on NotFound {
      // skip it
    }
  }
  return total;
}

// Join an exception's message with the messages of its causes.
String describe(Object error) =>
    error is BadLine ? '$error: ${describe(error.cause)}' : '$error';

void show(String label, int Function() run) {
  try {
    print('  $label -> ${run()}');
  } on Exception catch (error) {
    print('  $label -> error: ${describe(error)}');
  }
}

void main() {
  print('one file at a time:');
  for (final name in ['good.txt', 'bad.txt', 'missing.txt']) {
    show(name, () => sumFile(name));
  }
  print('several files, where a missing file counts as 0:');
  show('good.txt + missing.txt', () => sumAll(['good.txt', 'missing.txt']));
  show('good.txt + bad.txt', () => sumAll(['good.txt', 'bad.txt']));
}
```

**Kotlin**

```kotlin
// Pretend files, so the example does not touch the disk.
val files = mapOf("good.txt" to "1\n2\n3", "bad.txt" to "4\nfive\n6")

// One exception class per kind of failure, so callers can catch just one.
class NotFound(message: String) : Exception(message)

class NotANumber(message: String) : Exception(message)

class BadLine(message: String, cause: Throwable) : Exception(message, cause)

fun readFile(name: String): String = files[name] ?: throw NotFound("$name: no such file")

fun parseNumber(text: String): Int =
    text.toIntOrNull() ?: throw NotANumber("\"$text\" is not a number")

fun sumFile(name: String): Int {
    val text = readFile(name) // a NotFound passes straight through
    var total = 0
    for ((index, line) in text.lines().withIndex()) {
        try {
            total += parseNumber(line)
        } catch (e: NotANumber) {
            // Throw a new exception and keep the old one as its cause.
            throw BadLine("$name: line ${index + 1}", e)
        }
    }
    return total
}

// A missing file counts as 0, but any other error stops the sum.
fun sumAll(names: List<String>): Int {
    var total = 0
    for (name in names) {
        try {
            total += sumFile(name)
        } catch (_: NotFound) {}
    }
    return total
}

// Join an exception's message with the messages of its causes.
fun describe(error: Throwable): String =
    generateSequence(error) { it.cause }.joinToString(": ") { it.message ?: "" }

fun show(label: String, run: () -> Int) {
    try {
        println("  $label -> ${run()}")
    } catch (e: Exception) {
        println("  $label -> error: ${describe(e)}")
    }
}

fun main() {
    println("one file at a time:")
    for (name in listOf("good.txt", "bad.txt", "missing.txt")) {
        show(name) { sumFile(name) }
    }
    println("several files, where a missing file counts as 0:")
    show("good.txt + missing.txt") { sumAll(listOf("good.txt", "missing.txt")) }
    show("good.txt + bad.txt") { sumAll(listOf("good.txt", "bad.txt")) }
}
```

All nine print the same lines:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
one file at a time:
  good.txt -> 6
  bad.txt -> error: bad.txt: line 2: "five" is not a number
  missing.txt -> error: missing.txt: no such file
several files, where a missing file counts as 0:
  good.txt + missing.txt -> 6
  good.txt + bad.txt -> error: bad.txt: line 2: "five" is not a number
```

**Passing an error up unchanged.** In C and Go, it is a `return`: `return status;` and
`return 0, err`. In Rust, `read_file(name)?` does the same in one character: on `Err`,
it returns from `sum_file` with the error. On the way, `?` converts the error with
`From`, and the `impl From<NotFound> for SumError` is what lets it turn a `NotFound`
into a `SumError`. In the six exception languages, it takes nothing at all. The
`NotFound` thrown in `read_file` flies straight through `sum_file`, which has no `try`
around that call.

**Adding context.** `"five" is not a number` is not much use without the file and the
line. Go builds a new message with `fmt.Errorf("%s: line %d: %w", name, index+1, err)`,
and `%w` keeps the original error inside the new one. Rust's `map_err` turns the
`NotANumber` into a `BadLine` variant that keeps the original as its `source`. C prints
the message into a bigger buffer with the context in front. The exception languages
catch the error and throw a new one that holds the old one as its **cause**:
`raise BadLine(...) from error` in Python, `new Error(message, { cause: error })` in
JavaScript and TypeScript, `Exception(message, cause)` in Kotlin, and
`std::throw_with_nested` in C++. Dart's exceptions have no cause, so `BadLine` keeps its
own.

**Walking the chain.** Each `describe` starts at the outer error and joins the messages
of every cause inside it: Rust follows `source()`, Python `__cause__`, JavaScript and
Kotlin `cause`, and C++ `std::rethrow_if_nested`. Go needs no `describe`, because `%w`
already put the inner message into the outer one, and `errors.Unwrap` gives you the
inner error if you want it. Python prints the whole chain by itself when nothing catches
the outer exception. Here is a cut-down version of the Python example, with nothing to
catch the `BadLine`:

```text
$ python3 main.py
Traceback (most recent call last):
  File "/tmp/ch11/main.py", line 15, in sum_file
    parse_number("five")
    ~~~~~~~~~~~~^^^^^^^^
  File "/tmp/ch11/main.py", line 10, in parse_number
    raise NotANumber(f'"{text}" is not a number')
NotANumber: "five" is not a number

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/tmp/ch11/main.py", line 20, in <module>
    sum_file("bad.txt")
    ~~~~~~~~^^^^^^^^^^^
  File "/tmp/ch11/main.py", line 17, in sum_file
    raise BadLine(f"{name}: line 2") from error
BadLine: bad.txt: line 2
```

`parse_number` uses `from None` instead, because `int`'s own `ValueError` adds nothing
that `NotANumber` does not say. Without `from`, Python would print both anyway, joined
by "During handling of the above exception, another exception occurred". Python 3.11
also added `error.add_note("...")`, which adds a line of context to an exception without
wrapping it in a new one.

**Deciding by kind, through the wrapping.** `sum_all` has to tell a missing file from a
bad line. Go's `errors.Is(err, ErrNotFound)` looks through every `%w` to find the
sentinel. Rust matches the `SumError::NotFound` variant, and C compares the status code.
The exception languages catch only `NotFound`, so every other error keeps going up.
JavaScript and TypeScript can't catch by type, so `sumAll` catches everything, checks
`instanceof NotFound`, and throws the rest again.

**At the top, someone has to handle it.** Rust's `main` can return a `Result`, and an
`Err` ends the program with exit code 1:

```rust
use std::error::Error;

fn main() -> Result<(), Box<dyn Error>> {
    let port: u16 = "70000".parse()?;
    println!("port {port}");
    Ok(())
}
```

`Box<dyn Error>` holds any error type, so `?` can return any of them. The program prints
the error with `{:?}`, not with its message, which is fine for a quick tool and too raw
for users:

```text
$ ./main
Error: ParseIntError { kind: PosOverflow }
```

---

## 11.5 Error types that callers can match on

A caller can only react to the kinds of error it can tell apart. In this example,
`withdraw` takes money from an account and can fail in three ways: the amount is not
positive, the account does not have enough money, or the account is frozen. The second
carries numbers: how much was asked for and how much is there. The caller recovers from
a shortfall by taking what is there, gives advice for a frozen account, and only reports
a bad amount.

**C**

```c
#include <stdbool.h>
#include <stdio.h>

// The kinds of failure, plus a value that means success.
enum withdraw_kind {
    WITHDRAW_OK,
    INVALID_AMOUNT,
    INSUFFICIENT_FUNDS,
    ACCOUNT_FROZEN,
};

// The kind, plus the numbers a caller needs to recover.
struct withdraw_error {
    enum withdraw_kind kind;
    long amount;
    long available;
};

struct account {
    long balance;
    bool frozen;
};

struct withdraw_error withdraw(struct account *account, long amount) {
    if (amount <= 0) {
        return (struct withdraw_error){INVALID_AMOUNT, amount, 0};
    }
    if (account->frozen) {
        return (struct withdraw_error){ACCOUNT_FROZEN, amount, 0};
    }
    if (amount > account->balance) {
        return (struct withdraw_error){INSUFFICIENT_FUNDS, amount,
                                       account->balance};
    }
    account->balance -= amount;
    return (struct withdraw_error){WITHDRAW_OK, amount, 0};
}

void print_error(struct withdraw_error error) {
    // -Wall warns if this switch leaves out one of the enum's values.
    switch (error.kind) {
    case WITHDRAW_OK:
        printf("no error\n");
        break;
    case INVALID_AMOUNT:
        printf("invalid amount: %ld\n", error.amount);
        break;
    case INSUFFICIENT_FUNDS:
        printf("insufficient funds: need %ld, have %ld\n", error.amount,
               error.available);
        break;
    case ACCOUNT_FROZEN:
        printf("account is frozen\n");
        break;
    }
}

void attempt(struct account *account, long amount) {
    struct withdraw_error error = withdraw(account, amount);
    if (error.kind == WITHDRAW_OK) {
        printf("withdraw %ld: ok, %ld left\n", amount, account->balance);
        return;
    }
    printf("withdraw %ld: error: ", amount);
    print_error(error);
    if (error.kind == INSUFFICIENT_FUNDS) {
        withdraw(account, error.available);
        printf("  took %ld instead, %ld left\n", error.available,
               account->balance);
    } else if (error.kind == ACCOUNT_FROZEN) {
        printf("  call the bank to unfreeze it\n");
    }
}

int main(void) {
    struct account account = {100, false};
    printf("start with %ld\n", account.balance);
    long amounts[] = {30, -5, 100};
    for (int i = 0; i < 3; i++) {
        attempt(&account, amounts[i]);
    }
    printf("freeze the account\n");
    account.frozen = true;
    attempt(&account, 10);
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <stdexcept>
#include <string>

// The base class: catch it to handle every kind of withdraw failure.
struct WithdrawError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct InvalidAmount : WithdrawError {
    explicit InvalidAmount(long amount)
        : WithdrawError("invalid amount: " + std::to_string(amount)) {}
};

struct InsufficientFunds : WithdrawError {
    long needed;
    long available;
    InsufficientFunds(long needed, long available)
        : WithdrawError("insufficient funds: need " + std::to_string(needed) +
                        ", have " + std::to_string(available)),
          needed(needed), available(available) {}
};

struct AccountFrozen : WithdrawError {
    AccountFrozen() : WithdrawError("account is frozen") {}
};

struct Account {
    long balance;
    bool frozen = false;

    void withdraw(long amount) {
        if (amount <= 0) {
            throw InvalidAmount(amount);
        }
        if (frozen) {
            throw AccountFrozen();
        }
        if (amount > balance) {
            throw InsufficientFunds(amount, balance);
        }
        balance -= amount;
    }
};

void attempt(Account &account, long amount) {
    std::cout << "withdraw " << amount << ": ";
    try {
        account.withdraw(amount);
        std::cout << "ok, " << account.balance << " left\n";
    } catch (const InsufficientFunds &error) {
        // The exception carries data, so the caller can recover.
        std::cout << "error: " << error.what() << '\n';
        account.withdraw(error.available);
        std::cout << "  took " << error.available << " instead, "
                  << account.balance << " left\n";
    } catch (const AccountFrozen &error) {
        std::cout << "error: " << error.what() << '\n';
        std::cout << "  call the bank to unfreeze it\n";
    } catch (const WithdrawError &error) {
        // Catch blocks are tried in order, so the base class goes last.
        std::cout << "error: " << error.what() << '\n';
    }
}

int main() {
    Account account{100};
    std::cout << "start with " << account.balance << '\n';
    for (long amount : {30, -5, 100}) {
        attempt(account, amount);
    }
    std::cout << "freeze the account\n";
    account.frozen = true;
    attempt(account, 10);
}
```

**Rust**

```rust
use std::error::Error;
use std::fmt;

// Every way a withdraw can fail, with the data each one carries.
#[derive(Debug)]
enum WithdrawError {
    InvalidAmount(i64),
    InsufficientFunds { needed: i64, available: i64 },
    Frozen,
}

impl fmt::Display for WithdrawError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            WithdrawError::InvalidAmount(amount) => write!(f, "invalid amount: {amount}"),
            WithdrawError::InsufficientFunds { needed, available } => {
                write!(f, "insufficient funds: need {needed}, have {available}")
            }
            WithdrawError::Frozen => write!(f, "account is frozen"),
        }
    }
}

impl Error for WithdrawError {}

struct Account {
    balance: i64,
    frozen: bool,
}

impl Account {
    fn withdraw(&mut self, amount: i64) -> Result<(), WithdrawError> {
        if amount <= 0 {
            return Err(WithdrawError::InvalidAmount(amount));
        }
        if self.frozen {
            return Err(WithdrawError::Frozen);
        }
        if amount > self.balance {
            return Err(WithdrawError::InsufficientFunds {
                needed: amount,
                available: self.balance,
            });
        }
        self.balance -= amount;
        Ok(())
    }
}

fn attempt(account: &mut Account, amount: i64) {
    match account.withdraw(amount) {
        Ok(()) => println!("withdraw {amount}: ok, {} left", account.balance),
        // error @ binds the whole error and also pulls out available.
        Err(error @ WithdrawError::InsufficientFunds { available, .. }) => {
            println!("withdraw {amount}: error: {error}");
            account.withdraw(available).expect("the balance covers it");
            println!("  took {available} instead, {} left", account.balance);
        }
        Err(error @ WithdrawError::Frozen) => {
            println!("withdraw {amount}: error: {error}");
            println!("  call the bank to unfreeze it");
        }
        Err(error) => println!("withdraw {amount}: error: {error}"),
    }
}

fn main() {
    let mut account = Account {
        balance: 100,
        frozen: false,
    };
    println!("start with {}", account.balance);
    for amount in [30, -5, 100] {
        attempt(&mut account, amount);
    }
    println!("freeze the account");
    account.frozen = true;
    attempt(&mut account, 10);
}
```

**Go**

```go
package main

import (
	"errors"
	"fmt"
)

// A sentinel error: one fixed value that callers test with errors.Is.
var ErrFrozen = errors.New("account is frozen")

// An error type: it carries data, and callers find it with errors.As.
type InsufficientFundsError struct {
	Needed, Available int
}

func (e *InsufficientFundsError) Error() string {
	return fmt.Sprintf("insufficient funds: need %d, have %d", e.Needed, e.Available)
}

type Account struct {
	balance int
	frozen  bool
}

func (a *Account) Withdraw(amount int) error {
	if amount <= 0 {
		// A plain error: callers can only print it.
		return fmt.Errorf("invalid amount: %d", amount)
	}
	if a.frozen {
		return ErrFrozen
	}
	if amount > a.balance {
		return &InsufficientFundsError{Needed: amount, Available: a.balance}
	}
	a.balance -= amount
	return nil
}

func attempt(account *Account, amount int) {
	err := account.Withdraw(amount)
	if err == nil {
		fmt.Printf("withdraw %d: ok, %d left\n", amount, account.balance)
		return
	}
	fmt.Printf("withdraw %d: error: %v\n", amount, err)
	var short *InsufficientFundsError
	switch {
	case errors.As(err, &short):
		// errors.As found the error and set short to point at it.
		if err := account.Withdraw(short.Available); err != nil {
			panic(err)
		}
		fmt.Printf("  took %d instead, %d left\n", short.Available, account.balance)
	case errors.Is(err, ErrFrozen):
		fmt.Println("  call the bank to unfreeze it")
	}
}

func main() {
	account := &Account{balance: 100}
	fmt.Printf("start with %d\n", account.balance)
	for _, amount := range []int{30, -5, 100} {
		attempt(account, amount)
	}
	fmt.Println("freeze the account")
	account.frozen = true
	attempt(account, 10)
}
```

**Python**

```python
class WithdrawError(Exception):
    """The base class: catch it to handle every kind of withdraw failure."""


class InvalidAmount(WithdrawError):
    def __init__(self, amount: int):
        super().__init__(f"invalid amount: {amount}")
        self.amount = amount


class InsufficientFunds(WithdrawError):
    def __init__(self, needed: int, available: int):
        super().__init__(f"insufficient funds: need {needed}, have {available}")
        self.needed = needed
        self.available = available


class AccountFrozen(WithdrawError):
    def __init__(self):
        super().__init__("account is frozen")


class Account:
    def __init__(self, balance: int):
        self.balance = balance
        self.frozen = False

    def withdraw(self, amount: int) -> None:
        if amount <= 0:
            raise InvalidAmount(amount)
        if self.frozen:
            raise AccountFrozen()
        if amount > self.balance:
            raise InsufficientFunds(amount, self.balance)
        self.balance -= amount


def attempt(account: Account, amount: int) -> None:
    try:
        account.withdraw(amount)
        print(f"withdraw {amount}: ok, {account.balance} left")
    except InsufficientFunds as error:
        # The exception carries data, so the caller can recover.
        print(f"withdraw {amount}: error: {error}")
        account.withdraw(error.available)
        print(f"  took {error.available} instead, {account.balance} left")
    except AccountFrozen as error:
        print(f"withdraw {amount}: error: {error}")
        print("  call the bank to unfreeze it")
    except WithdrawError as error:
        print(f"withdraw {amount}: error: {error}")


account = Account(100)
print(f"start with {account.balance}")
for amount in [30, -5, 100]:
    attempt(account, amount)
print("freeze the account")
account.frozen = True
attempt(account, 10)
```

**JavaScript**

```js
// The base class: check for it to handle every kind of withdraw failure.
class WithdrawError extends Error {}

class InvalidAmount extends WithdrawError {
  constructor(amount) {
    super(`invalid amount: ${amount}`);
    this.amount = amount;
  }
}

class InsufficientFunds extends WithdrawError {
  constructor(needed, available) {
    super(`insufficient funds: need ${needed}, have ${available}`);
    this.needed = needed;
    this.available = available;
  }
}

class AccountFrozen extends WithdrawError {
  constructor() {
    super("account is frozen");
  }
}

class Account {
  constructor(balance) {
    this.balance = balance;
    this.frozen = false;
  }

  withdraw(amount) {
    if (amount <= 0) {
      throw new InvalidAmount(amount);
    }
    if (this.frozen) {
      throw new AccountFrozen();
    }
    if (amount > this.balance) {
      throw new InsufficientFunds(amount, this.balance);
    }
    this.balance -= amount;
  }
}

function attempt(account, amount) {
  try {
    account.withdraw(amount);
    console.log(`withdraw ${amount}: ok, ${account.balance} left`);
  } catch (error) {
    // catch receives anything thrown, so pass on what we do not expect.
    if (!(error instanceof WithdrawError)) {
      throw error;
    }
    console.log(`withdraw ${amount}: error: ${error.message}`);
    if (error instanceof InsufficientFunds) {
      account.withdraw(error.available);
      console.log(`  took ${error.available} instead, ${account.balance} left`);
    } else if (error instanceof AccountFrozen) {
      console.log("  call the bank to unfreeze it");
    }
  }
}

const account = new Account(100);
console.log(`start with ${account.balance}`);
for (const amount of [30, -5, 100]) {
  attempt(account, amount);
}
console.log("freeze the account");
account.frozen = true;
attempt(account, 10);
```

**TypeScript**

```ts
// The base class: check for it to handle every kind of withdraw failure.
class WithdrawError extends Error {}

class InvalidAmount extends WithdrawError {
  readonly amount: number;
  constructor(amount: number) {
    super(`invalid amount: ${amount}`);
    this.amount = amount;
  }
}

class InsufficientFunds extends WithdrawError {
  readonly needed: number;
  readonly available: number;
  constructor(needed: number, available: number) {
    super(`insufficient funds: need ${needed}, have ${available}`);
    this.needed = needed;
    this.available = available;
  }
}

class AccountFrozen extends WithdrawError {
  constructor() {
    super("account is frozen");
  }
}

class Account {
  balance: number;
  frozen = false;
  constructor(balance: number) {
    this.balance = balance;
  }

  // Nothing in this signature says it can throw.
  withdraw(amount: number): void {
    if (amount <= 0) {
      throw new InvalidAmount(amount);
    }
    if (this.frozen) {
      throw new AccountFrozen();
    }
    if (amount > this.balance) {
      throw new InsufficientFunds(amount, this.balance);
    }
    this.balance -= amount;
  }
}

function attempt(account: Account, amount: number): void {
  try {
    account.withdraw(amount);
    console.log(`withdraw ${amount}: ok, ${account.balance} left`);
  } catch (error) {
    // error has the type unknown until instanceof narrows it.
    if (!(error instanceof WithdrawError)) {
      throw error;
    }
    console.log(`withdraw ${amount}: error: ${error.message}`);
    if (error instanceof InsufficientFunds) {
      account.withdraw(error.available);
      console.log(`  took ${error.available} instead, ${account.balance} left`);
    } else if (error instanceof AccountFrozen) {
      console.log("  call the bank to unfreeze it");
    }
  }
}

const account = new Account(100);
console.log(`start with ${account.balance}`);
for (const amount of [30, -5, 100]) {
  attempt(account, amount);
}
console.log("freeze the account");
account.frozen = true;
attempt(account, 10);
```

**Dart**

```dart
// A sealed class: every subclass is in this file, so a switch over a
// WithdrawError can be checked for missing cases.
sealed class WithdrawError implements Exception {
  final String message;
  WithdrawError(this.message);
  @override
  String toString() => message;
}

class InvalidAmount extends WithdrawError {
  final int amount;
  InvalidAmount(this.amount) : super('invalid amount: $amount');
}

class InsufficientFunds extends WithdrawError {
  final int needed;
  final int available;
  InsufficientFunds(this.needed, this.available)
    : super('insufficient funds: need $needed, have $available');
}

class AccountFrozen extends WithdrawError {
  AccountFrozen() : super('account is frozen');
}

class Account {
  int balance;
  bool frozen = false;
  Account(this.balance);

  void withdraw(int amount) {
    if (amount <= 0) throw InvalidAmount(amount);
    if (frozen) throw AccountFrozen();
    if (amount > balance) throw InsufficientFunds(amount, balance);
    balance -= amount;
  }
}

void attempt(Account account, int amount) {
  try {
    account.withdraw(amount);
    print('withdraw $amount: ok, ${account.balance} left');
  } on WithdrawError catch (error) {
    print('withdraw $amount: error: $error');
    // Leave out a case and dart analyze reports it.
    switch (error) {
      case InsufficientFunds(:final available):
        account.withdraw(available);
        print('  took $available instead, ${account.balance} left');
      case AccountFrozen():
        print('  call the bank to unfreeze it');
      case InvalidAmount():
        break;
    }
  }
}

void main() {
  final account = Account(100);
  print('start with ${account.balance}');
  for (final amount in [30, -5, 100]) {
    attempt(account, amount);
  }
  print('freeze the account');
  account.frozen = true;
  attempt(account, 10);
}
```

**Kotlin**

```kotlin
// A sealed class: every subclass is in this file, so a when over a
// WithdrawException can be checked for missing cases.
sealed class WithdrawException(message: String) : Exception(message)

class InvalidAmount(val amount: Int) : WithdrawException("invalid amount: $amount")

class InsufficientFunds(val needed: Int, val available: Int) :
    WithdrawException("insufficient funds: need $needed, have $available")

class AccountFrozen : WithdrawException("account is frozen")

class Account(var balance: Int) {
    var frozen = false

    fun withdraw(amount: Int) {
        if (amount <= 0) throw InvalidAmount(amount)
        if (frozen) throw AccountFrozen()
        if (amount > balance) throw InsufficientFunds(amount, balance)
        balance -= amount
    }
}

fun attempt(account: Account, amount: Int) {
    try {
        account.withdraw(amount)
        println("withdraw $amount: ok, ${account.balance} left")
    } catch (e: WithdrawException) {
        println("withdraw $amount: error: ${e.message}")
        // Leave out a case and the compiler reports it.
        when (e) {
            is InsufficientFunds -> {
                account.withdraw(e.available)
                println("  took ${e.available} instead, ${account.balance} left")
            }
            is AccountFrozen -> println("  call the bank to unfreeze it")
            is InvalidAmount -> {}
        }
    }
}

fun main() {
    val account = Account(100)
    println("start with ${account.balance}")
    for (amount in listOf(30, -5, 100)) {
        attempt(account, amount)
    }
    println("freeze the account")
    account.frozen = true
    attempt(account, 10)
}
```

All nine print the same lines:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
start with 100
withdraw 30: ok, 70 left
withdraw -5: error: invalid amount: -5
withdraw 100: error: insufficient funds: need 100, have 70
  took 70 instead, 0 left
freeze the account
withdraw 10: error: account is frozen
  call the bank to unfreeze it
```

**C: a status enum, and a struct with the details.** `withdraw` returns a small struct
by value, holding the kind and the numbers. A `switch` over an enum that leaves out one
of its values is a warning under `-Wall`, and an error with `-Werror`. Delete the
`ACCOUNT_FROZEN` case and GCC says
`error: enumeration value 'ACCOUNT_FROZEN' not handled in switch [-Werror=switch]`.

**C++, Python, JavaScript, and TypeScript: a class hierarchy.** Each kind of failure is
a subclass of `WithdrawError`, with its data in fields. A handler for the base class
catches every kind, so it goes last: `catch` and `except` clauses are tried in order,
and a `WithdrawError` handler first would take them all. In JavaScript and TypeScript,
one `catch` receives everything that is thrown, so `attempt` checks
`instanceof WithdrawError` and throws anything else again, rather than mistake a bug for
a withdraw failure.

**Rust: an enum.** Each variant carries its own data, and a `match` must cover every
variant. Delete the `Frozen` line from `Display` and the build stops with
`error[E0004]: non-exhaustive patterns: '&WithdrawError::Frozen' not covered`. In
`attempt`, `error @ WithdrawError::InsufficientFunds { available, .. }` binds the whole
error to `error` and its `available` field to `available` in one pattern.

**Go: three styles side by side.** `ErrFrozen` is a sentinel, one fixed value that
callers compare with `errors.Is`. `InsufficientFundsError` is an error type that carries
data, and `errors.As` finds it and sets `short` to point at it. The invalid amount is a
plain `fmt.Errorf` error, which callers can only print. The standard library uses all
three: `io.EOF` is a sentinel, `*fs.PathError` is a type, and many functions return
plain messages.

**Dart and Kotlin: sealed classes.** A sealed class's subclasses must all be declared
next to it, in the same library in Dart and the same package and module in Kotlin, so
the compiler knows the complete list. A `switch` in Dart or a `when` in Kotlin over a
sealed type must then cover every subclass. Remove the `InvalidAmount` case and Dart
reports
`The type 'WithdrawError' isn't exhaustively matched by the switch cases since it doesn't match the pattern 'InvalidAmount()'`,
and Kotlin
`'when' expression must be exhaustive. Add the 'is InvalidAmount' branch or an 'else' branch.`
Both still throw the error as an exception, and match on it inside the `catch`.

---

## 11.6 Bugs are not errors

Some failures mean that the program itself is wrong. This example does four things that
a correct program never does. It reads index 5 of a three-item list, divides by zero,
uses the result of a lookup that found nothing, and asserts something false. Each
program catches what it can, so that all four lines run. That is for the demonstration;
section 11.10 says when real code should.

**C**

```c
#include <stddef.h>
#include <stdio.h>

struct user {
    int id;
    const char *name;
};

static const struct user users[] = {{1, "ada"}};

const struct user *find_user(int id) {
    for (size_t i = 0; i < sizeof users / sizeof users[0]; i++) {
        if (users[i].id == id) {
            return &users[i];
        }
    }
    return NULL;
}

int main(void) {
    int numbers[] = {10, 20, 30};
    size_t length = sizeof numbers / sizeof numbers[0];
    size_t index = 5;
    int zero = 0;

    // C checks none of these at run time. Each one is undefined behavior, so
    // the program has to test before it acts.
    if (index < length) {
        printf("numbers[5]: %d\n", numbers[index]);
    } else {
        printf("numbers[5]: skipped, because 5 is not less than %zu\n",
               length);
    }

    if (zero != 0) {
        printf("10 / zero: %d\n", 10 / zero);
    } else {
        printf("10 / zero: skipped, because zero is 0\n");
    }

    const struct user *user = find_user(7);
    if (user != NULL) {
        printf("find_user(7)->name: %s\n", user->name);
    } else {
        printf("find_user(7)->name: skipped, because find_user returned "
               "NULL\n");
    }
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

struct User {
    int id;
    std::string name;
};

const std::vector<User> users{{1, "ada"}};

std::optional<User> find_user(int id) {
    for (const User &user : users) {
        if (user.id == id) {
            return user;
        }
    }
    return std::nullopt;
}

template <typename Action> void attempt(const std::string &label, Action action) {
    std::cout << label << ": ";
    try {
        std::cout << action() << '\n';
    } catch (const std::out_of_range &) {
        std::cout << "threw std::out_of_range\n";
    } catch (const std::bad_optional_access &) {
        std::cout << "threw std::bad_optional_access\n";
    }
}

int main() {
    std::vector<int> numbers{10, 20, 30};
    std::size_t index = 5;
    int zero = 0;

    // The checked versions throw an exception.
    attempt("numbers.at(5)", [&] { return numbers.at(index); });
    attempt("find_user(7).value().name",
            [] { return find_user(7).value().name; });

    // The unchecked versions, numbers[5] and *find_user(7), are undefined
    // behavior, and so is dividing by zero. Test first instead.
    if (zero != 0) {
        std::cout << "10 / zero: " << 10 / zero << '\n';
    } else {
        std::cout << "10 / zero: skipped, because zero is 0\n";
    }
}
```

**Rust**

```rust
use std::panic;

struct User {
    id: u32,
    name: String,
}

fn find_user(users: &[User], id: u32) -> Option<&User> {
    users.iter().find(|user| user.id == id)
}

// Run action and report what it returned or the message it panicked with.
fn attempt(label: &str, action: impl FnOnce() -> String + panic::UnwindSafe) {
    match panic::catch_unwind(action) {
        Ok(value) => println!("{label}: {value}"),
        Err(payload) => {
            // A panic's payload is usually a String or a &str.
            let message = payload
                .downcast_ref::<String>()
                .map(String::as_str)
                .or_else(|| payload.downcast_ref::<&str>().copied())
                .unwrap_or("(no message)");
            println!("{label}: panicked: {message}");
        }
    }
}

fn main() {
    // By default a panic prints its message to stderr. This hook prints nothing,
    // so attempt can print the message itself.
    panic::set_hook(Box::new(|_| {}));

    let numbers = vec![10, 20, 30];
    let users = vec![User {
        id: 1,
        name: "ada".to_string(),
    }];
    let (index, zero) = (5, 0);

    // Each of these is a bug, and each one panics.
    attempt("numbers[5]", || numbers[index].to_string());
    attempt("10 / zero", || (10 / zero).to_string());
    attempt("find_user(7).unwrap().name", || {
        find_user(&users, 7).unwrap().name.clone()
    });
    attempt("assert!(numbers.len() == 4)", || {
        assert!(numbers.len() == 4);
        "passed".to_string()
    });

    // The checked versions return an Option instead.
    attempt("numbers.get(5)", || format!("{:?}", numbers.get(index)));
    attempt("10.checked_div(zero)", || {
        format!("{:?}", 10_i32.checked_div(zero))
    });
}
```

**Go**

```go
package main

import "fmt"

type User struct {
	Name string
}

var users = map[int]*User{1: {Name: "ada"}}

// findUser returns nil when there is no such user.
func findUser(id int) *User {
	return users[id]
}

// attempt runs action and reports what it returned or what it panicked with.
func attempt(label string, action func() any) {
	defer func() {
		// recover stops the panic, and returns the value it was raised with.
		if value := recover(); value != nil {
			fmt.Printf("%s: panic: %v\n", label, value)
		}
	}()
	fmt.Printf("%s: %v\n", label, action())
}

func main() {
	numbers := []int{10, 20, 30}
	index, zero := 5, 0

	// Each of these is a bug, and each one panics.
	attempt("numbers[5]", func() any { return numbers[index] })
	attempt("10 / zero", func() any { return 10 / zero })
	attempt("findUser(7).Name", func() any { return findUser(7).Name })
	attempt("len(numbers) != 4", func() any {
		// Go has no assert. You check, and panic yourself.
		if len(numbers) != 4 {
			panic("expected 4 numbers")
		}
		return "passed"
	})
}
```

**Python**

```python
from dataclasses import dataclass


@dataclass
class User:
    name: str


numbers = [10, 20, 30]
users = {1: User("ada")}
index, zero = 5, 0


def find_user(user_id: int) -> User | None:
    return users.get(user_id)


def attempt(label, action):
    """Run action and report what it returned or which exception it raised."""
    try:
        print(f"{label}: {action()}")
    except Exception as error:
        print(f"{label}: {type(error).__name__}: {error}")


def check_length():
    assert len(numbers) == 4, "expected 4 numbers"
    return "passed"


# Each of these is a bug, and Python raises an ordinary exception for each.
attempt("numbers[5]", lambda: numbers[index])
attempt("10 // zero", lambda: 10 // zero)
# A type checker such as mypy reports this line: find_user can return None.
attempt("find_user(7).name", lambda: find_user(7).name)
attempt("assert len(numbers) == 4", check_length)
```

**JavaScript**

```js
import assert from "node:assert";

const numbers = [10, 20, 30];
const users = new Map([[1, { name: "ada" }]]);
const [index, zero] = [5, 0];

// findUser returns undefined when there is no such user.
function findUser(id) {
  return users.get(id);
}

// Run action and report what it returned or which error it threw.
function attempt(label, action) {
  try {
    console.log(`${label}: ${action()}`);
  } catch (error) {
    console.log(`${label}: ${error.name}: ${error.message}`);
  }
}

// Only two of these throw. The other two give a value and carry on.
attempt("numbers[5]", () => numbers[index]);
attempt("10 / zero", () => 10 / zero);
attempt("findUser(7).name", () => findUser(7).name);
attempt("assert(numbers.length === 4)", () => {
  assert(numbers.length === 4, "expected 4 numbers");
  return "passed";
});
```

**TypeScript**

```ts
import assert from "node:assert";

interface User {
  name: string;
}

const numbers: number[] = [10, 20, 30];
const users = new Map<number, User>([[1, { name: "ada" }]]);
const [index, zero] = [5, 0];

function findUser(id: number): User | undefined {
  return users.get(id);
}

// Run action and report what it returned or which error it threw.
function attempt(label: string, action: () => unknown): void {
  try {
    console.log(`${label}: ${action()}`);
  } catch (error) {
    if (!(error instanceof Error)) {
      throw error;
    }
    console.log(`${label}: ${error.name}: ${error.message}`);
  }
}

// numbers[index] has the type number, even though it gives undefined here.
attempt("numbers[5]", () => numbers[index]);
attempt("10 / zero", () => 10 / zero);
// findUser(7).name does not compile: the result is possibly undefined.
// The ! tells the compiler "trust me", so it compiles, and fails when run.
attempt("findUser(7)!.name", () => findUser(7)!.name);
attempt("assert(numbers.length === 4)", () => {
  assert(numbers.length === 4, "expected 4 numbers");
  return "passed";
});
```

**Dart**

```dart
class User {
  final String name;
  User(this.name);
}

final numbers = [10, 20, 30];
final users = {1: User('ada')};
const index = 5, zero = 0;

User? findUser(int id) => users[id];

// Run action and report what it returned or what it threw.
void attempt(String label, Object? Function() action) {
  try {
    print('$label: ${action()}');
  } catch (error) {
    // A RangeError's message starts with its type, but a TypeError's does not.
    final type = error is TypeError ? 'TypeError: ' : '';
    print('$label: $type$error');
  }
}

void main() {
  attempt('numbers[5]', () => numbers[index]);
  attempt('10 ~/ zero', () => 10 ~/ zero);
  attempt('10 / zero', () => 10 / zero);
  attempt('findUser(7)!.name', () => findUser(7)!.name);
  attempt('assert(numbers.length == 4)', () {
    // dart run skips asserts unless you pass --enable-asserts.
    assert(numbers.length == 4, 'expected 4 numbers');
    return 'passed';
  });
  // The checked version returns null instead.
  attempt('numbers.elementAtOrNull(5)', () => numbers.elementAtOrNull(index));
}
```

**Kotlin**

```kotlin
class User(val name: String)

val numbers = listOf(10, 20, 30)
val users = mapOf(1 to User("ada"))

fun findUser(id: Int): User? = users[id]

// Run action and report what it returned or which exception it threw.
fun attempt(label: String, action: () -> Any?) {
    try {
        println("$label: ${action()}")
    } catch (e: Exception) {
        val message = e.message?.let { ": $it" } ?: ""
        println("$label: ${e::class.simpleName}$message")
    }
}

fun main() {
    val index = 5
    val zero = 0
    attempt("numbers[5]") { numbers[index] }
    attempt("10 / zero") { 10 / zero }
    attempt("findUser(7)!!.name") { findUser(7)!!.name }
    attempt("check(numbers.size == 4)") {
        check(numbers.size == 4) { "expected 4 numbers" }
        "passed"
    }
    // The checked version returns null instead.
    attempt("numbers.getOrNull(5)") { numbers.getOrNull(index) }
}
```

Each language does something different:

*C:*

```text
numbers[5]: skipped, because 5 is not less than 3
10 / zero: skipped, because zero is 0
find_user(7)->name: skipped, because find_user returned NULL
```

*C++:*

```text
numbers.at(5): threw std::out_of_range
find_user(7).value().name: threw std::bad_optional_access
10 / zero: skipped, because zero is 0
```

*Rust:*

```text
numbers[5]: panicked: index out of bounds: the len is 3 but the index is 5
10 / zero: panicked: attempt to divide by zero
find_user(7).unwrap().name: panicked: called `Option::unwrap()` on a `None` value
assert!(numbers.len() == 4): panicked: assertion failed: numbers.len() == 4
numbers.get(5): None
10.checked_div(zero): None
```

*Go:*

```text
numbers[5]: panic: runtime error: index out of range [5] with length 3
10 / zero: panic: runtime error: integer divide by zero
findUser(7).Name: panic: runtime error: invalid memory address or nil pointer dereference
len(numbers) != 4: panic: expected 4 numbers
```

*Python:*

```text
numbers[5]: IndexError: list index out of range
10 // zero: ZeroDivisionError: integer division or modulo by zero
find_user(7).name: AttributeError: 'NoneType' object has no attribute 'name'
assert len(numbers) == 4: AssertionError: expected 4 numbers
```

*JavaScript:*

```text
numbers[5]: undefined
10 / zero: Infinity
findUser(7).name: TypeError: Cannot read properties of undefined (reading 'name')
assert(numbers.length === 4): AssertionError: expected 4 numbers
```

*TypeScript:*

```text
numbers[5]: undefined
10 / zero: Infinity
findUser(7)!.name: TypeError: Cannot read properties of undefined (reading 'name')
assert(numbers.length === 4): AssertionError: expected 4 numbers
```

*Dart:*

```text
numbers[5]: RangeError (length): Invalid value: Not in inclusive range 0..2: 5
10 ~/ zero: IntegerDivisionByZeroException
10 / zero: Infinity
findUser(7)!.name: TypeError: Null check operator used on a null value
assert(numbers.length == 4): passed
numbers.elementAtOrNull(5): null
```

*Kotlin:*

```text
numbers[5]: ArrayIndexOutOfBoundsException: Index 5 out of bounds for length 3
10 / zero: ArithmeticException: / by zero
findUser(7)!!.name: NullPointerException
check(numbers.size == 4): IllegalStateException: expected 4 numbers
numbers.getOrNull(5): null
```

**C and C++: undefined behavior, so check first.** C checks none of the three. Each one
is undefined behavior: the program might crash, or carry on with garbage. On Linux on
x86-64, dividing an integer by zero usually kills the program with the signal `SIGFPE`,
and the shell prints `Floating point exception`. C++ adds checked versions that throw:
`.at()` throws `std::out_of_range`, and `std::optional`'s `.value()` throws
`std::bad_optional_access`. `numbers[5]`, `*find_user(7)`, and `10 / zero` stay
unchecked. An exception that nothing catches ends the program through `std::terminate`.
`assert(condition)` prints the condition and aborts, and building with `-DNDEBUG`
removes every `assert`. Here is a small program whose line 7 is
`assert(count == 4 && "expected 4 numbers");`, run when `count` is 3, then built again
with `-DNDEBUG`:

```text
$ cc -std=c17 -O2 main.c -o main && ./main
main: main.c:7: main: Assertion `count == 4 && "expected 4 numbers"' failed.
$ echo $?
134
$ cc -std=c17 -O2 -DNDEBUG main.c -o main && ./main
10
```

Exit code 134 is 128 plus 6, the number of the signal `SIGABRT`.

**Rust and Go: panic.** A panic is for bugs. Rust's prints the message with the file and
line, and ends the thread. When that is the main thread, the program exits with code
101. Go's prints the message and a stack trace, and exits with code 2. Both can be
stopped: Rust's `std::panic::catch_unwind` returns `Err` when the closure panics, and
Go's `recover`, called in a deferred function, stops the panic and returns its value.
Neither is meant for everyday errors. Go's own HTTP server recovers from a panic in one
request's handler, logs it, and closes that request's connection or stream, so that one
bad request does not end the server. That is the typical use: a boundary between
independent pieces of work. Rust also has checked versions that return `Option`:
`numbers.get(5)` and `10_i32.checked_div(zero)` give `None`.

**Python, Dart, and Kotlin: exceptions from a different family, or the same one.**
Python raises ordinary exceptions for bugs, `IndexError`, `ZeroDivisionError`, and
`AttributeError`, and an `except Exception` catches them along with real failures. That
is the main reason to catch the narrowest class you can. Dart keeps two families. A
subclass of `Exception`, such as `FormatException`, reports a failure that a caller can
handle. A subclass of `Error`, such as `RangeError` or the `TypeError` from `!` on a
null, reports a bug, and Dart's style guide says not to catch it. Kotlin's bugs are JVM
exceptions. `!!` throws `NullPointerException`, and `check` and `require` are Kotlin's
assertions, which throw `IllegalStateException` and `IllegalArgumentException`.

**JavaScript and TypeScript: often no error at all.** `numbers[5]` is `undefined`, and
`10 / zero` is `Infinity`, and the program carries on with them. Only reading a property
of `undefined` throws a `TypeError`. TypeScript helps with the last one:
`findUser(7).name` does not compile, with
`error TS2532: Object is possibly 'undefined'`, and the `!` in `findUser(7)!.name` tells
the compiler to trust you, which is how this program gets the `TypeError` back.
TypeScript still gives `numbers[index]` the type `number`, unless you turn on the
`noUncheckedIndexedAccess` option.

**Assertions can be switched off.** C and C++ drop `assert` when `NDEBUG` is defined,
and Python drops `assert` statements when run with `python3 -O`. Dart runs `assert` only
with `dart run --enable-asserts`, which is why the Dart line says `passed`. Kotlin's
`assert` is the JVM's, which is off unless you start Java with `-ea`, so the Kotlin
version uses `check`, which always runs. Rust's `assert!` always runs, and
`debug_assert!` runs only in debug builds. Node's `assert` module always runs, and Go
has no assert. Never put work the program needs inside an assertion, and never use one
to check input.

When nothing catches a bug, each language ends the program in its own way. These are the
first lines each one prints for an index out of range, or the nearest bug it reports,
and the exit code the shell sees:

| Language | First line when a bug goes uncaught | Exit code |
| --- | --- | --- |
| C | `Floating point exception`, from the shell, for a division by zero; often nothing for a bad index | 136, or whatever the program does next |
| C++ | `terminate called after throwing an instance of 'std::out_of_range'` | 134 |
| Rust | `thread 'main' (31687) panicked at main.rs:4:27:` | 101 |
| Go | `panic: runtime error: index out of range [5] with length 3` | 2 |
| Python | `Traceback (most recent call last):`, ending in `IndexError: list index out of range` | 1 |
| JavaScript, TypeScript | `TypeError: Cannot read properties of undefined (reading 'name')` | 1 |
| Dart | `Unhandled exception:` | 255 |
| Kotlin | `Exception in thread "main" java.lang.ArrayIndexOutOfBoundsException: Index 5 out of bounds for length 3` | 1 |

The number in brackets in Rust's line is the thread's ID, which changes on every run.

---

## 11.7 What the compiler checks

A failure is easiest to handle when the compiler will not let you forget it. Rust marks
`Result` with `#[must_use]`, so a call whose `Result` is thrown away gets a warning, and
`-D warnings` makes it an error:

```text
warning: unused `Result` that must be used
 --> main.rs:6:5
  |
6 |     parse("42");
  |     ^^^^^^^^^^^
  |
  = note: this `Result` may be an `Err` variant, which should be handled
```

C++17 and C23 have the same idea as an attribute that you put on the function:
`[[nodiscard]] bool save(const std::string &name)` makes GCC warn
`ignoring return value of 'bool save(const std::string&)', declared with attribute 'nodiscard'`.
Go checks half of it. `value := strconv.Atoi("42")` does not compile, because the error
has nowhere to go: `assignment mismatch: 1 variable but strconv.Atoi returns 2 values`.
But `strconv.Atoi("42")` alone, as a statement, compiles without a word, and so does
`value, _ := strconv.Atoi(text)`, which throws the error away on purpose. Linters such
as `errcheck` catch the first.

The exception languages check nothing. Java made some exceptions **checked**: a method
has to list them after `throws`, and every caller has to catch them or list them too.
Kotlin, which runs on the same JVM, left that out, and so did C#, Python, JavaScript,
TypeScript, and Dart. A function in any of them can throw anything, and its type will
not say so.

---

## 11.8 Why each language looks the way it does

- **C** grew up with Unix, where a system call returns -1 and leaves the reason in
  `errno`. That costs nothing and needs no language support, which suited the 1970s. It
  also means every call site has to check, and nothing notices when one does not.
  `errno` was a single global until threads arrived, and is now one per thread.
- **C++** added exceptions so that constructors and overloaded operators, which have no
  return value to spare, could fail. Exceptions and destructors were designed together:
  as an exception passes, every object on the way is destroyed, so RAII cleanup runs on
  the error path too. Implementations make the path with no exception almost free and
  the path with one slow. Some large codebases turn exceptions off entirely, and
  Google's C++ style guide says "We do not use C++ exceptions". `std::expected` in C++23
  gives those codebases a result type.
- **Rust** took sum types from the ML family of languages and made `Result` an ordinary
  enum, so that failure shows up in types and the compiler can check it. The `?`
  operator, which replaced an older `try!` macro, made passing errors up cheap to write.
  Panics are for bugs only, which keeps them rare enough that a program can choose to
  abort on panic instead of unwinding.
- **Go**'s designers chose plain values over exceptions. The Go FAQ says that tying
  exceptions to a control structure, as `try`/`catch`/`finally` does, "results in
  convoluted code". Rob Pike's essay "Errors are values" argues that an error is data a
  program can store, compare, and wrap like any other. Several proposals to shorten
  `if err != nil`, including a `try` built-in in 2019, were declined. Go 1.13 added
  wrapping with `%w`, `errors.Is`, and `errors.As`, and Go 1.20 added `errors.Join`.
- **Python** has used exceptions from the start, and uses them for ordinary control flow
  too: a `for` loop ends when the iterator raises `StopIteration`. Python 3.11 made a
  `try` block that raises nothing almost free, and added `ExceptionGroup` and `except*`,
  mainly so that `asyncio` can report several failed tasks at once.
- **JavaScript** had no exceptions in its first versions. `try`/`catch` arrived with ES3
  in 1999, and the older built-in functions report failure with values instead: `NaN`,
  `-1` from `indexOf`, and `undefined`. Promises added a second path for errors in
  asynchronous code, and ES2022 added `cause`.
- **TypeScript** describes JavaScript as it is, and JavaScript can throw any value at
  all, so TypeScript gives a caught value the type `unknown` under `--strict` and has no
  way to declare what a function throws. Code that wants checked failures returns a
  tagged union instead, as exercises 11.1 and 11.2 do.
- **Dart** split failures into `Exception`, for conditions a program should expect and
  handle, and `Error`, for mistakes in the program. It has no checked exceptions. Null
  safety, added in Dart 2.12, turned the most common bug, a null where none was
  expected, into a compile-time error.
- **Kotlin** dropped Java's checked exceptions. Its documentation argues that in large
  projects they lower productivity without making the code better, and lead to empty
  `catch` blocks written only to satisfy the compiler. For failures a caller should
  expect, Kotlin code returns a nullable type or a sealed class, and `runCatching` turns
  a throwing call into a `Result` value.

---

## 11.9 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | Calling `strtol` without setting `errno = 0` first | An old `ERANGE` from an earlier call looks like an overflow |
| C | `atoi` on input you don't control | `"abc"` and `"0"` both give 0, and nothing tells you which |
| C | Ignoring the return value of `fclose`, `fprintf`, or `snprintf` | A full disk or a cut-off string goes unnoticed |
| C, C++ | Real work inside `assert(...)` | `-DNDEBUG` removes the work along with the check |
| C++ | `catch (Base e)` by value | The exception is copied and cut down to `Base`; catch by `const &` |
| C++ | `throw error;` inside a `catch` | Throws a copy of the caught type and loses the real one; use `throw;` |
| C++ | Throwing from a destructor while another exception is passing | `std::terminate` |
| Rust | `.unwrap()` on input or I/O | A panic on the first bad line; return the error with `?` |
| Rust | `fn main() -> Result<...>` for a user-facing tool | Prints the error with `{:?}`: `Error: ParseIntError { kind: PosOverflow }` |
| Go | `value, _ := f()` | The error is gone, and `value` is the zero value |
| Go | `fmt.Errorf("...: %v", err)` when callers need `errors.Is` | `%v` keeps only the text; use `%w` |
| Go | Comparing errors with `==` | Misses wrapped errors; use `errors.Is` |
| Go | A `*MyError` that is `nil`, returned as an `error` | The `error` is not `nil`, so `err != nil` is true |
| Python | `except:` or `except Exception:` around a large block | Catches bugs too, such as `NameError` and `TypeError`, and hides them |
| Python | `raise NewError(...)` inside `except` without `from` | Works, but the traceback says "During handling of the above exception" instead of "direct cause" |
| JavaScript, TypeScript | `throw "message"` | No stack trace and no `.message`; throw an `Error` |
| JavaScript, TypeScript | An `async` call that nobody awaits, which rejects | Node prints the error and exits with code 1, after the code that followed has run |
| JavaScript, TypeScript | `catch (error)` with no type check | Catches bugs, such as a `TypeError`, along with the errors you meant |
| Dart | `catch (e)` with no `on` clause | Catches `Error`s, which are bugs |
| Dart | Relying on `assert` | `dart run` skips it unless you pass `--enable-asserts` |
| Kotlin | `assert(...)` | Does nothing unless the JVM runs with `-ea`; use `check` or `require` |
| Kotlin | `runCatching { ... }` around coroutine code | Also catches the `CancellationException` that cancels the coroutine |
| All | Logging an error and then returning it | It is reported twice, once at each level; do one or the other |

The Go `nil` pitfall deserves a sentence. An `error` holding a `nil` `*MyError` is not a
`nil` `error`, because the interface still holds a type. Return a literal `nil` from any
function whose result type is `error`.

---

## 11.10 Idiom check

**C: return a status, and check every one.** Return 0 or a negative code, or a status
enum, and hand results back through pointers. Set `errno = 0` before calls that use it,
use `strerror(errno)` or `perror` for messages, and use one exit with `goto` cleanup
labels so that every failure path frees what it took (chapter 10). Use `assert` for bugs
only, and never for input.

**C++: exceptions for failure, values for absence.** Throw a class derived from
`std::runtime_error` when a function can't do its job, catch by `const &`, and rethrow
with `throw;`. Use `std::optional` when "no value" is a normal answer, and
`std::expected` in C++23 code that avoids exceptions. Let RAII clean up on the way out,
mark functions that cannot throw `noexcept`, and catch only where you can do something
about the failure.

**Rust: `Result` and `?`, with an error enum.** Give each module an error enum that
implements `Display` and `std::error::Error`, with `From` impls so that `?` can convert
lower-level errors. Add context with `map_err`, and return the cause from `source()`.
Libraries such as `thiserror` write that boilerplate, and `anyhow` gives applications
one error type with context. Use `panic!`, `unwrap`, and `expect` only for bugs, and
give `expect` a message that says what was assumed.

**Go: check, wrap, and return.** Check `err` on the line after every call that returns
one. Add context with `fmt.Errorf("doing x: %w", err)`, export sentinel errors and error
types for the cases that callers should handle, and test them with `errors.Is` and
`errors.As`. Return `nil` explicitly for no error, and keep `panic` for bugs and
impossible states.

**Python: raise specific exceptions, catch narrow ones.** Define your own exceptions as
subclasses of `Exception` with clear messages, raise them with `raise ... from` to keep
the cause, and catch the narrowest class at the level that can act on it. Keep each
`try` block small, so that it does not catch errors from code you did not mean to guard.
Use `ExceptionGroup` when you really have several errors at once.

**JavaScript and TypeScript: throw `Error`s, and narrow what you catch.** Throw
instances of `Error` subclasses, pass `{ cause }` when you wrap, and in a `catch` check
`instanceof` and throw anything else again. Always `await` or `.catch()` a promise. In
TypeScript, consider returning a tagged union such as
`{ ok: true, value } | { ok: false, error }` from functions whose failures are expected,
so that the compiler checks the callers.

**Dart: `Exception`s for failures, `Error`s for bugs.** Implement `Exception` for
failures that callers should handle, and catch them with `on SomeException catch (e)`.
Don't catch `Error`. Offer `tryParse`-style functions that return `null` when failure is
common, and use sealed classes with `switch` when callers need to handle each case.

**Kotlin: nullable results, and exceptions for the rest.** Return `T?` when "not found"
or "could not parse" is a normal answer, and handle it with `?:`. Throw exceptions for
real failures, use `require` for bad arguments and `check` for bad state, and model
failures that callers must handle as a sealed class. Use `runCatching` at the edges of a
program, not deep inside it.

---

## 11.11 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [A strict integer parser](exercises/01-strict-int/) | An error type with one case per failure, overflow checks, and a parser that never calls the built-in one |
| 2 | [A config loader with typed errors](exercises/02-config-errors/) | Collecting every error in a file, errors that carry other errors, and counting errors by type |
| 3 | [Retry, skip, or stop](exercises/03-retries/) | Deciding what to do from an error's type, wrapping the last error, and rethrowing |

```sh
python3 tools/check.py 11                     # your answers, every language
python3 tools/check.py 11 --lang rust,go      # some languages
python3 tools/check.py 11 --solutions         # the reference answers
```

**Stretch:** take exercise 3 in Rust or Go and add a strict mode, in which `missing`
stops the batch too. How many places in your code had to change? Then do the same in
Python or JavaScript, and compare.

---

Previous: [Chapter 10: Ownership and resource management](../10-ownership-and-resource-management/) ·
Next: Chapter 12: Abstraction and OOP (coming soon)
