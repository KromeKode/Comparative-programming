# Chapter 19: Testing basics

Every program in this book has been tested since chapter 1: the checker runs it, gives
it an input, and compares what it prints with what it should. This chapter is about
tests that live next to the code instead. A test is a small function that calls a piece
of the program with an input someone chose, and checks that what comes back is what
should. A test runner finds the tests, runs them all, and reports which ones failed and
why, and the whole suite runs in seconds, after every change. The nine languages split
three ways on where the runner comes from. Rust and Go have one built into their
toolchains, JavaScript and TypeScript have one built into Node, and Dart, Kotlin, and
Python each have an official one, or one that nearly everyone uses, that a project adds
as a dependency. C and C++ have none, so this chapter's C and C++ examples bring their
own, in one header each. The rest of the chapter is about what to test, how to test code
that depends on something outside it, and how to tell whether the tests are any good.
The exercises turn the book's usual arrangement round: the code is given, the tests are
the exercise, and the checker puts bugs into the code to see whether your tests notice.

**Languages:** all nine · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Writing tests with each language's test framework, and running them | Debugging and profiling (chapter 20), where a failing test is where a bug hunt starts |
| Assertions, and failure messages that say what went wrong | Debugging and profiling (chapter 20) |
| Tables of cases, parametrized tests, and testing errors | Text parsing (chapter 22), sorting and searching (chapter 29) |
| Test doubles, and passing code what it depends on so that a test can replace it | SQLite from code (chapter 34), REST CRUD API (chapter 38) |
| What a test can see of a module, and testing a promise instead of an implementation | Design problems (chapter 31) |
| Coverage, mutation testing, flaky tests, and tests in CI | Complexity and benchmarking (chapter 24), REST CRUD API (chapter 38) |

---

## 19.1 The concept: tests

A **unit test** calls one piece of code, a function or a class, with inputs the test
chose, and checks the result with **assertions**: statements that something is true,
such as that `wrap("ab cd", 4)` is `["ab", "cd"]`. The test **passes** if every
assertion holds, and **fails** if one doesn't, or if the code it calls crashes or
throws. A **test runner** finds the tests, runs each one, keeps going after one fails,
reports which failed and why, and exits with a status that says whether all of them
passed, which is what lets a script or a CI server act on the result:

```text
  test file          runner                                report
  test_one()   ->    run it: every assertion holds    ->   ok    test_one
  test_two()   ->    run it: an assertion is false    ->   FAIL  test_two, with the file, the
                                                                 line, and both values
                     ...                                   exit status 1, since one failed
```

The **code under test** is what a test calls. Other tests are named by what they cover.
An **integration test** runs several pieces together, or a whole program, and the
checker's tests of every chapter so far are integration tests of that kind, since they
run each program and compare its output. A **regression test** is one written for a bug
after it was found, so that it can't come back unnoticed. A **test double** stands in
for something the code under test depends on, such as a clock, a sensor, or a server, so
that the test controls what it does; section 19.6 is about them.

Good tests are fast, so that they are run after every change. They are
**deterministic**, giving the same result every time on the same code, and
**independent**, so that each can run alone and in any order. Each fails for one reason,
and says what it was. A test that sometimes fails and sometimes passes, with nothing
changed, is **flaky**, and section 19.8 is about why tests become flaky and how to stop
them.

What tests can't do is show that code is right. Edsger Dijkstra put it in 1970: "Program
testing can be used to show the presence of bugs, but never to show their absence!" A
test shows that the code does the right thing for the inputs it tried, so testing is
mostly the work of choosing inputs. Bugs cluster at **edges**: an empty input, an input
of one, the first and the last, a value exactly at a limit and one past it. Each rule a
function promises has a boundary, and a good suite has a test on each side of it. How to
tell whether a suite is good enough is the subject of this chapter's exercises, where
the checker measures it the way **mutation testing** does: by putting a bug into the
code, and seeing whether any test fails.

---

## 19.2 What each language gives you

| Language | Test framework | Where tests go | A test is | Checks are | Run with |
| --- | --- | --- | --- | --- | --- |
| C | None in the standard library; Unity, CMocka, and Check are common, and this chapter uses `check.h`, which is about 90 lines in one header | Separate files, compiled with the code into a test program | A function that `main` calls | Macros: `CHECK` and friends here; `assert` from `<assert.h>` stops the program, and is for the program's own checks, as in chapter 11 | The test program; CMake's CTest runs test programs |
| C++ | None in the standard library; GoogleTest and Catch2 are the common ones, and this chapter uses `check.hpp`, which works like them in one header | Separate files | A `TEST(name) { ... }` block, which registers itself | Macros: `CHECK`, `CHECK_EQ`, and `CHECK_THROWS` here; `EXPECT_EQ` in GoogleTest | The test program; CTest |
| Rust | Built in: `#[test]`, and `cargo test` | Unit tests in a `#[cfg(test)]` module inside the crate; integration tests in `tests/` | A function marked `#[test]` | Macros that panic: `assert!`, `assert_eq!`, and `assert_ne!` | `cargo test` |
| Go | The standard library's `testing` package, and `go test` | Files whose names end in `_test.go`, next to the code | A function `TestXxx(t *testing.T)` | No assertions at all: an `if`, and `t.Errorf` or `t.Fatalf` | `go test` |
| Python | `unittest` in the standard library; pytest, a package, which most projects use | Files named `test_*.py` or `*_test.py` | A function whose name starts with `test` | A plain `assert`, which pytest rewrites to show both sides; `pytest.raises` | `pytest`, here `uv run pytest` |
| JavaScript | `node:test` and `node:assert`, built into Node since version 18; Jest, Vitest, and Mocha are packages | Files such as `*.test.js`, which `node --test` finds | A call to `test(name, fn)` | Functions that throw: `assert.equal`, `assert.deepEqual`, and `assert.throws` | `node --test` |
| TypeScript | JavaScript's | Files such as `*.test.ts` | The same | The same, with `tsc` to check the types | `node --test`, and `tsc` |
| Dart | `package:test`, from the Dart team | `test/`, in files whose names end in `_test.dart` | A call to `test(name, fn)`, often inside a `group` | `expect(actual, matcher)` | `dart test` |
| Kotlin | `kotlin.test`, which runs on JUnit 5 here; Kotest is another | `src/test/kotlin/`, compiled apart from the program | A method marked `@Test` | `assertEquals(expected, actual)`, `assertTrue`, and `assertFailsWith` | `gradle test` |

The table splits two ways. Read down the second column and the languages sort by when
they were designed. Go and Rust, from 2009 and 2015, put testing in the toolchain from
the start, Node added a runner in 2022 after a decade of packages, and Python, Dart, and
Kotlin have one official framework and one that many projects prefer. C and C++, which
are older than the idea of a unit-test framework, still have none. Read down the fourth
column and they sort by how a runner finds the tests. Go and pytest find them by name,
`TestXxx` and `test_`. Rust and Kotlin find them by a mark, `#[test]` and `@Test`, which
the compiler or JUnit's reflection looks for, as in chapter 15. JavaScript, TypeScript,
Dart, and C++ find them because each test registers itself: the file calls `test()` once
for each test as it loads, or C++'s `TEST` macro defines a global whose constructor adds
the test to a list before `main` runs, which is the load order of chapter 18. C finds
nothing on its own, so its `main` calls each test.

---

## 19.3 A first test

Example 01 tests a function that says whether a year is a leap year: one divisible by 4,
except for centuries, which must be divisible by 400. In C it is one line:

**C: `leap.c`**

```c
#include "leap.h"

bool is_leap_year(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}
```

These are the tests, in each language. Each has two, one for the rule of 4 and one for
centuries:

**C: `test_leap.c`**

```c
// Tests for leap.c. Each test is a function, and main runs them: C has no way to
// find them on its own.
#include "check.h"
#include "leap.h"

static void test_divisible_by_4(void) {
    CHECK(is_leap_year(2024));
    CHECK(!is_leap_year(2023));
}

static void test_centuries(void) {
    CHECK(!is_leap_year(1900));
    CHECK(is_leap_year(2000));
}

int main(void) {
    RUN_TEST(test_divisible_by_4);
    RUN_TEST(test_centuries);
    return check_exit_code();
}
```

**C++: `leap_test.cpp`**

```cpp
// Tests for leap.cpp. TEST registers each one, so main doesn't have to list them.
#include "check.hpp"
#include "leap.hpp"

TEST(divisible_by_4) {
    CHECK(leap::is_leap_year(2024));
    CHECK(!leap::is_leap_year(2023));
}

TEST(centuries) {
    CHECK(!leap::is_leap_year(1900));
    CHECK(leap::is_leap_year(2000));
}

int main() { return check::run_all_tests(); }
```

**Rust: `src/lib.rs`**

```rust
/// Whether `year` is a leap year in the Gregorian calendar: one divisible by 4,
/// except for centuries, which must be divisible by 400.
pub fn is_leap_year(year: i32) -> bool {
    year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)
}

// The tests are a module of this crate, compiled only by `cargo test`. They are
// often written in this file, as `mod tests { ... }`; here they are in tests.rs.
#[cfg(test)]
mod tests;
```

**Rust: `src/tests.rs`**

```rust
// A child module of lib.rs, so `super::*` brings in everything it defines,
// private items included.
use super::*;

#[test]
fn divisible_by_4() {
    assert!(is_leap_year(2024));
    assert!(!is_leap_year(2023));
}

#[test]
fn centuries() {
    assert!(!is_leap_year(1900));
    assert!(is_leap_year(2000));
}
```

**Go: `leap_test.go`**

```go
// Files whose names end in _test.go are compiled only by go test. This one is
// in package leap itself, so it could use the package's unexported names too.
package leap

import "testing"

// go test runs every function named TestXxx that takes a *testing.T.
func TestDivisibleBy4(t *testing.T) {
	if !IsLeapYear(2024) {
		t.Error("IsLeapYear(2024) = false, want true")
	}
	if IsLeapYear(2023) {
		t.Error("IsLeapYear(2023) = true, want false")
	}
}

func TestCenturies(t *testing.T) {
	if IsLeapYear(1900) {
		t.Error("IsLeapYear(1900) = true, want false")
	}
	if !IsLeapYear(2000) {
		t.Error("IsLeapYear(2000) = false, want true")
	}
}
```

**Python: `test_leap.py`**

```python
# pytest runs every function whose name starts with test_, in every file whose
# name starts with test_. A plain assert is enough: when one fails, pytest shows
# the values on each side.
from leap import is_leap_year


def test_divisible_by_4() -> None:
    assert is_leap_year(2024)
    assert not is_leap_year(2023)


def test_centuries() -> None:
    assert not is_leap_year(1900)
    assert is_leap_year(2000)
```

**JavaScript: `leap.test.js`**

```js
// node --test runs files whose names end in .test.js, among others. Each call to
// test() registers a test, which the runner runs after the file has loaded.
import assert from "node:assert/strict";
import { test } from "node:test";
import { isLeapYear } from "./leap.js";

test("divisible by 4", () => {
  assert.equal(isLeapYear(2024), true);
  assert.equal(isLeapYear(2023), false);
});

test("centuries", () => {
  assert.equal(isLeapYear(1900), false);
  assert.equal(isLeapYear(2000), true);
});
```

TypeScript's test is the same, with `"./leap.ts"` in the import.

**Dart: `test/leap_test.dart`**

```dart
// dart test runs every file in test/ whose name ends in _test.dart. Its main
// registers the tests by calling test(), and the runner runs them afterward.
import 'package:leap/leap.dart';
import 'package:test/test.dart';

void main() {
  test('divisible by 4', () {
    expect(isLeapYear(2024), isTrue);
    expect(isLeapYear(2023), isFalse);
  });

  test('centuries', () {
    expect(isLeapYear(1900), isFalse);
    expect(isLeapYear(2000), isTrue);
  });
}
```

**Kotlin: `src/test/kotlin/LeapTest.kt`**

```kotlin
// Gradle compiles src/test/kotlin separately from the program, and JUnit runs every
// method annotated @Test, which it finds by reflection.
import kotlin.test.Test
import kotlin.test.assertFalse
import kotlin.test.assertTrue

class LeapTest {
    @Test
    fun divisibleBy4() {
        assertTrue(isLeapYear(2024))
        assertFalse(isLeapYear(2023))
    }

    @Test
    fun centuries() {
        assertFalse(isLeapYear(1900))
        assertTrue(isLeapYear(2000))
    }
}
```

**Where the test framework comes from.** Rust, Go, and Node need nothing besides the
test file. Python, Dart, and Kotlin get theirs as a **dev dependency** of the project,
chapter 18's kind of dependency, but one that only people working on the project need,
so it isn't installed for the program's users. Python's is a dependency group, which uv
installs with the project and records in `uv.lock`, Dart's is under `dev_dependencies`,
and Kotlin's is `testImplementation`, which Gradle puts on the class path of the tests
and not of the program:

**Python: `pyproject.toml`**

```toml
[project]
name = "leap"
version = "0.1.0"
requires-python = ">=3.12"
dependencies = []

# Tools for working on the project, which aren't installed for its users.
[dependency-groups]
dev = [
    "pytest>=9.2.0",
]

[tool.uv]
package = false
```

**Dart: `pubspec.yaml`**

```yaml
name: leap
publish_to: none
environment:
  sdk: ^3.9.0
dev_dependencies:
  test: ^1.33.0
```

**Kotlin: `build.gradle.kts`**

```kotlin
plugins {
    kotlin("jvm") version "2.4.21"
}

dependencies {
    // kotlin.test's assertions, and its @Test, which runs on JUnit 5.
    testImplementation(kotlin("test"))
}

kotlin {
    compilerOptions {
        allWarningsAsErrors = true
    }
}

tasks.test {
    // Run the tests on the JUnit Platform, which is how JUnit 5 finds and runs them.
    useJUnitPlatform()
    // Print each failing test in full, with its message and not only its class, even
    // with --quiet, which the checker runs Gradle with.
    testLogging.quiet {
        events("failed")
        exceptionFormat = org.gradle.api.tasks.testing.logging.TestExceptionFormat.FULL
    }
}
```

**Running them.** Each is run the way the table in 19.2 says. When everything passes,
most runners say little, and Gradle says nothing about tests at all unless one fails:

```text
$ go test
PASS
ok  	example.com/leap	0.002s

$ uv run pytest
...
test_leap.py ..                                                          [100%]

============================== 2 passed in 0.02s ===============================

$ cargo test
...
running 2 tests
test tests::divisible_by_4 ... ok
test tests::centuries ... ok

test result: ok. 2 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

The interesting output is a failure. The example's `tests/02-no-400-rule.in` puts in the
bug that the tests are there to catch, a leap-year rule without the exception for every
400th year, `year % 4 == 0 && year % 100 != 0`, which makes 2000 not a leap year. This
is what each runner says then, with long paths shortened and the less useful lines left
out:

```text
C:
ok   test_divisible_by_4
test_leap.c:13: CHECK(is_leap_year(2000)) failed
FAIL test_centuries
1 checks failed

C++:
ok   divisible_by_4
leap_test.cpp:12: CHECK(leap::is_leap_year(2000)) failed
FAIL centuries
1 passed, 1 failed

Rust:
running 2 tests
test tests::divisible_by_4 ... ok
test tests::centuries ... FAILED

failures:

---- tests::centuries stdout ----

thread 'tests::centuries' (8314) panicked at src/tests.rs:14:5:
assertion failed: is_leap_year(2000)
...
test result: FAILED. 1 passed; 1 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s

Go:
--- FAIL: TestCenturies (0.00s)
    leap_test.go:22: IsLeapYear(2000) = false, want true
FAIL
exit status 1
FAIL	example.com/leap	0.002s

Python:
test_leap.py .F                                                          [100%]

=================================== FAILURES ===================================
________________________________ test_centuries ________________________________

    def test_centuries() -> None:
        assert not is_leap_year(1900)
>       assert is_leap_year(2000)
E       assert False
E        +  where False = is_leap_year(2000)

test_leap.py:14: AssertionError
=========================== short test summary info ============================
FAILED test_leap.py::test_centuries - assert False
========================= 1 failed, 1 passed in 0.02s ==========================

JavaScript and TypeScript, with --test-reporter=spec:
✔ divisible by 4 (0.59587ms)
✖ centuries (0.628851ms)
...
✖ failing tests:

test at leap.test.js:12:1
✖ centuries (0.628851ms)
  AssertionError [ERR_ASSERTION]: Expected values to be strictly equal:

  false !== true

      at TestContext.<anonymous> (file:///.../leap.test.js:14:10)
...

Dart:
00:00 +1 -1: test/leap_test.dart: centuries [E]
  Expected: true
    Actual: <false>

  package:matcher           expect
  test/leap_test.dart 14:5  main.<fn>

00:00 +1 -1: Some tests failed.

Kotlin:
LeapTest > centuries() FAILED
    org.opentest4j.AssertionFailedError: Expected value to be true.
        at app//org.junit.jupiter.api.AssertionUtils.fail(AssertionUtils.java:38)
        ...
        at app//LeapTest.centuries(LeapTest.kt:17)

2 tests completed, 1 failed
```

Every one says which test failed and on which line, and every one exits with a status
other than 0: 1, except for Cargo's 101. They differ in what else they say. pytest
prints the failing line, and the value of each call in it, which it can do because it
rewrites each `assert` in a test file as it imports it, into code that keeps the values.
Go prints only what the test's own `t.Error` said, which is why Go's tests spell out the
call, what it returned, and what was wanted. Rust's `assert!` and C's `CHECK` print the
condition's source text, which the macro turns into a string, and the JVM prints a stack
trace that ends at the test's line. Section 19.4 is about making these messages say
more.

**`check.h` and `check.hpp`.** C and C++ have no test framework, so each of this
chapter's C and C++ folders has a small one. Here is the core of `check.h`:

```c
static int check_failures = 0;

#define CHECK(condition)                                                         \
    do {                                                                         \
        if (!(condition)) {                                                      \
            printf("%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #condition); \
            check_failures++;                                                    \
        }                                                                        \
    } while (0)
```

It has to be a macro, for the reasons of chapter 15. A function couldn't print the
condition's text, which `#condition` turns into a string, or the file and line of the
test, which `__FILE__` and `__LINE__` are where the macro is used. `CHECK` counts a
failure and carries on, so that one run reports every failure, and `RUN_TEST(name)`
calls a test and prints `ok` or `FAIL` after it, by comparing the count before and
after. `CHECK_INT_EQ` and `CHECK_STR_EQ` print both values when they differ, and
`check_exit_code()` gives `main` its exit status.

`check.hpp` adds what GoogleTest and Catch2 have: tests that register themselves, so
that `main` doesn't list them. `TEST(name)` declares a function, defines a global object
whose constructor adds the function to a list, and then starts the function's body,
which the code after the macro fills in:

```cpp
#define TEST(name)                                               \
    static void name();                                          \
    static const check::Registrar name##_registrar(#name, name); \
    static void name()
```

The globals are constructed before `main` starts, so `check::run_all_tests()` finds
every test in the list. Its `CHECK_EQ(actual, expected)` is a template that prints both
values, a string in quotes, a container as `{a, b, c}`, and an optional as its value or
`std::nullopt`, and `CHECK_THROWS(statement, type)` checks that a statement throws. The
real frameworks look much the same. These are the same tests in GoogleTest 1.14, built
with `-lgtest -lgtest_main -pthread`, and Catch2 3.4, built with
`-lCatch2Main -lCatch2`:

```cpp
// GoogleTest
#include <gtest/gtest.h>

#include "leap.hpp"

TEST(Leap, Centuries) {
    EXPECT_FALSE(leap::is_leap_year(1900));
    EXPECT_TRUE(leap::is_leap_year(2000));
    EXPECT_EQ(leap::is_leap_year(2000), true);
}

// Catch2
#include <catch2/catch_test_macros.hpp>

#include "leap.hpp"

TEST_CASE("centuries") {
    CHECK_FALSE(leap::is_leap_year(1900));
    CHECK(leap::is_leap_year(2000));
    CHECK(leap::is_leap_year(2000) == true);
}
```

With the bug in, GoogleTest says:

```text
[ RUN      ] Leap.Centuries
leap_gtest.cpp:7: Failure
Value of: leap::is_leap_year(2000)
  Actual: false
Expected: true

leap_gtest.cpp:8: Failure
Expected equality of these values:
  leap::is_leap_year(2000)
    Which is: false
  true

[  FAILED  ] Leap.Centuries (0 ms)
```

and Catch2, which takes the comparison inside `CHECK` apart with operator overloading
and prints each side's value:

```text
leap_catch.cpp:8: FAILED:
  CHECK( leap::is_leap_year(2000) == true )
with expansion:
  false == true
```

---

## 19.4 Assertions and failure messages

An assertion has two jobs: to fail when the code is wrong, and to say what is wrong. The
second is what saves time. `assert!(is_leap_year(2000) == true)` fails in Rust with
`assertion failed: is_leap_year(2000) == true`, which says what was checked but not what
came back, while `assert_eq!` prints both values. Each language has an assertion of
equality that prints both sides, and the clearest way to compare them is on the same
failure. In exercise 19.1, one of the bugs leaves out the last line of the wrapped text,
and this is what each solution's test says about
`wrap("the quick brown fox jumps", 10)`, which should be three lines and is two:

```text
C, comparing the lines as one string, with \n written out:
test_wrap.c:31: CHECK_STR_EQ(got, cases[i].want)
  actual:   "the quick\nbrown fox\njumps"
  expected: "the quick\nbrown fox\njumps\n"

C++, with check.hpp's CHECK_EQ:
wrap_test.cpp:12: CHECK_EQ(wrap::wrap("the quick brown fox jumps", 10), Lines({"the quick", "brown fox", "jumps"}))
  actual:   {"the quick", "brown fox"}
  expected: {"the quick", "brown fox", "jumps"}

Rust, with assert_eq! and a message:
assertion `left == right` failed: wrap("the quick brown fox jumps", 10)
  left: ["the quick", "brown fox"]
 right: ["the quick", "brown fox", "jumps"]

Go, with the test's own message:
wrap_test.go:34: Wrap("the quick brown fox jumps", 10) = ["the quick" "brown fox"], want ["the quick" "brown fox" "jumps"]

Python:
E       AssertionError: assert ['the quick', 'brown fox'] == ['the quick',...fox', 'jumps']
E
E         Right contains one more item: 'jumps'

JavaScript, with assert.deepEqual:
  AssertionError [ERR_ASSERTION]: Expected values to be strictly deep-equal:
  + actual - expected

    [
      'the quick',
      'brown fox',
  -   'jumps'
    ]

Dart:
  Expected: ['the quick', 'brown fox', 'jumps']
    Actual: ['the quick', 'brown fox']
     Which: at location [2] is ['the quick', 'brown fox'] which shorter than expected

Kotlin:
    org.opentest4j.AssertionFailedError: expected: <[the quick, brown fox, jumps]> but was: <[the quick, brown fox]>
```

Dart's message is quoted as it is printed, grammar and all.

**Which argument is which.** An equality assertion has two arguments, and the frameworks
don't agree on their order, which matters because the message names them:

| Framework | The call | The message calls them |
| --- | --- | --- |
| `check.h`, `check.hpp` | `CHECK_EQ(actual, expected)` | `actual` and `expected` |
| GoogleTest | `EXPECT_EQ(a, b)` | Neither: it prints each expression and its value |
| Rust | `assert_eq!(left, right)` | `left` and `right`, so either order reads correctly |
| Go | No assertion; the test writes its message | Whatever it says: by convention, what the call returned, then `want` |
| pytest | `assert actual == expected` | The two sides, with `Left` and `Right` in its notes |
| `node:assert` | `assert.equal(actual, expected)` | `actual` and `expected` |
| Dart | `expect(actual, matcher)` | `Actual` and `Expected` |
| Kotlin | `assertEquals(expected, actual)` | `expected` and `was` |

Kotlin's order is the one most often got wrong, since it is JUnit's, the opposite of
`node:assert`'s, and nothing checks it. With the arguments swapped, the leap-year test
still fails, but its message reads backwards: `assertEquals(isLeapYear(2000), true)`
says `expected: <false> but was: <true>`, when what was expected was `true`.

**Stop or carry on.** Most assertions stop the test at the first one that fails. In Rust
an assertion panics, in Python, JavaScript, Dart, and Kotlin it throws, and in C++'s
GoogleTest and Catch2 an `ASSERT_EQ` or a `REQUIRE` returns from the test. The others
carry on, and report every failure in the test: GoogleTest's `EXPECT_` assertions,
Catch2's `CHECK`, `check.h` and `check.hpp`'s `CHECK`, and Go's `t.Error` and
`t.Errorf`, beside `t.Fatal` and `t.Fatalf`, which stop. Carrying on is what let Go's
test of the word wrapper report all eight of the cases that the bug broke, in one run,
where Rust's, a loop of `assert_eq!` in one test, stopped at the first. Stopping is
right when what follows can't work, such as when a function returned an error and the
test is about to use its result.

**Equality has to exist.** An assertion of equality needs the values to have an
equality, and a way to print them, and chapter 15's derived code is where most of them
come from. Rust's `assert_eq!` needs `PartialEq` and `Debug`, which is why example 02's
error type says `#[derive(Debug, PartialEq)]`. Kotlin's data classes and Python's
dataclasses get `equals` and a printable form from their fields, and so do Dart's
records. Go's `==` works on structs, and `slices.Equal` and `maps.Equal` on slices and
maps. `node:assert`'s `deepEqual` compares by structure, where `equal` compares objects
by identity, and Dart's `expect` compares lists and maps element by element. C compares
strings with `strcmp`, which `CHECK_STR_EQ` calls, since `==` would compare their
addresses.

**Floating point.** Results that are computed with floating point rarely equal what a
test writes down exactly, for the reasons of chapter 3, so tests compare them with a
tolerance: `pytest.approx(0.3)` in Python, `closeTo(0.3, 1e-9)` in Dart,
`assertEquals(0.3, actual, absoluteTolerance = 1e-9)` in Kotlin, `EXPECT_NEAR` in
GoogleTest, and `(actual - expected).abs() < 1e-9` by hand in Rust, Go, JavaScript, and
C.

---

## 19.5 Tables of cases, and testing errors

Example 02 tests a function that writes a number from 1 to 3999 in Roman numerals, and
gives an error for any other, in each language's style from chapter 11: `false` in C, an
exception in C++, Python, JavaScript, TypeScript, Dart, and Kotlin, an
`Err(OutOfRange(n))` in Rust, and an error that wraps `ErrOutOfRange` in Go. Most of its
tests are the same check on different numbers, so each language puts the cases in a
**table**: a list of inputs and what each should give, which one piece of test code goes
through. Adding a case is then one line, and the table is easy to read for which cases
are there and which are missing:

**C: `test_roman.c`**

```c
#include "check.h"
#include "roman.h"

// A table of cases, and one loop that checks them all. A failure inside the loop
// says which line of code failed, but not which case, so the loop says that too.
static void test_numerals(void) {
    static const struct {
        int n;
        const char *want;
    } cases[] = {
        {1, "I"},
        {4, "IV"},
        {9, "IX"},
        {14, "XIV"},
        {40, "XL"},
        {90, "XC"},
        {400, "CD"},
        {1994, "MCMXCIV"},
        {2024, "MMXXIV"},
        {3999, "MMMCMXCIX"},
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        int failures_before = check_failures;
        char got[16] = "";
        CHECK(to_roman(cases[i].n, got, sizeof got));
        CHECK_STR_EQ(got, cases[i].want);
        if (check_failures > failures_before) printf("  in the case for %d\n", cases[i].n);
    }
}

// The errors are part of what to_roman promises, so they get tests too.
static void test_out_of_range(void) {
    char got[16] = "unchanged";
    CHECK(!to_roman(0, got, sizeof got));
    CHECK(!to_roman(4000, got, sizeof got));
    CHECK(!to_roman(-1, got, sizeof got));
    CHECK_STR_EQ(got, "unchanged");
}

int main(void) {
    RUN_TEST(test_numerals);
    RUN_TEST(test_out_of_range);
    return check_exit_code();
}
```

**C++: `roman_test.cpp`**

```cpp
#include <stdexcept>
#include <string>
#include <utility>

#include "check.hpp"
#include "roman.hpp"

// A table of cases, and one loop that checks them all. CHECK_EQ prints the
// expected value when a case fails, which says which case it was.
TEST(numerals) {
    const std::pair<int, std::string> cases[] = {
        {1, "I"},
        {4, "IV"},
        {9, "IX"},
        {14, "XIV"},
        {40, "XL"},
        {90, "XC"},
        {400, "CD"},
        {1994, "MCMXCIV"},
        {2024, "MMXXIV"},
        {3999, "MMMCMXCIX"},
    };
    for (const auto& [n, want] : cases) {
        CHECK_EQ(roman::to_roman(n), want);
    }
}

// The errors are part of what to_roman promises, so they get tests too.
TEST(out_of_range) {
    CHECK_THROWS(roman::to_roman(0), std::out_of_range);
    CHECK_THROWS(roman::to_roman(4000), std::out_of_range);
    CHECK_THROWS(roman::to_roman(-1), std::out_of_range);
}

int main() { return check::run_all_tests(); }
```

**Rust: `src/tests.rs`**

```rust
use super::*;

// A table of cases, and one loop that checks them all. The message after the two
// values says which case failed.
#[test]
fn numerals() {
    let cases = [
        (1, "I"),
        (4, "IV"),
        (9, "IX"),
        (14, "XIV"),
        (40, "XL"),
        (90, "XC"),
        (400, "CD"),
        (1994, "MCMXCIV"),
        (2024, "MMXXIV"),
        (3999, "MMMCMXCIX"),
    ];
    for (n, want) in cases {
        assert_eq!(to_roman(n), Ok(want.to_string()), "to_roman({n})");
    }
}

// The errors are part of what to_roman promises, so they get tests too.
#[test]
fn out_of_range() {
    assert_eq!(to_roman(0), Err(OutOfRange(0)));
    assert_eq!(to_roman(4000), Err(OutOfRange(4000)));
}
```

**Go: `roman_test.go`**

```go
package roman

import (
	"errors"
	"fmt"
	"testing"
)

// A table of cases, and a loop that runs each one as a subtest, named after its
// number, so a failure says which case it was, and `go test -run 'TestToRoman$/1994'`
// runs one of them.
func TestToRoman(t *testing.T) {
	tests := []struct {
		n    int
		want string
	}{
		{1, "I"},
		{4, "IV"},
		{9, "IX"},
		{14, "XIV"},
		{40, "XL"},
		{90, "XC"},
		{400, "CD"},
		{1994, "MCMXCIV"},
		{2024, "MMXXIV"},
		{3999, "MMMCMXCIX"},
	}
	for _, tt := range tests {
		t.Run(fmt.Sprint(tt.n), func(t *testing.T) {
			got, err := ToRoman(tt.n)
			if err != nil || got != tt.want {
				t.Errorf("ToRoman(%d) = %q, %v; want %q, nil", tt.n, got, err, tt.want)
			}
		})
	}
}

// The errors are part of what ToRoman promises, so they get tests too.
func TestToRomanOutOfRange(t *testing.T) {
	for _, n := range []int{0, 4000, -1} {
		if _, err := ToRoman(n); !errors.Is(err, ErrOutOfRange) {
			t.Errorf("ToRoman(%d) error = %v, want ErrOutOfRange", n, err)
		}
	}
}
```

**Python: `test_roman.py`**

```python
import pytest

from roman import to_roman


# parametrize makes one test of each pair, named after its values, such as
# test_numerals[1994-MCMXCIV], so a failure says which case it was.
@pytest.mark.parametrize(
    ("n", "want"),
    [
        (1, "I"),
        (4, "IV"),
        (9, "IX"),
        (14, "XIV"),
        (40, "XL"),
        (90, "XC"),
        (400, "CD"),
        (1994, "MCMXCIV"),
        (2024, "MMXXIV"),
        (3999, "MMMCMXCIX"),
    ],
)
def test_numerals(n: int, want: str) -> None:
    assert to_roman(n) == want


# The errors are part of what to_roman promises, so they get tests too.
@pytest.mark.parametrize("n", [0, 4000, -1])
def test_out_of_range(n: int) -> None:
    with pytest.raises(ValueError, match="out of range"):
        to_roman(n)
```

**JavaScript: `roman.test.js`**

```js
import assert from "node:assert/strict";
import { test } from "node:test";
import { toRoman } from "./roman.js";

const cases = [
  [1, "I"],
  [4, "IV"],
  [9, "IX"],
  [14, "XIV"],
  [40, "XL"],
  [90, "XC"],
  [400, "CD"],
  [1994, "MCMXCIV"],
  [2024, "MMXXIV"],
  [3999, "MMMCMXCIX"],
];

// A test for each case, named after it, so a failure says which case it was.
for (const [n, want] of cases) {
  test(`toRoman(${n})`, () => {
    assert.equal(toRoman(n), want);
  });
}

// The errors are part of what toRoman promises, so they get tests too.
test("out of range", () => {
  for (const n of [0, 4000, -1]) {
    assert.throws(() => toRoman(n), RangeError);
  }
});
```

**Dart: `test/roman_test.dart`**

```dart
import 'package:roman/roman.dart';
import 'package:test/test.dart';

void main() {
  const cases = [
    (1, 'I'),
    (4, 'IV'),
    (9, 'IX'),
    (14, 'XIV'),
    (40, 'XL'),
    (90, 'XC'),
    (400, 'CD'),
    (1994, 'MCMXCIV'),
    (2024, 'MMXXIV'),
    (3999, 'MMMCMXCIX'),
  ];

  // A group names the tests in it, and a test for each case, named after it,
  // says which case failed.
  group('toRoman', () {
    for (final (n, want) in cases) {
      test('$n', () => expect(toRoman(n), want));
    }
  });

  // The errors are part of what toRoman promises, so they get tests too.
  test('out of range', () {
    for (final n in [0, 4000, -1]) {
      expect(() => toRoman(n), throwsRangeError);
    }
  });
}
```

**Kotlin: `src/test/kotlin/RomanTest.kt`**

```kotlin
import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertFailsWith
import org.junit.jupiter.params.ParameterizedTest
import org.junit.jupiter.params.provider.CsvSource

class RomanTest {
    // JUnit runs this once for each line, and names each run after its values, so
    // a failure says which case it was.
    @ParameterizedTest
    @CsvSource(
        "1, I",
        "4, IV",
        "9, IX",
        "14, XIV",
        "40, XL",
        "90, XC",
        "400, CD",
        "1994, MCMXCIV",
        "2024, MMXXIV",
        "3999, MMMCMXCIX",
    )
    fun numerals(n: Int, want: String) {
        assertEquals(want, toRoman(n))
    }

    // The errors are part of what toRoman promises, so they get tests too.
    @Test
    fun outOfRange() {
        for (n in listOf(0, 4000, -1)) {
            assertFailsWith<IllegalArgumentException> { toRoman(n) }
        }
    }
}
```

TypeScript's is JavaScript's, with a type on the table, `[number, string][]`.

**One test, or one test for each case.** The languages split on what a case is. In C,
C++, and Rust the table is a loop inside one test, so a failure needs a message that
says which case it was, which is what C's `printf` and Rust's `"to_roman({n})"` are for.
The others make each case a test of its own, with its own name, which the runner
reports, and can run on its own. Go does it with **subtests**, `t.Run(name, func)`,
named here after the number. pytest does it with `@pytest.mark.parametrize`, which makes
one test of each row, and Kotlin with JUnit's `@ParameterizedTest`, which runs the
method once for each line of `@CsvSource`. JavaScript, TypeScript, and Dart need nothing
special, since a loop can call `test()` once for each case, and Dart wraps them in a
`group`, which puts `toRoman` before each one's name. Example 02's bug `02-no-cm` takes
the line for 900 out of the table of numerals, and with it in, Go and pytest name the
cases that failed:

```text
--- FAIL: TestToRoman (0.00s)
    --- FAIL: TestToRoman/1994 (0.00s)
        roman_test.go:32: ToRoman(1994) = "MDCDXCIV", <nil>; want "MCMXCIV", nil
    --- FAIL: TestToRoman/3999 (0.00s)
        roman_test.go:32: ToRoman(3999) = "MMMDCDXCIX", <nil>; want "MMMCMXCIX", nil

FAILED test_roman.py::test_numerals[1994-MCMXCIV] - AssertionError: assert 'M...
FAILED test_roman.py::test_numerals[3999-MMMCMXCIX] - AssertionError: assert ...
```

and Rust, whose loop stops at the first, names one:

```text
thread 'tests::numerals' (13261) panicked at src/tests.rs:20:9:
assertion `left == right` failed: to_roman(1994)
  left: Ok("MDCDXCIV")
 right: Ok("MCMXCIV")
```

Rust's crates rstest and test-case add parametrized tests, and GoogleTest has `TEST_P`.

**Testing errors.** What a function does with bad input is part of what it promises, so
it gets tests too, and each language has a way to say that a call should fail. Where the
error is an exception, the test framework catches it and checks its type:
`pytest.raises(ValueError)`, `assert.throws(() => toRoman(n), RangeError)`,
`expect(() => toRoman(n), throwsRangeError)`,
`assertFailsWith<IllegalArgumentException>`, and `check.hpp`'s `CHECK_THROWS`. Each
takes the call wrapped in a function, or in a block, so that the framework can make the
call itself, inside a `try`. Where the error is a value, the test checks the value like
any other: `assert_eq!(to_roman(0), Err(OutOfRange(0)))` in Rust,
`errors.Is(err, ErrOutOfRange)` in Go, which looks through the wrapping, and
`CHECK(!to_roman(0, ...))` in C, which also checks that the output was left alone. A
test checks the error's type, and its message only when the message is part of the
promise, since it is the part most likely to change. pytest's `match="out of range"`
checks the message with a regular expression, for when it is.

---

## 19.6 Test doubles

Code that depends on something outside itself is hard to test with that thing in place.
A clock gives a different time on every run, a server may be down, and a heater is real.
Example 03 is a thermostat that reads a temperature sensor and turns a heater on when
the room is more than a degree below its target, and off when it is more than a degree
above. It doesn't make its own sensor and heater: it is given them, as interfaces from
chapter 12, so a test can give it **test doubles** instead, objects that look like a
sensor and a heater to the code, and do what the test needs. This is **dependency
injection**: passing code what it depends on, instead of letting it reach for it. In Go:

**Go: `thermostat.go`**

```go
// Package thermostat keeps a room near a target temperature.
package thermostat

// A Sensor reads the temperature.
type Sensor interface {
	ReadCelsius() int
}

// A Heater can be turned on and off.
type Heater interface {
	TurnOn()
	TurnOff()
}

// Regulate turns the heater on if the room is more than a degree below target,
// and off if it is more than a degree above. Within a degree, it leaves the
// heater alone.
func Regulate(sensor Sensor, heater Heater, target int) {
	celsius := sensor.ReadCelsius()
	if celsius < target-1 {
		heater.TurnOn()
	} else if celsius > target+1 {
		heater.TurnOff()
	}
}
```

The tests use two kinds of double. A **stub** gives the code under test the inputs the
test wants, here a sensor that reads whatever temperature the test chose. A **spy**
records what the code under test does to it, here a heater that writes down each call,
for the test to check afterwards:

**C: `test_thermostat.c`**

```c
#include "check.h"
#include "thermostat.h"

// A stub: a sensor that reads whatever temperature the test gives it.
static int read_stub(void *context) { return *(int *)context; }

static Sensor stub_sensor(int *celsius) { return (Sensor){celsius, read_stub}; }

// A spy: a heater that counts the calls made to it, for the test to check.
typedef struct {
    int ons;
    int offs;
} Calls;

static void count_on(void *context) { ((Calls *)context)->ons++; }
static void count_off(void *context) { ((Calls *)context)->offs++; }

static Heater spy_heater(Calls *calls) { return (Heater){calls, count_on, count_off}; }

static void test_too_cold(void) {
    int celsius = 17;
    Calls calls = {0, 0};
    regulate(stub_sensor(&celsius), spy_heater(&calls), 20);
    CHECK_INT_EQ(calls.ons, 1);
    CHECK_INT_EQ(calls.offs, 0);
}

static void test_too_hot(void) {
    int celsius = 23;
    Calls calls = {0, 0};
    regulate(stub_sensor(&celsius), spy_heater(&calls), 20);
    CHECK_INT_EQ(calls.ons, 0);
    CHECK_INT_EQ(calls.offs, 1);
}

static void test_close_enough(void) {
    for (int celsius = 19; celsius <= 21; celsius++) {
        Calls calls = {0, 0};
        regulate(stub_sensor(&celsius), spy_heater(&calls), 20);
        CHECK_INT_EQ(calls.ons + calls.offs, 0);
    }
}

int main(void) {
    RUN_TEST(test_too_cold);
    RUN_TEST(test_too_hot);
    RUN_TEST(test_close_enough);
    return check_exit_code();
}
```

**C++: `thermostat_test.cpp`**

```cpp
#include "check.hpp"
#include "thermostat.hpp"

using thermostat::regulate;

// A stub: a sensor that reads whatever temperature the test gives it.
struct StubSensor : thermostat::Sensor {
    int celsius;
    explicit StubSensor(int celsius) : celsius(celsius) {}
    int read_celsius() override { return celsius; }
};

// A spy: a heater that counts the calls made to it, for the test to check.
struct SpyHeater : thermostat::Heater {
    int ons = 0;
    int offs = 0;
    void turn_on() override { ons++; }
    void turn_off() override { offs++; }
};

TEST(too_cold) {
    StubSensor sensor(17);
    SpyHeater heater;
    regulate(sensor, heater, 20);
    CHECK_EQ(heater.ons, 1);
    CHECK_EQ(heater.offs, 0);
}

TEST(too_hot) {
    StubSensor sensor(23);
    SpyHeater heater;
    regulate(sensor, heater, 20);
    CHECK_EQ(heater.ons, 0);
    CHECK_EQ(heater.offs, 1);
}

TEST(close_enough) {
    for (int celsius = 19; celsius <= 21; celsius++) {
        StubSensor sensor(celsius);
        SpyHeater heater;
        regulate(sensor, heater, 20);
        CHECK_EQ(heater.ons + heater.offs, 0);
    }
}

int main() { return check::run_all_tests(); }
```

**Rust: `src/tests.rs`**

```rust
use super::*;

/// A stub: a sensor that reads whatever temperature the test gives it.
struct StubSensor(i32);

impl Sensor for StubSensor {
    fn read_celsius(&self) -> i32 {
        self.0
    }
}

/// A spy: a heater that records the calls made to it, for the test to check.
#[derive(Default)]
struct SpyHeater {
    calls: Vec<&'static str>,
}

impl Heater for SpyHeater {
    fn turn_on(&mut self) {
        self.calls.push("on");
    }
    fn turn_off(&mut self) {
        self.calls.push("off");
    }
}

#[test]
fn too_cold() {
    let mut heater = SpyHeater::default();
    regulate(&StubSensor(17), &mut heater, 20);
    assert_eq!(heater.calls, ["on"]);
}

#[test]
fn too_hot() {
    let mut heater = SpyHeater::default();
    regulate(&StubSensor(23), &mut heater, 20);
    assert_eq!(heater.calls, ["off"]);
}

#[test]
fn close_enough() {
    for celsius in 19..=21 {
        let mut heater = SpyHeater::default();
        regulate(&StubSensor(celsius), &mut heater, 20);
        assert!(heater.calls.is_empty(), "at {celsius}: {:?}", heater.calls);
    }
}
```

**Go: `thermostat_test.go`**

```go
package thermostat

import (
	"slices"
	"testing"
)

// A stub: a sensor that reads whatever temperature the test gives it.
type stubSensor int

func (s stubSensor) ReadCelsius() int { return int(s) }

// A spy: a heater that records the calls made to it, for the test to check.
type spyHeater struct {
	calls []string
}

func (h *spyHeater) TurnOn()  { h.calls = append(h.calls, "on") }
func (h *spyHeater) TurnOff() { h.calls = append(h.calls, "off") }

func TestTooCold(t *testing.T) {
	heater := &spyHeater{}
	Regulate(stubSensor(17), heater, 20)
	if !slices.Equal(heater.calls, []string{"on"}) {
		t.Errorf("calls = %q, want [on]", heater.calls)
	}
}

func TestTooHot(t *testing.T) {
	heater := &spyHeater{}
	Regulate(stubSensor(23), heater, 20)
	if !slices.Equal(heater.calls, []string{"off"}) {
		t.Errorf("calls = %q, want [off]", heater.calls)
	}
}

func TestCloseEnough(t *testing.T) {
	for celsius := 19; celsius <= 21; celsius++ {
		heater := &spyHeater{}
		Regulate(stubSensor(celsius), heater, 20)
		if len(heater.calls) != 0 {
			t.Errorf("at %d: calls = %q, want none", celsius, heater.calls)
		}
	}
}
```

**Python: `test_thermostat.py`**

```python
from unittest.mock import Mock

from thermostat import Heater, Sensor, regulate


class StubSensor:
    """A stub: a sensor that reads whatever temperature the test gives it."""

    def __init__(self, celsius: int) -> None:
        self.celsius = celsius

    def read_celsius(self) -> int:
        return self.celsius


class SpyHeater:
    """A spy: a heater that records the calls made to it, for the test to check."""

    def __init__(self) -> None:
        self.calls: list[str] = []

    def turn_on(self) -> None:
        self.calls.append("on")

    def turn_off(self) -> None:
        self.calls.append("off")


def test_too_cold() -> None:
    heater = SpyHeater()
    regulate(StubSensor(17), heater, 20)
    assert heater.calls == ["on"]


def test_too_hot() -> None:
    heater = SpyHeater()
    regulate(StubSensor(23), heater, 20)
    assert heater.calls == ["off"]


def test_close_enough() -> None:
    for celsius in [19, 20, 21]:
        heater = SpyHeater()
        regulate(StubSensor(celsius), heater, 20)
        assert heater.calls == [], f"at {celsius}"


# The same test as test_too_hot, with the standard library's Mock instead: a mock
# records each call, and returns what it was told to. With a spec, it accepts only
# the methods that the class it stands in for has.
def test_too_hot_with_mocks() -> None:
    sensor = Mock(spec=Sensor)
    sensor.read_celsius.return_value = 23
    heater = Mock(spec=Heater)
    regulate(sensor, heater, 20)
    heater.turn_off.assert_called_once_with()
    heater.turn_on.assert_not_called()
```

**JavaScript: `thermostat.test.js`**

```js
import assert from "node:assert/strict";
import { mock, test } from "node:test";
import { regulate } from "./thermostat.js";

// A stub: a sensor that reads whatever temperature the test gives it.
function stubSensor(celsius) {
  return { readCelsius: () => celsius };
}

// A spy: a heater that records the calls made to it, for the test to check.
function spyHeater() {
  const calls = [];
  return {
    calls,
    turnOn: () => calls.push("on"),
    turnOff: () => calls.push("off"),
  };
}

test("too cold", () => {
  const heater = spyHeater();
  regulate(stubSensor(17), heater, 20);
  assert.deepEqual(heater.calls, ["on"]);
});

test("too hot", () => {
  const heater = spyHeater();
  regulate(stubSensor(23), heater, 20);
  assert.deepEqual(heater.calls, ["off"]);
});

test("close enough", () => {
  for (const celsius of [19, 20, 21]) {
    const heater = spyHeater();
    regulate(stubSensor(celsius), heater, 20);
    assert.deepEqual(heater.calls, [], `at ${celsius}`);
  }
});

// The same test as "too hot", with node:test's mock functions instead, which
// record their calls.
test("too hot, with mocks", () => {
  const heater = { turnOn: mock.fn(), turnOff: mock.fn() };
  regulate({ readCelsius: () => 23 }, heater, 20);
  assert.equal(heater.turnOff.mock.callCount(), 1);
  assert.equal(heater.turnOn.mock.callCount(), 0);
});
```

**Dart: `test/thermostat_test.dart`**

```dart
import 'package:test/test.dart';
import 'package:thermostat/thermostat.dart';

/// A stub: a sensor that reads whatever temperature the test gives it.
class StubSensor implements Sensor {
  final int celsius;
  StubSensor(this.celsius);

  @override
  int readCelsius() => celsius;
}

/// A spy: a heater that records the calls made to it, for the test to check.
class SpyHeater implements Heater {
  final calls = <String>[];

  @override
  void turnOn() => calls.add('on');

  @override
  void turnOff() => calls.add('off');
}

void main() {
  test('too cold', () {
    final heater = SpyHeater();
    regulate(StubSensor(17), heater, 20);
    expect(heater.calls, ['on']);
  });

  test('too hot', () {
    final heater = SpyHeater();
    regulate(StubSensor(23), heater, 20);
    expect(heater.calls, ['off']);
  });

  test('close enough', () {
    for (final celsius in [19, 20, 21]) {
      final heater = SpyHeater();
      regulate(StubSensor(celsius), heater, 20);
      expect(heater.calls, isEmpty, reason: 'at $celsius');
    }
  });
}
```

**Kotlin: `src/test/kotlin/ThermostatTest.kt`**

```kotlin
import kotlin.test.Test
import kotlin.test.assertEquals

/** A spy: a heater that records the calls made to it, for the test to check. */
class SpyHeater : Heater {
    val calls = mutableListOf<String>()

    override fun turnOn() {
        calls += "on"
    }

    override fun turnOff() {
        calls += "off"
    }
}

class ThermostatTest {
    // The stubs are lambdas: Sensor { 17 } is a sensor that always reads 17.
    @Test
    fun tooCold() {
        val heater = SpyHeater()
        regulate(Sensor { 17 }, heater, 20)
        assertEquals(listOf("on"), heater.calls)
    }

    @Test
    fun tooHot() {
        val heater = SpyHeater()
        regulate(Sensor { 23 }, heater, 20)
        assertEquals(listOf("off"), heater.calls)
    }

    @Test
    fun closeEnough() {
        for (celsius in 19..21) {
            val heater = SpyHeater()
            regulate(Sensor { celsius }, heater, 20)
            assertEquals(emptyList(), heater.calls, "at $celsius")
        }
    }
}
```

**Five kinds of double.** The names come from Gerard Meszaros's *xUnit Test Patterns*,
of 2007. A **dummy** is passed only to fill a parameter, and never used. A **stub**
answers calls with what the test chose. A **spy** is a stub that also records the calls
made to it. A **mock** is told in advance which calls to expect, and checks them itself.
A **fake** is a working version of the real thing, simpler, such as a map instead of a
database, or exercise 19.2's clock, which says whatever time the test sets.

**Mocks from a library.** Writing a spy takes a few lines in each language, as the
examples show, but Python and JavaScript also have a mock object in their standard
libraries, which the last test of each uses. Python's `unittest.mock.Mock` accepts any
call, records it, and returns what it was told to, and
`heater.turn_off.assert_called_once_with()` checks afterwards that a call was made.
`node:test`'s `mock.fn()` is a function that records its calls, with `mock.callCount()`.
The other languages have mocking libraries: Mockito and MockK for Kotlin, mockall for
Rust, gomock for Go, GoogleMock for C++, and mocktail and mockito for Dart.

A mock that accepts any call also accepts the wrong ones. If the thermostat's code said
`heater.turnoff()`, a typo, a test with `heater = Mock()` would pass, since a `Mock`
makes up a `turnoff` method on the spot. With `Mock(spec=Heater)`, as in the example, it
knows the methods a heater has, and the test fails with
`AttributeError: Mock object has no attribute 'turnoff'. Did you mean: 'turn_off'?`. The
spies in the examples don't have this problem, since a typed language rejects a call to
a method that the interface doesn't have, and mypy checks Python's `SpyHeater` against
the `Heater` protocol.

**When to use one.** Use a double for what is slow, what isn't deterministic, such as
time, randomness, and the network, and what is outside the program, such as hardware.
Use the real thing for the rest, even when it is another class of your own, since a test
that replaces everything a function calls tests that the function makes those calls, not
that it works. A test that checks which methods were called, in which order, also breaks
when the code is changed to get the same result another way, which is the opposite of
what tests are for.

---

## 19.7 What tests can see

A test is code like any other, so the visibility rules of chapter 18 apply to it, and
the languages differ in where they put tests relative to them. Each row says what a test
of example 02 can reach, and what it says when it reaches for the private table of
numerals:

| Language | A test can see | Reaching for a private name |
| --- | --- | --- |
| C | What the headers declare; a `static` function or variable can't be reached from another file | ``undefined reference to `NUMERALS'``, from the linker, since `NUMERALS` is `static` in `roman.c` |
| C++ | The same, where a name in an unnamed namespace is like a `static` one in C; and only the public members of a class, unless the class declares the test a `friend` | ``undefined reference to `roman::NUMERALS'``, since `NUMERALS` is in an unnamed namespace in `roman.cpp`; for a private member, `error: 'int Counter::count_' is private within this context` |
| Rust | The private items of the module it tests, and of that module's parents, from a `#[cfg(test)]` module inside it, which `use super::*` brings in; only `pub` items from an integration test in `tests/`, which is a crate of its own | From `tests/`: ``error[E0603]: constant `NUMERALS` is private`` |
| Go | Everything in the package, from a test file in the package; only exported names from one in `package roman_test`, which imports the package as a user would | From `roman_test`: `undefined: roman.numerals` |
| Python | Everything; a leading `_` is a convention, which a test may break | Nothing stops it |
| JavaScript | Only what the module exports | `SyntaxError: The requested module './roman.js' does not provide an export named 'NUMERALS'` |
| TypeScript | The same | From `tsc`: `error TS2459: Module '"./roman.ts"' declares 'NUMERALS' locally, but it is not exported.` |
| Dart | The public names of the libraries it imports; a `_` name is private to its library, which here is its file | `error - Undefined name '_numerals'.` |
| Kotlin | Everything `internal`, since the tests are compiled as part of the same module, but not `private` | `Cannot access 'val numerals: List<Pair<Int, String>>': it is private in file.` |

**Test the promise.** That some languages let a test see private code doesn't mean it
should look. A function's interface is a promise, and its implementation is free to
change: a test of the promise keeps passing while the implementation is rewritten, which
is how a test tells you that a rewrite worked. A test of a private helper fails when the
helper is renamed or merged into something else, even if nothing a user can see has
changed. Each exercise of this chapter lists what its code promises, and its tests may
rely on that and nothing else, so that they would pass on any correct version of the
code. Private code is tested through the public functions that use it. When a private
piece is complicated enough to want tests of its own, it is often a module of its own
waiting to be split out, and Rust's and Go's in-package tests are there for the times
when it isn't.

---

## 19.8 Running tests

### Running some of them

Every runner can run a subset, which is what you want while fixing one failure:

```text
cargo test out_of_range                      # Rust: tests whose names contain it
go test -run 'TestToRoman$/1994'             # Go: a regular expression for each level
uv run pytest -k 1994                        # Python: names that match an expression
uv run pytest test_roman.py::test_out_of_range
node --test --test-name-pattern=1994         # JavaScript and TypeScript
dart test --name 1994                        # Dart: a regular expression
gradle test --tests 'RomanTest.outOfRange'   # Kotlin: a class, or a class and method
```

Each says what it left out: Cargo `1 passed; ... 1 filtered out`, and pytest
`1 passed, 12 deselected`. Go's `-run` takes one regular expression for each level of
test, split at `/`, and each is matched anywhere in a name, so `-run TestToRoman/1994`
also runs `TestToRomanOutOfRange`, whose name contains `TestToRoman`. The `$` stops
that. `check.h` and `check.hpp` always run every test, and GoogleTest takes
`--gtest_filter=Leap.*`.

### Order, isolation, and running at the same time

Tests should pass in any order, and alone, which means that none may depend on what
another left behind. The runners differ in how much they test that for you:

- **Rust** runs a crate's tests at the same time, on several threads: two tests that
  each sleep for a second finish in `1.00s`, and in `2.00s` with
  `cargo test -- --test-threads=1`.
- **Go** runs a package's tests one at a time, in the order of the source, unless a test
  calls `t.Parallel()`, and `go test -shuffle=on` runs them in a random order, printing
  the seed, as `-test.shuffle 1795371518514663368`, so that
  `-shuffle=1795371518514663368` can repeat an order that failed.
- **pytest** runs tests in the order of the files, one at a time. The pytest-randomly
  plugin shuffles them, and pytest-xdist runs them in several processes.
- **`node --test`** runs each test file in a process of its own, several files at once,
  and the tests in a file one after another.
- **Dart** runs several test files at once, and shuffles the tests in each with
  `--test-randomize-ordering-seed=random`, which prints the seed:
  `Shuffling test order with --test-randomize-ordering-seed=3471680479`.
- **JUnit 5** runs a class's tests in an order its documentation calls deterministic but
  intentionally nonobvious, so that no one comes to rely on it, and Gradle can run test
  classes in several processes with `maxParallelForks`.

The safest way to keep tests independent is for each to make what it uses. JUnit makes a
new object of the test class for each test, so the fields of exercise 19.2's Kotlin
solution start fresh every time. A pytest fixture runs again for each test that asks for
it, and Dart's `setUp` runs before each test. In C and C++, everything in one test
program shares its globals, so a `static` that one test changes is changed for the next.

### Coverage

A **coverage** tool records which lines, and often which branches, of the code the tests
ran. Most of the languages have one: `go test -cover`,
`node --test --experimental-test-coverage`, `dart test --coverage-path=lcov.info`,
coverage.py and pytest-cov for Python, cargo-llvm-cov for Rust, JaCoCo and Kover for
Kotlin, and `gcc --coverage` with `gcov` for C and C++. This is exercise 19.1's word
wrapper, with its starter's one test and then with the solution's ten:

```text
                 the starter's one test       the solution's tests
go test -cover   coverage: 78.6% of statements   coverage: 100.0% of statements
node             wrap.js | 77.27 | 66.67 | ...   wrap.js | 100.00 | 100.00 | ...
gcov             Lines executed:91.30% of 23     Lines executed:100.00% of 23
coverage.py      wrap.py   15   3   80%          wrap.py   15   0   100%
```

The starter's one test runs more than three quarters of the code, and catches one of the
six bugs. Coverage counts what ran, not what was checked: a Go test that calls `Wrap` on
four texts and looks at none of the results has `coverage: 100.0% of statements`, and
catches none of them. So coverage is good at one thing, finding code that no test runs,
which is certainly untested, and says nothing about whether the code that ran was tested
well. **Mutation testing** measures that, by changing the code, a `<` to a `<=`, or a
line deleted, and running the tests on each change: a change that no test notices is a
**surviving mutant**, either a missing test or code that doesn't matter. The checker
does that by hand in this chapter's exercises, with bugs chosen to be the plausible
ones, and tools do it automatically, with hundreds of changes: cargo-mutants for Rust,
mutmut for Python, Stryker for JavaScript and TypeScript, and PIT for the JVM.

### Flaky tests

A flaky test fails sometimes, with nothing changed, and it is worse than it sounds: once
a suite has one, a red result stops meaning that something is broken, and people stop
looking. Nearly every flaky test depends on something that isn't the same on every run:

- **Time.** A test that reads the real clock passes or fails depending on when it runs,
  and a test that sleeps for a second and expects something to have happened fails when
  the machine is busy. Pass a clock in, as exercise 19.2 does, and set it.
- **Randomness.** Seed the random number generator with a fixed number in tests, as the
  stretch exercises do, or print the seed, as Go and Dart do when they shuffle, so that
  a failure can be repeated.
- **Order and shared state.** A test that passes only after another one has run, or
  fails only after one, shares something with it: a global, a file, or a row in a
  database. Give each test its own.
- **Concurrency.** A test of threads that passes because one thread usually wins the
  race, as in chapter 16. Wait for what the test needs, with a join or a channel, and
  not for a length of time.
- **The world.** The network, the files on the machine, environment variables, the time
  zone, and the locale. Use a double, or a temporary folder that the test makes and
  deletes.

### Tests on every push

A suite helps most when it runs on every change, which is what chapter 17's CI is for.
This book's own workflow runs `python3 tools/check.py all --solutions` on every push to
`main` and every pull request, which runs every test in every chapter, and GitHub shows
the result on the pull request. A CI server only reads the exit status, which is why
every runner exits with something other than 0 when a test fails. The lock files of
chapter 18 matter here too: `uv.lock` pins pytest at 9.2.0, and `pubspec.lock` pins
`package:test` at 1.33.0, so CI runs the tests with the same framework as your machine
does.

### What the checker does in this chapter

A language folder with test files in it, such as `leap_test.go` or `test_leap.py`, is a
test suite to the checker, which builds and runs it with the language's own runner,
instead of comparing a program's output. Each `tests/NAME.in` is a version of the code:
an empty one is the code as given, and the others are a line saying what is wrong, then
edits to each language's files. The checker copies the folder for each version, makes
the edits, runs the tests, and checks that they pass on the code as given and fail on
every edited copy. For the examples the edits are the bugs that the text describes, and
for the exercises they are the bugs your tests must catch.

---

## 19.9 Why each language looks the way it does

- **C** has had `assert` since the 1970s, from Unix, and it is for the program's own
  checks, as in chapter 11: it stops the program, prints the condition, and is compiled
  out completely with `-DNDEBUG`. The C standard has never had more, so C projects use
  frameworks such as Unity, CMocka, and Check, or a header like this chapter's.
- **C++** has no standard framework either. GoogleTest, which Google released in 2008,
  and Catch, which Phil Nash wrote in 2010, became the common ones. Both use the trick
  that `check.hpp` shows: a macro that defines a global object, whose constructor
  registers the test before `main` runs.
- **Rust** put tests in the compiler and Cargo from 1.0, in 2015. `#[test]` marks a
  function, `rustc --test` builds the crate with a runner that calls every marked
  function, and because the tests are a module inside the code they test, they see its
  private items, so testing a private function needs no special arrangement. Code in
  documentation comments is run as a test too, so that the examples in a crate's
  documentation can't go out of date.
- **Go** has had `testing` and `go test` from 1.0, in 2012, and deliberately no
  assertions. The Go FAQ explains why: testing frameworks tend to grow into small
  languages of their own, with their own conditions and their own ways of printing, and
  Go would rather tests be written in Go. So a Go test is an `if` and a message, and the
  convention of saying what was called, what it returned, and what was wanted is what
  makes up for the missing assertions.
- **Python** got `unittest` in Python 2.1, in 2001, from PyUnit, a copy of Java's JUnit,
  which is why its methods are named in camel case, `assertEqual`, unlike the rest of
  the standard library. pytest grew from py.test, which Holger Krekel started in 2004,
  and won by needing less: a test is a function, and a check is a plain `assert`, which
  it rewrites as it imports each test file so that a failure can show the values.
- **JavaScript** had no test runner in the language or in Node for most of its life, and
  packages filled the gap: Jasmine, Mocha, and Jest, which Facebook released in 2014.
  Node added `node:test` in version 18, in 2022, and made it stable in version 20.
- **TypeScript** uses JavaScript's test runners, either on the JavaScript that `tsc`
  writes, or, as here, on the TypeScript itself with Node stripping the types, which
  leaves checking them to `tsc`.
- **Dart** has `package:test`, written and maintained by the Dart team, with `expect`
  and matchers in the style of Java's Hamcrest library, and Flutter's tests are built on
  it.
- **Kotlin** inherited JUnit from Java. Kent Beck and Erich Gamma wrote JUnit in 1997,
  after Beck's SUnit for Smalltalk, and its design, of test cases, assertions, set-up,
  and a runner, is copied so widely that the family is called **xUnit**. `kotlin.test`
  is a thin layer of Kotlin assertions over whichever framework a platform has, JUnit on
  the JVM and others for JavaScript and native code, so that the same tests can run on
  every platform Kotlin compiles to.

---

## 19.10 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | Testing with `assert` from `<assert.h>` | ``Assertion `is_leap_year(2000)' failed.`` and the program aborts, which ends the run at the first failure; and with `-DNDEBUG`, as release builds often have, the test passes, since every `assert` is compiled out |
| C | A test function that `main` doesn't call | It never runs, and the run says `all checks passed`; if the function is `static`, `-Wall` warns `'test_centuries' defined but not used [-Wunused-function]` |
| C | `==` on two strings | It compares their addresses; use `strcmp`, or `CHECK_STR_EQ` |
| C++ | A global or `static` that one test changes | The next test starts with the change, since one program runs them all; make what each test needs inside it |
| Rust | A test function without `#[test]` | It doesn't run, and the only sign is ``warning: function `centuries` is never used`` |
| Rust | `assert!(a == b)` | `assertion failed: is_leap_year(2000) == true`, without the values; use `assert_eq!` |
| Go | A test named `Testcenturies` | `go test` refuses it: `Testcenturies has malformed name: first letter after 'Test' must not be lowercase` |
| Go | `t.Error` where the rest of the test needs what failed | The test carries on, and may panic on a nil value; use `t.Fatal` when nothing after can work |
| Go | `go test -run TestToRoman/1994` | Also runs `TestToRomanOutOfRange`, since each part is matched anywhere in a name; anchor it, `'TestToRoman$/1994'` |
| Python | A test function whose name doesn't start with `test` | It isn't collected; with none at all, pytest says `no tests ran` and exits with status 5 |
| Python | `assert (x, "message")` | A tuple is always true, so the test passes; pytest warns `assertion is always true, perhaps remove parentheses?`; write `assert x, "message"` |
| Python | `Mock()` for an object with a known interface | It accepts any call, misspelled ones included; use `Mock(spec=Heater)`; a misspelled assertion, such as `assert_called_once_wiht()`, fails with `'assert_called_once_wiht' is not a valid assertion.` |
| JavaScript | `assert.equal` on arrays or objects | `Values have same structure but are not reference-equal`; use `assert.deepEqual` |
| JavaScript | `import assert from "node:assert"`, without `/strict` | `assert.equal(1, "1")` passes, since the old `equal` compares with `==`; import `node:assert/strict` |
| JavaScript | An assertion in a callback that runs after the test returns, such as in a `setTimeout` | The test is reported as passing, and the run fails with `generated asynchronous activity after the test ended`; make the test `async` and `await` what it waits for |
| TypeScript | A type error in a test | Node runs it anyway, since it strips the types without checking them: `isLeapYear("2000")` passes; `tsc` says `error TS2345: Argument of type 'string' is not assignable to parameter of type 'number'.` |
| Dart | `expect(toRoman(0), throwsRangeError)` | `toRoman(0)` throws before `expect` is called, so the test fails with the error it meant to check: `RangeError (n): Invalid value: Not in inclusive range 1..3999: 0`; write `expect(() => toRoman(0), throwsRangeError)` |
| Kotlin | `assertEquals(actual, expected)` | Wrong failures read backwards: `expected: <false> but was: <true>` |
| All | A test that runs code and checks nothing | It passes, counts toward coverage, and catches no bug |
| All | A test that reads the real clock, sleeps, or uses an unseeded random number | It is flaky: pass the clock and the seed in |
| All | Tests that test private helpers, or which calls were made | They break when the code is changed in a way that breaks nobody; test what the code promises |

---

## 19.11 Idiom check

**C: a test program, and a framework.** Put tests in their own files, compiled with the
code into a test program whose exit status says whether they passed, and register it
with CMake's `add_test` so that `ctest` runs it. Use a framework, Unity, CMocka, or a
header like `check.h`, rather than `assert`, which stops at the first failure and
disappears with `NDEBUG`. Make each test function `static`, so that the compiler warns
about one that `main` forgot.

**C++: GoogleTest or Catch2.** Use GoogleTest or Catch2, through CMake with
`FetchContent` or a package manager, with `EXPECT_` assertions, or `CHECK` in Catch2,
for most checks and `ASSERT_` or `REQUIRE` where the rest of a test can't go on. Use
fixtures, `TEST_F` in GoogleTest, for set-up that several tests share, and GoogleMock
for mocks of interfaces.

**Rust: `#[test]` and `assert_eq!`.** Put unit tests in a `#[cfg(test)] mod tests` at
the end of the file, or in a `tests.rs` beside it, and tests of the public interface in
`tests/`. Derive `Debug` and `PartialEq` on types that tests compare, use `assert_eq!`
with a message for tables of cases, and return `Result` from a test,
`fn parses() -> Result<(), Error>`, to use `?` in it. Write examples in documentation
comments, which `cargo test` runs.

**Go: tables and subtests.** Write tests as tables of structs, run each case with
`t.Run`, and write messages as `Func(args) = got, want want`. Use `t.Helper()` in
helpers, so that failures point at the test, `t.Fatal` only when the test can't go on,
and `go test -race` for code with goroutines. Put tests of the public interface in a
`package name_test`, and use `go test -shuffle=on` now and then.

**Python: pytest.** Write plain functions with plain `assert`, use
`@pytest.mark.parametrize` for tables, `pytest.raises` for errors, and fixtures, not
globals, for set-up. Give mocks a `spec`, or better, write a small fake class. Add
pytest as a dev dependency, as `uv add --dev pytest` does, and keep the lock file.

**JavaScript: `node:test` and `node:assert/strict`.** Use `node:test` for libraries and
Node programs, or Vitest or Jest where a project already has one, and always the strict
`assert`. Use `deepEqual` for arrays and objects, `assert.throws` and `assert.rejects`
for errors, and `async` tests that `await` what they start. Use `mock.fn()` and
`mock.timers` for doubles.

**TypeScript: JavaScript's tests, type-checked.** Write the tests in TypeScript, run
them with `node --test` or a runner that understands TypeScript, and run `tsc` as well,
in CI, since running the tests doesn't check the types. Give fakes the interface they
stand in for, `class FakeClock implements Clock`, so that the compiler checks them.

**Dart: `package:test`.** Use `test` and `group`, `expect` with matchers, `setUp` for
each test's set-up, and `throwsA` or the `throws...` matchers with the call wrapped in a
closure. Run `dart test` in CI with `dart analyze`. Flutter apps use `flutter_test`,
which builds on it.

**Kotlin: `kotlin.test` on JUnit 5.** Use `kotlin.test`'s `@Test` and assertions, with
the expected value first, `assertFailsWith` for exceptions, and JUnit's
`@ParameterizedTest` for tables. Let JUnit's fresh instance for each test do the set-up
in fields, use `fun interface` and lambdas for small fakes, and MockK where a real
mocking library is needed.

---

## 19.12 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Test a word wrapper](exercises/01-word-wrap/) | Tests at the edges of each rule, written as a table of cases |
| 2 | [Test an expiring cache](exercises/02-expiring-cache/) | A fake clock, passed in, and tests that tell a short story with it |
| 3 | [Test a duration parser](exercises/03-durations/) | Testing what a parser refuses, testing errors, and round trips |

```sh
python3 tools/check.py 19                     # your tests, every language
python3 tools/check.py 19 --lang go,python    # some languages
python3 tools/check.py 19 --solutions         # the reference tests, and the examples
```

In each exercise the code is given and the tests are the exercise. The checker runs your
tests on the code as given, where they must pass, and then on copies of it that each
have one bug, where they must fail, and it says which bugs they missed:
`missed 5 of 6 bugs: 02-exact-fit, ...`. The starter tests pass, and miss nearly every
bug. Don't change the code, since the bugs are edits to it, and don't read the bugs in
`tests/` until your tests are written, since then you would be testing for the bugs you
know of, and the point is to find the ones you don't. Each exercise's README lists what
its code promises: test that, and only that.

**Stretch:** pick a function of your own, from an earlier chapter's exercise, write down
what it promises, as these exercises' READMEs do, and test it until you can't think of a
bug your tests would miss. Then make five bugs in it yourself, one at a time, the kind a
tired person would make, and see how many your tests catch. Each one that survives is a
test to write.

---

Previous: [Chapter 18: Modules, packages, builds](../18-modules-packages-and-builds/) ·
Next: [Chapter 20: Debugging and profiling](../20-debugging-and-profiling/)
