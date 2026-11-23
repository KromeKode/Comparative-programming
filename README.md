# Comparative Programming

A textbook that teaches nine languages side by side: **C, C++, Rust, Go, Python,
JavaScript, TypeScript, Dart, and Kotlin**. Every chapter solves the same problems in
each language, so the differences in design are on the page instead of in the abstract.

It starts at syntax and works up through language design, tooling, data structures
and algorithms, SQL, and web servers. Bonus parts cover number theory, linear algebra,
a 2D wave shooter, multiplayer networking, and GUI apps. See the full
[syllabus](SYLLABUS.md).

## Quick start

```sh
git clone https://github.com/KromeKode/comparative-programming
cd comparative-programming
npm install                        # TypeScript compiler, used to type-check exercises
python3 tools/check.py --doctor    # shows which of the 9 toolchains you have
```

Then open [Chapter 1](chapters/01-toolchains-and-hello-world/), which walks through
installing anything that is missing.

## How the book is laid out

```text
chapters/
  01-toolchains-and-hello-world/
    README.md          the chapter text
    examples/          every code listing from the text, runnable
    exercises/         one folder per exercise: rules, tests, and a starter file per language
    solutions/         reference answers
tools/
  check.py             builds and tests exercises in all 9 languages
  git-sandbox.sh       runs chapter 17's Git exercises in a throwaway repository
SYLLABUS.md            all 49 chapters and their status
```

Every chapter follows the same template:

1. **Concept** in plain, language-neutral terms
2. **Side-by-side code** for the same program in each language
3. **Comparison table** of the design choices that matter in that chapter
4. **Design rationale**: why each language made the choice it did
5. **Pitfalls** a newcomer from another language will hit
6. **Idiom check**: working code compared with code that looks native
7. **Exercises** with tests

## Checking your answers

Every exercise reads stdin and writes stdout, so one checker works for all nine
languages:

```sh
python3 tools/check.py 01                  # your chapter 1 exercises, every language
python3 tools/check.py 01 --lang rust,go   # only some languages
python3 tools/check.py 01 --solutions      # the reference answers
python3 tools/check.py all --solutions     # everything, which is what CI runs
```

Chapter 17, on Git, is the exception: its exercises are scripts of Git commands, which
the checker runs in a throwaway repository. Chapter 19, on testing, turns the usual
arrangement round: the code is given and the exercises are its tests, which the checker
runs with each language's own test runner, on the code as given and then on copies of it
with bugs put in. Chapter 20, on debugging and profiling, gives you a wrong program and
a slow one to fix, and the checker gives each test 20 seconds, which the slow one runs
out of. Chapter 21, on files, JSON, and the command line, gives each program its
arguments and files in an empty folder, and checks the files it leaves there and its
exit status as well as what it prints.

Write your answers in the starter files under `exercises/`. Languages whose toolchain
is not installed are skipped rather than failed. From chapter 18 on, some examples are
projects that build with each language's own build tool, such as CMake or Cargo, and
`python3 tools/check.py --doctor` lists which of those are installed.

## Versions

| Language | Version | Compiled with |
| --- | --- | --- |
| C | C17 | GCC 13+ or Clang 16+, `-Wall -Wextra -Werror` |
| C++ | C++20 | GCC 13+ or Clang 16+, `-Wall -Wextra -Werror` |
| Rust | 1.95+, 2024 edition | `rustc -D warnings` |
| Go | 1.23+ | `go vet`, then `go build` |
| Python | 3.12+ | CPython |
| JavaScript | Node 22.18+ | Node |
| TypeScript | 7 | `tsc --strict --noEmit`, run by Node's built-in type stripping |
| Dart | 3.x | `dart analyze`, then `dart run` |
| Kotlin | 2.4+ on JDK 21 | `kotlinc -Werror`, then `java -jar` |
| Git (chapter 17) | 2.43+ | `bash`, in a throwaway repository |
| Build tools (chapter 18) | CMake 3.20+, Gradle 8.14+, and uv | Cargo, `go`, npm, and `dart pub` come with their languages |
| Test frameworks (chapter 19) | pytest 9.2, `package:test` 1.33, and `kotlin.test` on JUnit 5 | Rust, Go, and Node have theirs built in, and C and C++ use a header in each folder |
| Debuggers and profilers (chapter 20) | gdb 15, Valgrind 3.22, and each language's own | Optional: the exercises are checked by what they print, and the text shows each tool's output |
| Libraries for JSON and options (chapter 21) | serde_json 1.0 and clap 4, nlohmann/json 3.12, cJSON 1.7.18, kotlinx.serialization 1.9 and Clikt 5, and `package:args` 2 | Downloaded by each project's build file; Go, Python, JavaScript, and Dart have JSON built in |
