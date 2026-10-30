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
SYLLABUS.md            all 48 chapters and their status
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

Write your answers in the starter files under `exercises/`. Languages whose toolchain
is not installed are skipped rather than failed.

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
