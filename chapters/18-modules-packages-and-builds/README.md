# Chapter 18: Modules, packages, builds

Every program so far has been one file. Real programs are many files, written by several
people, and they use code that other people published. This chapter is about the three
ideas that make that work. A **module** is a piece of a program with a name and a
boundary: it decides which of its names other code may use, and hides the rest. A
**package** is code with a name and a version that other programs can depend on. A
**build** is the work of turning all those files into a program, in the right order, and
of redoing as little of it as possible after a change. The nine languages differ most in
the first idea. C has no modules at all, only files that the preprocessor pastes
together and the linker joins, and the other eight each draw the boundary somewhere
different: around a file, a folder, or a class.

**Languages:** all nine · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Splitting a program into modules, and choosing what each one exports | Testing basics (chapter 19), design problems (chapter 31) |
| How C and C++ compile files one at a time and link them, and what headers, include guards, and `static` are for | Debugging and profiling (chapter 20), FFI and interop (chapter 23) |
| Manifests, registries, semantic versions, and version ranges | Every chapter from here on that uses a library, starting with files, JSON, and the CLI (chapter 21) |
| Lock files, and why a build should give the same result on every machine | Testing basics (chapter 19), REST CRUD API (chapter 38) |
| When a module's own code runs, and in what order | Debugging and profiling (chapter 20), GUI and desktop apps (chapter 49) |

---

## 18.1 The concept: modules, packages, and builds

A **module** has two sides. Its **interface** is the names it **exports**: the
functions, types, and constants that other code may use. Everything else is its
**implementation**, which is private, so the module's author can change it without
breaking anyone. Code that wants an exported name **imports** the module, and the
imports between modules form a **dependency graph**. A module also gives its names a
**namespace**, so that `textstats.word_count` and `stock.count` can't clash with each
other, or with a `count` of your own.

A **package** is one or more modules with a name, a version, and a **manifest**: a file
such as `Cargo.toml` or `package.json` that says what the package is and which other
packages it depends on. A **registry**, such as crates.io or npm, is where packages are
published. A **package manager** reads the manifest, picks a version of each dependency,
downloads it, and writes down what it picked in a **lock file**, so the next build gets
exactly the same versions.

Versions are written as **semantic versions**, `MAJOR.MINOR.PATCH`, and the three
numbers make a promise. A new patch version fixes bugs, a new minor version adds things
without breaking anything, and a new major version may break code that used the old one.
That promise is what lets a manifest ask for a range, such as `^1.2.0`, meaning any 1.x
from 1.2.0 on, instead of one exact version.

A **build** turns source files into a program. In C and C++ it has two stages that you
can see. The compiler turns each source file into an **object file** of machine code, on
its own, without looking at the others, and the **linker** then joins the object files
into one program, connecting each call to the function it names:

```text
  source files      compile, one at a time      object files      link        program
  main.c         ->  cc -c main.c            ->  main.o       \
                                                                 ->  cc  ->  main
  textstats.c    ->  cc -c textstats.c       ->  textstats.o  /
```

Each language's build tool has the same two jobs. It builds each part after the parts it
depends on, and after a change it rebuilds only what the change could affect: the
changed part, and everything that depends on it. Exercise 18.2 writes the piece of a
build tool that works out both.

---

## 18.2 What each language gives you

| Language | A module is | Private unless | Package manager and manifest | Registry | Lock file |
| --- | --- | --- | --- | --- | --- |
| C | A `.c` file and its header, by convention only; `#include` pastes a header's text in | Visible to every file unless `static`; the header lists what other files should use | None in the language; CMake or make for builds, and vcpkg or Conan for packages | None | None |
| C++ | The same as C, with namespaces; C++20 adds real modules, `export module`, which few projects use yet | The same as C, plus `private` members, and unnamed namespaces for file-private names | The same as C | None | None |
| Rust | A `mod`, in its own file or inline; a crate, a library or a program, is what the compiler builds | Private unless `pub` | Cargo, `Cargo.toml` | crates.io | `Cargo.lock` |
| Go | A package, which is a folder of `.go` files; a module is a tree of packages with a `go.mod` | Private unless the name starts with a capital letter | The `go` command, `go.mod` | None central: a module's path says where to fetch it, and proxy.golang.org keeps copies | `go.sum` holds hashes; `go.mod` already pins versions |
| Python | A `.py` file; a package is a folder of them | Public; a leading `_` means private, by agreement only | pip or uv, `pyproject.toml` | PyPI | `uv.lock`, with uv |
| JavaScript | A file, as an ES module | Private unless `export` | npm, `package.json` | npm | `package-lock.json` |
| TypeScript | JavaScript's | JavaScript's, plus `private` members checked by `tsc` | JavaScript's | npm | `package-lock.json` |
| Dart | A file, which Dart calls a library; a package is a folder with a `pubspec.yaml` and a `lib/` | Public; a leading `_` makes a name private to its library | `dart pub`, `pubspec.yaml` | pub.dev | `pubspec.lock` |
| Kotlin | A file, whose names are in the package its `package` line names; a module is the set of files compiled together | Public; `private` limits a name to its file or class, `internal` to its module | Gradle or Maven, `build.gradle.kts` | Maven Central | Optional `gradle.lockfile`; the wrapper pins Gradle itself |

The table splits two ways. Read down the second column and the languages sort by what a
module is. In Python, JavaScript, TypeScript, Dart, and Kotlin it is a file. In Go it is
a folder, and every file in the folder shares one set of names. In Rust it is whatever
`mod` says, a file or a block inside one, and in C and C++ it isn't a thing the language
knows about at all. Read down the third column and they sort by what is private by
default. Rust, Go, JavaScript, and TypeScript hide everything that isn't marked, which
makes exporting a decision. Python, Dart, and Kotlin show everything that isn't marked,
and C shows everything to the linker unless it is `static`. The last three columns split
the old languages from the new. Every language designed after 2000 came with a package
manager, or grew one that everyone uses, and C and C++, which are older than the idea,
still have none of their own.

---

## 18.3 Splitting a program into files

Example 01 splits a small program in two. The module `textstats` counts the words in a
text and finds the longest one, with a helper that splits the text into words, which it
keeps private, and a version number, which it exports. The main program reads standard
input and prints what the module says about it. Here is each language's module, and the
main program that uses it:

**C: `textstats.h`**

```c
// The interface of the textstats module: what other files may use. A file that
// uses the module includes this header, so it holds declarations, not code.
//
// #include pastes the header's text in, and a file can end up including it
// twice, through two other headers. The include guard makes the second copy
// empty: TEXTSTATS_H is defined the first time, and #ifndef skips the rest.
#ifndef TEXTSTATS_H
#define TEXTSTATS_H

#include <stddef.h>

#define TEXTSTATS_VERSION "1.0"

// The number of words in text. A word is a run of ASCII letters.
size_t word_count(const char *text);

// Copies the first of the longest words in text into out, which has room for
// size bytes, cutting it short if it doesn't fit.
void longest_word(const char *text, char *out, size_t size);

#endif
```

**C: `textstats.c`**

```c
#include "textstats.h"

#include <ctype.h>
#include <string.h>

// static makes next_word private to this file: the linker never sees its name,
// so no other file can call it, and another file can have a next_word of its own.
static const char *next_word(const char *text, size_t *length) {
    while (*text != '\0' && !isalpha((unsigned char)*text)) {
        text++;
    }
    const char *start = text;
    while (isalpha((unsigned char)*text)) {
        text++;
    }
    *length = (size_t)(text - start);
    return start;
}

size_t word_count(const char *text) {
    size_t count = 0;
    size_t length;
    for (const char *word = next_word(text, &length); length > 0;
         word = next_word(word + length, &length)) {
        count++;
    }
    return count;
}

void longest_word(const char *text, char *out, size_t size) {
    const char *best = text;
    size_t best_length = 0;
    size_t length;
    for (const char *word = next_word(text, &length); length > 0;
         word = next_word(word + length, &length)) {
        if (length > best_length) {
            best = word;
            best_length = length;
        }
    }
    if (best_length >= size) {
        best_length = size - 1;
    }
    memcpy(out, best, best_length);
    out[best_length] = '\0';
}
```

**C: `main.c`**

```c
#include <stdio.h>
#include <stdlib.h>

#include "textstats.h"

int main(void) {
    // Read all of stdin into one string.
    size_t size = 0, capacity = 256;
    char *text = malloc(capacity);
    if (text == NULL) {
        return 1;
    }
    size_t got;
    while ((got = fread(text + size, 1, capacity - size - 1, stdin)) > 0) {
        size += got;
        if (capacity - size == 1) {
            char *bigger = realloc(text, capacity * 2);
            if (bigger == NULL) {
                free(text);
                return 1;
            }
            text = bigger;
            capacity *= 2;
        }
    }
    text[size] = '\0';

    char longest[64];
    longest_word(text, longest, sizeof longest);
    printf("textstats %s\n", TEXTSTATS_VERSION);
    printf("words: %zu\n", word_count(text));
    printf("longest: %s\n", longest);
    free(text);
    return 0;
}
```

**C++: `textstats.hpp`**

```cpp
// The interface of the textstats module. #pragma once does what an include
// guard does: a second #include of this file in the same translation unit adds
// nothing. It isn't in the standard, but every major compiler supports it.
#pragma once

#include <cstddef>
#include <string>
#include <string_view>

// A namespace keeps the module's names apart from everyone else's, so callers
// write textstats::word_count, or bring the names in with a using-declaration.
namespace textstats {

// inline lets every file that includes the header share one definition.
inline constexpr std::string_view version = "1.0";

// The number of words in text. A word is a run of ASCII letters.
std::size_t word_count(std::string_view text);

// The first of the longest words in text.
std::string longest_word(std::string_view text);

}  // namespace textstats
```

**C++: `textstats.cpp`**

```cpp
#include "textstats.hpp"

#include <cctype>
#include <vector>

namespace textstats {

// An unnamed namespace makes words private to this file, as static does in C.
namespace {

std::vector<std::string_view> words(std::string_view text) {
    std::vector<std::string_view> found;
    std::size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && !std::isalpha(static_cast<unsigned char>(text[i]))) {
            i++;
        }
        std::size_t start = i;
        while (i < text.size() && std::isalpha(static_cast<unsigned char>(text[i]))) {
            i++;
        }
        if (i > start) {
            found.push_back(text.substr(start, i - start));
        }
    }
    return found;
}

}  // namespace

std::size_t word_count(std::string_view text) { return words(text).size(); }

std::string longest_word(std::string_view text) {
    std::string_view best;
    for (std::string_view word : words(text)) {
        if (word.size() > best.size()) {
            best = word;
        }
    }
    return std::string(best);
}

}  // namespace textstats
```

**C++: `main.cpp`**

```cpp
#include <iostream>
#include <iterator>
#include <string>

#include "textstats.hpp"

int main() {
    std::string text(std::istreambuf_iterator<char>(std::cin), {});
    std::cout << "textstats " << textstats::version << '\n';
    std::cout << "words: " << textstats::word_count(text) << '\n';
    std::cout << "longest: " << textstats::longest_word(text) << '\n';
}
```

**Rust: `textstats.rs`**

```rust
//! The textstats module. `mod textstats;` in main.rs makes this file a module
//! of the crate, and everything here is private to it unless marked `pub`.

pub const VERSION: &str = "1.0";

/// The words in text: runs of ASCII letters. Private, so only this module can
/// call it, and it can change without breaking anyone.
fn words(text: &str) -> impl Iterator<Item = &str> {
    text.split(|c: char| !c.is_ascii_alphabetic())
        .filter(|word| !word.is_empty())
}

/// The number of words in text.
pub fn word_count(text: &str) -> usize {
    words(text).count()
}

/// The first of the longest words in text.
pub fn longest_word(text: &str) -> &str {
    words(text).fold(
        "",
        |best, word| if word.len() > best.len() { word } else { best },
    )
}
```

**Rust: `main.rs`**

```rust
// The compiler finds this module's code in textstats.rs, next to this file.
mod textstats;

use std::io;

fn main() {
    let text = io::read_to_string(io::stdin()).expect("stdin should be text");
    println!("textstats {}", textstats::VERSION);
    println!("words: {}", textstats::word_count(&text));
    println!("longest: {}", textstats::longest_word(&text));
}
```

**Go: `go.mod`**

```text
module example.com/stats

go 1.23
```

**Go: `textstats/textstats.go`**

```go
// Package textstats counts the words in a text. A package is a folder: every
// .go file in it starts with "package textstats", and they all share one set of
// names. A name that starts with a capital letter is exported, and can be used
// by other packages. Any other name is private to the package.
package textstats

import "strings"

const Version = "1.0"

// words is unexported: only the files of this package can call it. It splits
// the text at every character that isn't an ASCII letter.
func words(text string) []string {
	return strings.FieldsFunc(text, func(r rune) bool {
		return !('a' <= r && r <= 'z' || 'A' <= r && r <= 'Z')
	})
}

// WordCount is the number of words in text. A word is a run of ASCII letters.
func WordCount(text string) int {
	return len(words(text))
}

// LongestWord is the first of the longest words in text.
func LongestWord(text string) string {
	best := ""
	for _, word := range words(text) {
		if len(word) > len(best) {
			best = word
		}
	}
	return best
}
```

**Go: `main.go`**

```go
package main

import (
	"fmt"
	"io"
	"os"

	// The import path is the module's path from go.mod, then the folder.
	"example.com/stats/textstats"
)

func main() {
	data, err := io.ReadAll(os.Stdin)
	if err != nil {
		panic(err)
	}
	text := string(data)
	fmt.Println("textstats", textstats.Version)
	fmt.Println("words:", textstats.WordCount(text))
	fmt.Println("longest:", textstats.LongestWord(text))
}
```

**Python: `textstats.py`**

```python
"""The textstats module. Any .py file is a module, named after the file, and
everything at its top level can be imported. A leading underscore marks a name
as private, but only by agreement: Python doesn't stop anyone using it."""

import re
from collections.abc import Iterator

VERSION = "1.0"


def _words(text: str) -> Iterator[str]:
    """The words in text: runs of ASCII letters."""
    return (match.group() for match in re.finditer(r"[A-Za-z]+", text))


def word_count(text: str) -> int:
    """The number of words in text."""
    return sum(1 for _ in _words(text))


def longest_word(text: str) -> str:
    """The first of the longest words in text."""
    return max(_words(text), key=len, default="")
```

**Python: `main.py`**

```python
import sys

# Python looks for textstats.py in the folder of the script it is running, then
# on the rest of sys.path, and runs it once, the first time it is imported.
import textstats


def main() -> None:
    text = sys.stdin.read()
    print(f"textstats {textstats.VERSION}")
    print(f"words: {textstats.word_count(text)}")
    print(f"longest: {textstats.longest_word(text)}")


if __name__ == "__main__":
    main()
```

**JavaScript: `textstats.js`**

```js
// The textstats module. Every file is a module with names of its own, and only
// the names marked export can be imported by other files.

export const VERSION = "1.0";

// Not exported, so private to this file.
function words(text) {
  return text.match(/[A-Za-z]+/g) ?? [];
}

// The number of words in text. A word is a run of ASCII letters.
export function wordCount(text) {
  return words(text).length;
}

// The first of the longest words in text.
export function longestWord(text) {
  return words(text).reduce((best, word) => (word.length > best.length ? word : best), "");
}
```

**JavaScript: `main.js`**

```js
import { readFileSync } from "node:fs";

// A relative import names the file, extension and all. Node runs textstats.js
// first, once, however many modules import it.
import { VERSION, longestWord, wordCount } from "./textstats.js";

const text = readFileSync(0, "utf8");
console.log(`textstats ${VERSION}`);
console.log(`words: ${wordCount(text)}`);
console.log(`longest: ${longestWord(text)}`);
```

**TypeScript: `textstats.ts`**

```ts
// The textstats module. Every file is a module with names of its own, and only
// the names marked export can be imported by other files.

export const VERSION = "1.0";

// Not exported, so private to this file.
function words(text: string): string[] {
  return text.match(/[A-Za-z]+/g) ?? [];
}

// The number of words in text. A word is a run of ASCII letters.
export function wordCount(text: string): number {
  return words(text).length;
}

// The first of the longest words in text.
export function longestWord(text: string): string {
  return words(text).reduce((best, word) => (word.length > best.length ? word : best), "");
}
```

**TypeScript: `main.ts`**

```ts
import { readFileSync } from "node:fs";

// Node runs .ts files by removing the types, and doesn't change import paths,
// so the import names the .ts file. tsc allows that with
// --allowImportingTsExtensions, when it only checks types and writes nothing.
import { VERSION, longestWord, wordCount } from "./textstats.ts";

const text = readFileSync(0, "utf8");
console.log(`textstats ${VERSION}`);
console.log(`words: ${wordCount(text)}`);
console.log(`longest: ${longestWord(text)}`);
```

**Dart: `textstats.dart`**

```dart
// The textstats library. In Dart every file is a library, and a name that
// starts with an underscore is private to it. There is no keyword for public:
// every other name can be imported.

const version = '1.0';

final _word = RegExp('[A-Za-z]+');

// Private: only code in this file can call it.
Iterable<String> _words(String text) =>
    _word.allMatches(text).map((m) => m[0]!);

/// The number of words in [text]. A word is a run of ASCII letters.
int wordCount(String text) => _words(text).length;

/// The first of the longest words in [text].
String longestWord(String text) =>
    _words(text)
        .fold('', (best, word) => word.length > best.length ? word : best);
```

**Dart: `main.dart`**

```dart
import 'dart:io';

// A relative import names the file. "as" puts its names under a prefix, so
// they read as textstats.wordCount and can't clash with names of this file.
import 'textstats.dart' as textstats;

void main() {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    lines.add(line!);
  }
  final text = lines.join('\n');
  print('textstats ${textstats.version}');
  print('words: ${textstats.wordCount(text)}');
  print('longest: ${textstats.longestWord(text)}');
}
```

**Kotlin: `textstats.kt`**

```kotlin
// The package statement puts this file's names in the package textstats.
// Top-level names are public by default; private ones can be used only in this
// file, and internal ones only in the same module, the set of files that are
// compiled together.
package textstats

const val VERSION = "1.0"

private val word = Regex("[A-Za-z]+")

// Private to this file.
private fun words(text: String): Sequence<String> = word.findAll(text).map { it.value }

/** The number of words in [text]. A word is a run of ASCII letters. */
fun wordCount(text: String): Int = words(text).count()

/** The first of the longest words in [text]. */
fun longestWord(text: String): String =
    words(text).fold("") { best, word -> if (word.length > best.length) word else best }
```

**Kotlin: `main.kt`**

```kotlin
// main.kt has no package statement, so its names are in the default package.
// It imports the names it uses from textstats; kotlinc compiles both files.
import textstats.VERSION
import textstats.longestWord
import textstats.wordCount

fun main() {
    val text = generateSequence(::readlnOrNull).joinToString("\n")
    println("textstats $VERSION")
    println("words: ${wordCount(text)}")
    println("longest: ${longestWord(text)}")
}
```

All nine print the same, for the two sentences in the test's input:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
textstats 1.0
words: 18
longest: remembered
```

**How each one finds the module.** Only Rust and Go learn where the module is from the
code alone. Rust's `mod textstats;` tells the compiler to read `textstats.rs` next to
`main.rs`, so `rustc main.rs` builds both, and Go's import path is the module path from
`go.mod`, `example.com/stats`, then the folder, `textstats`. Python looks for
`textstats.py` in the folder of the script it is running, and then along `sys.path`.
JavaScript and TypeScript name the file itself, extension and all, `./textstats.js` and
`./textstats.ts`, and Dart names it too. C, C++, and Kotlin don't find anything: the
compiler is given every file at once, `cc main.c textstats.c`, or
`kotlinc main.kt textstats.kt`, and C and C++ need the header as well, which
`#include "textstats.h"` pastes in. That is how the checker builds them, from every
source file in the folder.

**What each one exports.** C's header holds declarations,
`size_t word_count(const char *text);`, which say what exists without the code. C++'s
does the same inside `namespace textstats`, which is why its main program writes
`textstats::word_count`. The others mark what is exported where it is defined: `pub` in
Rust, a capital letter in Go, `export` in JavaScript and TypeScript, and nothing at all
in Python, Dart, and Kotlin, where every name is public unless a leading `_` or
`private` says otherwise.

**How the private helper stays private.** Each module has a helper that splits the text
into words. C makes it `static`, which keeps its name out of the object file's list of
names for other files. C++ puts it in an unnamed namespace, which does the same, and the
other languages use `fn` without `pub` in Rust, a lower-case `words` in Go, `_words` in
Python and Dart, no `export` in JavaScript and TypeScript, and `private` in Kotlin. The
next section shows what happens when the main program tries to call it.

**Constants across files.** C's version is a macro, `TEXTSTATS_VERSION`, which the
preprocessor pastes into `main.c` as the text `"1.0"`. C++'s is `inline constexpr`,
where `inline` lets every file that includes the header have the same definition without
the linker complaining that there are several, as 18.5 shows. The others export a
constant like any other name.

---

## 18.4 Visibility: what a module hides

Here is what each compiler or runtime says when the main program of example 01 calls the
private helper:

| Language | The call | What happens |
| --- | --- | --- |
| C | `next_word(...)`, with no declaration | `error: implicit declaration of function 'next_word'; did you mean 'longest_word'?` |
| C | The same, after declaring `next_word` by hand | It compiles, and the link fails: `undefined reference to 'next_word'` |
| C++ | `textstats::words(...)` | `error: 'words' is not a member of 'textstats'` |
| Rust | `textstats::words(...)` | `error[E0603]: function 'words' is private` |
| Go | `textstats.words(...)` | `undefined: textstats.words` |
| Python | `textstats._words(...)` | It works, and returns `['a', 'b']` |
| JavaScript | `import { words } from "./textstats.js"` | `SyntaxError: The requested module './textstats.js' does not provide an export named 'words'`, before any of the program runs |
| TypeScript | `import { words } from "./textstats.ts"` | `error TS2459: Module '"./textstats.ts"' declares 'words' locally, but it is not exported.` |
| Dart | `textstats._words(...)` | `error - The function '_words' isn't defined.` |
| Kotlin | `import textstats.words` | `error: cannot access 'fun words(text: String): Sequence<String>': it is private in file.` |

The C rows show that C has two kinds of hiding, and they work at different times.
Leaving a function out of the header hides it from the compiler, but only as a courtesy,
since any file can declare the function itself. `static` hides it from the linker, and
that is the one that can't be got around: the name never leaves `textstats.o`. C++ adds
a third, `private`, for the members of a class, and the stock module of exercise 18.1
uses it. Go's message is the same one it gives for a name that doesn't exist at all, and
Python's line shows that its rule is only an agreement. Linters enforce it if asked:
Ruff reports `SLF001 Private member accessed` with its `SLF` rules turned on.

Most languages have more than two levels. These are the ones between "only here" and
"everyone":

| Language | Narrowest | In between | Widest |
| --- | --- | --- | --- |
| C | `static`: this file | | Everything else: the whole program |
| C++ | `private`: this class; unnamed namespace or `static`: this file | `protected`: this class and its subclasses | `public`, and everything outside a class |
| Rust | The default: this module and the modules inside it | `pub(super)`: the parent module; `pub(crate)`: this crate | `pub` |
| Go | Lower case: this package | A package in a folder called `internal` can be imported only from the folder that holds `internal` and the folders inside it | Capitalized |
| Python | `_name`, by agreement | `__all__` lists the names that `from module import *` takes | Everything |
| JavaScript | Not exported: this file; a `#` field: this class | | `export` |
| TypeScript | JavaScript's, and `private`: this class, checked only by `tsc` | `protected` | `export`, `public` |
| Dart | `_name`: this library, which is this file and its `part` files | | Everything else |
| Kotlin | `private`: this file, or this class | `protected`; `internal`: this module | `public`, the default |

Go's `internal` rule is enforced by the `go` command: a program outside
`example.com/lib` that imports `example.com/lib/textstats/internal/split` stops with
`use of internal package example.com/lib/textstats/internal/split not allowed`, while
`example.com/lib/textstats` itself can use it. Kotlin's `internal` means the files
compiled together, which in a Gradle build is one project. In example 02 below,
textstats and the program are two projects, so making textstats's `words` `internal` and
calling it from the program gives
`Cannot access 'fun words(text: String): Sequence<String>': it is internal in file.`

**Hiding a type's insides.** A module can export a type and still keep what is inside it
private, so that other code can hold a value of the type and pass it around, but only
the module's own functions can look inside. That is what exercise 18.1's stock module
does, and each language has a way:

- **C** exports an **opaque type**: the header says only `typedef struct Stock Stock;`,
  and only `stock.c` says what is in the struct. Other files can hold a `Stock *`, but
  `Stock s;` is `error: storage size of 's' isn't known`, and `s->length` is
  `error: invalid use of incomplete typedef 'Stock'`. Since other files can't know how
  big a `Stock` is, they can't make one, so the module provides `stock_new` and
  `stock_free`.
- **C++** declares the member `private`, so using it is
  `error: '... stock::Stock::counts_' is private within this context`. But the member is
  still in the header, so every file that includes it is compiled again when it changes.
  The "pointer to implementation" idiom, or **pimpl**, keeps only a pointer in the
  header, to a struct that the `.cpp` file defines.
- **Rust** and **Go** export the type but not its field:
  `error[E0616]: field 'counts' of struct 'Stock' is private`, and
  `store.counts undefined (cannot refer to unexported field counts)`. Other modules
  can't build a value with fields they can't name, so the module exports a function that
  makes one: `Stock::new()` and `stock.New()`.
- **JavaScript** and **TypeScript** use a `#` field, which no code outside the class can
  name: `s.#counts` is a `SyntaxError` when the file is loaded, and `tsc` reports error
  TS18013, that `'#counts'` is
  `not accessible outside class 'Stock' because it has a private identifier`.
  TypeScript's own `private` keyword is checked by `tsc` and then removed, so at run
  time the field is an ordinary one that any code can read.
- **Dart**'s `_counts` is private to `stock.dart`, and **Kotlin**'s `private val counts`
  to the class: `The getter '_counts' isn't defined for the type 'Stock'`, and
  `cannot access 'val counts: SortedMap<String, Long>': it is private in 'stock.Stock'`.
- **Python**'s `_counts` is private by agreement, and `stock._counts` works.

---

## 18.5 How a build works

### Compiling and linking C

The C compiler works on one **translation unit** at a time: one `.c` file, with every
header it includes pasted in. Pasting is all `#include` does. `main.c` in example 01 is
35 lines long, and after the preprocessor has pasted in `stdio.h`, `stdlib.h`, and
`textstats.h`, the translation unit is 1,983 lines, as `cc -E main.c | wc -l` shows. The
compiler turns that into `main.o` without ever looking at `textstats.c`. All it knows
about `word_count` is the declaration from the header, which is enough to compile a call
to it.

An object file keeps a table of **symbols**: the names it defines, and the names it uses
but doesn't define, which it leaves for the linker. `nm` lists them. `T` is a function
defined here and visible to other files, `t` is one defined here and private to the
file, and `U` is a name this file uses that another file has to define:

```console
$ cc -std=c17 -Wall -Wextra -Werror -O2 -c textstats.c
$ cc -std=c17 -Wall -Wextra -Werror -O2 -c main.c
$ nm textstats.o
                 U __ctype_b_loc
                 U __stack_chk_fail
00000000000000e0 T longest_word
                 U memcpy
0000000000000000 t next_word
0000000000000070 T word_count
$ nm main.o | grep -E "word|main"
                 U longest_word
0000000000000000 T main
                 U word_count
```

The lower-case `t` is what `static` does: `next_word` is in `textstats.o`, but marked as
local, so the linker won't connect anything else to it. `__ctype_b_loc` is how glibc
implements `isalpha`, and `memcpy` comes from the C library too, which the linker adds
on its own. The linker's job is to find a `T` for every `U`, and its two errors are the
two ways that can fail:

```console
$ cc main.o -o main
/usr/bin/ld: main.o: in function `main':
main.c:(.text.startup+0xc3): undefined reference to `longest_word'
/usr/bin/ld: main.c:(.text.startup+0xe5): undefined reference to `word_count'
collect2: error: ld returned 1 exit status
```

That is a name used and never defined, here because `textstats.o` was left out. The
other error is a name defined twice. A second file that defines its own `word_count`
compiles without complaint, and then the link fails with
`multiple definition of 'word_count'`, and says where it was `first defined here`:
`textstats.o:textstats.c:(.text+0x70)`. Each file was fine on its own, so only the
linker, which sees them all, can notice. A second file could have its own `static`
`next_word`, though, since the linker never sees either one. That is why C programmers
make every function `static` unless other files need it: each name that isn't static is
a name that no other file in the whole program may use.

### Headers and include guards

A header is the part of a module that other files paste in, so it should hold only what
they need to compile against it: declarations of functions, definitions of types,
macros, and constants. It shouldn't hold the definition of a function or a variable,
since every file that includes it would define the name again, and the link would fail
with `multiple definition`.

A header can end up pasted twice into one translation unit, when a file includes it and
also includes another header that includes it. For declarations alone that is harmless:
with its include guard taken out, `textstats.h` can be included twice and the file still
compiles, since C allows the same function to be declared twice. For a type it isn't. A
`point.h` that defines `struct point`, included directly and again through `shape.h`,
gives:

```text
In file included from shape.h:1,
                 from use.c:2:
point.h:1:8: error: redefinition of 'struct point'
```

The **include guard** in `textstats.h` prevents that. The first time the header is
pasted in, `TEXTSTATS_H` isn't defined, so `#ifndef` lets the rest through, and the next
line defines it. The second time, `#ifndef` skips everything down to `#endif`. C++'s
`#pragma once` does the same with one line. It isn't in the C or C++ standard, but GCC,
Clang, and MSVC all support it.

Guards stop a header from being pasted twice, but they can't fix two headers that need
each other. If `order.h` includes `customer.h` and defines `struct order` with a
`struct customer` inside it, and `customer.h` includes `order.h` to use
`struct order *`, then a file that includes `order.h` first compiles, and a file that
includes `customer.h` first doesn't. Its guard lets `order.h` in before
`struct customer` is defined: `order.h:7:21: error: field 'buyer' has incomplete type`.
The fix is to break the cycle. `customer.h` needs only a pointer, and a pointer to a
struct needs only the line `struct order;`, which says the type exists without including
anything.

### The one-definition rule in C++

C++ has the same rule as C under a name, the **one-definition rule**: each function and
variable is defined once in the whole program. It also has more things that have to go
in headers, such as templates and the bodies of small functions that should be inlined,
so it has an exception. A function or variable marked `inline` may be defined in every
translation unit that includes it, as long as every definition is the same, and the
linker keeps one. So in a header, `int calls = 0;` gives
`multiple definition of 'calls'` as soon as two files include it, and
`inline int calls = 0;` gives one variable that both files share. The same goes for a
function defined in a header without `inline`: `multiple definition of 'twice(int)'`.
Member functions defined inside a class are `inline` without saying so, and so are
templates and `constexpr` functions. A `const` variable at namespace scope is private to
each file in C++, so `const int limit = 10;` in a header gives each file its own copy,
and links.

### make and CMake

A build tool decides what to compile again after a change. `make` reads a **Makefile**,
a list of **targets**, each with the files it depends on and the command that makes it:

```make
CFLAGS = -std=c17 -Wall -Wextra -Werror -O2

# main is made from the two object files, by linking them.
main: main.o textstats.o
	$(CC) main.o textstats.o -o main

# Each object file is made from its .c file, and depends on the headers it includes.
main.o: main.c textstats.h
	$(CC) $(CFLAGS) -c main.c

textstats.o: textstats.c textstats.h
	$(CC) $(CFLAGS) -c textstats.c
```

`make` remakes a target when it is older than any file it depends on, and remakes those
first. So after a change to `textstats.c`, it compiles that one file and links. After a
change to `textstats.h`, it compiles both, since both include it. And if nothing
changed, it does nothing:

```console
$ make
cc -std=c17 -Wall -Wextra -Werror -O2 -c main.c
cc -std=c17 -Wall -Wextra -Werror -O2 -c textstats.c
cc main.o textstats.o -o main
$ make
make: 'main' is up to date.
$ touch textstats.c
$ make
cc -std=c17 -Wall -Wextra -Werror -O2 -c textstats.c
cc main.o textstats.o -o main
$ touch textstats.h
$ make
cc -std=c17 -Wall -Wextra -Werror -O2 -c main.c
cc -std=c17 -Wall -Wextra -Werror -O2 -c textstats.c
cc main.o textstats.o -o main
```

The weak point is the header lists. They are written by hand, and a header missing from
one means a change to it doesn't rebuild the files that use it, which then run with an
old idea of a struct's layout. Compilers can write those lists themselves, with `-MMD`,
and most real Makefiles use that, or don't write Makefiles at all.

**CMake** is what most C and C++ projects use instead, and it is what chapter 1 said a
real C or C++ project starts with. It doesn't build anything itself. It reads a
`CMakeLists.txt`, finds the compiler, and writes a build for another tool, such as make
or Ninja, with the header lists worked out for you. Here are example 02's, in which
textstats is a library in a folder of its own:

**C: `CMakeLists.txt`**

```cmake
# The program's build: CMake reads this and writes a build for make or Ninja.
cmake_minimum_required(VERSION 3.20)
project(stats LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror)

# The library has a CMakeLists.txt of its own, which defines the target textstats.
add_subdirectory(textstats)

add_executable(main main.c)
# Linking to textstats also gives main its include folder, which it made PUBLIC.
target_link_libraries(main PRIVATE textstats)
```

**C: `textstats/CMakeLists.txt`**

```cmake
# A static library: its object files, bundled into libtextstats.a.
add_library(textstats STATIC textstats.c)
# PUBLIC: this library and everything that links to it see include/.
target_include_directories(textstats PUBLIC include)
```

A **target** is a program or a library, and
`target_link_libraries(main PRIVATE textstats)` says that main uses textstats. That one
line links the library in, and also gives `main.c` the library's `include` folder,
because the library made it `PUBLIC`, which means "for me and for whatever links to me".
`PRIVATE` would mean "for me only", which is what main says about textstats, since
nothing links to main. `add_library(textstats STATIC ...)` makes a **static library**,
`libtextstats.a`, which is an archive of object files that the linker copies into each
program that uses it. A **shared library**, `.so` on Linux, `.dylib` on macOS, and
`.dll` on Windows, is loaded when the program starts instead, so several programs can
share one copy, and chapter 23 uses them to call C from the other languages.

The checker builds it with `cmake -S . -B build` and `cmake --build build`, and this is
what the second command prints, first from nothing, then with nothing changed, and then
after a change to the library's `.c` file and to its header:

```console
$ cmake --build build
[ 25%] Building C object textstats/CMakeFiles/textstats.dir/textstats.c.o
[ 50%] Linking C static library libtextstats.a
[ 50%] Built target textstats
[ 75%] Building C object CMakeFiles/main.dir/main.c.o
[100%] Linking C executable main
[100%] Built target main
$ cmake --build build
[ 50%] Built target textstats
[100%] Built target main
$ echo "// changed" >> textstats/textstats.c
$ cmake --build build
[ 25%] Building C object textstats/CMakeFiles/textstats.dir/textstats.c.o
[ 50%] Linking C static library libtextstats.a
[ 50%] Built target textstats
[ 75%] Linking C executable main
[100%] Built target main
$ echo "// changed" >> textstats/include/textstats.h
$ cmake --build build
[ 25%] Building C object textstats/CMakeFiles/textstats.dir/textstats.c.o
[ 50%] Linking C static library libtextstats.a
[ 50%] Built target textstats
[ 75%] Building C object CMakeFiles/main.dir/main.c.o
[100%] Linking C executable main
[100%] Built target main
```

A change to `textstats.c` rebuilds the library and links main again, but doesn't compile
`main.c`. A change to the header compiles both, without anyone having listed the header
as a dependency. In a big C++ project that difference is the whole game: a header that a
thousand files include costs a thousand compilations each time it changes, which is why
C++ projects keep headers small, and why pimpl exists.

### The units of the other languages

The other languages draw the unit of compilation bigger, and work out the dependencies
themselves:

- **Rust** compiles one **crate** at a time, a library or a program with all its
  modules, which is why `rustc main.rs` is enough for example 01. Cargo builds each
  crate a program depends on separately, in parallel where it can, and keeps the results
  in `target/` for the next build. Within a crate, the compiler keeps notes of its own,
  so a small change doesn't redo everything.
- **Go** compiles one **package** at a time, and keeps the results in a cache keyed by a
  hash of each package's source files and everything that went into compiling it. So
  `touch` changes nothing: after touching every `.go` file in example 02, `go build -x`
  runs no compiler at all. A real change to `main.go` runs it once, for the package
  `main`.
- **Kotlin** compiles the files of one module together into `.class` files, one for each
  class. Top-level functions go into a class named after their file, so `main.kt`'s
  become `MainKt`, which is why example 02's build names `MainKt` as the main class.
  Gradle builds each project of a build separately and checks a hash of its inputs, so a
  touched file leaves the task `UP-TO-DATE`.
- **Python** compiles each module to bytecode the first time it is imported, and saves
  it in `__pycache__`, so after one run of example 01 there is a
  `textstats.cpython-313.pyc`. The script Python was asked to run isn't saved, only what
  it imports.
- **JavaScript** has no build step at all: Node parses each module when it is first
  imported. **TypeScript** adds one, `tsc`, which checks types across every file the
  program imports. Node removes the types and runs the files as they are.
- **Dart** compiles the program each time `dart run` starts it, and `dart compile exe`
  compiles the whole program ahead of time into one executable.

Cargo still compares the times files were changed, as `make` does: touching
`textstats/src/lib.rs` in example 02 rebuilds textstats and the program that depends on
it.

---

## 18.6 Packages and dependencies

Example 02 is example 01 as two packages: textstats is a library in a folder of its own,
with its own manifest, and the program depends on it. The dependency is a **path
dependency**, one that names a folder instead of a registry, so getting it needs no
network, and the checker builds the Cargo, npm, and pub projects with `--offline`. Only
Gradle goes online, the first time, for the Kotlin plugin and the Kotlin library. A
dependency from a registry would change only the line that names the folder, or in
Kotlin the project. Here are the manifests, the program's first and then the library's:

**Rust: `Cargo.toml`**

```toml
[package]
name = "stats"
version = "0.1.0"
edition = "2024"

# The program's name; it would be "stats", after the package, without this.
[[bin]]
name = "main"
path = "src/main.rs"

[dependencies]
# A path dependency: the crate in the folder textstats, not one from crates.io.
textstats = { path = "textstats" }
```

**Rust: `textstats/Cargo.toml`**

```toml
[package]
name = "textstats"
version = "1.0.0"
edition = "2024"
```

**Go: `go.mod`**

```text
module example.com/stats

go 1.23

require example.com/textstats v1.0.0

// Use the module in ./textstats instead of downloading example.com/textstats.
replace example.com/textstats => ./textstats
```

**Go: `textstats/go.mod`**

```text
module example.com/textstats

go 1.23
```

**Python: `pyproject.toml`**

```toml
[project]
name = "stats"
version = "0.1.0"
requires-python = ">=3.12"
dependencies = ["textstats"]

# This project is a program, not a package to install, so uv only installs its
# dependencies.
[tool.uv]
package = false

# Where uv gets textstats: from the folder, instead of from PyPI.
[tool.uv.sources]
textstats = { path = "textstats" }
```

**Python: `textstats/pyproject.toml`**

```toml
[project]
name = "textstats"
version = "1.0.0"
requires-python = ">=3.12"

# The tool that turns the folder into a package that pip or uv can install.
[build-system]
requires = ["uv_build>=0.11,<0.12"]
build-backend = "uv_build"
```

**JavaScript: `package.json`**

```json
{
  "name": "stats",
  "private": true,
  "type": "module",
  "dependencies": {
    "textstats": "file:./textstats"
  }
}
```

**JavaScript: `textstats/package.json`**

```json
{
  "name": "textstats",
  "version": "1.0.0",
  "type": "module",
  "exports": "./index.js"
}
```

**Dart: `pubspec.yaml`**

```yaml
name: stats
publish_to: none
environment:
  sdk: ^3.9.0
dependencies:
  # A path dependency: the package in the folder textstats, not one from pub.dev.
  textstats:
    path: textstats
```

**Dart: `textstats/pubspec.yaml`**

```yaml
name: textstats
version: 1.0.0
environment:
  sdk: ^3.9.0
```

**Kotlin: `settings.gradle.kts`**

```kotlin
// The build has two projects: this folder's, the program, and textstats.
rootProject.name = "main"
include("textstats")

// Where Gradle finds its plugins and the libraries they need.
pluginManagement {
    repositories {
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositories {
        mavenCentral()
    }
}
```

**Kotlin: `build.gradle.kts`**

```kotlin
plugins {
    kotlin("jvm") version "2.4.21"
    // application adds tasks to run the program and to package it with a script.
    application
}

dependencies {
    // A dependency on the other project of this build, not on a published library.
    implementation(project(":textstats"))
}

kotlin {
    compilerOptions {
        allWarningsAsErrors = true
    }
}

application {
    // Top-level functions in main.kt compile into a class called MainKt.
    mainClass = "MainKt"
}
```

**Kotlin: `textstats/build.gradle.kts`**

```kotlin
plugins {
    kotlin("jvm")
}

kotlin {
    compilerOptions {
        allWarningsAsErrors = true
    }
}
```

From a registry, each of those lines would name a version instead of a folder, or in
Kotlin a project: `textstats = "1.0"` in `Cargo.toml`, `"textstats": "^1.0.0"` in
`package.json`, `textstats: ^1.0.0` in `pubspec.yaml`, and
`implementation("com.example:textstats:1.0.0")` in Kotlin, which names the library's
group as well as its name. Python's `dependencies` would stay as it is, without the
`[tool.uv.sources]` table that says where to find it, and Go's `require` line already
names a version, so only the `replace` line would go.

TypeScript's manifests are JavaScript's, except that the library's `"exports"` is
`./index.ts`, since Node runs it as it is. The C and C++ builds are the `CMakeLists.txt`
files above. The program code doesn't change from example 01 except in how it names the
module. Rust's main program no longer has a `mod` line, since a dependency's crate is in
scope by its name. Go's imports `example.com/textstats`, JavaScript's and TypeScript's
`"textstats"`, a bare name, which Node looks up in `node_modules`, and Dart's
`package:textstats/textstats.dart`, a file in the package's `lib` folder. All of them
print what example 01 printed.

**What a package manager does with that.** Each tool reads the manifest, works out every
package the program needs, gets them, and puts them where the compiler or runtime will
look:

| Language | Build command | What it does with the dependency |
| --- | --- | --- |
| C, C++ | `cmake -S . -B build`, `cmake --build build` | Builds `libtextstats.a` and links it into `main` |
| Rust | `cargo build --release` | Builds the textstats crate, then the program against it, into `target/release/main` |
| Go | `go build` | The `replace` line points `example.com/textstats` at the folder; `go list -m all` shows `example.com/textstats v1.0.0 => ./textstats` |
| Python | `uv sync`, then `uv run main.py` | Builds textstats into a package with `uv_build`, and installs it into a **virtual environment**, `.venv`, a folder with its own copy of the interpreter's package list |
| JavaScript, TypeScript | `npm install` | Makes `node_modules/textstats` a link to `../textstats` |
| Dart | `dart pub get` | Writes `.dart_tool/package_config.json`, which tells the compiler that `package:textstats/` is `../textstats/lib/` |
| Kotlin | `gradle installDist` | Builds `textstats.jar`, then `main.jar`, and puts both in `build/install/main/lib`, with the Kotlin library and a script, `bin/main`, that runs the program |

**A dependency from a registry.** In a real project, the library would be published, and
the program's manifest would name a version range instead of a path. Each tool adds a
dependency with one command, and writes a range of its own choosing, from the newest
version there is. These are the versions they chose in November 2026:

| Language | Add a dependency | Writes | Which means |
| --- | --- | --- | --- |
| Rust | `cargo add rand` | `rand = "0.10.4"` | `^0.10.4`, which is `>=0.10.4 <0.11.0`: Cargo's ranges are carets unless they say otherwise |
| Go | `go get golang.org/x/text` | `require golang.org/x/text v0.44.0 // indirect` | At least v0.44.0, and `// indirect` until some code imports it |
| Python | `uv add requests` | `"requests>=2.34.2"` | At least 2.34.2, with no upper end |
| JavaScript, TypeScript | `npm install lodash` | `"lodash": "^4.18.1"` | `>=4.18.1 <5.0.0-0` |
| Dart | `dart pub add args` | `args: ^2.7.0` | `>=2.7.0 <3.0.0` |
| Kotlin | Written by hand, in `dependencies { }` | `implementation("com.google.guava:guava:33.4.5-jre")` | Exactly that version, unless another dependency asks for a newer one |

`go get` also raised the `go` line in `go.mod`, from 1.24.7 to 1.26.0, since that
version of `x/text` needs a newer Go than the module had asked for.

C and C++ have no row. Their libraries come from the operating system's package manager,
from a C++ package manager such as vcpkg or Conan, or from CMake itself, whose
`FetchContent` downloads a library's source at a fixed version and builds it with the
program.

Exercise 18.3 writes npm's range rules. They are worth knowing exactly, since the same
caret means different things in different places. `^1.2.3` allows anything below 2.0.0,
but `^0.2.3` allows only 0.2.x, because before 1.0.0, Semantic Versioning says anything
may change, and npm treats the first number that isn't 0 as the one that signals a
breaking change. Cargo and pub do the same.

**Choosing the versions.** A program's dependencies have dependencies of their own, and
the package manager has to choose one version of each package that every range allows.
That is **dependency resolution**, and the tools do it two ways:

- **The newest version that fits.** Cargo, npm, uv, pub, and Gradle pick the highest
  version each range allows, so the same manifest can resolve to newer versions next
  month than today. That is why they write lock files.
- **The oldest version that fits.** Go's **minimal version selection** picks, for each
  module, the highest of the minimum versions that anything requires, and nothing newer
  than that. If the program requires textstats v1.2.0 and a library it uses requires
  v1.3.0, the build gets v1.3.0, even if v1.9.0 exists. The result depends only on the
  `go.mod` files, never on what was published since, so Go needs no lock file to get the
  same build twice. Its `go.sum` holds a hash of each module's contents, so that a
  module changed after the fact, on the server or on the way, is caught.

**Diamonds.** Sometimes two dependencies need different versions of a third: the program
uses `a` and `b`, `a` needs `c` 1.x, and `b` needs `c` 2.x. npm installs both, with one
copy of `c` in `node_modules/a/node_modules`. Cargo builds both too, as long as they are
semver-incompatible, such as 1.x and 2.x, or 0.7 and 0.8, but a value of a type from one
`c` can't be passed to code that expects the type from the other, since they are
different crates. Go gives a module's major versions from 2 on different import paths,
`example.com/c/v2`, so both fit. Python, Dart, and the JVM can load only one version of
a package, so uv and pub report that no version satisfies both, and Gradle quietly picks
the newer one, which `a` may not work with.

### Lock files

A lock file records the exact version of every package the program uses, directly or
not, and usually a hash of each one's contents. Here are example 02's, which are short
because the only dependency is a folder:

**Rust: `Cargo.lock`**

```text
# This file is automatically @generated by Cargo.
# It is not intended for manual editing.
version = 4

[[package]]
name = "stats"
version = "0.1.0"
dependencies = [
 "textstats",
]

[[package]]
name = "textstats"
version = "1.0.0"
```

**Python: `uv.lock`**

```text
version = 1
revision = 3
requires-python = ">=3.12"

[[package]]
name = "stats"
version = "0.1.0"
source = { virtual = "." }
dependencies = [
    { name = "textstats" },
]

[package.metadata]
requires-dist = [{ name = "textstats", directory = "textstats" }]

[[package]]
name = "textstats"
version = "1.0.0"
source = { directory = "textstats" }
```

**JavaScript: `package-lock.json`**

```json
{
  "name": "stats",
  "lockfileVersion": 3,
  "requires": true,
  "packages": {
    "": {
      "name": "stats",
      "dependencies": {
        "textstats": "file:./textstats"
      }
    },
    "node_modules/textstats": {
      "resolved": "textstats",
      "link": true
    },
    "textstats": {
      "version": "1.0.0"
    }
  }
}
```

**Dart: `pubspec.lock`**

```text
# Generated by pub
# See https://dart.dev/tools/pub/glossary#lockfile
packages:
  textstats:
    dependency: "direct main"
    description:
      path: textstats
      relative: true
    source: path
    version: "1.0.0"
sdks:
  dart: ">=3.9.0 <4.0.0"
```

With a registry, each entry would have a version from the registry, a `checksum` or
`integrity` hash, and the URL it came from. The tools use lock files in two ways. An
ordinary install, `npm install` or `cargo build`, uses the versions in the lock file if
they still satisfy the manifest, and updates the lock file if they don't, or if the
manifest changed. A strict install fails instead of changing anything: `npm ci`,
`uv sync --locked`, and `cargo build --locked`. CI should use the strict one, so that a
build that would differ from yours stops and says so, and the checker uses `npm ci`,
`uv sync --locked`, and `dart pub get --offline`, which install exactly what is in the
lock file.

Chapter 17's table of what to commit had the lock files on the right. Commit them.
Cargo's guidance once said to commit `Cargo.lock` only for programs, and since 2023 it
says to commit it for libraries too, so that their CI builds are repeatable. A library's
lock file doesn't reach the programs that use it, which resolve versions for themselves.
The exception is the Gradle wrapper, which isn't a lock file for libraries but for
Gradle itself. `gradle init` writes `gradlew`, `gradlew.bat`, and
`gradle/wrapper/gradle-wrapper.properties`, whose `distributionUrl` names one release,
`gradle-8.14.3-bin.zip`, so that `./gradlew build` downloads and uses that Gradle on
every machine. Example 02 runs the `gradle` that CI installs instead, so that the
repository holds no binary `gradle-wrapper.jar`.

### Starting a project

Chapter 1 listed a command that starts a real project in each language. Here is what
each one makes:

| Language | Command | Makes |
| --- | --- | --- |
| C, C++ | None: write `CMakeLists.txt` by hand | |
| Rust | `cargo new hello` | `Cargo.toml`, `src/main.rs`, `.gitignore`, and a Git repository |
| Go | `go mod init example.com/hello` | `go.mod`, with the module path and the Go version |
| Python | `uv init hello` | `pyproject.toml`, `main.py`, `README.md`, `.python-version`, `.gitignore`, and a Git repository |
| JavaScript, TypeScript | `npm init -y` | `package.json` |
| Dart | `dart create hello` | `pubspec.yaml`, `bin/hello.dart`, `lib/hello.dart`, a test, `analysis_options.yaml`, `.gitignore`, a README and a changelog, and it runs `dart pub get` |
| Kotlin | `gradle init --type kotlin-application` | `settings.gradle.kts`, `app/build.gradle.kts`, a source file and a test, a version catalog in `gradle/libs.versions.toml`, `.gitignore`, and the wrapper |

`npm init -y`'s `package.json` has no `"type"`, so Node treats its `.js` files as
CommonJS, which 18.9 is about. Add `"type": "module"`, as every package in this book
does.

---

## 18.7 When a module's code runs

A module can have code at its top level, outside any function: a constant whose value
needs a function call, a table that is built once, or a line that prints. Example 03 has
a module `settings` whose value is made by a function that prints `settings: loading`,
and a main program that prints before using it:

**C: `settings.c`**

```c
#include "settings.h"

#include <stdio.h>

const char *settings_greeting(void) {
    // A static variable inside a function keeps its value between calls.
    static const char *greeting = NULL;
    if (greeting == NULL) {
        puts("settings: loading");
        greeting = "Hello";
    }
    return greeting;
}
```

**C++: `settings.cpp`**

```cpp
#include "settings.hpp"

#include <iostream>

namespace settings {

namespace {

std::string load() {
    std::cout << "settings: loading\n";
    return "Hello";
}

}  // namespace

// A global whose value needs code is set up before main runs. With several
// files, the order in which their globals are set up isn't defined.
const std::string greeting = load();

}  // namespace settings
```

**Rust: `settings.rs`**

```rust
use std::sync::LazyLock;

// Rust runs no code before main. A LazyLock runs its closure the first time it is
// used, once, even if several threads use it at the same moment.
pub static GREETING: LazyLock<String> = LazyLock::new(|| {
    println!("settings: loading");
    String::from("Hello")
});
```

**Go: `settings/settings.go`**

```go
// Package settings has a variable whose value comes from a function call.
package settings

import "fmt"

// Package variables are set before main runs, and before the packages that
// import this one are set up, so a package's dependencies are always ready.
var Greeting = load()

func load() string {
	fmt.Println("settings: loading")
	return "Hello"
}
```

**Python: `settings.py`**

```python
# The first import of a module runs its top-level code, from top to bottom. Later
# imports find it in sys.modules and don't run it again.
print("settings: loading")
GREETING = "Hello"
```

**JavaScript: `settings.js`**

```js
// A module's top-level code runs once, before the code of any module that
// imports it.
console.log("settings: loading");
export const GREETING = "Hello";
```

**Dart: `settings.dart`**

```dart
// A top-level variable is set the first time it is read, not when the program
// starts, so a library costs nothing until it is used.
final greeting = _load();

String _load() {
  print('settings: loading');
  return 'Hello';
}
```

**Kotlin: `settings.kt`**

```kotlin
package settings

// Top-level properties of a file are set when the file's class is first used,
// the first time anything in the file is read or called.
val greeting = load()

private fun load(): String {
    println("settings: loading")
    return "Hello"
}
```

TypeScript's is JavaScript's with a type. Each main program prints `main: start`, then
the greeting twice:

*C, Rust, Dart and Kotlin:*

```text
main: start
settings: loading
main: the greeting is Hello
main: the greeting is still Hello
```

*C++, Go, Python, JavaScript and TypeScript:*

```text
settings: loading
main: start
main: the greeting is Hello
main: the greeting is still Hello
```

The languages split into two groups, and the split is a choice between surprise and
cost:

- **Before main.** C++, Go, Python, JavaScript, and TypeScript run a module's top-level
  code when the program loads it, before the code that imported it goes on. Go runs
  every package's variables and `init` functions before `main`, each package after the
  packages it imports, so a package can rely on its dependencies being ready. Python
  runs a module's code the first time it is imported, and keeps the module in
  `sys.modules`, so later imports get the same module without running it again.
  JavaScript runs each module once, before the module that imports it, and imports are
  **hoisted**: `settings.js` runs before the first line of `main.js`, wherever in
  `main.js` the `import` is written.
- **When first used.** C, Rust, Dart, and Kotlin run nothing before `main`. C has no way
  to, so the C version keeps a `static` inside a function, which is set the first time
  the function is called. Rust doesn't allow code before `main` either, and `LazyLock`
  runs its closure the first time the value is used, once, even if several threads use
  it at the same moment. Dart sets a top-level variable the first time it is read, and
  Kotlin's top-level properties are set when the JVM first uses the class that their
  file compiles into.

C++ has one rule that makes its group the dangerous one. Globals in one file are set up
in the order they are written, but between files the order isn't specified. A global in
one file whose value uses a global from another can see it before it has been set up.
This is known as the **static initialization order fiasco**, and the usual fix is C's:
put the value in a function, as a `static` local, which is set up the first time the
function is called. Since C++11 that is also safe with threads.

---

## 18.8 Why each language looks the way it does

- **C** was compiled one file at a time from the start, in the early 1970s, on machines
  that couldn't hold a whole program in memory, and joined by a linker that was already
  there for assembly language. `#include` was the simplest way to share declarations:
  paste them in. With no namespaces, C libraries put a prefix on every name, as
  `stock_new` and `textstats_` would, and as `pthread_create` and `SDL_Init` do.
- **C++** kept C's model, so that it could use C's headers and linkers, and added
  namespaces, classes with `private` members, and the one-definition rule. Templates
  have to be in headers, so a C++ program pastes in and compiles the same code in every
  file that includes it, which is the main reason big C++ projects take so long to
  build. C++20's modules are the fix, after decades of headers: a module is compiled
  once, and exports only what it says.
- **Rust** came with Cargo, crates.io, and `Cargo.lock` from 1.0, in 2015, designed by
  people who had built package managers for Ruby, and with Semantic Versioning required.
  Modules are private by default because a public name is a promise, and Rust's
  designers wanted promises made on purpose.
- **Go** was designed at Google in 2007 partly because C++ builds of Google's code took
  most of an hour. A Go package is compiled once and its result reused. Imports can't
  form a cycle, so packages can always be compiled in order, and an unused import is a
  compile error, so a file never pays for a package it doesn't use. A capital letter
  marks an exported name, so a reader can tell at every use whether a name comes from
  outside. Modules and `go.mod` replaced one shared workspace, `GOPATH`, in Go 1.11, in
  2018, with minimal version selection, which Russ Cox designed to need no lock file.
- **Python** had modules from its first release in 1991, and packaging grew around them
  in layers: `distutils`, setuptools, pip in 2008, wheels, `pyproject.toml` in 2016 and
  its `[project]` table in 2020, and uv, from Astral, in 2024. A leading underscore is
  private by agreement because Python's view is that "we are all consenting adults
  here".
- **JavaScript** had no modules for its first twenty years: every script in a page
  shared one global scope. Node brought CommonJS, `require` and `module.exports`, in
  2009, and npm in 2010, and the language standardized its own modules, `import` and
  `export`, in ES2015. Node ran them without a flag from 2019, and supporting both kinds
  is why Node still asks each package which kind its `.js` files are. npm's lock file
  arrived in npm 5, in 2017.
- **TypeScript** follows whatever module system its JavaScript runs under, so its
  settings for modules, such as `--module nodenext`, are about telling it which one that
  is.
- **Dart** had packages and pub from its early releases, and a leading `_` for privacy,
  so that whether a name is private can be seen wherever it is used, with no keyword to
  look up.
- **Kotlin** took packages, `.class` files, jars, Maven Central, and Gradle from the
  Java world, and added file-level `private` and `internal`, which Java doesn't have.
  Gradle's build scripts were written in Groovy at first. Since Gradle 8.2, in 2023, new
  builds use Kotlin, as the `.kts` files here do.

---

## 18.9 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | A header that defines a variable or a function, included by two files | `multiple definition of '...'` when they are linked |
| C | A header with a type in it and no include guard | `error: redefinition of 'struct point'` when it is pasted in twice |
| C | Two headers that include each other | Whether a file compiles depends on which it includes first: `error: field 'buyer' has incomplete type`; declare `struct order;` instead |
| C | A helper that isn't `static` | Its name is taken in every other file of the program: `multiple definition of 'next_word'` the day another file has one |
| C | A source file left out of the build | `undefined reference to '...'`, from the linker, not the compiler |
| C | A Makefile that doesn't list a header | A changed struct, and old object files that don't know; generate the lists with `-MMD`, or use CMake |
| C++ | A non-`inline` variable or function defined in a header | `multiple definition of 'calls'` |
| C++ | One global's value using another file's global | Either order is allowed; put it in a function as a `static` local |
| Rust | Calling a function without `pub` from another module | `error[E0603]: function 'words' is private` |
| Rust | Two crates that depend on each other | `error: cyclic package dependency: package 'customers v0.1.0 (...)' depends on itself`; modules inside one crate may use each other |
| Go | Two packages that import each other | `imports example.com/shop/orders from customers.go: import cycle not allowed`; move what both need to a third package |
| Go | A lower-case name used from another package | `undefined: textstats.words`, as if it didn't exist |
| Python | Two modules that each run `from other import NAME` at the top | In Python 3.13, `ImportError: cannot import name 'SHOP' from 'orders' (consider renaming '.../orders.py' if it has the same name as a library you intended to import)`, though nothing needs renaming; write `import orders` and use `orders.SHOP` |
| Python | `from . import textstats` in a script run as `python3 main.py` | `ImportError: attempted relative import with no known parent package`; relative imports work only inside a package, run with `python3 -m` |
| Python | A file named like a library, such as `random.py` next to your script | It is imported instead of the library, since the script's folder comes first on `sys.path`; Python 3.13 says so when a name is missing: `AttributeError: module 'random' has no attribute 'randint' (consider renaming '.../random.py' since it has the same name as the standard library module named 'random' ...)` |
| JavaScript | `import` in a package whose `package.json` has no `"type"` | Node 22 runs it as an ES module after a warning: `Reparsing as ES module because module syntax was detected. This incurs a performance overhead.` |
| JavaScript | `require` in an ES module | `ReferenceError: require is not defined in ES module scope, you can use import instead` |
| JavaScript | `import` in a `.js` file of a `"type": "commonjs"` package | `SyntaxError: Cannot use import statement outside a module` |
| JavaScript | Two modules that import each other, one using the other's `const` at its top level | `ReferenceError: Cannot access 'NAME' before initialization` |
| TypeScript | `import ... from "./textstats"`, with no extension, in an ES module | `error TS2835: Relative import paths need explicit file extensions ... Did you mean './textstats.js'?` |
| TypeScript | Following that advice and running the `.ts` file with Node | `Error [ERR_MODULE_NOT_FOUND]: Cannot find module '.../textstats.js'`; `tsc` maps `.js` to `.ts`, and Node doesn't; import `./textstats.ts`, with `--allowImportingTsExtensions` |
| TypeScript | `private` for a secret field | Removed when the types are, so any JavaScript can read it; use `#` |
| Dart | Expecting `_name` to be private to a class | It is private to the file, so other classes in the file can use it |
| Kotlin | `internal` across Gradle projects | `Cannot access '...': it is internal in file.`; each project is a module |
| All | Committing `node_modules/`, `target/`, `.venv/`, or `build/` | Megabytes of files that any machine can make again; commit the lock file instead |
| All | No lock file, or a non-strict install in CI | A build that passes for you and fails on CI, or the other way round, after a dependency releases a new version |

---

## 18.10 Idiom check

**C: a header, a source file, and `static`.** Give each module a header with an include
guard, holding only declarations, types, and macros, and a `.c` file that includes its
own header first, so the compiler checks that they agree. Make every function `static`
unless the header declares it, prefix the exported names with the module's name, and
hide a struct's fields with an opaque type when other files don't need them. Build with
CMake, with a target for each library.

**C++: namespaces, small headers, and CMake.** Put each module in a namespace, keep
headers to declarations and what has to be there, such as templates and `inline`
functions, and put file-private helpers in an unnamed namespace. Use `#pragma once`, and
pimpl for classes whose insides change often. Build with CMake, and get libraries from
vcpkg, Conan, or `FetchContent`. Try C++20 modules where your compiler and CMake support
them.

**Rust: private by default, Cargo for everything.** Use `mod` to split a crate into
files, and `pub` only for what other modules need, with `pub(crate)` for what the crate
shares inside itself. Split a big program into several crates in a Cargo **workspace**.
Commit `Cargo.lock`, and build CI with `--locked`.

**Go: small packages, capital letters, and `internal`.** One folder is one package,
named for what it provides, with a short exported API and everything else lower case.
Put packages that only your module may use under `internal/`. Start with `go mod init`,
add dependencies with `go get`, run `go mod tidy` to keep `go.mod` and `go.sum` in step
with the code, and commit both.

**Python: packages, `pyproject.toml`, and uv.** Split code into modules and packages,
mark private names with `_`, and list the public ones in `__all__` for a library.
Describe the project in `pyproject.toml`, manage it with uv, which keeps a virtual
environment and `uv.lock`, and commit the lock file. Run a module inside a package with
`python3 -m package.module`, so that relative imports work.

**JavaScript: ES modules, and `npm ci`.** Set `"type": "module"`, use `import` and
`export` with full file names, and export only what other files need. Give a library an
`"exports"` field, which also keeps other packages from importing its internal files.
Commit `package-lock.json`, and install with `npm ci` in CI.

**TypeScript: JavaScript's modules, checked.** Use `--module nodenext`, so that `tsc`
checks imports the way Node resolves them. Import `.ts` files with their extension and
`--allowImportingTsExtensions` when Node runs the TypeScript itself, or `.js` when `tsc`
compiles to JavaScript first. Use `#` for fields that must be private at run time.

**Dart: a package, `lib/`, and `_`.** Put a package's public libraries in `lib/`, and
the rest in `lib/src/`, which other packages shouldn't import, and export the public
parts from one library named after the package. Use `_` for private names, and
`dart pub add` for dependencies. Commit `pubspec.lock` for a program.

**Kotlin: packages and Gradle projects.** Name packages after the project, use `private`
for file helpers and `internal` for what one Gradle project shares inside itself, and
split a big program into several projects in one build. Use the Gradle wrapper, and
commit it, so everyone builds with the same Gradle, and keep dependency versions in one
place, the version catalog.

---

## 18.11 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [A stock module](exercises/01-stock-module/) | Writing a module to a given interface, and keeping its state private in each language's strongest way |
| 2 | [Build order](exercises/02-build-order/) | The dependency graph of a build: the order to build in, finding import cycles, and what to rebuild after a change |
| 3 | [Version ranges](exercises/03-version-ranges/) | Semantic version order and npm's ranges, written as a module of their own |

```sh
python3 tools/check.py 18                     # your answers, every language
python3 tools/check.py 18 --lang go,python    # some languages
python3 tools/check.py 18 --solutions         # the reference answers, and the examples
```

The checker builds every source file in each language's folder, so a module goes next to
`main`, and for exercises 18.1 and 18.3 the starter doesn't build until you write one.
The examples' projects build with CMake, Cargo, uv, npm, `dart pub`, and Gradle, and the
checker skips a language whose build tool isn't installed.
`python3 tools/check.py --doctor` lists which are.

**Stretch:** turn exercise 18.3's solution in your favorite language into a real
package. Give it a manifest, move the tests for `semver` into the package's own test
folder, and make the main program a second package that depends on it by path, as
example 02 does. Then publish nothing, but run the tool's dry run of publishing,
`cargo publish --dry-run`, `uv build`, `npm pack --dry-run`, or
`dart pub publish --dry-run`, and read what it would have uploaded.

---

Previous: [Chapter 17: Version control with Git](../17-version-control-with-git/) ·
Next: Chapter 19: Testing basics (coming soon)
