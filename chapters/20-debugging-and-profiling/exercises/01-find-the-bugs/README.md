# Exercise 20.1: Find the bugs

Each language's folder has a program that prints a report of students' scores, and it
has four bugs. Find them and fix them. Try to find each one with the tools in this
chapter, a stack trace, a debugger, or a well-placed print, before you read the code
from the top: the program is short enough to read, and a real one isn't.

**Input.** Each line is a name and then that student's scores, separated by spaces or
tabs. A name can come on more than one line, and its scores add up. A line with a name
and no scores adds that student, with none. Scores are whole numbers from 0 to 100, and
blank lines are skipped.

**Output.** A line for each student, in the order they first appear, with the average of
their scores, rounded to the nearest whole number with halves rounded up, or `-` if they
have no scores. Then `class average: N`, the average of every score in the class,
rounded the same way, which isn't the average of the students' averages, or `-` if there
are no scores at all. Then `top: NAME`, the student with the highest average as it was
printed, the first of them in the report if two or more have it, or `-` if no student
has a score.

For example:

```text
ada 92 85 78
grace 88 95
alan 70
ada 90
linus
grace 75
```

prints:

```text
ada 86
grace 86
alan 70
linus -
class average: 84
top: ada
```

Ada's four scores add up to 345, and 345 / 4 is 86.25. Grace's three are 258, and 86
exactly. The two averages are the same, so the top is Ada, who comes first.

So that a debugger can keep the terminal to itself, the program reads the file named on
its command line if it is given one, and standard input if not. That way you can run it
on a test's input, which the checker gives it on standard input:

| Language | Build it for debugging | Run it in a debugger |
| --- | --- | --- |
| C | `cc -std=c17 -g -O0 main.c -o main` | `gdb --args ./main ../tests/01-example.in` |
| C++ | `c++ -std=c++20 -g -O0 main.cpp -o main` | `gdb --args ./main ../tests/01-example.in` |
| Rust | `rustc --edition 2024 -g main.rs` | `rust-gdb --args ./main ../tests/01-example.in` |
| Go | `go build -gcflags=all="-N -l" -o main main.go` | `gdb --args ./main ../tests/01-example.in`, or Delve: `dlv exec ./main -- ../tests/01-example.in` |
| Python | | `python3 -m pdb main.py ../tests/01-example.in` |
| JavaScript | | `node inspect main.js ../tests/01-example.in` |
| TypeScript | | `node inspect main.ts ../tests/01-example.in` |
| Dart | | `dart run --observe --pause-isolates-on-start main.dart ../tests/01-example.in`, and open the DevTools link it prints |
| Kotlin | `kotlinc main.kt -include-runtime -d main.jar` | `jdb -classpath main.jar MainKt ../tests/01-example.in` |

Section 20.5 has the commands each debugger takes, and an editor's debugger, such as VS
Code's or IntelliJ's, does the same with buttons. Then run the tests:

```sh
python3 tools/check.py 20 --lang rust
```

Each of the four bugs makes at least one test fail when it is the only one, so when all
eleven pass, you have found them all.

**Things to notice:**

- **One bug, many symptoms.** In seven of the nine languages, the first thing you see is
  a crash, at the same line: a division by zero, when a student has no scores. C and C++
  stop with `SIGFPE`, which the checker reports as exit code -8, for signal 8. The name
  stands for "floating-point exception", though the division is of integers: in C and
  C++ dividing an integer by zero is undefined behavior, and on x86-64 the processor
  traps. Rust panics, Go panics, Python raises `ZeroDivisionError`, Dart throws
  `IntegerDivisionByZeroException`, and Kotlin throws `ArithmeticException`. JavaScript
  and TypeScript divide numbers that are all floating point, so they print
  `linus Infinity`, and then make Linus top, since `Infinity` is higher than every
  average.
- **Stopping where it went wrong.** A debugger started on a program that crashes stops
  at the crash, with the program as it was: gdb stops at the `SIGFPE`, `python3 -m pdb`
  says `Entering post mortem debugging`, and jdb stops at an exception that nothing
  catches. Then `up` moves to `main`, where printing the variables shows more than the
  crash does: the `total` it was dividing is 235, for a student with no scores.
- **The bugs a crash doesn't show.** With the crash fixed, the report is wrong, but
  nothing says where. A breakpoint at the line that prints each student, and a look at
  the variables each time it stops, will show two of the bugs. gdb's `watch total` stops
  whenever `total` changes, wherever that happens.
- **Shrinking the input.** When a test fails, make its input smaller, one line at a
  time, for as long as it still fails. Two or three lines that fail are far easier to
  follow in a debugger than the whole input, and the smallest input often says what the
  bug is on its own.
- **Sanitizers.** `cc -fsanitize=undefined` reports the division before it happens, with
  its line and column, `c/main.c:53:32: runtime error: division by zero`, though the
  program still crashes after it. Section 20.6 has the rest.

**Stretch:** for each of the four bugs, find the smallest input that shows it, put that
bug back by itself, and check that the input fails with it and passes without it. These
are the inputs that a test suite for this program needs, and each is a test that would
have caught its bug when it was written.
