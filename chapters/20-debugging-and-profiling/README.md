# Chapter 20: Debugging and profiling

A test that fails says that something is wrong, and rarely where. This chapter is about
finding out: reading what a program prints when it crashes, printing and logging what it
does, stopping it in a debugger to look at its variables, turning on checks that catch
bugs which don't crash, and measuring where a slow program spends its time. The nine
languages split two ways here. C, C++, Rust, and Go compile to machine code, and share
the tools that work on machine code: gdb and lldb, the sanitizers built into GCC and
Clang, Valgrind, and the profilers that sample a running process. Python, JavaScript,
TypeScript, Dart, and Kotlin run on a virtual machine, and each VM has a debugger and a
profiler built in, which tools reach through a protocol of their own. The other split is
in what a bug looks like. The same mistake, an index one letter out of range, crashes
with a clear report in four of the languages, and in the other five gives a wrong answer
with no sign at all, which is why the second half of the chapter is about checks that
make bugs loud.

**Languages:** all nine · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Reading a crash: panics, exceptions, tracebacks, and stack traces, and which way up each one is | Files, JSON, and the CLI (chapter 21), where bad input comes from outside the program |
| Printing values for debugging, and logging, with what each language prints for a value | REST CRUD API (chapter 38), where a server's logs are how it is debugged |
| Using a debugger: breakpoints, stepping, the call stack, watchpoints, and debugging after a crash | Recursion, backtracking, dynamic programming (chapter 30) |
| Sanitizers and runtime checks that find bugs a crash doesn't show | FFI and interop (chapter 23), security basics (chapter 40) |
| Profiling: sampling and instrumenting profilers, self time and total time, and what inlining does to a profile | Complexity and benchmarking (chapter 24) |
| Shrinking a failing input, and finding the commit that broke something | Text parsing (chapter 22) |

---

## 20.1 The concept: debugging

A bug has three parts, in the words of Andreas Zeller's book *Why Programs Fail*. The
**defect** is the mistake in the code, such as a `>=` that should be `>`. When the
program runs the defect, it can make a wrong value, an **infection**, which spreads as
other code uses it. And when a wrong value reaches something you can see, a crash or a
wrong line of output, that is the **failure**. Debugging runs the chain backwards, from
the failure you can see to the defect that caused it, and the two can be far apart: a
crash is where a wrong value finally did harm, and it was often made somewhere else.

```text
  defect                        infection                        failure
  total = 0 before the loop  -> total still holds the last     -> "alan 673", for a student
  instead of inside it          student's scores when the next    whose only score is 70
                                one starts
```

That is exercise 20.1's report with one of its four bugs: the first student's average is
right, and every one after it is wrong, by more each time. So debugging is a search, and
it goes best the way any search does: by narrowing it down.

1. **Reproduce it.** Find an input that fails every time, and make it a test, as in
chapter 19, so that you can run it after every change. A bug that only happens sometimes
is a bug that depends on something you haven't found yet, such as the time, the order of
threads, or an uninitialized variable. 2. **Shrink it.** Cut the input down to the
smallest one that still fails. Section 20.8 has ways to do it. 3. **Find where it goes
wrong.** Read the crash report, print values, or stop the program in a debugger, and
find the first place where a value is wrong. Everything before that point is right, and
the defect is between the last right value and the first wrong one. 4. **Fix the
cause.** A fix at the failure, such as a check for zero just before the division, hides
the bug instead of fixing it. 5. **Keep the test.** The test that reproduced the bug is
now a regression test, and it stops the bug coming back.

A program can also be right and slow, and finding out why is a search of the same kind,
with a different tool. A **profiler** runs the program and reports where the time went:
which functions ran, how often, and for how long. Donald Knuth's warning about speed,
from 1974, is often quoted as half a sentence: "We should forget about small
efficiencies, say about 97% of the time: premature optimization is the root of all evil.
Yet we should not pass up our opportunities in that critical 3%." A profiler is how you
find the 3%, and section 20.7 shows that it is rarely where you would guess.

---

## 20.2 What each language gives you

| Language | A bug that goes uncaught | Debugger | Checks you can turn on | Profiler |
| --- | --- | --- | --- | --- |
| C | Often nothing: a bad index or overflow is undefined behavior; sometimes a signal, such as `SIGSEGV` or `SIGFPE` | gdb or lldb, on a build with `-g` | AddressSanitizer and UndefinedBehaviorSanitizer, `-fsanitize=address,undefined`; Valgrind | Callgrind, gprof, and perf, from outside |
| C++ | The same, or `terminate called after throwing an instance of ...` for an exception | gdb or lldb | The sanitizers; `-D_GLIBCXX_ASSERTIONS` for bounds checks in the standard library | The same as C |
| Rust | A panic, with the file, line, and column; a stack trace with `RUST_BACKTRACE=1` | gdb or lldb, through `rust-gdb` and `rust-lldb` | Overflow checks, on in debug builds; Miri, on nightly Rust, for `unsafe` code | The same as C |
| Go | A panic, with the stack of the goroutine that panicked | Delve, `dlv`; gdb works, less well | The race detector, `-race` | pprof, built into the standard library and `go tool` |
| Python | An exception, and a traceback with the most recent call last | pdb, in the standard library, and `breakpoint()` | Development mode, `python3 -X dev` | cProfile, in the standard library |
| JavaScript | An exception with a stack trace; a bad index gives `undefined`, silently | `node inspect`, or Chrome's DevTools with `node --inspect` | None for indexes; strict mode, which modules have | `node --prof`, and `node --cpu-prof` for DevTools |
| TypeScript | JavaScript's | JavaScript's, on the TypeScript, since Node strips the types | `noUncheckedIndexedAccess`, which the compiler checks | JavaScript's |
| Dart | An exception, `Unhandled exception:`, with a stack trace | DevTools, through the Dart VM service, or an editor | `assert`, with `--enable-asserts` | DevTools' CPU Profiler |
| Kotlin | An exception with the JVM's stack trace | jdb, or IntelliJ's debugger | `assert`, with the JVM's `-ea` | Java Flight Recorder, built into the JVM |

Read down the third column and the languages sort by what they run on. The four that
compile to machine code are debugged with the same two debuggers, gdb and lldb, which
read the debugging information the compiler writes into the program with `-g`, in a
format called DWARF: the name and type of each variable, and which machine instructions
came from which line. The five on a VM debug through the VM, which knows each variable
and line already, and gives a debugger a way in: Python's interpreter calls a debugger's
hooks, through `sys.settrace` and, since Python 3.12, `sys.monitoring`, and the others
let a debugger connect from outside, Node's and Chrome's through the Chrome DevTools
Protocol, Dart's through its VM service, and the JVM's through JDWP, the Java Debug Wire
Protocol. Editors don't speak each of these: VS Code and others use the Debug Adapter
Protocol, with one adapter for each debugger, so that the same buttons work in all nine
languages.

Read down the second column and they sort by what they check. Rust, Go, Python, Dart,
and Kotlin check every index, and fail at the line with the bad one. C and C++ check
nothing unless asked, and JavaScript gives `undefined` for an index out of range, which
turns into `NaN` in arithmetic. Python checks indexes, but a negative one counts from
the end, so a small negative number is not an error at all. Section 20.3 shows what each
does with the same mistake.

---

## 20.3 Reading a crash

Example 01 counts the letters in its input, and draws a bar for each. It expects
lowercase text, and doesn't check:

**C**

```c
// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
#include <ctype.h>
#include <stdio.h>

static void count_letters(FILE *in, int counts[26]) {
    int c;
    while ((c = getc(in)) != EOF) {
        if (isalpha(c)) counts[c - 'a']++;
    }
}

int main(void) {
    int counts[26] = {0};
    count_letters(stdin, counts);
    for (int i = 0; i < 26; i++) {
        if (counts[i] == 0) continue;
        printf("%c %d ", 'a' + i, counts[i]);
        for (int j = 0; j < counts[i]; j++) putchar('#');
        putchar('\n');
    }
    return 0;
}
```

**C++**

```cpp
// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
#include <array>
#include <cctype>
#include <iostream>
#include <string>

std::array<int, 26> count_letters(std::istream& in) {
    std::array<int, 26> counts{};
    char c;
    while (in.get(c)) {
        if (std::isalpha(static_cast<unsigned char>(c))) counts[c - 'a']++;
    }
    return counts;
}

int main() {
    std::array<int, 26> counts = count_letters(std::cin);
    for (int i = 0; i < 26; i++) {
        if (counts[i] == 0) continue;
        std::cout << char('a' + i) << ' ' << counts[i] << ' ' << std::string(counts[i], '#')
                  << '\n';
    }
}
```

**Rust**

```rust
// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
use std::io::Read;

fn count_letters(text: &str) -> [usize; 26] {
    let mut counts = [0; 26];
    for c in text.chars() {
        if c.is_alphabetic() {
            counts[c as usize - 'a' as usize] += 1;
        }
    }
    counts
}

fn main() {
    let mut text = String::new();
    std::io::stdin().read_to_string(&mut text).unwrap();
    let counts = count_letters(&text);
    for (i, &n) in counts.iter().enumerate() {
        if n > 0 {
            println!("{} {} {}", (b'a' + i as u8) as char, n, "#".repeat(n));
        }
    }
}
```

**Go**

```go
// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
package main

import (
	"fmt"
	"io"
	"os"
	"strings"
	"unicode"
)

func countLetters(text string) [26]int {
	var counts [26]int
	for _, c := range text {
		if unicode.IsLetter(c) {
			counts[c-'a']++
		}
	}
	return counts
}

func main() {
	text, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	counts := countLetters(string(text))
	for i, n := range counts {
		if n > 0 {
			fmt.Println(string(rune('a'+i)), n, strings.Repeat("#", n))
		}
	}
}
```

**Python**

```python
# Counts the letters in its input, and draws a bar for each one. It expects lowercase
# text, and doesn't check: section 20.3 gives it capitals, to see what happens.
import sys


def count_letters(text: str) -> list[int]:
    counts = [0] * 26
    for c in text:
        if c.isalpha():
            counts[ord(c) - ord("a")] += 1
    return counts


def main() -> None:
    counts = count_letters(sys.stdin.read())
    for i, n in enumerate(counts):
        if n > 0:
            print(chr(ord("a") + i), n, "#" * n)


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
import { readFileSync } from "node:fs";

function countLetters(text) {
  const counts = new Array(26).fill(0);
  for (const c of text) {
    if (/\p{L}/u.test(c)) {
      counts[c.codePointAt(0) - 97]++;
    }
  }
  return counts;
}

const counts = countLetters(readFileSync(0, "utf8"));
counts.forEach((n, i) => {
  if (n > 0) {
    console.log(`${String.fromCharCode(97 + i)} ${n} ${"#".repeat(n)}`);
  }
});
```

**TypeScript**

```ts
// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
import { readFileSync } from "node:fs";

function countLetters(text: string): number[] {
  const counts: number[] = new Array<number>(26).fill(0);
  for (const c of text) {
    if (/\p{L}/u.test(c)) {
      counts[c.codePointAt(0)! - 97]++;
    }
  }
  return counts;
}

const counts = countLetters(readFileSync(0, "utf8"));
counts.forEach((n, i) => {
  if (n > 0) {
    console.log(`${String.fromCharCode(97 + i)} ${n} ${"#".repeat(n)}`);
  }
});
```

**Dart**

```dart
// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
import 'dart:io';

final letter = RegExp(r'\p{L}', unicode: true);

List<int> countLetters(String text) {
  final counts = List.filled(26, 0);
  for (final c in text.split('')) {
    if (letter.hasMatch(c)) counts[c.codeUnitAt(0) - 'a'.codeUnitAt(0)]++;
  }
  return counts;
}

void main() {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    lines.add(line!);
  }
  final counts = countLetters(lines.join('\n'));
  for (var i = 0; i < 26; i++) {
    if (counts[i] > 0) {
      print('${String.fromCharCode(0x61 + i)} ${counts[i]} ${'#' * counts[i]}');
    }
  }
}
```

**Kotlin**

```kotlin
// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
fun countLetters(text: String): IntArray {
    val counts = IntArray(26)
    for (c in text) {
        if (c.isLetter()) counts[c - 'a']++
    }
    return counts
}

fun main() {
    val counts = countLetters(System.`in`.bufferedReader().readText())
    for (i in counts.indices) {
        if (counts[i] > 0) println("${'a' + i} ${counts[i]} ${"#".repeat(counts[i])}")
    }
}
```

On lowercase text, all nine agree:

```console
$ echo "banana, 3 times: banana banana!" | ./main
a 9 #########
b 3 ###
e 1 #
i 1 #
m 1 #
n 6 ######
s 1 #
t 1 #
```

Given `Hello, World`, with two capitals, `'H' - 'a'` is 72 − 97, which is −25, and
`'W' - 'a'` is −10, so each program uses an index outside the 26 counts. These are the
results, from the same builds as the checker's:

| Language | What `Hello, World` does | Exit code |
| --- | --- | --- |
| C, C++ | Prints the counts for `d`, `e`, `l`, `o`, and `r`, and leaves out `H` and `W`: the program added 1 to memory below the array, and nothing noticed | 0 |
| Rust | `index out of bounds: the len is 26 but the index is 18446744073709551591` | 101 |
| Go | `panic: runtime error: index out of range [-25]`, and the stack | 2 |
| Python | Prints `b 1 #` and `q 1 #` among the others: `counts[-25]` is `counts[1]`, the count for `b`, since a negative index counts from the end | 0 |
| JavaScript, TypeScript | Prints the same as C: `counts[-25]++` makes a property named `"-25"` on the array, with the value `NaN`, and the loop that prints never looks at it | 0 |
| Dart | `RangeError (length): Invalid value: Not in inclusive range 0..25: -25`, and the stack | 255 |
| Kotlin | `java.lang.ArrayIndexOutOfBoundsException: Index -25 out of bounds for length 26`, and the stack | 1 |

Five of the nine give a wrong answer and exit 0, which is the worst thing a bug can do:
nothing tells you to look. Python needs a capital further from `a` to fail, such as the
`A` in `A cat`, since −32 is out of range even counting from the end:

```console
$ echo "A cat" | python3 main.py
Traceback (most recent call last):
  File "/tmp/ch20/01-crash-reports/python/main.py", line 22, in <module>
    main()
    ~~~~^^
  File "/tmp/ch20/01-crash-reports/python/main.py", line 15, in main
    counts = count_letters(sys.stdin.read())
  File "/tmp/ch20/01-crash-reports/python/main.py", line 10, in count_letters
    counts[ord(c) - ord("a")] += 1
    ~~~~~~^^^^^^^^^^^^^^^^^^^
IndexError: list index out of range
```

A **stack trace** lists the calls that were running when the program stopped, one
**frame** for each, with the function, the file, and the line it had reached. Python's
is printed oldest first, as its first line says, "most recent call last": `<module>`
called `main`, which called `count_letters`, which failed at line 10. The `~~~~^^^^`
under the line marks the part of it that failed, since Python 3.11. The others print the
innermost call first:

```console
$ echo "Hello, World" | ./main
panic: runtime error: index out of range [-25]

goroutine 1 [running]:
main.countLetters({0xc000114d10, 0xd})
	/tmp/ch20/01-crash-reports/go/main.go:17 +0x1a9
main.main()
	/tmp/ch20/01-crash-reports/go/main.go:29 +0xb9
```

```console
$ echo "Hello, World" | java -jar main.jar
Exception in thread "main" java.lang.ArrayIndexOutOfBoundsException: Index -25 out of bounds for length 26
	at MainKt.countLetters(main.kt:6)
	at MainKt.main(main.kt:12)
	at MainKt.main(main.kt)
```

```console
$ echo "Hello, World" | dart run main.dart
Unhandled exception:
RangeError (length): Invalid value: Not in inclusive range 0..25: -25
#0      _Array.[] (dart:core-patch/array.dart)
#1      countLetters (file:///tmp/ch20/01-crash-reports/dart/main.dart:10:35)
#2      main (file:///tmp/ch20/01-crash-reports/dart/main.dart:20:18)
#3      _delayEntrypointInvocation.<anonymous closure> (dart:isolate-patch/isolate_patch.dart:313:19)
#4      _RawReceivePort._handleMessage (dart:isolate-patch/isolate_patch.dart:192:12)
```

Reading one is the same in every language. Find the top frame that is in your own code,
since frames above it are the library that noticed the problem, such as Dart's
`_Array.[]`, and frames below it are the runtime that started your program, such as
Dart's `_RawReceivePort` and Kotlin's second `MainKt.main`, the `main(String[])` that
the compiler adds for the JVM to call, which calls yours. That frame is where the
failure happened, and the frames under it are how the program got there. Go prints the
arguments of each call, as raw words, `{0xc000114d10, 0xd}` being the string's pointer
and its length, 13, and `+0x1a9` is the offset of the instruction in the function. Go
prints only the goroutine that panicked, `goroutine 1`, which is `main`'s. With the
environment variable `GOTRACEBACK=all` it prints every goroutine's stack, which a
program with many goroutines often needs: in one that had started a goroutine to sleep
before it panicked, that goroutine showed up as `goroutine 5 [sleep]:`, under the one
that panicked.

Rust prints a stack trace only when asked, with the environment variable
`RUST_BACKTRACE`:

```console
$ echo "Hello, World" | ./main

thread 'main' (801) panicked at main.rs:9:13:
index out of bounds: the len is 26 but the index is 18446744073709551591
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace
```

The index is 2^64 − 25: `c as usize - 'a' as usize` is a subtraction of unsigned
numbers, and in a release build, `rustc -O`, it wraps round. In a debug build, without
`-O`, Rust checks every integer operation for overflow, and the same input fails one
step earlier, at the subtraction itself, in column 20 instead of the indexing in column
13:

```console
$ rustc --edition 2024 main.rs && echo "Hello, World" | RUST_BACKTRACE=1 ./main

thread 'main' (822) panicked at main.rs:9:20:
attempt to subtract with overflow
stack backtrace:
   0: __rustc::rust_begin_unwind
             at /rustc/58cf8f9c35b355199727627d07ae34a4e2846d67/library/std/src/panicking.rs:679:5
   1: core::panicking::panic_fmt
             at /rustc/58cf8f9c35b355199727627d07ae34a4e2846d67/library/core/src/panicking.rs:80:14
   2: core::panicking::panic_const::panic_const_sub_overflow
             at /rustc/58cf8f9c35b355199727627d07ae34a4e2846d67/library/core/src/panicking.rs:175:17
   3: main::count_letters
   4: main::main
   5: <fn() as core::ops::function::FnOnce<()>>::call_once
note: Some details are omitted, run with `RUST_BACKTRACE=full` for a verbose backtrace.
```

The frames for `count_letters` and `main` have no line numbers, because this build has
no debugging information; `rustc -g` adds it. The number after `thread 'main'` is the
thread's ID, and changes from run to run, as section 11.6 says.

The exit code says which way a program ended, and section 11.6 has each language's. A
shell reports a program killed by a signal as 128 plus the signal's number: 134 for
`SIGABRT`, which `abort` and failed C assertions raise, 136 for `SIGFPE`, and 139 for
`SIGSEGV`. Python's `subprocess`, which the book's checker uses, reports the same as a
negative number, so the checker says `exit code -8` for a program killed by `SIGFPE`,
signal 8.

The C and C++ versions are the hard case. Nothing crashed, the output looks plausible,
and the defect, writing outside an array, is undefined behavior, so the next run, or the
next compiler, may do something else. Section 20.6 is about the tools that catch it.

---

## 20.4 Printing and logging

The oldest way to debug is to print things, and it is still the most used, because it
needs nothing but the program. Three habits make it work better. Print to standard
error, so that the program's real output stays as it was, and so that a test comparing
the output isn't confused. Print the expression as well as its value, and where it was
printed, so that a screen of numbers can be read. And print at the boundaries, such as
where a function starts and returns, to find the first wrong value by halving, as
section 20.1 says.

Most languages have a way to print a value with its source. Rust's `dbg!` macro prints
the file, line, column, the expression, and its value, to standard error, and gives the
value back, so it can go in the middle of an expression; with `{:#?}` formatting, it
prints a struct over several lines:

```rust
let d = dbg!(p.x * p.x + p.y * p.y);
dbg!(p);
```

```text
[rust/main.rs:8:13] p.x * p.x + p.y * p.y = 25
[rust/main.rs:9:5] p = Point {
    x: 3,
    y: 4,
}
```

Python's f-strings do the same with `=` after the expression, since Python 3.8:
`print(f"{p=}")` prints `p=Point(x=3, y=4)`, and `print(f"{p.x * p.x + p.y * p.y = }")`
prints `p.x * p.x + p.y * p.y = 25`. In JavaScript and TypeScript, putting the values in
an object literal labels them: `console.log({ p, sum: p.x + p.y })` prints
`{ p: Point { x: 3, y: 4 }, sum: 7 }`. In C, `__FILE__`, `__LINE__`, and `__func__` give
the place:

```c
fprintf(stderr, "%s:%d: %s: p = {x = %d, y = %d}\n", __FILE__, __LINE__, __func__, p.x, p.y);
```

```text
c/main.c:9: inner: p = {x = 3, y = 4}
```

C++20's `std::source_location` does it without a macro, and has a trick: as a default
argument, `std::source_location::current()` is evaluated where the function is called,
so a logging function that takes one reports its caller's line, not its own. A function
`inner(Point p, std::source_location here = std::source_location::current())`, called
from `outer` at line 15, printed `cpp/main.cpp:15: void outer(Point): p = {3, 4}`.

What a value prints as, with no work, differs more than anything else here:

| Language | A struct or object, printed for debugging | Without the work |
| --- | --- | --- |
| C | Only what `printf` is told, one field at a time | No way to print a struct whole |
| C++ | What its `operator<<` prints, which you write | `std::cout << p` doesn't compile: `error: no match for 'operator<<'` |
| Rust | `{:?}` with `#[derive(Debug)]`: `Point { x: 3, y: 4 }`, and `{:#?}` over several lines | ``error[E0277]: `Point` doesn't implement `Debug` `` |
| Go | `%v` prints `{3 4}`, `%+v` prints `{X:3 Y:4}`, and `%#v` prints `main.point{X:3, Y:4}`, the Go syntax for the value | Nothing to write: every value prints |
| Python | A dataclass's `repr`: `Point(x=3, y=4)` | A plain class: `<__main__.Plain object at 0x7f72344ada90>` |
| JavaScript, TypeScript | `console.log` shows the class and fields: `Point { x: 3, y: 4 }` | Nothing to write, but below two levels of nesting it prints `[Object]`: `{ a: { b: { c: [Object] } } }`, and `console.dir(value, { depth: null })` prints all of it |
| Dart | What `toString` returns, which a class overrides; a record prints `(x: 3, y: 4)` | `Instance of 'Point'` |
| Kotlin | A data class's `toString`: `Point(x=3, y=4)` | Any other class: `Plain@7f31245a`, its name and hash code |

Sometimes the question is not what a value is but how the program got here, and each
language can print the stack without crashing: `traceback.print_stack()` in Python,
`console.trace()` in JavaScript and TypeScript, `debug.PrintStack()` from
`runtime/debug` in Go, `std::backtrace::Backtrace::force_capture()` in Rust,
`print(StackTrace.current)` in Dart, `Thread.dumpStack()` in Kotlin, and
`std::stacktrace::current()` in C++23, which GCC 13 has when the program is linked with
`-lstdc++_libbacktrace`. C has none in its standard library; glibc's `backtrace` gives
addresses, and names only functions that aren't `static`, and only in a program linked
with `-rdynamic`.

### Logging

A print you add to find one bug is deleted when it's found. A **log** is a record the
program keeps on purpose, of what it did, and stays in the code: a server's log is often
the only way to find out what happened at 3 a.m. Logging libraries add four things to a
print: a **level** for each message, such as debug, info, warning, and error, so that
the less important ones can be turned off without editing the code; the time; the place,
such as the module that logged it; and a choice of where the messages go and in what
form, such as JSON lines that a program can search.

| Language | In the standard library | What most projects use |
| --- | --- | --- |
| C | Nothing; POSIX has `syslog` | A small library, or `fprintf(stderr, ...)` |
| C++ | Nothing | spdlog |
| Rust | Nothing | The `log` crate, an interface that libraries log through, with `env_logger` or another crate to print the messages; or `tracing` |
| Go | `log`, and `log/slog`, for structured logs, since Go 1.21 | `log/slog` |
| Python | `logging` | `logging` |
| JavaScript, TypeScript | `console.debug`, `console.info`, `console.warn`, and `console.error`, which have no levels to turn off | pino, or winston |
| Dart | `log` in `dart:developer`, which sends messages to DevTools, not to the terminal | `package:logging`, from the Dart team |
| Kotlin | The JVM's `java.util.logging` and `System.Logger` | SLF4J, with Logback behind it |

Go's `log` adds the date and time, and `slog` adds a level and keys with values, as text
or as JSON:

```go
log.Printf("p = %+v", p)
slog.Info("moved", "x", p.X, "y", p.Y)
slog.New(slog.NewJSONHandler(os.Stderr, nil)).Info("moved", "x", p.X, "y", p.Y)
```

```text
2026/11/23 06:38:30 p = {X:3 Y:4}
2026/11/23 06:38:30 INFO moved x=3 y=4
{"time":"2026-11-23T06:38:30.45735457Z","level":"INFO","msg":"moved","x":3,"y":4}
```

Python's `logging` prints only warnings and worse until it is told otherwise:
`logging.debug("debug")`, `logging.info("info")`, and `logging.warning("warning")` print
one line, `WARNING:root:warning`. After `logging.basicConfig(level=logging.DEBUG)`, all
three print. A module logs through its own logger, `logging.getLogger(__name__)`, so
that its messages can be turned up or down apart from the rest:
`logging.getLogger("bank").warning("refused %d", 5)` prints `WARNING:bank:refused 5`.
The arguments are passed separately, not formatted into the message first, so that a
message at a level that is turned off costs almost nothing.

---

## 20.5 Debuggers

A **debugger** runs a program under its control, so that it can stop the program, look
at its memory, and start it again. Its main tools are few:

- A **breakpoint** stops the program when it reaches a line or a function. A
  **conditional breakpoint** stops only when an expression is true, such as
  `count == 0`, which saves stopping a thousand times to find the one time that matters.
- **Stepping** runs one line at a time: **step over** (`next`) runs the line, calls and
  all, **step into** (`step`) goes into the function the line calls, and **step out**
  (`finish`, `return`, or `out`) runs to the end of the current function and stops in
  its caller.
- The **call stack**, `bt` or `where`, shows the frames, as a stack trace does, and `up`
  and `down` move between them, to look at the variables of the function that made the
  call.
- **Printing** shows a variable, or evaluates an expression in the program's own
  language.
- A **watchpoint** stops the program whenever a variable changes, wherever the change is
  made.

Example 02 is a recursive descent parser, an evaluator of arithmetic with `+`, `-`, `*`,
and parentheses, with one function for each rule of its grammar. The rules call each
other, so the call stack grows as the expression nests, which makes it a good program to
watch:

**Python**

```python
# Evaluates arithmetic with +, -, *, and parentheses: each argument, or each line of
# input if there are none. Section 20.5 runs it in a debugger, to watch the parser
# call itself. It expects well-formed expressions, and doesn't check.
import sys


class Parser:
    """A recursive descent parser, with one method for each rule of the grammar."""

    def __init__(self, text: str) -> None:
        self.text = text
        self.pos = 0

    def peek(self) -> str:
        """Skips spaces, and gives the next character, or "" at the end."""
        while self.pos < len(self.text) and self.text[self.pos] == " ":
            self.pos += 1
        return self.text[self.pos] if self.pos < len(self.text) else ""

    # expr = term { ("+" | "-") term }
    def expr(self) -> int:
        value = self.term()
        while self.peek() in ("+", "-"):
            op = self.text[self.pos]
            self.pos += 1
            right = self.term()
            value = value + right if op == "+" else value - right
        return value

    # term = factor { "*" factor }
    def term(self) -> int:
        value = self.factor()
        while self.peek() == "*":
            self.pos += 1
            value *= self.factor()
        return value

    # factor = number | "(" expr ")"
    def factor(self) -> int:
        if self.peek() == "(":
            self.pos += 1
            value = self.expr()
            self.peek()
            self.pos += 1  # the ")"
            return value
        start = self.pos
        while self.pos < len(self.text) and self.text[self.pos].isdigit():
            self.pos += 1
        return int(self.text[start : self.pos])


def evaluate(text: str) -> int:
    return Parser(text).expr()


def main() -> None:
    for text in sys.argv[1:] or sys.stdin.read().splitlines():
        print(evaluate(text))


if __name__ == "__main__":
    main()
```

The other eight are the same, with a parser struct or class and a function or method for
each rule. Each takes its expressions as arguments, so that the debugger can have the
terminal, and the session below runs it on `2 * (3 + 4)`. It stops at `factor` each time
it is called, three times, until it reaches the `3` inside the parentheses, and then
looks around.

### gdb: C, C++, Rust, and Go

gdb needs the program built with debugging information, `-g`, and without optimization,
`-O0`, for reasons that come at the end of this section:

```console
$ cc -std=c17 -g -O0 main.c -o calc
$ gdb -q --args ./calc "2 * (3 + 4)"
(gdb) break factor
Breakpoint 1 at 0x122b: file main.c, line 24.
(gdb) run

Breakpoint 1, factor (p=0x7fffffffa2f0) at main.c:24
24	    if (peek(p) == '(') {
(gdb) continue

Breakpoint 1, factor (p=0x7fffffffa2f0) at main.c:24
24	    if (peek(p) == '(') {
(gdb) continue

Breakpoint 1, factor (p=0x7fffffffa2f0) at main.c:24
24	    if (peek(p) == '(') {
(gdb) bt
#0  factor (p=0x7fffffffa2f0) at main.c:24
#1  0x0000555555555339 in term (p=0x7fffffffa2f0) at main.c:41
#2  0x000055555555539d in expr (p=0x7fffffffa2f0) at main.c:51
#3  0x000055555555525b in factor (p=0x7fffffffa2f0) at main.c:26
#4  0x000055555555535f in term (p=0x7fffffffa2f0) at main.c:44
#5  0x000055555555539d in expr (p=0x7fffffffa2f0) at main.c:51
#6  0x000055555555545f in evaluate (text=0x7fffffffbcb6 "2 * (3 + 4)") at main.c:63
#7  0x00005555555554da in main (argc=2, argv=0x7fffffffb478) at main.c:67
(gdb) print *p
$1 = {text = 0x7fffffffbcb6 "2 * (3 + 4)", pos = 5}
(gdb) finish
0x0000555555555339 in term (p=0x7fffffffa2f0) at main.c:41
41	    long long value = factor(p);
Value returned is $2 = 3
(gdb) delete
(gdb) continue
14
[Inferior 1 (process 1173) exited normally]
```

`break factor` stops at the first line of `factor`. The first stop is for the `2`, the
second for the `(`, and the third, inside the parentheses, for the `3`. The backtrace
shows the recursion, read from the top: `factor` was called by `term`, which was called
by `expr`, which was called by the `factor` that read the `(`, at line 26, and so on
down to `main`. `print *p` shows the whole parser struct, with `pos` at 5, the index of
the `3`, and `finish` runs `factor` to its end and shows what it returned. `delete`
removes every breakpoint, and the last `continue` runs the program to its end, where it
prints 14.

C++, Rust, and Go use the same gdb. In C++, the backtrace names the methods,
`Parser::factor (this=0x7fffffffb2b0) at main.cpp:44`, and `print *this` shows
`{text_ = "2 * (3 + 4)", pos_ = 5}`. Rust's toolchain has `rust-gdb`, a wrapper that
loads scripts to print Rust's types, so `print *self` shows the struct:

```text
$1 = main::Parser {text: &[u8](size=11) = {50, 32, 42, 32, 40, 51, 32, 43, 32, 52, 41}, pos: 5}
```

The Rust parser keeps its text as bytes, and gdb prints them as numbers, 50 for `'2'`
and 32 for a space. Go needs its build to turn off both optimization and inlining, with
`go build -gcflags=all="-N -l"`, and its methods have names like
`main.(*parser).factor`. gdb doesn't understand Go well: it printed
`main.(*parser).factor (p=0xc000194e20, ~r0=5711648)`, where `~r0` is the result, which
hasn't been set yet, and the Go documentation says that Delve "understands the Go
runtime, data structures, and expressions better than GDB." Delve, started with
`dlv debug`, has commands much like gdb's, and is what Go's editors use.

### pdb: Python

pdb is in Python's standard library, and `python3 -m pdb main.py ARGS` starts the
program stopped at its first line. Its commands are gdb's, mostly, with `where` for the
stack and `return` for step out. This is the same session:

```console
$ python3 -m pdb main.py "2 * (3 + 4)"
> /tmp/ch20/02-debugger/python/main.py(4)<module>()
-> import sys
(Pdb) break 40
Breakpoint 1 at /tmp/ch20/02-debugger/python/main.py:40
(Pdb) continue
> /tmp/ch20/02-debugger/python/main.py(40)factor()
-> if self.peek() == "(":
(Pdb) continue
> /tmp/ch20/02-debugger/python/main.py(40)factor()
-> if self.peek() == "(":
(Pdb) continue
> /tmp/ch20/02-debugger/python/main.py(40)factor()
-> if self.peek() == "(":
(Pdb) where
  /usr/lib/python3.13/bdb.py(680)run()
-> exec(cmd, globals, locals)
  <string>(1)<module>()
  /tmp/ch20/02-debugger/python/main.py(62)<module>()
-> main()
  /tmp/ch20/02-debugger/python/main.py(58)main()
-> print(evaluate(text))
  /tmp/ch20/02-debugger/python/main.py(53)evaluate()
-> return Parser(text).expr()
  /tmp/ch20/02-debugger/python/main.py(22)expr()
-> value = self.term()
  /tmp/ch20/02-debugger/python/main.py(35)term()
-> value *= self.factor()
  /tmp/ch20/02-debugger/python/main.py(42)factor()
-> value = self.expr()
  /tmp/ch20/02-debugger/python/main.py(22)expr()
-> value = self.term()
  /tmp/ch20/02-debugger/python/main.py(32)term()
-> value = self.factor()
> /tmp/ch20/02-debugger/python/main.py(40)factor()
-> if self.peek() == "(":
(Pdb) p self.pos, self.text[self.pos:]
(5, '3 + 4)')
(Pdb) return
--Return--
> /tmp/ch20/02-debugger/python/main.py(49)factor()->3
-> return int(self.text[start : self.pos])
(Pdb) clear 1
Deleted breakpoint 1 at /tmp/ch20/02-debugger/python/main.py:40
(Pdb) continue
14
The program finished and will be restarted
> /tmp/ch20/02-debugger/python/main.py(4)<module>()
-> import sys
(Pdb) quit
```

The stack is printed oldest first, as in a traceback, with an arrow, `>`, at the current
frame, and its first two frames are pdb's own, which started the program. `p` takes any
Python expression, so `self.text[self.pos:]` shows the rest of the input, `'3 + 4)'`.
After `return`, pdb shows `--Return--` and the value, `factor()->3`. When the program
ends, pdb starts it again, which is what `The program finished and will be restarted`
means.

A program can also stop itself: a call to `breakpoint()`, since Python 3.7, starts pdb
at that line, with no change to how the program is run. And `python3 -m pdb` on a
program that crashes stops at the exception, `Entering post mortem debugging`, with the
program as it was, so that `up` and `p` show the variables that led to it; exercise 20.1
uses it.

### node inspect: JavaScript and TypeScript

Node has a debugger built in, `node inspect`, and the same protocol lets Chrome's
DevTools or an editor connect to a program started with `node --inspect`. Its
breakpoints are set with `sb(LINE)`, its stack is `bt`, and `exec` evaluates an
expression:

```console
$ node inspect main.js "2 * (3 + 4)"
< Debugger listening on ws://127.0.0.1:9229/f05439d6-a02a-41eb-aaac-a6c964717925
< For help, see: https://nodejs.org/en/docs/inspector
<
connecting to 127.0.0.1:9229 ... ok
< Debugger attached.
<
Break on start in main.js:1
> 1 // Evaluates arithmetic with +, -, *, and parentheses: each argument, or each line of
  2 // input if there are none. Section 20.5 runs it in a debugger, to watch the parser
  3 // call itself. It expects well-formed expressions, and doesn't check.
debug> sb(42)
 37     return value;
 38   }
 39
 40   // factor = number | "(" expr ")"
 41   factor() {
>42     if (this.peek() === "(") {
 43       this.pos++;
 44       const value = this.expr();
 45       this.peek();
 46       this.pos++; // the ")"
 47       return value;
debug> cont
break in main.js:42
 40   // factor = number | "(" expr ")"
 41   factor() {
>42     if (this.peek() === "(") {
 43       this.pos++;
 44       const value = this.expr();
debug> cont
break in main.js:42
 40   // factor = number | "(" expr ")"
 41   factor() {
>42     if (this.peek() === "(") {
 43       this.pos++;
 44       const value = this.expr();
debug> cont
break in main.js:42
 40   // factor = number | "(" expr ")"
 41   factor() {
>42     if (this.peek() === "(") {
 43       this.pos++;
 44       const value = this.expr();
debug> bt
#0 factor main.js:42:13
#1 term main.js:32:21
#2 expr main.js:21:21
#3 factor main.js:44:25
#4 term main.js:35:20
#5 expr main.js:21:21
#6 evaluate main.js:56:26
#7 (anonymous) main.js:63:14
#8 run node:internal/modules/esm/module_job:343:24
debug> exec [this.pos, this.text.slice(this.pos)]
[ 5, '3 + 4)' ]
debug> out
step in main.js:33
 31   term() {
 32     let value = this.factor();
>33     while (this.peek() === "*") {
 34       this.pos++;
 35       value *= this.factor();
debug> clearBreakpoint("main.js", 42)
debug> cont
< 14
<
< Waiting for the debugger to disconnect...
<
debug> .exit
```

The stack shows the column as well as the line, and at the bottom, `(anonymous)` is the
module's top level, and `run` is Node's loader. Node runs TypeScript by stripping its
types, so `node inspect main.ts` debugs the TypeScript itself, with its own line
numbers, as exercise 20.1 does.

### jdb: Kotlin

The JVM has a command-line debugger too, jdb, though most people debug Kotlin in
IntelliJ, which uses the same protocol, JDWP. Breakpoints are set by class and line,
`stop at Parser:38`, before the class is even loaded:

```console
$ kotlinc main.kt -include-runtime -d main.jar
$ jdb -classpath main.jar MainKt "2 * (3 + 4)"
Initializing jdb ...
> stop at Parser:38
Deferring breakpoint Parser:38.
It will be set after the class is loaded.
> run
run MainKt "2 * (3 + 4)"
Set uncaught java.lang.Throwable
Set deferred uncaught java.lang.Throwable
>
VM Started: Set deferred breakpoint Parser:38

Breakpoint hit: "thread=main", Parser.factor(), line=38 bci=0

main[1] cont
>
Breakpoint hit: "thread=main", Parser.factor(), line=38 bci=0

main[1] cont
>
Breakpoint hit: "thread=main", Parser.factor(), line=38 bci=0

main[1] where
  [1] Parser.factor (main.kt:38)
  [2] Parser.term (main.kt:28)
  [3] Parser.expr (main.kt:17)
  [4] Parser.factor (main.kt:40)
  [5] Parser.term (main.kt:31)
  [6] Parser.expr (main.kt:17)
  [7] MainKt.evaluate (main.kt:51)
  [8] MainKt.main (main.kt:54)
main[1] print pos
 pos = 5
main[1] print text.substring(pos)
 text.substring(pos) = "3 + 4)"
main[1] step up
>
Step completed: "thread=main", Parser.term(), line=28 bci=4

main[1] clear Parser:38
Removed: breakpoint Parser:38
main[1] cont
> 14

The application exited
```

Kotlin's top-level functions are in a class named after the file, `MainKt`, so that is
the class jdb runs. `print` takes Java's expressions, not Kotlin's, so
`text.substring(pos)` works, since a Kotlin `String` is a Java one, but Kotlin's own
`text.drop(pos)` would not. `step up` is jdb's step out.

### Dart: DevTools

Dart has no command-line debugger. `dart run --observe` starts the Dart VM service,
which DevTools, a debugger and profiler that runs in a browser, connects to, and
`--pause-isolates-on-start` holds the program until it does:

```console
$ dart run --observe --pause-isolates-on-start main.dart "2 * (3 + 4)"
The Dart VM service is listening on http://127.0.0.1:8181/TOKEN/
The Dart DevTools debugger and profiler is available at: http://127.0.0.1:8181/TOKEN/devtools/?uri=ws://127.0.0.1:8181/TOKEN/ws
vm-service: isolate(...) 'main' has no debugger attached and is paused at start.  Connect to the Dart VM service at http://127.0.0.1:8181/TOKEN/ to debug.
```

The `TOKEN` is a random string that changes each run, so that only someone who has the
link can connect. DevTools has the same breakpoints, stepping, and stack as the others,
with buttons; so does the Dart extension for VS Code, which starts the VM service
itself.

### Commands

| | gdb | pdb | node inspect | jdb |
| --- | --- | --- | --- | --- |
| Start | `gdb --args ./main ARGS`, then `run` | `python3 -m pdb main.py ARGS` | `node inspect main.js ARGS` | `jdb -classpath main.jar MainKt ARGS`, then `run` |
| Break at a line | `break main.c:24` | `break 40` | `sb(42)` | `stop at Parser:38` |
| Break at a function | `break factor` | `break Parser.factor`, once the class is defined | None; put a `debugger;` statement in it | `stop in Parser.factor` |
| Carry on | `continue` | `continue` | `cont` | `cont` |
| Step over, into, out | `next`, `step`, `finish` | `next`, `step`, `return` | `next`, `step`, `out` | `next`, `step`, `step up` |
| Call stack, and moving in it | `bt`, `up`, `down` | `where`, `up`, `down` | `bt` | `where`, `up`, `down` |
| Print | `print *p` | `p self.pos` | `exec this.pos` | `print pos` |
| Remove a breakpoint | `delete`, or `clear factor` | `clear 1` | `clearBreakpoint("main.js", 42)` | `clear Parser:38`, or `clear Parser.factor` |

### Debug builds, and what optimization does to them

A debugger maps each machine instruction back to a line, and each variable to where it
lives. An optimizing compiler breaks both maps: it keeps variables in registers, or
nowhere at all, it reorders and merges lines, and it **inlines** small functions,
copying their code into their callers so that the call disappears. The same session on a
build with `-O2` shows each of these:

```console
$ cc -std=c17 -g -O2 main.c -o calc
$ gdb -q --args ./calc "2 * (3 + 4)"
(gdb) break factor
Breakpoint 1 at 0x1420: file main.c, line 18.
(gdb) run

Breakpoint 1, factor (p=p@entry=0x7fffffffa310) at main.c:18
18	    while (p->text[p->pos] == ' ') p->pos++;
(gdb) continue

Breakpoint 1, factor (p=p@entry=0x7fffffffa310) at main.c:18
18	    while (p->text[p->pos] == ' ') p->pos++;
(gdb) continue

Breakpoint 1, factor (p=p@entry=0x7fffffffa310) at main.c:18
18	    while (p->text[p->pos] == ' ') p->pos++;
(gdb) bt
#0  factor (p=p@entry=0x7fffffffa310) at main.c:18
#1  0x0000555555555300 in term (p=0x7fffffffa310) at main.c:41
#2  expr (p=p@entry=0x7fffffffa310) at main.c:51
#3  0x00005555555554c0 in factor (p=p@entry=0x7fffffffa310) at main.c:26
#4  0x0000555555555353 in term (p=0x7fffffffa310) at main.c:44
#5  expr (p=p@entry=0x7fffffffa310) at main.c:51
#6  0x0000555555555164 in evaluate (text=<optimized out>) at main.c:63
#7  main (argc=<optimized out>, argv=<optimized out>) at main.c:67
(gdb) print *p
$1 = {text = 0x7fffffffbcb5 "2 * (3 + 4)", pos = 5}
(gdb) print value
$2 = <optimized out>
(gdb) finish
term (p=0x7fffffffa310) at main.c:42
42	    while (peek(p) == '*') {
Value returned is $3 = 3
```

The breakpoint at `factor` stops at line 18, which is in `peek`, since `peek` has been
inlined into `factor`, and its first line is now the first thing `factor` does. Frames
`#2`, `#5`, and `#7` have no address, because each shares one real frame with the frame
above it, a function that was inlined into it: `info frame` on frame 1 says
`inlined into frame 2`, since `term` was inlined into `expr`, and on frame 6,
`inlined into frame 7`, since `evaluate` was inlined into `main`. `evaluate`'s argument
and the local `value` are `<optimized out>`, and `finish` returns to line 42 instead of
41, because the compiler moved the code around. None of this is a bug in the compiler,
which promises only that the program does the same; that is why debugging is done on a
build with `-O0`, and why Go's needs `-N -l`. Rust's `cargo build` makes that build by
default, and CMake's `Debug` configuration makes it for C and C++. The VM languages have
no such problem when a debugger is attached, since their VMs drop the optimized code of
a function when a breakpoint is set in it.

---

## 20.6 Sanitizers and checks

The C and C++ versions of example 01 printed a wrong answer, and exited 0. A
**sanitizer** is a compiler option that adds checks to a program as it is built, so that
a bug like this one stops the program at the line that caused it. GCC and Clang have
several: **AddressSanitizer**, `-fsanitize=address`, checks every memory access against
the bounds of what was allocated, on the heap and on the stack;
**UndefinedBehaviorSanitizer**, `-fsanitize=undefined`, checks for overflow of signed
integers, bad shifts, division by zero, and other undefined behavior; and
**ThreadSanitizer**, `-fsanitize=thread`, finds data races, as in section 16.4. Chapter
10 used AddressSanitizer and Valgrind to find leaks and use-after-free bugs in section
10.10. On example 01, AddressSanitizer finds the bad index at once:

```console
$ cc -std=c17 -g -fsanitize=address main.c -o main && echo "Hello, World" | ./main
=================================================================
==847==ERROR: AddressSanitizer: stack-buffer-underflow on address 0x7fda1af00008 at pc 0x559b8b3733cb bp 0x7ffdc8f2f630 sp 0x7ffdc8f2f620
READ of size 4 at 0x7fda1af00008 thread T0
    #0 0x559b8b3733ca in count_letters /tmp/ch20/01-crash-reports/c/main.c:9
    #1 0x559b8b373550 in main /tmp/ch20/01-crash-reports/c/main.c:15
    ...

Address 0x7fda1af00008 is located in stack of thread T0 at offset 8 in frame
    #0 0x559b8b3733fe in main /tmp/ch20/01-crash-reports/c/main.c:13

  This frame has 1 object(s):
    [48, 152) 'counts' (line 14)
```

Read it from the top. The program read 4 bytes, an `int`, at an address in `main`'s
stack frame, 8 bytes into it, and the only object in that frame is `counts`, which runs
from offset 48 to 152, 26 `int`s. So the read was 40 bytes below the start of `counts`,
which is 10 `int`s: the `W`. An **underflow** is an access below the start of an object,
and an **overflow** one past its end. The report goes on to show the memory around the
address, which ASan marks as either usable or a red zone it put between objects, and
that is how it noticed.

The `H` came first, and ASan didn't catch it: −25 `int`s is 100 bytes below `counts`,
past the red zone that ASan puts round it and into memory that belongs to something
else, so it looks like an ordinary access. Given `Hello` alone, the ASan build printed
`e 1 #`, `l 2 ##`, and `o 1 #`, and exited 0. A sanitizer catches what its checks cover,
and an access far enough out of bounds can land somewhere they allow.

The other tools don't all find it, and why is worth knowing:

| Tool | On example 01 | Why |
| --- | --- | --- |
| C, `-fsanitize=address` | `stack-buffer-underflow` at `c/main.c:9` | ASan puts red zones round arrays on the stack as well as the heap |
| C, `-fsanitize=undefined` | Nothing | It checks an index against an array's bounds only where it knows them, and in `count_letters`, `counts` is a pointer |
| C, Valgrind | Nothing | Valgrind tracks memory that `malloc` gave out, and the stack is one block to it, so an access below `counts` is still inside it |
| C++, `-D_GLIBCXX_ASSERTIONS` | `/usr/include/c++/13/array:202: ... Assertion '__n < this->size()' failed.`, exit 134 | It turns on checks in GCC's standard library, such as `std::array`'s `operator[]` |
| C++, `-fsanitize=address` | `stack-buffer-underflow` at `cpp/main.cpp:12` | The same as C |
| Rust, debug build | `attempt to subtract with overflow` | Debug builds check arithmetic; release builds check only indexes |
| TypeScript, `noUncheckedIndexedAccess` | `main.ts(9,7): error TS2532: Object is possibly 'undefined'.` | With it, `counts[i]` has the type `number \| undefined`, so `counts[i]++` doesn't compile until the code checks |
| Python, JavaScript | Nothing | A negative index is allowed in Python, and any index in JavaScript |

So no tool finds everything, and the way to use them is to run the tests under each one
that applies: AddressSanitizer and UndefinedBehaviorSanitizer together for C and C++,
ThreadSanitizer for code with threads, Go's `-race` for goroutines, Rust's debug build
for overflow, and Valgrind for a program that can't be rebuilt. Sanitizers cost time:
AddressSanitizer and UndefinedBehaviorSanitizer together made example 03's C version
about seven times slower, 4.0 seconds instead of 0.5 to 0.65, which is fine for tests
and too slow for production. ASan also reports leaks when the program ends: with the
line that frees the words removed from example 03's C version, it printed
`ERROR: LeakSanitizer: detected memory leaks`,
`Direct leak of 16 byte(s) in 4 object(s) allocated from:`, and the stack of the
`malloc` in `copy`, and so did Valgrind, `16 bytes in 4 blocks are definitely lost`.

The VM languages have fewer checks to turn on, since they check memory always, but a few
are worth knowing:

- **Python's development mode**, `python3 -X dev`, turns on warnings that are off by
  default. A function that returns `open("/etc/hostname").read()` runs quietly, but in
  development mode prints a `ResourceWarning: unclosed file`, with the file's name and
  the line that opened it, since the file was left for the garbage collector to close.
- **Assertions** that are off by default: Dart's run only with
  `dart run --enable-asserts`, which then fails with
  `Failed assertion: line 2 pos 10: '1 > 2': never`, and Kotlin's only with `java -ea`.
  Section 11.6 has the rest.
- **`console.assert`** in JavaScript doesn't throw: `console.assert(1 > 2, "never")`
  prints `Assertion failed: never`, and the program carries on and exits 0.
  `node:assert` throws.
- **Go's race detector** found the data race in a program where two goroutines each
  added 1 to a counter a thousand times, though both runs without `-race` printed the
  right answer, 2000. It reports `WARNING: DATA RACE`, with the line of each access, and
  exits with status 66.

---

## 20.7 Profiling

Example 03 counts the words in its input, and prints the ten most common. It keeps its
counts in a list of pairs, and looks each word up by going through the list, which is
fine for a sentence:

**Python**

```python
# Counts the words in its input, and prints the ten most common. It is slow on a long
# text, and section 20.7 runs it under a profiler to find out why.
import sys


def split_words(text: str) -> list[str]:
    """Gives the words of the text: runs of the letters a to z, in lowercase."""
    words = []
    word = ""
    for c in text.lower():
        if "a" <= c <= "z":
            word += c
        elif word:
            words.append(word)
            word = ""
    if word:
        words.append(word)
    return words


def find(counts: list[tuple[str, int]], word: str) -> int:
    """Gives the index of the word's count, or -1 if it has none."""
    for i, (w, _) in enumerate(counts):
        if w == word:
            return i
    return -1


def count_words(words: list[str]) -> list[tuple[str, int]]:
    counts: list[tuple[str, int]] = []
    for word in words:
        i = find(counts, word)
        if i == -1:
            counts.append((word, 1))
        else:
            counts[i] = (word, counts[i][1] + 1)
    return counts


def main() -> None:
    counts = count_words(split_words(sys.stdin.read()))
    counts.sort(key=lambda pair: (-pair[1], pair[0]))
    for word, n in counts[:10]:
        print(n, word)


if __name__ == "__main__":
    main()
```

On a longer text, it is slow. This text is 100,000 words, 7,500 of them different:

```sh
awk 'BEGIN {
  for (i = 0; i < 100000; i++) {
    n = int((i * 7919 % 10000) ^ 2 / 10000); w = ""
    do { w = w sprintf("%c", 97 + n % 26); n = int(n / 26) } while (n > 0)
    printf "%s%s", w, (i % 10 == 9 ? "\n" : " ")
  }
}' > big.txt
```

| Language | Time on `big.txt` |
| --- | --- |
| C | 0.59 s |
| C++ | 0.74 s |
| Rust | 0.62 s |
| Go | 0.96 s |
| Python | 13.1 s |
| JavaScript | 1.9 s |
| TypeScript | 1.9 s |
| Dart | 1.8 s |
| Kotlin | 1.2 s |

These are from one run of each, built as the checker builds them, and all nine print the
same ten words. Guessing where the time goes is easy here, since the program is short,
but in a real program a guess is usually wrong, and a profiler shows it.

### Two kinds of profiler

An **instrumenting** profiler records every call: it adds code at the start and end of
each function, or the language's runtime calls it there, and counts the calls and times
them exactly. Python's cProfile works this way, and so does Valgrind's Callgrind, which
runs the program on a simulated processor and counts every instruction. A **sampling**
profiler stops the program many times a second, records the stack of each thread, and
lets it go on: a function that was on top of the stack in 40% of the samples took about
40% of the time. Go's pprof, Node's `--prof`, Java Flight Recorder, DevTools, and
Linux's perf are sampling profilers. Sampling costs little, and runs the program at
nearly its normal speed, but it is a statistic, and misses functions that are too short
to be caught often; instrumenting counts everything, but its cost for each call makes
programs with many small calls look slower than they are.

Both report two times for each function. **Self time**, which some profilers call
**flat** or **exclusive** time, is time spent in the function's own code. **Total
time**, also called **cumulative** or **inclusive** time, is that plus the time in every
function it called. A function with a large total and a small self time is one to look
inside; a function with a large self time is where the work is. Exercise 20.3 is about
working these out from a trace.

### Python: cProfile

```console
$ python3 -m cProfile -s tottime main.py < big.txt
         215021 function calls in 12.466 seconds

   Ordered by: internal time

   ncalls  tottime  percall  cumtime  percall filename:lineno(function)
   100000   12.352    0.000   12.352    0.000 main.py:21(find)
        1    0.058    0.058   12.411   12.411 main.py:29(count_words)
        1    0.037    0.037    0.048    0.048 main.py:6(split_words)
   107500    0.013    0.000    0.013    0.000 {method 'append' of 'list' objects}
        1    0.004    0.004    0.005    0.005 {method 'sort' of 'list' objects}
     7500    0.001    0.000    0.001    0.000 main.py:42(<lambda>)
        1    0.001    0.001   12.466   12.466 main.py:40(main)
```

`tottime` is self time and `cumtime` is total, and `-s tottime` sorts by self time.
`find` was called 100,000 times, once for each word, and took 12.352 of the 12.466
seconds. Each call goes through the list of words found so far, which grows to 7,500, so
the program makes hundreds of millions of string comparisons, and the time grows with
the square of the text. Callgrind, below, counted 320 million calls to C's `strcmp`.

### Go: pprof

Go's profiler is in the standard library, and a program turns it on itself. Example 03's
Go version takes a `-cpuprofile` flag, with the dozen lines that the `runtime/pprof`
documentation suggests, and `go tool pprof` reads the file it writes:

```console
$ go build -o wc main.go
$ ./wc -cpuprofile cpu.prof < big.txt > /dev/null
$ go tool pprof -top wc cpu.prof
File: wc
Build ID: fa780fe879369939e54a4446717594d0f136ba88
Type: cpu
Time: 2026-11-23 05:53:12 UTC
Duration: 1.10s, Total samples = 940ms (85.12%)
Showing nodes accounting for 940ms, 100% of 940ms total
      flat  flat%   sum%        cum   cum%
     450ms 47.87% 47.87%      450ms 47.87%  memeqbody
     250ms 26.60% 74.47%      790ms 84.04%  main.find (inline)
     110ms 11.70% 86.17%      900ms 95.74%  main.countWords (inline)
      90ms  9.57% 95.74%       90ms  9.57%  runtime.memequal
      20ms  2.13% 97.87%       20ms  2.13%  runtime.memmove
```

`flat` is self time and `cum` total. The top line isn't the program's code at all:
`memeqbody` is the runtime's function for comparing memory, which is how Go compares two
strings. The `cum` column says who called it: `main.find`, with 84% of the time below
it. pprof marks `find` as `(inline)`: the compiler copied it into `countWords`, but Go
keeps enough information to put the time back where it belongs.
`go tool pprof -http=:8080 wc cpu.prof` shows the same profile in a browser, as a call
graph or a flame graph.

### C, C++, and Rust: Callgrind

For machine code, Valgrind's Callgrind counts instructions, which take the place of
time. The program runs much slower under it, 67 seconds instead of 0.8 here, so it suits
a short run, but the count is exact, and nearly the same every time:

```console
$ cc -std=c17 -O2 -g main.c -o wc
$ valgrind --tool=callgrind --callgrind-out-file=cg.out ./wc < big.txt > /dev/null
$ callgrind_annotate cg.out
...
10,445,135,543 (100.0%)  PROGRAM TOTALS

Ir                      file:function
6,900,501,138 (66.06%)  ./string/../sysdeps/x86_64/multiarch/strcmp-avx2.S:__strcmp_avx2 [/usr/lib/x86_64-linux-gnu/libc.so.6]
2,884,259,943 (27.61%)  main.c:count_words [/tmp/ch20/03-profiling/c/wc]
  640,016,196 ( 6.13%)  ???:0x0000000000109140 [???]
```

`Ir` is instructions read, and this is self time: `strcmp`, from the C library, did 66%
of the work, and `count_words` 28%. `find` and `add` aren't there at all: at `-O2`, GCC
inlined both into `count_words`, and Callgrind counts by function, without splitting an
inlined function's instructions out of its caller the way pprof does. The `???` line is
the small stub through which the program calls `strcmp` in the shared library. With the
source available, `callgrind_annotate` also shows the count for each line, which finds
the loop even when the function is gone:

```text
1,279,838,637 (12.25%)      for (size_t i = 0; i < counts->length; i++) {
1,599,635,800 (15.31%)          if (strcmp(counts->items[i].word, word) == 0) return (long)i;
```

`callgrind_annotate --inclusive=yes cg.out` shows total time instead, which puts
`count_words` at 99.92%. C++ and Rust are much the same: in C++, `memcmp` had 60.58%,
and in Rust `memcmp` had 64.08% and the rest was all in `main::main`, since Rust inlined
`find`, `count_words`, and the rest into `main`. KCachegrind and QCachegrind draw
Callgrind's files as call graphs.

The older profiler for C, gprof, is a poor fit for this program. Built with `-pg`, it
reported `_init`, which the program never calls itself, at 60%, and `count_words` at
40%, and its samples added up to 0.20 seconds of a run that took about 0.6: it samples
only the program's own code, so the time in `strcmp`, in the C library, is missing, and
what is left is misleading. Linux's perf, a sampling profiler that sees every function,
the kernel's too, is the usual choice today where it is available.

### JavaScript, Kotlin, and Dart

Node's `--prof` writes V8's samples to a log, and `--prof-process` summarizes it:

```console
$ node --prof main.js < big.txt > /dev/null
$ node --prof-process isolate*.log
 [JavaScript]:
   ticks  total  nonlib   name
   1156   66.8%   67.2%  JS: *countWords file:///tmp/ch20/03-profiling/javascript/main.js:18:20
    397   22.9%   23.1%  Builtin: StringEqual
     65    3.8%    3.8%  JS: *find file:///tmp/ch20/03-profiling/javascript/main.js:11:14
```

A `*` means V8 compiled the function with its optimizing compiler, which inlined most of
`find` into `countWords`, and `StringEqual` is V8's string comparison. `node --cpu-prof`
writes a `.cpuprofile` file instead, which Chrome's DevTools opens as a flame chart.

The JVM has Java Flight Recorder built in, which samples with little cost, and the JDK's
`jfr` tool reads its recordings:

```console
$ java -XX:StartFlightRecording=filename=rec.jfr -jar main.jar < big.txt > /dev/null
$ jfr view hot-methods rec.jfr
                                          Java Methods that Executes the Most

Method                                                                                                  Samples Percent
------------------------------------------------------------------------------------------------------- ------- -------
MainKt.find(List, String)                                                                                    42  93.33%
kotlin.comparisons.ComparisonsKt__ComparisonsKt.compareValuesByImpl$ComparisonsKt__ComparisonsKt(...)         1   2.22%
```

Dart's profiler is in DevTools: run the program with
`dart run --observe --pause-isolates-on-exit`, so that its samples are still there when
it finishes, and open the CPU Profiler. Reading the same samples from the VM service
gave `==`, Dart's string comparison, 45.5% of the samples as self time, and `find` 22.6%
self and 68.2% total.

### What the profiles agree on

Every profiler found the same thing, the lookup in `find`, but they showed it in
different ways, and three lessons come from the differences.

- **The top line is often a library's.** `strcmp`, `memcmp`, `memeqbody`, `StringEqual`,
  and Dart's `==` are each language's way of comparing strings. They aren't slow; in C,
  Callgrind counted 320 million calls to `strcmp`. The question is always who called
  them, which the total time answers.
- **Inlining moves time.** C, Rust, and JavaScript put most of `find`'s time in its
  caller, since the optimizer copied `find` into it, and C++'s Callgrind profile showed
  `find` three times, once for each header whose code was inlined into it:
  `basic_string.h`, `char_traits.h`, and `stl_algobase.h`. A function that seems to do
  little, with a large self time, may be the small functions it swallowed, and the
  per-line view finds them.
- **Profile the build you ship.** A debug build spends its time differently, in checks
  and in code the optimizer would have removed, so profiling it finds the wrong
  problems. Profile an optimized build with debugging information, `-O2 -g`, which costs
  nothing at run time and gives the profiler its names.

### The fix

The fix is a hash map, which finds a word in about the same time however many words it
holds. In Python:

```python
def count_words(words: list[str]) -> list[tuple[str, int]]:
    counts: dict[str, int] = {}
    for word in words:
        counts[word] = counts.get(word, 0) + 1
    return list(counts.items())
```

The output is the same, since a `dict` keeps the order in which its keys were added, and
the time on `big.txt` went from 12.8 seconds, in that run, to 0.047, 270 times faster.
Profiled again, the program's slowest function is now `split_words`, at 0.038 seconds of
self time, which is how a profile looks when nothing is wrong: the work spread across
the program, in the parts that have to touch every character. Exercise 20.2 is the same
fix, in all nine languages, in a program with two slow parts instead of one.

### Memory

A program can be slow because it allocates too much, or fail because it runs out of
memory, and each language has a tool that shows where memory goes: Valgrind's Massif and
heaptrack for C, C++, and Rust; pprof's heap profile, `-memprofile` or
`pprof.WriteHeapProfile`, for Go; `tracemalloc`, in Python's standard library; heap
snapshots in Chrome's DevTools for Node; the memory view in Dart's DevTools; and JFR's
allocation events and heap dumps for the JVM. They answer two questions, which take
different tools: where the memory that is in use now was allocated, which finds leaks,
and how much was allocated in all, which finds code that makes garbage faster than it
needs to.

---

## 20.8 Shrinking the input, and searching history

When a bug needs a long input to show itself, the first job is to make the input short.
Take out half of it: if it still fails, keep the half, and if not, put it back and try
the other half, or a smaller piece. The input that remains, when nothing more can be
taken out, is often small enough to say what the bug is by itself, and to follow in a
debugger. Andreas Zeller turned this into an algorithm, **delta debugging**, which tools
such as C-Reduce and its successor, C-Vise, run on C and C++ programs that crash a
compiler. Property-based testing libraries, which exercise 19.1's stretch mentioned, do
the same with their random inputs: Hypothesis, proptest, fast-check, and Kotest each
**shrink** a failing case to a small one before they report it.

When a bug is new, the other search is through history: if the test passes at an old
commit and fails now, one of the commits between broke it, and `git bisect run` finds
which, by halving, as section 17.8 shows. A test that reproduces the bug, from step 1 of
section 20.1, is what `git bisect run` needs, and a hundred commits take about seven
tries.

---

## 20.9 Why each language looks the way it does

- **C** leaves every check to the programmer, since the language was designed to compile
  to what the machine does and no more, so a bad index is undefined behavior and usually
  silent. Its debuggers came from Unix: gdb, the GNU debugger, started in 1986, and
  DWARF, the format of the debugging information, from Unix System V. The checks C lacks
  arrived as tools beside it: Valgrind in 2002, and AddressSanitizer, from Google, in
  Clang in 2012 and GCC in 2013.
- **C++** has the same tools as C, and exceptions, so a bug that throws ends with
  `terminate`. Its standard library can check more than the standard asks, with
  `-D_GLIBCXX_ASSERTIONS` in GCC's and `_LIBCPP_HARDENING_MODE` in Clang's, and C++26
  makes some of these checks part of the standard, as hardened implementations. C++20
  added `std::source_location` and C++23 `std::stacktrace`, so that the place and the
  stack no longer need macros or libraries.
- **Rust** checks indexes always, and overflow in debug builds, so that a release build
  is fast and a debug build is strict; the overflow check can be kept in a release build
  with `overflow-checks = true` in `Cargo.toml`. Rust compiles to machine code through
  LLVM, so it uses gdb, lldb, and the C tools, with wrapper scripts to print its types.
  Panics print a stack trace only when asked, which keeps the message short for the
  people who use a program rather than write it.
- **Go** was designed with its tools. Its profiler, named after the pprof of Google's
  C++ performance tools, has been in the standard library since Go 1.0, in 2012, and a
  server can serve its own profiles over HTTP with `net/http/pprof`, so that it can be
  profiled while it runs, in production. Go's debugger, Delve, came from outside the Go
  team, in 2014, since gdb didn't understand goroutines.
- **Python** has had pdb and the `profile` module in the standard library since the
  1990s, and cProfile since 2006. pdb is built on a hook, `sys.settrace`, that calls a
  function for each line and each call, which makes a program much slower while a
  debugger is attached, so Python 3.12 added `sys.monitoring`, which costs only where a
  tool asks for an event. Python 3.11 added the markers under the part of a line that
  failed.
- **JavaScript** grew up in browsers, where the debugger became part of the browser, and
  Node took Chrome's: V8 speaks the Chrome DevTools Protocol, so the same DevTools debug
  a web page and a server. Its looseness about indexes dates from the language's first
  version, in 1995, which preferred to keep a web page going rather than stop it.
- **TypeScript** can't add checks at run time, since it compiles to JavaScript and
  erases its types, so its checks are all at compile time, and
  `noUncheckedIndexedAccess` is off by default because it makes every array access need
  a check, which most code finds too noisy.
- **Dart**'s VM service, a protocol over a WebSocket, dates from Observatory, the Dart
  VM's first debugger and profiler, which ran in a browser. DevTools replaced it, and is
  written in Flutter itself; since the VM service works over a network, the same
  DevTools debug a Flutter app running on a phone.
- **Kotlin** inherited the JVM's tools, which are among the most complete there are:
  JDWP for debuggers, and Java Flight Recorder, which began in the JRockit JVM and
  became part of OpenJDK in Java 11, in 2018. `jcmd PID JFR.start` starts a recording in
  a JVM that is already running.

---

## 20.10 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | A bad index, in an ordinary build | Often nothing: example 01 printed a plausible answer and exited 0; build the tests with `-fsanitize=address,undefined` |
| C | Debugging an `-O2` build | Variables are `<optimized out>`, breakpoints stop on other lines, and frames go missing; use `-g -O0` to debug, and `-O2 -g` to profile |
| C | Expecting Valgrind to check arrays on the stack | It checks heap blocks; example 01's bad index went unnoticed; AddressSanitizer found it |
| C, C++ | Trusting gprof's flat profile | Time in libraries that weren't built with `-pg` goes missing, and example 03's profile put 60% in `_init` |
| C++ | `std::array` or `std::vector`'s `operator[]` with a bad index | No check, and undefined behavior; use `.at()`, which throws, or build with `-D_GLIBCXX_ASSERTIONS` |
| Rust | Testing only the release build | Overflow isn't checked, so `c as usize - 'a' as usize` wraps to 18446744073709551591 instead of failing at the subtraction |
| Rust | Wanting the stack of a panic | Set `RUST_BACKTRACE=1`, and build with `-g`, or `debug = true` in a release profile, for line numbers |
| Go | Debugging with gdb | `-gcflags=all="-N -l"` is needed, and some values are garbage, such as `~r0=5711648`; use Delve |
| Go | A data race that passes every test | Run the tests with `-race`; the example's race printed the right answer without it |
| Python | Expecting a negative index to fail | `counts[-25]` is `counts[1]`, and example 01 counted `H` as `b` |
| Python | Believing cProfile's times for small functions | Its cost for each call is counted; exercise 20.2's generator expression looked twice as slow as it was |
| Python | `logging.debug` and `logging.info` that print nothing | The root logger shows warnings and worse until `logging.basicConfig(level=logging.DEBUG)` |
| JavaScript | A bad index | `undefined`, and `NaN` after arithmetic, with no error; example 01 dropped `H` and `W` silently |
| JavaScript | `console.log` of a nested object | Below two levels it prints `[Object]`; use `console.dir(value, { depth: null })` |
| JavaScript | `console.assert` to stop on a broken rule | It prints `Assertion failed:` and carries on; use `assert` from `node:assert` |
| TypeScript | Trusting `number` from an array index | It may be `undefined`; turn on `noUncheckedIndexedAccess` |
| Dart | `print` of an object | `Instance of 'Point'`; override `toString`, or use a record |
| Dart | `log` from `dart:developer` to see what happened | It goes to DevTools, and the terminal shows nothing |
| Dart | `assert` in a program run with `dart run` | It is skipped; use `--enable-asserts` |
| Kotlin | `println` of an ordinary class | `Plain@7f31245a`; make it a `data class`, or override `toString` |
| Kotlin | `assert` without `-ea` | It is skipped; use `check` or `require` for checks that must run |
| All | Printing debug output to standard output | It mixes with the real output, and breaks the tests; print to standard error |
| All | Optimizing before profiling | The time is rarely where it seems: example 03's was in string comparisons inside a lookup |
| All | Fixing the line that crashed | The defect is often earlier; find the first wrong value |

---

## 20.11 Idiom check

**C: sanitizers in every test build.** Build the tests with
`-g -fsanitize=address,undefined`, and keep a debug build, `-g -O0`, for gdb or lldb.
Use `assert` for the program's own rules, with `-DNDEBUG` only in release builds.
Profile an `-O2 -g` build with perf where there is one, and Callgrind where there isn't,
and read `callgrind_annotate`'s per-line counts when the optimizer has inlined the
functions away.

**C++: the same, with the library's checks.** Add `-D_GLIBCXX_ASSERTIONS`, or Clang's
hardening mode, to test and debug builds, so that `operator[]` is checked. Give types an
`operator<<` or a `std::formatter` so that they can be printed, use
`std::source_location` in logging helpers, and spdlog for logs.

**Rust: `dbg!`, `RUST_BACKTRACE`, and debug builds.** Use `dbg!` for quick prints,
`#[derive(Debug)]` on every type, and `RUST_BACKTRACE=1` to see where a panic came from.
Run tests in debug builds, which check overflow, and keep `overflow-checks = true` in
release if the code can't trust its arithmetic. Log through the `log` crate or
`tracing`, and profile with perf or Callgrind, or `cargo flamegraph`.

**Go: Delve, `-race`, and pprof.** Debug with Delve, through an editor or `dlv debug`.
Run tests with `-race`, always in CI. Print with `%+v`, log with `log/slog`, and profile
with pprof: `go test -cpuprofile` for a benchmark, or `net/http/pprof` for a running
server.

**Python: `breakpoint()`, f-strings with `=`, and `logging`.** Use `print(f"{x=}")` or
`breakpoint()` to look, `python3 -m pdb` for a crash, and `logging.getLogger(__name__)`
in every module. Give classes a `__repr__`, or make them dataclasses. Profile with
cProfile, and with a sampling profiler, such as py-spy, when the program has many small
calls, and test with `-X dev` now and then.

**JavaScript: DevTools.** Debug with `node --inspect` and Chrome's DevTools or an
editor, print with `console.log({ a, b })`, and use `console.dir` with `depth: null` for
deep objects. Profile with `--cpu-prof` and DevTools, and use a logging library with
levels, such as pino, in servers.

**TypeScript: JavaScript's tools, and the compiler's checks.** Debug the TypeScript
itself, with source maps if it is compiled, or directly when Node strips the types. Turn
on `strict` and `noUncheckedIndexedAccess`, so that the compiler finds what JavaScript
would let through.

**Dart: DevTools.** Debug and profile through DevTools, from an editor or
`dart run --observe`, run tests and debug builds with `--enable-asserts`, which
`dart test` and Flutter's debug builds turn on, and override `toString` on classes that
will be printed. Use `package:logging` for logs.

**Kotlin: IntelliJ, data classes, and JFR.** Debug in IntelliJ, or with jdb where there
is no IDE. Make value types `data class`es so that they print, use `check` and `require`
for rules that must hold, and log through SLF4J. Profile with Java Flight Recorder,
`-XX:StartFlightRecording`, which is cheap enough to leave on in production, and read
the recording in JDK Mission Control or with `jfr`.

---

## 20.12 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Find the bugs](exercises/01-find-the-bugs/) | Reading a crash, a debugger, post-mortem debugging, and shrinking a failing input |
| 2 | [Make it fast](exercises/02-make-it-fast/) | Profiling, finding two hot spots, and a hash map |
| 3 | [Profile a trace](exercises/03-profile-a-trace/) | How a profiler works out calls, total time, and self time, with a stack |

```sh
python3 tools/check.py 20                     # your code, every language
python3 tools/check.py 20 --lang go,python    # some languages
python3 tools/check.py 20 --solutions         # the solutions, and the examples
```

The first two exercises give you a program with something wrong with it, and the third
asks you to write part of a profiler. The starters fail: exercise 20.1's crash or print
a wrong report, exercise 20.2's run out of the checker's 20 seconds on the last two
tests, and exercise 20.3's print nothing. Each README says what the program should do,
and how to run it under the tools from this chapter.

**Stretch:** take the slowest program you wrote for an earlier chapter's exercise,
profile it in two languages, and compare what the two profiles say. Is the slow part the
same in both? If it isn't, find out which of the languages' choices, such as a library
function, an allocation, or what the optimizer inlined, made the difference.

---

Previous: [Chapter 19: Testing basics](../19-testing-basics/) ·
Next: Chapter 21: Files, JSON, and the CLI (coming soon)
