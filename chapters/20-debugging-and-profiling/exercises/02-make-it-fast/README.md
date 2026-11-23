# Exercise 20.2: Make it fast

Each language's folder has a program that makes up a day of payments between bank
accounts, and prints a summary. It gives the right answers, but slowly: it passes the
first six tests, and on the last two it runs out of the checker's 20 seconds, by
minutes. Make it fast enough to pass them, without changing what it prints. Don't guess
where the time goes: profile it, and fix what the profile shows.

**Input.** One line, `ACCOUNTS TRANSFERS SEED`: the number of accounts, from 1 to
1,000,000, the number of transfers, from 0 to 1,000,000, and a seed, from 1 to
2,147,483,646.

**The accounts.** They are named `acct000000`, `acct000001`, and so on, with six digits,
and each starts with 1000.

**The transfers.** Each number the program needs comes from the Park–Miller generator:
the state starts as the seed, and each next number is the state times 48,271, modulo
2,147,483,647, which becomes the new state. Each transfer takes three numbers in turn:
the account it comes from is the one numbered `next % ACCOUNTS`, the account it goes to
is the one numbered `next % ACCOUNTS`, and the amount is `next % 500 + 1`. The transfer
is refused if the first account has less than the amount, and an account can pay itself.

**Output.** `transfers: M, refused: R`, with the number of transfers and the number
refused, then `total: T`, with all the accounts' money added up, and then the five
accounts with the most money, or all of them if there are fewer than five, as
`NAME BALANCE`. They are in order of balance, highest first, and of name on a tie.

For example, `5 10 1` prints:

```text
transfers: 10, refused: 0
total: 5000
acct000004 1801
acct000000 1565
acct000003 958
acct000001 605
acct000002 71
```

The program also checks that no money has been made or lost, and stops with an error if
the total has changed. Keep that check: it is how you would find out that a faster
version had a bug.

Profile it on an input that takes a second or two, such as `20000 20000 1`, or
`5000 5000 1` in Python, and on one a tenth of that size under Callgrind, which runs a
program many times slower. These are the profilers that section 20.7 uses:

| Language | Build it | Profile it |
| --- | --- | --- |
| C | `cc -std=c17 -O2 -g main.c -o main` | `echo 2000 2000 1 \| valgrind --tool=callgrind ./main`, then `callgrind_annotate callgrind.out.*` |
| C++ | `c++ -std=c++20 -O2 -g main.cpp -o main` | The same as C |
| Rust | `rustc --edition 2024 -O -g main.rs` | The same as C |
| Go | Copy the `-cpuprofile` flag from example 03's `main.go` into `main`, and `go build -o main main.go` | `echo 20000 20000 1 \| ./main -cpuprofile cpu.prof`, then `go tool pprof -top main cpu.prof` |
| Python | | `echo 5000 5000 1 \| python3 -m cProfile -s tottime main.py` |
| JavaScript | | `echo 20000 20000 1 \| node --prof main.js`, then `node --prof-process isolate*.log` |
| TypeScript | | The same, with `node --prof main.ts`, since Node runs TypeScript by stripping its types |
| Dart | | `echo 20000 20000 1 \| dart run --observe --pause-isolates-on-exit main.dart`, then the CPU Profiler in the DevTools link it prints |
| Kotlin | `kotlinc main.kt -include-runtime -d main.jar` | `echo 20000 20000 1 \| java -XX:StartFlightRecording=filename=rec.jfr -jar main.jar`, then `jfr view hot-methods rec.jfr` |

Then run the tests:

```sh
python3 tools/check.py 20 --lang kotlin
```

The solutions take under three seconds on each test, in every language.

**Things to notice:**

- **How the time grows.** On `5000 5000 1` the C starter takes 0.06 seconds, and on
  `20000 20000 1`, with four times as many accounts and four times as many transfers, it
  takes 1.2 seconds, twenty times as long. Python goes from 1.2 seconds to 23. Time that
  grows with the square of the input is the sign of a loop inside a loop, and at the
  size of test 7 it means minutes in C and more than an hour in Python.
- **Two hot spots.** The profile shows two places that take the time, and fixing one of
  them leaves the program nearly as slow as before. In Python, on `20000 20000 1`, the
  starter takes 22 seconds, and with either fix alone, 10 or 11. In C, either fix alone
  leaves test 7 taking more than 40 seconds. A profile is worth reading again after each
  fix, since what was second is now first.
- **The top line may not be your code.** The function with the most time is often a
  library's: `strcmp` in C, `memeqbody` in Go, `StringEqual` in JavaScript,
  `String.equals` in Kotlin, and `==` in Dart. What matters is which of your functions
  called it, which is what the inclusive columns show: `cum` in pprof, `total` in
  DevTools, and `cumtime` in cProfile.
- **What an instrumenting profiler adds.** cProfile times every call, and on
  `5000 5000 1` it says that adding up the total takes 88% of the time, and finding the
  accounts 11%. Timing the two fixes says it is nearer 40% and 60%. cProfile counts each
  step of the generator expression that adds up the total as a call, 25 million of them,
  and its cost for each call makes code with many small calls look slower than it is.
  The sampling profilers, pprof, JFR, `node --prof`, and DevTools, look at what the
  program is doing many times a second instead, and add almost nothing.
- **Checks cost time too.** Checking the total after every transfer was a good idea when
  the program was being written, and a slow one to keep. Checking it once at the end
  catches the same bugs, since nothing can put the total right again once it is wrong.
  Assertions that can be turned off are made for checks like this: C's `assert` is gone
  with `-DNDEBUG`, Rust's `debug_assert!` in a release build, Python's `assert` with
  `python3 -O`, and Kotlin's `assert` unless the JVM is run with `-ea`.

**Stretch:** profile your fast version on test 8. What takes the time now is setting up:
making a million accounts, their names, and a map from names to accounts, and then
sorting all million of them to print five. The account names are made from their
numbers, so the map can go. Find the top five without sorting all the accounts, and see
how much faster test 8 gets.
