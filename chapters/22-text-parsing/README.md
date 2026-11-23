# Chapter 22: Text parsing

Chapter 21 read JSON with a library, and left the hard part to it. This chapter does the
hard part: it turns text into structure, and when the text is wrong, says where. It
writes three programs in all nine languages: a log scanner, which reads each line with a
regular expression; a CSV reader, which is a state machine, one character at a time; and
a calculator, which splits each line into tokens and builds a tree from them with a
Pratt parser, and puts a caret under the first mistake. The three programs are short and
agree on what they print, but the libraries under them disagree more than their
documentation lets on: about what `\d` and `$` mean, about what `$1x` in a replacement
is, about whether `" 12 "` and `"٣"` are numbers, and about how long a regular
expression may take, which for some patterns is longer than the age of the universe.

**Languages:** all nine · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Regular expressions in each language: groups, whole-line matches, replacements, and flags | Text handling in web servers (chapter 37), which checks with them what strangers send |
| Where regular expressions disagree, and the patterns that take a backtracking engine forever | Security basics (chapter 40), where a slow regex is a way to stop a server |
| Turning text into numbers strictly, and what each language's parser lets through | REST CRUD API (chapter 38), whose IDs and limits arrive as text |
| State machines, for formats such as CSV that a regex can't read well | HTTP from sockets (chapter 36), whose requests are read the same way, a byte at a time |
| Lexers, recursive descent, and precedence, with a Pratt parser and a tree | Trees and heaps (chapter 27), and recursion (chapter 30), where trees and backtracking come back |
| Errors with a line, a column, and a caret, and going on after one | FFI and interop (chapter 23), where the program at the other end needs to know what went wrong |

---

## 22.1 The concept: from characters to structure

To **parse** text is to find its structure. Most text that a program reads has structure
on two levels. The first is its words, or **tokens**: in `price * (1 + rate)`, they are
a name, `*`, `(`, a number, `+`, another name, and `)`, and the spaces only keep them
apart. The second is how the tokens fit together: `1 + rate` is a sum, and the sum is
multiplied by the price. A program that reads a language usually has a part for each
level. A **lexer**, or tokenizer, turns characters into tokens, and a **parser** turns
tokens into a **tree**, whose nodes are the parts, such as a product whose right side is
a sum. Compilers, JSON readers, and calculators all work this way, and so does example
03 in this chapter.

What a text may look like is written down as a **grammar**, a set of rules, each of
which says what one part is made of. This is the grammar of example 03, without its `%`
and `^`:

```text
expression = term (("+" | "-") term)*
term       = factor (("*" | "/") factor)*
factor     = number | "(" expression ")" | "-" factor
```

In the notation, which is a form of EBNF, the Extended Backus–Naur Form, quotes hold
text that appears as it is, `|` separates choices, `*` means "any number of times", and
a name means "one of those". An expression is terms with `+` or `-` between them, and a
term is factors with `*` or `/` between them, so in `1 + 2 * 3` the `*` is inside a term
and binds more tightly than the `+`. The grammar has levels for precedence.

The grammar also refers to itself: a factor can be an expression in parentheses, which
holds factors. That is what a regular expression can't do. The languages that regular
expressions describe, the **regular** languages, are exactly the ones that a machine
with a fixed number of states can recognize, reading one character at a time, and
balanced parentheses need a state for every depth: after `((((`, the machine has to
remember four. Grammars whose rules can contain themselves describe the **context-free**
languages, and recognizing those takes a stack, such as the call stack of a parser with
a function for each rule, which calls itself for each `(`. So the tools for parsing come
in order of power, and this chapter uses each where it fits:

- **String functions**, such as `split` and `find`, for formats as simple as
  `name=value`.
- **Regular expressions**, for tokens and for lines with no nesting, such as a line of a
  web server's log, in example 01, or a date, in exercise 22.1.
- **State machines**, which are what a regular expression becomes when it runs, written
  out by hand when you need to know where you are, say what went wrong, or read
  something that's regular but awkward to write as a regex, such as CSV's quoted fields,
  in example 02.
- **Parsers**, for anything that nests: arithmetic, in example 03, JSON, in exercise
  22.2, and regular expressions themselves, in exercise 22.3.

A parser for a real program has a second job, which is to explain. When the text is
wrong, the person who wrote it needs to know where, as a line and a column, and what the
parser expected there and found instead: `expected ")", found the end of the line` is
something they can fix, and `syntax error` isn't. Columns count characters, as chapter 6
does, so that a column points at what the person sees, not at a byte; each language
counts them in its own way, as section 22.7 shows.

---

## 22.2 What each language gives you

| Language | Regular expressions | How they run | A whole-string match | Text to an integer |
| --- | --- | --- | --- | --- |
| C | `<regex.h>`, from POSIX: extended syntax, no `\d`, no named groups, and no replace | glibc's, which takes the longest match, as POSIX says | `^` and `$` | `strtol`, which skips leading spaces, stops at the first character that isn't a digit, and sets `errno` on overflow |
| C++ | `<regex>`, with ECMAScript's syntax by default and no named groups | Backtracking, with a call for each step, so a long match can run out of stack | `std::regex_match` | `std::from_chars`, which takes no spaces and no `+`, or `std::stoi`, which skips spaces and stops early |
| Rust | None: the `regex` crate, from the Rust project | Automata, in time proportional to the text, with no backreferences or lookaround | `^` and `$` | `str::parse`, which takes the whole string or fails |
| Go | `regexp`, with RE2's syntax | Automata, as Rust's, from RE2's design | `^` and `$` | `strconv.Atoi` and `ParseInt`, which take the whole string or fail |
| Python | `re` | Backtracking | `re.fullmatch` | `int`, which takes spaces around the number, `_` between digits, and any Unicode digit |
| JavaScript | `RegExp`, built into the language | Backtracking | `^` and `$` | `Number`, which takes spaces, `0x`, and `""`, as 0, or `parseInt`, which stops early |
| TypeScript | JavaScript's | JavaScript's | JavaScript's | JavaScript's |
| Dart | `RegExp`, with JavaScript's syntax and meaning | Backtracking | `^` and `$` | `int.parse` and `int.tryParse`, which take spaces around the number and `0x` |
| Kotlin | `Regex`, over Java's `java.util.regex` | Backtracking | `matches` and `matchEntire` | `toInt` and `toIntOrNull`, which take any Unicode digit |

Every language but Rust has regular expressions in its standard library, and Rust's
comes from the Rust project, as a crate, so example 01 and exercise 22.1 are Cargo
projects in Rust that name `regex = "1"`. The column that matters most is the third. The
engines of C++, Python, JavaScript, Dart, and Kotlin **backtrack**: they try one way
through the pattern, and when it fails, go back and try the next, which lets them offer
backreferences, such as `(a)\1`, and lookaround, such as `(?=...)`, and which makes some
patterns take exponential time, as section 22.4 shows. Go's and Rust's run the pattern
as an automaton, which reads each character once, and leave out what an automaton can't
do: Go's `regexp.Compile` turns down `(a)\1` with `invalid escape sequence`. C's POSIX
`regexec` is glibc's own, which uses automata where it can.

No language here has a parser generator in its standard library. A parser generator,
such as yacc, Bison, or ANTLR, reads a grammar and writes the parser for you, and
they're used, but most parsers that people depend on are written by hand, as this
chapter's are: GCC's, Clang's, Go's, Rust's, and V8's are all hand-written recursive
descent, because that gives the best error messages and the most control. Python's is
generated, from a PEG grammar, since Python 3.9.

---

## 22.3 Regular expressions

Example 01 reads a web server's log, one request on each line, such as

```text
2026-11-23 14:05:12 GET /books/42 200 12ms
```

and prints, for each method and route, how many requests there were, how many failed
with a status of 400 or more, and the slowest. A **route** is the path with each `/` and
the digits after it written `/:id`, so that `/books/42` and `/books/7` count together. A
line that isn't a request gets a message on standard error, and the exit status is 1.
Each version reads the line with one regular expression, with a group for each field,
and makes the route with a second, `/[0-9]+`, and a replace.

**C**

```c
// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
#define _POSIX_C_SOURCE 200809L  // for getline and strndup, from POSIX
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *key;  // the method, a space, and the route
    int requests, failed, slowest;
} Route;

// POSIX extended regular expressions have no \d and no named groups, so the groups are
// numbered: 1 is the date, 2 the time, 3 the method, 4 the path, 5 the status, and 6 the
// time taken. Group 0 is the whole match.
static const char LINE[] =
    "^([0-9]{4}-[0-9]{2}-[0-9]{2}) ([0-9]{2}:[0-9]{2}:[0-9]{2})"
    " (GET|POST|PUT|DELETE) (/[^ ]*)"
    " ([0-9]{3}) ([0-9]{1,9})ms$";
enum { GROUPS = 7, METHOD = 3, PATH = 4, STATUS = 5, MS = 6 };

static void *checked(void *p) {
    if (p == NULL) {
        fputs("logscan: out of memory\n", stderr);
        exit(1);
    }
    return p;
}

static regex_t compile(const char *pattern) {
    regex_t re;
    int err = regcomp(&re, pattern, REG_EXTENDED);
    if (err != 0) {
        char message[256];
        regerror(err, &re, message, sizeof message);
        fprintf(stderr, "logscan: %s: %s\n", pattern, message);
        exit(2);
    }
    return re;
}

// Returns "METHOD ROUTE", with each match of id in the path written "/:id". C has no
// replace, so this copies the text between the matches and "/:id" for each match.
static char *key_of(const char *line, const regmatch_t *m, const regex_t *id) {
    size_t method = (size_t)(m[METHOD].rm_eo - m[METHOD].rm_so);
    size_t path = (size_t)(m[PATH].rm_eo - m[PATH].rm_so);
    // "/:id" is at most twice as long as the "/" and digit or more that it replaces.
    char *key = checked(malloc(method + 1 + 2 * path + 1));
    char *out = key;
    memcpy(out, line + m[METHOD].rm_so, method);
    out += method;
    *out++ = ' ';
    // regexec reads up to a '\0', so the search needs the path on its own.
    char *rest = checked(strndup(line + m[PATH].rm_so, path));
    const char *at = rest;
    regmatch_t found;
    while (regexec(id, at, 1, &found, 0) == 0) {
        memcpy(out, at, (size_t)found.rm_so);
        out += found.rm_so;
        memcpy(out, "/:id", 4);
        out += 4;
        at += found.rm_eo;
    }
    strcpy(out, at);
    free(rest);
    return key;
}

int main(void) {
    regex_t line_re = compile(LINE), id = compile("/[0-9]+");
    // C has no map, so this is a list, searched from the start: fine for a few hundred
    // routes, and it keeps them in the order they came in.
    Route *routes = NULL;
    size_t count = 0, capacity = 0;
    int status = 0;
    char *line = NULL;
    size_t size = 0;
    ssize_t length;
    // getline keeps the '\n', and a '\r' before it.
    for (long number = 1; (length = getline(&line, &size, stdin)) != -1; number++) {
        if (length > 0 && line[length - 1] == '\n') line[--length] = '\0';
        if (length > 0 && line[length - 1] == '\r') line[--length] = '\0';
        regmatch_t m[GROUPS];
        // A '\0' in the line would end the search early, so it can't match.
        if (strlen(line) != (size_t)length || regexec(&line_re, line, GROUPS, m, 0) != 0) {
            fprintf(stderr, "logscan: line %ld: not a log line\n", number);
            status = 1;
            continue;
        }
        char *key = key_of(line, m, &id);
        size_t i = 0;
        while (i < count && strcmp(routes[i].key, key) != 0) i++;
        if (i < count) {
            free(key);
        } else {
            if (count == capacity) {
                capacity = capacity == 0 ? 16 : capacity * 2;
                routes = checked(realloc(routes, capacity * sizeof *routes));
            }
            routes[count++] = (Route){.key = key};
        }
        // The regex let through only digits, few enough to fit, and strtol stops at the
        // space or the "ms" after them.
        int code = (int)strtol(line + m[STATUS].rm_so, NULL, 10);
        int ms = (int)strtol(line + m[MS].rm_so, NULL, 10);
        routes[i].requests++;
        if (code >= 400) routes[i].failed++;
        if (ms > routes[i].slowest) routes[i].slowest = ms;
    }
    for (size_t i = 0; i < count; i++) {
        printf("%s: %d request%s, %d failed, slowest %dms\n", routes[i].key,
               routes[i].requests, routes[i].requests == 1 ? "" : "s", routes[i].failed,
               routes[i].slowest);
        free(routes[i].key);
    }
    free(routes);
    free(line);
    regfree(&line_re);
    regfree(&id);
    return status;
}
```

**C++**

```cpp
// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
#include <algorithm>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Route {
    int requests = 0;
    int failed = 0;
    int slowest = 0;
};

int main() {
    // std::regex has no named groups, so these are numbered: 1 is the date, 2 the time,
    // 3 the method, 4 the path, 5 the status, and 6 the time taken. regex_match matches
    // the whole line, so the pattern needs no ^ or $. [0-9], not \d: here \d asks the
    // locale. GCC's std::regex calls itself once for each character that [^ ]* takes, so a
    // path of some 33,000 characters runs out of stack, and the program crashes.
    const std::regex line_re(
        "([0-9]{4}-[0-9]{2}-[0-9]{2}) ([0-9]{2}:[0-9]{2}:[0-9]{2})"
        " (GET|POST|PUT|DELETE) (/[^ ]*)"
        " ([0-9]{3}) ([0-9]{1,9})ms");
    const std::regex id("/[0-9]+");

    // An unordered_map has no order, so the routes are kept in a vector, with an index.
    std::vector<std::pair<std::string, Route>> routes;
    std::unordered_map<std::string, std::size_t> index;
    int status = 0;
    std::string line;
    // getline splits at "\n" only, and keeps a "\r" before it.
    for (int number = 1; std::getline(std::cin, line); number++) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::smatch match;
        if (!std::regex_match(line, match, line_re)) {
            std::cerr << "logscan: line " << number << ": not a log line\n";
            status = 1;
            continue;
        }
        // In a replacement, "$&" and "$1" stand for what matched; "/:id" has no "$".
        std::string key = match[3].str() + " " + std::regex_replace(match[4].str(), id, "/:id");
        auto [at, added] = index.try_emplace(key, routes.size());
        if (added) routes.emplace_back(key, Route{});
        Route& route = routes[at->second].second;
        // The regex let through only digits, few enough to fit, so stoi can't fail.
        route.requests++;
        if (std::stoi(match[5].str()) >= 400) route.failed++;
        route.slowest = std::max(route.slowest, std::stoi(match[6].str()));
    }
    for (const auto& [key, route] : routes) {
        std::cout << key << ": " << route.requests << " request"
                  << (route.requests == 1 ? "" : "s") << ", " << route.failed
                  << " failed, slowest " << route.slowest << "ms\n";
    }
    return status;
}
```

**Rust**

```rust
//! Reads a web server's log, one request on each line, such as
//!   2026-11-23 14:05:12 GET /books/42 200 12ms
//! and prints, for each method and route, how many requests there were, how many failed,
//! and the slowest. A route is the path with each "/" and the digits after it written
//! "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
//! gets a message on stderr, and the exit status is then 1.
use std::collections::HashMap;
use std::io::{self, Read};
use std::process::ExitCode;

use regex::Regex;

#[derive(Default)]
struct Route {
    requests: u32,
    failed: u32,
    slowest: u32,
}

fn main() -> ExitCode {
    // ^ and $, because captures finds a match anywhere in the line. Without the m flag,
    // $ matches only at the very end. [0-9], not \d: here \d matches any Unicode digit.
    let line_re = Regex::new(concat!(
        r"^(?<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?<time>[0-9]{2}:[0-9]{2}:[0-9]{2})",
        r" (?<method>GET|POST|PUT|DELETE) (?<path>/[^ ]*)",
        r" (?<status>[0-9]{3}) (?<ms>[0-9]{1,9})ms$",
    ))
    .unwrap();
    let id = Regex::new("/[0-9]+").unwrap();

    let mut input = String::new();
    if let Err(err) = io::stdin().read_to_string(&mut input) {
        eprintln!("logscan: {err}");
        return ExitCode::FAILURE;
    }
    // A HashMap has no order, so the routes are kept in a Vec, with an index to find them.
    let mut routes: Vec<(String, Route)> = Vec::new();
    let mut index: HashMap<String, usize> = HashMap::new();
    let mut status = ExitCode::SUCCESS;
    // lines() splits at "\n" and removes a "\r" before it.
    for (number, line) in input.lines().enumerate() {
        let Some(caps) = line_re.captures(line) else {
            eprintln!("logscan: line {}: not a log line", number + 1);
            status = ExitCode::FAILURE;
            continue;
        };
        let path = id.replace_all(&caps["path"], "/:id");
        let key = format!("{} {path}", &caps["method"]);
        let at = *index.entry(key.clone()).or_insert_with(|| {
            routes.push((key, Route::default()));
            routes.len() - 1
        });
        let route = &mut routes[at].1;
        // The regex let through only digits, few enough to fit, so parse can't fail.
        let code: u32 = caps["status"].parse().unwrap();
        let ms: u32 = caps["ms"].parse().unwrap();
        route.requests += 1;
        route.failed += u32::from(code >= 400);
        route.slowest = route.slowest.max(ms);
    }
    for (key, route) in &routes {
        let plural = if route.requests == 1 { "" } else { "s" };
        println!(
            "{key}: {} request{plural}, {} failed, slowest {}ms",
            route.requests, route.failed, route.slowest
        );
    }
    status
}
```

**Go**

```go
// Reads a web server's log, one request on each line, such as
//
//	2026-11-23 14:05:12 GET /books/42 200 12ms
//
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
package main

import (
	"bufio"
	"fmt"
	"os"
	"regexp"
	"strconv"
)

// ^ and $, because FindStringSubmatch finds a match anywhere in the line. Without the m
// flag, $ matches only at the very end. In Go, \d is only [0-9], but this says so.
var line = regexp.MustCompile(
	`^(?P<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?P<time>[0-9]{2}:[0-9]{2}:[0-9]{2})` +
		` (?P<method>GET|POST|PUT|DELETE) (?P<path>/[^ ]*)` +
		` (?P<status>[0-9]{3}) (?P<ms>[0-9]{1,9})ms$`)
var id = regexp.MustCompile(`/[0-9]+`)

// The groups are numbered in the match, so these find their numbers once.
var (
	method = line.SubexpIndex("method")
	path   = line.SubexpIndex("path")
	code   = line.SubexpIndex("status")
	ms     = line.SubexpIndex("ms")
)

type route struct {
	requests, failed, slowest int
}

func main() {
	var keys []string // a map has no order, so this keeps the order they came in
	routes := map[string]*route{}
	status := 0
	// A Scanner splits at "\n" and removes a "\r" before it. A line longer than 64 KiB
	// stops it with the error "token too long".
	scanner := bufio.NewScanner(os.Stdin)
	for number := 1; scanner.Scan(); number++ {
		match := line.FindStringSubmatch(scanner.Text())
		if match == nil {
			fmt.Fprintf(os.Stderr, "logscan: line %d: not a log line\n", number)
			status = 1
			continue
		}
		key := match[method] + " " + id.ReplaceAllLiteralString(match[path], "/:id")
		r := routes[key]
		if r == nil {
			r = &route{}
			routes[key] = r
			keys = append(keys, key)
		}
		// The regex let through only digits, few enough to fit, so Atoi can't fail.
		c, _ := strconv.Atoi(match[code])
		t, _ := strconv.Atoi(match[ms])
		r.requests++
		if c >= 400 {
			r.failed++
		}
		r.slowest = max(r.slowest, t)
	}
	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "logscan:", err)
		os.Exit(1)
	}
	for _, key := range keys {
		r := routes[key]
		plural := "s"
		if r.requests == 1 {
			plural = ""
		}
		fmt.Printf("%s: %d request%s, %d failed, slowest %dms\n",
			key, r.requests, plural, r.failed, r.slowest)
	}
	os.Exit(status)
}
```

**Python**

```python
# Reads a web server's log, one request on each line, such as
#   2026-11-23 14:05:12 GET /books/42 200 12ms
# and prints, for each method and route, how many requests there were, how many failed,
# and the slowest. A route is the path with each "/" and the digits after it written
# "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
# gets a message on stderr, and the exit status is then 1.
import re
import sys
from dataclasses import dataclass

# [0-9], not \d: in Python, \d matches any Unicode digit, such as the Arabic-Indic ٢.
# The time has at most nine digits, so that it fits in 32 bits in every language.
LINE = re.compile(
    r"(?P<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?P<time>[0-9]{2}:[0-9]{2}:[0-9]{2})"
    r" (?P<method>GET|POST|PUT|DELETE) (?P<path>/[^ ]*)"
    r" (?P<status>[0-9]{3}) (?P<ms>[0-9]{1,9})ms"
)
ID = re.compile(r"/[0-9]+")


@dataclass
class Route:
    requests: int = 0
    failed: int = 0
    slowest: int = 0


def main() -> int:
    routes: dict[str, Route] = {}  # a dict keeps its keys in the order they went in
    status = 0
    # On Linux and macOS, sys.stdin keeps a "\r" before each "\n". splitlines() would
    # also split at characters such as U+2028, so this splits at "\n" only.
    lines = sys.stdin.read().split("\n")
    if lines[-1] == "":  # the input ended with a newline, or was empty
        lines.pop()
    for number, line in enumerate(lines, 1):
        line = line.removesuffix("\r")
        # fullmatch: the whole line, not just its start, as match would.
        match = LINE.fullmatch(line)
        if match is None:
            print(f"logscan: line {number}: not a log line", file=sys.stderr)
            status = 1
            continue
        key = match["method"] + " " + ID.sub("/:id", match["path"])
        route = routes.setdefault(key, Route())
        route.requests += 1
        route.failed += int(match["status"]) >= 400
        route.slowest = max(route.slowest, int(match["ms"]))
    for key, route in routes.items():
        plural = "" if route.requests == 1 else "s"
        print(
            f"{key}: {route.requests} request{plural}, {route.failed} failed,"
            f" slowest {route.slowest}ms"
        )
    return status


if __name__ == "__main__":
    sys.exit(main())
```

**JavaScript**

```js
// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
import { readFileSync } from "node:fs";

// ^ and $, because exec finds a match anywhere in the line. Without the m flag, $ matches
// only at the very end. In JavaScript, \d is only [0-9], even with the u flag, but this
// says so.
const LINE = new RegExp(
  String.raw`^(?<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?<time>[0-9]{2}:[0-9]{2}:[0-9]{2})` +
    String.raw` (?<method>GET|POST|PUT|DELETE) (?<path>/[^ ]*)` +
    String.raw` (?<status>[0-9]{3}) (?<ms>[0-9]{1,9})ms$`,
  "u",
);
const ID = /\/[0-9]+/gu; // replaceAll needs the g flag

// A Map keeps its keys in the order they went in.
const routes = new Map();
let status = 0;
const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop(); // the input ended with a newline, or was empty
lines.forEach((text, index) => {
  const line = text.endsWith("\r") ? text.slice(0, -1) : text;
  const match = LINE.exec(line);
  if (match === null) {
    console.error(`logscan: line ${index + 1}: not a log line`);
    status = 1;
    return;
  }
  const { method, path, status: code, ms } = match.groups;
  const key = `${method} ${path.replaceAll(ID, "/:id")}`;
  let route = routes.get(key);
  if (route === undefined) {
    route = { requests: 0, failed: 0, slowest: 0 };
    routes.set(key, route);
  }
  route.requests += 1;
  if (Number(code) >= 400) route.failed += 1;
  route.slowest = Math.max(route.slowest, Number(ms));
});
for (const [key, { requests, failed, slowest }] of routes) {
  const plural = requests === 1 ? "" : "s";
  console.log(
    `${key}: ${requests} request${plural}, ${failed} failed, slowest ${slowest}ms`,
  );
}
process.exitCode = status;
```

**TypeScript**

```ts
// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
import { readFileSync } from "node:fs";

interface Route {
  requests: number;
  failed: number;
  slowest: number;
}

// ^ and $, because exec finds a match anywhere in the line. Without the m flag, $ matches
// only at the very end. In TypeScript, \d is only [0-9], even with the u flag, but this
// says so.
const LINE = new RegExp(
  String.raw`^(?<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?<time>[0-9]{2}:[0-9]{2}:[0-9]{2})` +
    String.raw` (?<method>GET|POST|PUT|DELETE) (?<path>/[^ ]*)` +
    String.raw` (?<status>[0-9]{3}) (?<ms>[0-9]{1,9})ms$`,
  "u",
);
const ID = /\/[0-9]+/gu; // replaceAll needs the g flag

// A Map keeps its keys in the order they went in.
const routes = new Map<string, Route>();
let status = 0;
const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop(); // the input ended with a newline, or was empty
lines.forEach((text, index) => {
  const line = text.endsWith("\r") ? text.slice(0, -1) : text;
  // The type checker can't read the pattern, so to it, groups may be missing, and any
  // name in them is a string.
  const groups = LINE.exec(line)?.groups;
  if (groups === undefined) {
    console.error(`logscan: line ${index + 1}: not a log line`);
    status = 1;
    return;
  }
  const key = `${groups.method} ${groups.path.replaceAll(ID, "/:id")}`;
  let route = routes.get(key);
  if (route === undefined) {
    route = { requests: 0, failed: 0, slowest: 0 };
    routes.set(key, route);
  }
  route.requests += 1;
  if (Number(groups.status) >= 400) route.failed += 1;
  route.slowest = Math.max(route.slowest, Number(groups.ms));
});
for (const [key, { requests, failed, slowest }] of routes) {
  const plural = requests === 1 ? "" : "s";
  console.log(
    `${key}: ${requests} request${plural}, ${failed} failed, slowest ${slowest}ms`,
  );
}
process.exitCode = status;
```

**Dart**

```dart
// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
import 'dart:convert';
import 'dart:io';
import 'dart:math';

// ^ and $, because firstMatch finds a match anywhere in the line. Without multiLine, $
// matches only at the very end. In Dart, \d is only [0-9], but this says so.
final line = RegExp(
  r'^(?<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?<time>[0-9]{2}:[0-9]{2}:[0-9]{2})'
  r' (?<method>GET|POST|PUT|DELETE) (?<path>/[^ ]*)'
  r' (?<status>[0-9]{3}) (?<ms>[0-9]{1,9})ms$',
);
final id = RegExp(r'/[0-9]+');

class Route {
  int requests = 0;
  int failed = 0;
  int slowest = 0;
}

Future<void> main() async {
  // A map literal keeps its keys in the order they went in.
  final routes = <String, Route>{};
  var status = 0;
  // LineSplitter would also split at a "\r" on its own, so this splits at "\n" only.
  final lines = (await utf8.decodeStream(stdin)).split('\n');
  // The last is empty when the input ended with a newline, or was empty.
  if (lines.last == '') lines.removeLast();
  for (final (index, text) in lines.indexed) {
    final match = line.firstMatch(
      text.endsWith('\r') ? text.substring(0, text.length - 1) : text,
    );
    if (match == null) {
      stderr.writeln('logscan: line ${index + 1}: not a log line');
      status = 1;
      continue;
    }
    // namedGroup returns a String?, which is null only for a group that didn't take
    // part in the match.
    String group(String name) => match.namedGroup(name)!;
    final key = '${group('method')} ${group('path').replaceAll(id, '/:id')}';
    final route = routes.putIfAbsent(key, Route.new);
    route.requests++;
    if (int.parse(group('status')) >= 400) route.failed++;
    route.slowest = max(route.slowest, int.parse(group('ms')));
  }
  for (final MapEntry(:key, value: route) in routes.entries) {
    final plural = route.requests == 1 ? '' : 's';
    print(
      '$key: ${route.requests} request$plural, ${route.failed} failed, '
      'slowest ${route.slowest}ms',
    );
  }
  exitCode = status;
}
```

**Kotlin**

```kotlin
// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
import kotlin.system.exitProcess

// matchEntire matches the whole line, so the pattern needs no ^ or $. In Kotlin on the
// JVM, \d is only [0-9], but this says so.
val LINE =
    Regex(
        """(?<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?<time>[0-9]{2}:[0-9]{2}:[0-9]{2})""" +
            """ (?<method>GET|POST|PUT|DELETE) (?<path>/[^ ]*)""" +
            """ (?<status>[0-9]{3}) (?<ms>[0-9]{1,9})ms"""
    )
val ID = Regex("/[0-9]+")

class Route {
    var requests = 0
    var failed = 0
    var slowest = 0
}

fun main() {
    // A LinkedHashMap keeps its keys in the order they went in.
    val routes = LinkedHashMap<String, Route>()
    var status = 0
    // A BufferedReader's lines would also split at a "\r" on its own, so this splits at
    // "\n" only.
    val lines = System.`in`.bufferedReader().readText().split("\n").toMutableList()
    // The last is empty when the input ended with a newline, or was empty.
    if (lines.last() == "") lines.removeLast()
    for ((index, line) in lines.withIndex()) {
        val match = LINE.matchEntire(line.removeSuffix("\r"))
        if (match == null) {
            System.err.println("logscan: line ${index + 1}: not a log line")
            status = 1
            continue
        }
        // groups[name] is null only for a group that didn't take part in the match.
        fun group(name: String) = match.groups[name]!!.value
        // In a replacement, "$" and "\" are special; "/:id" has neither.
        val key = group("method") + " " + ID.replace(group("path"), "/:id")
        val route = routes.getOrPut(key) { Route() }
        route.requests++
        if (group("status").toInt() >= 400) route.failed++
        route.slowest = maxOf(route.slowest, group("ms").toInt())
    }
    for ((key, route) in routes) {
        val plural = if (route.requests == 1) "" else "s"
        println(
            "$key: ${route.requests} request$plural, ${route.failed} failed, " +
                "slowest ${route.slowest}ms"
        )
    }
    exitProcess(status)
}
```

```text
$ logscan < access.log
logscan: line 5: not a log line
GET /books/:id: 3 requests, 1 failed, slowest 812ms
POST /books: 1 request, 0 failed, slowest 30ms
GET /books/:id/reviews: 1 request, 0 failed, slowest 45ms
DELETE /books/:id: 1 request, 0 failed, slowest 18ms
```

The nine patterns are nearly the same text, and the differences are worth reading:

- **Named groups.** Python names a group `(?P<name>...)`, and turns down `(?<name>...)`
  with `unknown extension`. JavaScript, Dart, and Kotlin name it `(?<name>...)`, and Go
  and Rust take both. C's and C++'s regular expressions have no names at all, so their
  groups are numbered, 1 for the date up to 6 for the time taken, and a comment says
  which is which.
- **`[0-9]`, not `\d`.** In Python and Rust, `\d` matches any Unicode digit, such as
  `٣`, the Arabic-Indic three, which `int` would then read as 3. In the other languages,
  it matches only `0` to `9`, but `[0-9]` says so, in all nine.
- **The whole line.** Python's `fullmatch` and Kotlin's `matchEntire` match the whole
  string, and C++'s `regex_match` too; the others need `^` and `$` around the pattern,
  or they would find a request inside a longer line. Python's `match` only fixes the
  start, so `re.match("a", "ab")` matches.
- **The route.** Python's `sub`, JavaScript's `replaceAll`, Rust's `replace_all`, Go's
  `ReplaceAllLiteralString`, Dart's `replaceAll`, Kotlin's `replace`, and C++'s
  `regex_replace` all replace every match. JavaScript's `replace` replaces only the
  first unless the regex has the `g` flag, and its `replaceAll` insists on the flag,
  with a `TypeError` otherwise. C has no replace, so `key_of` copies the text between
  the matches itself, with `regexec` called again after each one.
- **Line ends.** Each version takes a `\r` off the end of each line, so that a file from
  Windows reads the same, and splits at `\n` only: Dart's `LineSplitter` and Java's
  `BufferedReader.readLine` would also split at a `\r` on its own, and Python's
  `splitlines` at characters such as U+2028.
- **The numbers.** The pattern lets through only one to nine digits for the time, so
  that converting it can't fail or overflow in any language, and the comments say so.

Test 03 is twelve lines that nearly match, such as a lowercase `get`, a status of
`2000`, and a space at the end of the line, and each is `not a log line` in all nine.

---

## 22.4 Where regular expressions disagree

### What `\d`, `\w`, and `\b` mean

| Language | `\d` matches `٣` | `\w` matches `é` | To change it |
| --- | --- | --- | --- |
| C | No `\d` in POSIX | No `\w` in POSIX, though glibc takes it | Use `[0-9]` and `[[:alpha:]]` |
| C++ | No | No: the text is bytes | |
| Rust | Yes | Yes | `(?-u:\d)`, or `[0-9]` |
| Go | No | No | `\p{Nd}` for Unicode's digits |
| Python | Yes | Yes | `re.ASCII`, or `[0-9]` |
| JavaScript | No, even with the `u` flag | No | `\p{Nd}`, with `u` |
| Dart | No, even with `unicode: true` | No | `\p{Nd}`, with `unicode: true` |
| Kotlin | No | No | `(?U)`, Java's `UNICODE_CHARACTER_CLASS` |

The split is between the languages that made their strings Unicode first and their
regular expressions after, Python 3 and Rust, and those whose regular expressions are
older than that. Neither is wrong, but a pattern that checks a ZIP code with `\d{5}`
accepts `٣٣٣٣٣` in Python, and `int` accepts it too, so a check that seems to keep out
everything but digits lets in digits from other scripts. When you mean ASCII, write
`[0-9]`. And `\b`, the boundary between a `\w` and anything else, follows `\w`, so in
Python and Rust it takes `é` for a letter, as exercise 22.1 shows.

Case doesn't fold the same way either. With `re.IGNORECASE`, Python matches `k` with
U+212A, the Kelvin sign, which is a capital K to Unicode, and so do Go, Rust, and
Kotlin, and JavaScript and Dart do only with the `u` flag or `unicode: true`. None of
those six matches `ß` with `SS`, which Unicode's full case folding would.

### Where `$` matches

`$` means "the end", but which end? In Python and in Java, which Kotlin's `Regex` is,
`$` also matches just before a `\n` at the very end of the string, so
`re.search("a$", "a\n")` finds a match, and `Regex("a$").containsMatchIn("a\n")` is
`true`. In JavaScript, Dart, Go, Rust, C++, and C, it doesn't. All of them have a
multiline mode, `m` or `(?m)`, `multiLine: true` in Dart, `std::regex::multiline` in
C++, and `REG_NEWLINE` in C, in which `^` and `$` match at every line, and Python's `\Z`
and `fullmatch` match only at the very end. This is why example 01 takes the line ends
off first: a search for `^[0-9]+$` in a line that still has its `\n` finds one in Python
and Kotlin, and doesn't in the others.

### Replacement strings

| Language | Group 1, then `x` | A named group | Notes |
| --- | --- | --- | --- |
| C | No replace | No replace | |
| C++ | `$1x` | No names | `$&` is the whole match |
| Rust | `${1}x`: `$1x` is the group named `1x`, which isn't there, so it's empty | `${name}` | |
| Go | `${1}x`, as in Rust | `${name}` | |
| Python | `\1x`, or `\g<1>x` | `\g<name>` | `$1` is just `$1` |
| JavaScript | `$1x` | `$<name>` | `$&` is the whole match |
| Dart | No groups in strings: `replaceAll(re, r'$1x')` writes `$1x` | No groups in strings | Use `replaceAllMapped` and a function |
| Kotlin | `$1x` | `${name}` | |

The traps are Go's and Rust's `$1x`, which take the longest name they can and quietly
put nothing there, and Dart's `replaceAll`, which takes every `$` literally. A function
is the safe replacement everywhere it's offered, because it can't be misread: Python's
`sub`, JavaScript's `replace`, Rust's `replace_all`, Dart's `replaceAllMapped`, and
Kotlin's `replace` all take one, as exercise 22.1 does.

### Backtracking, and patterns that never finish

A backtracking engine that meets `(a*)*b` on a line of a's with no `b` tries every way
of sharing the a's out among the turns of the outer `*`, and there are twice as many
ways for each a more. These are the seconds each took on this book's machine for a line
of a's:

| Engine | `(a*)*b`, 22 a's | 24 a's | 26 a's | 28 a's |
| --- | --- | --- | --- | --- |
| Python's `re` | 0.6 | 2.0 | 8.1 | |
| JavaScript's `RegExp` | 0.1 | 0.5 | 1.7 | 6.3 |
| Dart's `RegExp` | 0.5 | 2.6 | 9.9 | |
| Go's `regexp` and Rust's `regex` | Under a millisecond for each, and a few milliseconds for 30,000 a's | | | |

Java's engine, under Kotlin's `Regex`, sees through this pattern and answers at once for
26 a's, but not for others: on `a?a?…a?aa…a`, with thirty of each, against thirty a's,
it took 14 seconds, and on `(a|aa)*c` against 30,000 a's it threw `StackOverflowError`,
since it calls itself for each turn of the `*`. GCC's `std::regex` crashed with a
segmentation fault on that line, and example 01's comment notes that it does the same on
a path of about 33,000 characters, matched by `[^ ]*`. A pattern like these, which a
stranger can send or reach, is how a **ReDoS**, a regular-expression denial of service,
stops a server: one request with a long enough line keeps a thread busy for years.
Exercise 22.3 is to write a matcher that can't be stopped this way, and its tests 26 to
28 are these patterns.

There are three ways out. Write patterns in which each character can be matched only one
way: `a*b`, not `(a*)*b`. Use an engine that promises linear time, such as Go's, Rust's,
or RE2, which C++ programs can use as a library. Or, in Python since 3.11, use an atomic
group, `(?>(a*)*)b`, or a possessive quantifier, `(a*+)*b`, which never go back into
what they matched: either of those took 0.2 seconds on 30,000 a's. Even `a*b` took 0.36
seconds on that line in Python, because a search tries every place to start, and each
try reads to the end of the line.

### The first match, or the longest

When two alternatives both match, `a|ab` against `ab`, Python, JavaScript, Go, Rust, and
the other engines here take the first alternative that leads to a match, `a`. POSIX says
to take the longest, `ab`, and C's `regexec` does, and so does Go's
`regexp.MustCompilePOSIX`. It matters for alternatives such as `Jan|January`, as
exercise 22.1 shows.

---

## 22.5 Parsing numbers

A program that reads a number from text has to decide what a number looks like, and each
language's parser has decided already. These are what each gives for the same strings:

| Language and function | `" 12 "` | `"+5"` | `"٣"` | `"1_000"` | `"0x1F"` | `"12abc"` | `""` | 20 nines |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| C `strtol(s, &end, 10)` | 12, stops at the second space | 5 | 0, using nothing | 1, stops at `_` | 0, stops at `x` | 12, stops at `a` | 0, using nothing | `LONG_MAX`, with `errno` set to `ERANGE` |
| C `atoi` | 12 | 5 | 0 | 1 | 0 | 12 | 0 | −1 |
| C++ `std::stoi` | 12 | 5 | `invalid_argument` | 1 | 0 | 12 | `invalid_argument` | `out_of_range` |
| C++ `std::from_chars` | Error | Error | Error | 1, stops at `_` | 0, stops at `x` | 12, stops at `a` | Error | `result_out_of_range` |
| Rust `s.parse::<i64>()` | `InvalidDigit` | 5 | `InvalidDigit` | `InvalidDigit` | `InvalidDigit` | `InvalidDigit` | `Empty` | `PosOverflow` |
| Go `strconv.Atoi` | Error | 5 | Error | Error | Error | Error | Error | `value out of range` |
| Python `int` | 12 | 5 | 3 | 1000 | `ValueError` | `ValueError` | `ValueError` | The number itself |
| JavaScript `Number` | 12 | 5 | `NaN` | `NaN` | 31 | `NaN` | 0 | 100000000000000000000, rounded |
| JavaScript `parseInt(s, 10)` | 12 | 5 | `NaN` | 1 | 0 | 12 | `NaN` | 100000000000000000000, rounded |
| Dart `int.tryParse` | 12 | 5 | `null` | `null` | 31 | `null` | `null` | `null` |
| Kotlin `toLongOrNull` | `null` | 5 | 3 | `null` | `null` | `null` | `null` | `null` |

Only Rust's `parse` and Go's `Atoi` take exactly the decimal digits and an optional
sign, and turn everything else down with an error that says why. The rest let something
through: spaces in Python, Dart, JavaScript, C, and C++'s `stoi`, digits from other
scripts in Python and Kotlin, `0x` in JavaScript's `Number` and Dart's `tryParse`, a
prefix of the text in C, C++, and `parseInt`, and in JavaScript's `Number`, an empty
string, which is 0. C's `atoi` can't report an error at all: it gives 0 for text that
isn't a number, which is also what it gives for `"0"`, and for 20 nines, which overflow,
glibc's gives −1, and the C standard says the behavior is undefined. Go's
`strconv.ParseInt(s, 0, 64)`, with a base of 0, takes the prefixes of Go's own literals,
so `0x1F` is 31 and `1_000` is 1000.

Floating point has its own surprises. All nine read `1e400` as infinity, or report it as
out of range, as C's `strtod` and C++'s `stod` and `from_chars` do. Python, Go, and Rust
read all of `"nan"`, `"NaN"`, `"inf"`, and `"Infinity"`, and C's `strtod` and C++'s
`stod` and `from_chars` read `"nan"` and `"inf"`; Dart and Kotlin read only `"NaN"` and
`"Infinity"`, and JavaScript's `Number` only `"Infinity"`. Kotlin's `toDoubleOrNull`,
which is Java's `Double.parseDouble`, takes `"1.5d"` and `"1.5f"`, with Java's suffixes,
and `" 1.5 "`, with the spaces, as does Dart's `double.tryParse`.

So the reliable way to read a number is the way this chapter's examples do it: decide
what the number may look like, check the text against that first, with a regex such as
`[0-9]{1,9}` or with the lexer, and only then convert it, when the conversion can no
longer fail or be fooled. Example 01 lets through at most nine digits, so that the time
fits in 32 bits everywhere, and example 03's lexer reads only ASCII digits and checks
the value against 64 bits itself.

---

## 22.6 State machines

Example 02 reads CSV, as RFC 4180 describes it, and prints each record as a JSON array
of strings, one record on each line. A field can be in quotes, and then it can hold
commas, line breaks, and `""` for a quote. Lines can end with `\r\n` or `\n`. Every
record must have as many fields as the first, and at the first error, the program prints
where it is and exits with 1.

```text
$ cat people.csv
name,city,quote
Ada Lovelace,London,"Numbers, then poetry"
"Grace ""Amazing"" Hopper",New York,"It's easier to ask
forgiveness"
$ csv2json < people.csv
["name","city","quote"]
["Ada Lovelace","London","Numbers, then poetry"]
["Grace \"Amazing\" Hopper","New York","It's easier to ask\nforgiveness"]
```

Splitting each line at commas gets the first line right and the others wrong, since a
comma or a line break inside quotes doesn't end anything. What a character means depends
on what came before it, and a **state machine** keeps exactly that: a **state**, one of
a few named situations, and for each state and each kind of character, what to do and
which state comes next. This one has six states:

| State | Means | On `"` | On `,` | On `\n` | On anything else |
| --- | --- | --- | --- | --- | --- |
| `RECORD_START` | Nothing of this record read yet | Start a quoted field | End an empty field | End the record | Start an unquoted field |
| `FIELD_START` | Just after a comma | Start a quoted field | End an empty field | End the record | Start an unquoted field |
| `UNQUOTED` | In a field without quotes | Error | End the field | End the record | Add it to the field |
| `QUOTED` | In a field in quotes | Go to `QUOTE` | Add it | Add it | Add it |
| `QUOTE` | Just after a quote in a quoted field | Add a `"`, and go back to `QUOTED` | End the field | End the record | Error |
| `CR` | Just after a `\r` outside quotes | Error | Error | End the record | Error |

A `\r` outside quotes goes to `CR`, which takes only a `\n` next. The end of the input
goes through the machine too, as one more character: `""` in Python, JavaScript, and
TypeScript, `None` in Rust, `null` in Dart and Kotlin, and −1 in Go, C, and C++. In
`QUOTED`, the end is an error, `quoted field is never closed`, at the quote that opened
the field, and in `CR` it's an error too. In the other states it ends the record being
read, if there is one, so that the last record needs no line break after it.

**C**

```c
// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    RECORD_START,  // nothing of this record read yet
    FIELD_START,   // just after a comma
    UNQUOTED,      // in a field that doesn't start with a quote
    QUOTED,        // in a field that does
    QUOTE,         // just after a quote in a quoted field: its end, or the first of ""
    CR,            // just after a "\r" outside quotes, which needs a "\n" next
} State;

enum { END = -1 };  // stands for the end, so that it goes through the machine as well

typedef struct {
    char *data;
    size_t length, capacity;
} Buffer;

static void append(Buffer *b, const char *s, size_t n) {
    if (b->length + n > b->capacity) {
        while (b->length + n > b->capacity) b->capacity = b->capacity == 0 ? 64 : 2 * b->capacity;
        b->data = realloc(b->data, b->capacity);
        if (b->data == NULL) {
            fputs("csv2json: out of memory\n", stderr);
            exit(1);
        }
    }
    memcpy(b->data + b->length, s, n);
    b->length += n;
}

static void append_char(Buffer *b, char c) { append(b, &c, 1); }

// Ends a field: appends it to the record, which is kept as JSON text, after a comma
// unless it's the first. Writes it as JavaScript's JSON.stringify does. Bytes of UTF-8
// above 0x7F are written as they are.
static void end_field(Buffer *record, Buffer *field, size_t *count) {
    if (*count > 0) append_char(record, ',');
    append_char(record, '"');
    for (size_t i = 0; i < field->length; i++) {
        unsigned char c = (unsigned char)field->data[i];
        switch (c) {
            case '"': append(record, "\\\"", 2); break;
            case '\\': append(record, "\\\\", 2); break;
            case '\b': append(record, "\\b", 2); break;
            case '\f': append(record, "\\f", 2); break;
            case '\n': append(record, "\\n", 2); break;
            case '\r': append(record, "\\r", 2); break;
            case '\t': append(record, "\\t", 2); break;
            default:
                if (c < 0x20) {
                    char escape[7];
                    snprintf(escape, sizeof escape, "\\u%04x", c);
                    append(record, escape, 6);
                } else {
                    append_char(record, (char)c);
                }
        }
    }
    append_char(record, '"');
    field->length = 0;
    (*count)++;
}

// Prints a message for the error at line and column, and returns 1, the exit status.
static int error(int line, int column, const char *format, ...) {
    fprintf(stderr, "csv2json: line %d, column %d: ", line, column);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    return 1;
}

// Prints each record in text, and returns 0, or 1 at the first error.
static int parse(const char *text, size_t size) {
    State state = RECORD_START;
    Buffer record = {0}, field = {0};
    size_t count = 0;  // fields in the record so far
    size_t width = 0;  // how many fields the first record has, once it has ended
    int line = 1, column = 0;  // where c is: columns count characters, from 1
    int start_line = 0, start_column = 0;  // where a quoted field's quote is, or the "\r"
    int record_line = 1;
    int status = 0;
    for (size_t i = 0; i <= size && status == 0; i++) {
        int c = i < size ? (unsigned char)text[i] : END;
        // The text is UTF-8, so a character is one to four bytes. All but the first are
        // 10xxxxxx, and they don't move the column. None of them is a quote, a comma, or
        // a line end, so they go into the field with the rest of their character.
        if ((c & 0xC0) != 0x80) column++;
        if (state == RECORD_START) record_line = line;
        int ended = 0;  // whether c ends a record
        if (state == QUOTED) {
            if (c == END) {
                status = error(start_line, start_column, "quoted field is never closed");
            } else if (c == '"') {
                state = QUOTE;
            } else {
                append_char(&field, (char)c);
            }
        } else if (state == QUOTE && c == '"') {
            append_char(&field, '"');
            state = QUOTED;
        } else if (state == CR) {
            if (c == '\n') {
                ended = 1;
            } else {
                status = error(start_line, start_column,
                               "a carriage return without a line feed after it");
            }
        } else if (state == RECORD_START && c == END) {
            // the end, at the start or just after a line end: no record
        } else if (c == '"' && state != QUOTE) {
            if (state == UNQUOTED) {
                status = error(line, column, "a quote in an unquoted field");
            } else {
                start_line = line;
                start_column = column;
                state = QUOTED;
            }
        } else if (c == ',') {
            end_field(&record, &field, &count);
            state = FIELD_START;
        } else if (c == '\n' || c == END) {
            ended = 1;
        } else if (c == '\r') {
            start_line = line;
            start_column = column;
            state = CR;
        } else if (state == QUOTE) {
            status = error(line, column, "expected a comma or a line end after a closing quote");
        } else {
            append_char(&field, (char)c);
            state = UNQUOTED;
        }
        if (ended) {
            end_field(&record, &field, &count);
            if (width == 0) width = count;
            if (count != width) {
                status = error(record_line, 1, "record has %zu field%s, but the first has %zu",
                               count, count == 1 ? "" : "s", width);
            } else {
                printf("[%.*s]\n", (int)record.length, record.data);
            }
            record.length = 0;
            count = 0;
            state = RECORD_START;
        }
        if (c == '\n') {
            line++;
            column = 0;
        }
    }
    free(record.data);
    free(field.data);
    return status;
}

int main(void) {
    Buffer input = {0};
    char chunk[65536];
    size_t got;
    while ((got = fread(chunk, 1, sizeof chunk, stdin)) > 0) append(&input, chunk, got);
    if (ferror(stdin)) {
        perror("csv2json");
        return 1;
    }
    int status = parse(input.data, input.length);
    free(input.data);
    return status;
}
```

**C++**

```cpp
// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
#include <cstdio>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

enum class State {
    RecordStart,  // nothing of this record read yet
    FieldStart,   // just after a comma
    Unquoted,     // in a field that doesn't start with a quote
    Quoted,       // in a field that does
    Quote,        // just after a quote in a quoted field: its end, or the first of ""
    Cr,           // just after a "\r" outside quotes, which needs a "\n" next
};

struct CsvError : std::runtime_error {
    CsvError(int line, int column, const std::string& message)
        : std::runtime_error("line " + std::to_string(line) + ", column " +
                             std::to_string(column) + ": " + message) {}
};

static std::string fields(std::size_t n) {
    return n == 1 ? "1 field" : std::to_string(n) + " fields";
}

// Calls emit with each record in text. Throws a CsvError at the first error.
template <typename Emit>
void parse(const std::string& text, Emit emit) {
    using enum State;  // C++20: Quoted, not State::Quoted
    constexpr int end = -1;  // stands for the end, so that it goes through the machine too
    State state = RecordStart;
    std::vector<std::string> record;
    std::string field;
    std::size_t width = 0;  // how many fields the first record has, once it has ended
    int line = 1, column = 0;  // where c is: columns count characters, from 1
    int start_line = 0, start_column = 0;  // where a quoted field's quote is, or the "\r"
    int record_line = 1;
    for (std::size_t i = 0; i <= text.size(); i++) {
        int c = i < text.size() ? static_cast<unsigned char>(text[i]) : end;
        // The text is UTF-8, so a character is one to four bytes. All but the first are
        // 10xxxxxx, and they don't move the column. None of them is a quote, a comma, or
        // a line end, so they go into the field with the rest of their character.
        if ((c & 0xC0) != 0x80) column++;
        if (state == RecordStart) record_line = line;
        bool ended = false;  // whether c ends a record
        if (state == Quoted) {
            if (c == end) throw CsvError(start_line, start_column, "quoted field is never closed");
            if (c == '"') state = Quote;
            else field += static_cast<char>(c);
        } else if (state == Quote && c == '"') {
            field += '"';
            state = Quoted;
        } else if (state == Cr) {
            if (c != '\n') {
                throw CsvError(start_line, start_column,
                               "a carriage return without a line feed after it");
            }
            ended = true;
        } else if (state == RecordStart && c == end) {
            // the end, at the start or just after a line end: no record
        } else if (c == '"' && state != Quote) {
            if (state == Unquoted) throw CsvError(line, column, "a quote in an unquoted field");
            start_line = line;
            start_column = column;
            state = Quoted;
        } else if (c == ',') {
            record.push_back(std::move(field));
            field.clear();
            state = FieldStart;
        } else if (c == '\n' || c == end) {
            ended = true;
        } else if (c == '\r') {
            start_line = line;
            start_column = column;
            state = Cr;
        } else if (state == Quote) {
            throw CsvError(line, column, "expected a comma or a line end after a closing quote");
        } else {
            field += static_cast<char>(c);
            state = Unquoted;
        }
        if (ended) {
            record.push_back(std::move(field));
            field.clear();
            if (width == 0) {
                width = record.size();
            } else if (record.size() != width) {
                throw CsvError(record_line, 1, "record has " + fields(record.size()) +
                                                   ", but the first has " + std::to_string(width));
            }
            emit(record);
            record.clear();
            state = RecordStart;
        }
        if (c == '\n') {
            line++;
            column = 0;
        }
    }
}

// Writes s as a JSON string, as JavaScript's JSON.stringify does. Bytes of UTF-8 above
// 0x7F are written as they are.
static void write_json(std::ostream& out, const std::string& s) {
    out << '"';
    for (unsigned char c : s) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (c < 0x20) {
                    char escape[7];
                    std::snprintf(escape, sizeof escape, "\\u%04x", c);
                    out << escape;
                } else {
                    out << static_cast<char>(c);
                }
        }
    }
    out << '"';
}

int main() {
    // The whole of stdin, as bytes.
    std::string text{std::istreambuf_iterator<char>(std::cin), {}};
    try {
        parse(text, [](const std::vector<std::string>& record) {
            std::cout << '[';
            for (std::size_t i = 0; i < record.size(); i++) {
                if (i > 0) std::cout << ',';
                write_json(std::cout, record[i]);
            }
            std::cout << "]\n";
        });
    } catch (const CsvError& err) {
        std::cout.flush();
        std::cerr << "csv2json: " << err.what() << '\n';
        return 1;
    }
    return 0;
}
```

**Rust**

```rust
//! Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
//! one record on each line. A field in quotes can hold commas, line breaks, and "" for a
//! quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
//! first. At the first error, it prints the line and column, and the exit status is 1.
use std::fmt;
use std::io::{self, BufWriter, Read, Write};
use std::process::ExitCode;

#[derive(Clone, Copy, PartialEq)]
enum State {
    RecordStart, // nothing of this record read yet
    FieldStart,  // just after a comma
    Unquoted,    // in a field that doesn't start with a quote
    Quoted,      // in a field that does
    Quote,       // just after a quote in a quoted field: its end, or the first of ""
    Cr,          // just after a "\r" outside quotes, which needs a "\n" next
}

struct CsvError {
    line: usize,
    column: usize,
    message: String,
}

impl fmt::Display for CsvError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "line {}, column {}: {}",
            self.line, self.column, self.message
        )
    }
}

fn error(line: usize, column: usize, message: &str) -> Result<(), CsvError> {
    let message = message.to_string();
    Err(CsvError {
        line,
        column,
        message,
    })
}

fn fields(n: usize) -> String {
    if n == 1 {
        "1 field".to_string()
    } else {
        format!("{n} fields")
    }
}

/// Calls emit with each record in text, and returns the first error.
fn parse(text: &str, mut emit: impl FnMut(&[String])) -> Result<(), CsvError> {
    use State::*;
    let mut state = RecordStart;
    let mut record: Vec<String> = Vec::new();
    let mut field = String::new();
    let mut width = 0; // how many fields the first record has, once it has ended
    let (mut line, mut column) = (1, 0); // where char is: columns count characters, from 1
    let mut start = (0, 0); // where the quote that began a quoted field is, or the "\r"
    let mut record_line = 1;
    // chars() goes through the text's chars, which are code points, so "🦊" is one. The
    // None after them stands for the end, so that the end goes through the machine too.
    for char in text.chars().map(Some).chain([None]) {
        column += 1;
        if state == RecordStart {
            record_line = line;
        }
        let mut ended = false; // whether char ends a record
        // Each arm is a state and a character; the first that fits is taken.
        match (state, char) {
            (Quoted, None) => return error(start.0, start.1, "quoted field is never closed"),
            (Quoted, Some('"')) => state = Quote,
            (Quoted, Some(c)) => field.push(c),
            (Quote, Some('"')) => {
                field.push('"');
                state = Quoted;
            }
            (Cr, Some('\n')) => ended = true,
            (Cr, _) => {
                return error(
                    start.0,
                    start.1,
                    "a carriage return without a line feed after it",
                );
            }
            (RecordStart, None) => {}
            (RecordStart | FieldStart, Some('"')) => {
                start = (line, column);
                state = Quoted;
            }
            (Unquoted, Some('"')) => return error(line, column, "a quote in an unquoted field"),
            // From here on, the state is RecordStart, FieldStart, Unquoted, or Quote.
            (_, Some(',')) => {
                record.push(std::mem::take(&mut field));
                state = FieldStart;
            }
            (_, Some('\n') | None) => ended = true,
            (_, Some('\r')) => {
                start = (line, column);
                state = Cr;
            }
            (Quote, Some(_)) => {
                return error(
                    line,
                    column,
                    "expected a comma or a line end after a closing quote",
                );
            }
            (RecordStart | FieldStart | Unquoted, Some(c)) => {
                field.push(c);
                state = Unquoted;
            }
        }
        if ended {
            record.push(std::mem::take(&mut field));
            if width == 0 {
                width = record.len();
            } else if record.len() != width {
                let n = fields(record.len());
                return error(
                    record_line,
                    1,
                    &format!("record has {n}, but the first has {width}"),
                );
            }
            emit(&record);
            record.clear();
            state = RecordStart;
        }
        if char == Some('\n') {
            (line, column) = (line + 1, 0);
        }
    }
    Ok(())
}

/// Writes s as a JSON string, as JavaScript's JSON.stringify does.
fn write_string(out: &mut impl Write, s: &str) -> io::Result<()> {
    out.write_all(b"\"")?;
    for c in s.chars() {
        match c {
            '"' => out.write_all(b"\\\"")?,
            '\\' => out.write_all(b"\\\\")?,
            '\u{8}' => out.write_all(b"\\b")?,
            '\u{c}' => out.write_all(b"\\f")?,
            '\n' => out.write_all(b"\\n")?,
            '\r' => out.write_all(b"\\r")?,
            '\t' => out.write_all(b"\\t")?,
            c if c < ' ' => write!(out, "\\u{:04x}", c as u32)?,
            c => write!(out, "{c}")?,
        }
    }
    out.write_all(b"\"")
}

/// Writes record as a JSON list of strings, on a line of its own.
fn write_record(out: &mut impl Write, record: &[String]) -> io::Result<()> {
    out.write_all(b"[")?;
    for (i, field) in record.iter().enumerate() {
        if i > 0 {
            out.write_all(b",")?;
        }
        write_string(out, field)?;
    }
    out.write_all(b"]\n")
}

fn main() -> ExitCode {
    let mut text = String::new();
    if let Err(err) = io::stdin().read_to_string(&mut text) {
        eprintln!("csv2json: {err}");
        return ExitCode::FAILURE;
    }
    // Rust's stdout writes each line as it ends; a BufWriter saves them up.
    let mut out = BufWriter::new(io::stdout().lock());
    let result = parse(&text, |record| {
        write_record(&mut out, record).expect("can't write to stdout");
    });
    out.flush().expect("can't write to stdout");
    match result {
        Ok(()) => ExitCode::SUCCESS,
        Err(err) => {
            eprintln!("csv2json: {err}");
            ExitCode::FAILURE
        }
    }
}
```

**Go**

```go
// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strings"
)

type state int

// Go has no enums: these are numbered constants of their own type, and iota counts them.
const (
	recordStart state = iota // nothing of this record read yet
	fieldStart               // just after a comma
	unquoted                 // in a field that doesn't start with a quote
	quoted                   // in a field that does
	quote                    // just after a quote in a quoted field: its end, or the first of ""
	cr                       // just after a "\r" outside quotes, which needs a "\n" next
)

// end stands for the end of the text, so that the end goes through the machine as well.
const end rune = -1

type csvError struct {
	line, column int
	message      string
}

func (e *csvError) Error() string {
	return fmt.Sprintf("line %d, column %d: %s", e.line, e.column, e.message)
}

func fields(n int) string {
	if n == 1 {
		return "1 field"
	}
	return fmt.Sprintf("%d fields", n)
}

// parse calls emit with each record in text, and returns the first error.
func parse(text string, emit func(record []string)) error {
	s := recordStart
	var record []string
	var field strings.Builder
	width := 0           // how many fields the first record has, once it has ended
	line, column := 1, 0 // where char is: columns count characters, from 1
	var start [2]int     // where the quote that began a quoted field is, or the "\r"
	recordLine := 1
	// []rune decodes the UTF-8, so that "🦊" is one character.
	for _, char := range append([]rune(text), end) {
		column++
		if s == recordStart {
			recordLine = line
		}
		ended := false // whether char ends a record
		switch {
		case s == quoted && char == end:
			return &csvError{start[0], start[1], "quoted field is never closed"}
		case s == quoted && char == '"':
			s = quote
		case s == quoted:
			field.WriteRune(char)
		case s == quote && char == '"':
			field.WriteByte('"')
			s = quoted
		case s == cr && char == '\n':
			ended = true
		case s == cr:
			return &csvError{start[0], start[1], "a carriage return without a line feed after it"}
		case s == recordStart && char == end:
		case (s == recordStart || s == fieldStart) && char == '"':
			start = [2]int{line, column}
			s = quoted
		case s == unquoted && char == '"':
			return &csvError{line, column, "a quote in an unquoted field"}
		// From here on, s is recordStart, fieldStart, unquoted, or quote.
		case char == ',':
			record = append(record, field.String())
			field.Reset()
			s = fieldStart
		case char == '\n' || char == end:
			ended = true
		case char == '\r':
			start = [2]int{line, column}
			s = cr
		case s == quote:
			return &csvError{line, column, "expected a comma or a line end after a closing quote"}
		default:
			field.WriteRune(char)
			s = unquoted
		}
		if ended {
			record = append(record, field.String())
			field.Reset()
			if width == 0 {
				width = len(record)
			} else if len(record) != width {
				message := fmt.Sprintf("record has %s, but the first has %d",
					fields(len(record)), width)
				return &csvError{recordLine, 1, message}
			}
			emit(record)
			record = nil
			s = recordStart
		}
		if char == '\n' {
			line, column = line+1, 0
		}
	}
	return nil
}

// quoteJSON writes s as a JSON string, as JavaScript's JSON.stringify does. json.Marshal
// would also write <, >, &, U+2028, and U+2029 as escapes.
func quoteJSON(out *bufio.Writer, s string) {
	out.WriteByte('"')
	for _, r := range s {
		switch r {
		case '"':
			out.WriteString(`\"`)
		case '\\':
			out.WriteString(`\\`)
		case '\b':
			out.WriteString(`\b`)
		case '\f':
			out.WriteString(`\f`)
		case '\n':
			out.WriteString(`\n`)
		case '\r':
			out.WriteString(`\r`)
		case '\t':
			out.WriteString(`\t`)
		default:
			if r < 0x20 {
				fmt.Fprintf(out, `\u%04x`, r)
			} else {
				out.WriteRune(r)
			}
		}
	}
	out.WriteByte('"')
}

func main() {
	data, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintln(os.Stderr, "csv2json:", err)
		os.Exit(1)
	}
	// os.Stdout has no buffer of its own, so this gives it one, to write in big pieces.
	out := bufio.NewWriter(os.Stdout)
	err = parse(string(data), func(record []string) {
		out.WriteByte('[')
		for i, field := range record {
			if i > 0 {
				out.WriteByte(',')
			}
			quoteJSON(out, field)
		}
		out.WriteString("]\n")
	})
	out.Flush()
	if err != nil {
		fmt.Fprintln(os.Stderr, "csv2json:", err)
		os.Exit(1)
	}
}
```

**Python**

```python
# Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
# one record on each line. A field in quotes can hold commas, line breaks, and "" for a
# quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
# first. At the first error, it prints the line and column, and the exit status is 1.
import json
import sys
from collections.abc import Iterator
from enum import Enum, auto


class State(Enum):
    RECORD_START = auto()  # nothing of this record read yet
    FIELD_START = auto()  # just after a comma
    UNQUOTED = auto()  # in a field that doesn't start with a quote
    QUOTED = auto()  # in a field that does
    QUOTE = auto()  # just after a quote in a quoted field: its end, or the first of ""
    CR = auto()  # just after a "\r" outside quotes, which needs a "\n" next


class CsvError(Exception):
    def __init__(self, line: int, column: int, message: str) -> None:
        super().__init__(f"line {line}, column {column}: {message}")


def fields(n: int) -> str:
    return "1 field" if n == 1 else f"{n} fields"


def records(text: str) -> Iterator[list[str]]:
    """The records in text, one at a time. Raises CsvError at the first error."""
    state = State.RECORD_START
    record: list[str] = []
    field: list[str] = []  # its characters so far, to join at the end
    width = 0  # how many fields the first record has, once it has ended
    line, column = 1, 0  # where char is: columns count characters, from 1
    start = (0, 0)  # where the quote that began a quoted field is, or the "\r"
    record_line = 1
    # The text, then "" for its end, so that the end goes through the machine as well.
    for char in [*text, ""]:
        column += 1
        if state is State.RECORD_START:
            record_line = line
        ended = False  # whether char ends a record
        # Each case is a state and a character; the first that fits is taken.
        match state, char:
            case State.QUOTED, "":
                raise CsvError(*start, "quoted field is never closed")
            case State.QUOTED, '"':
                state = State.QUOTE
            case State.QUOTED, _:
                field.append(char)
            case State.QUOTE, '"':
                field.append('"')
                state = State.QUOTED
            case State.CR, "\n":
                ended = True
            case State.CR, _:
                raise CsvError(*start, "a carriage return without a line feed after it")
            case State.RECORD_START, "":
                pass
            case State.RECORD_START | State.FIELD_START, '"':
                start = (line, column)
                state = State.QUOTED
            case State.UNQUOTED, '"':
                raise CsvError(line, column, "a quote in an unquoted field")
            # From here on, the state is RECORD_START, FIELD_START, UNQUOTED, or QUOTE.
            case _, ",":
                record.append("".join(field))
                field.clear()
                state = State.FIELD_START
            case _, "\n" | "":
                ended = True
            case _, "\r":
                start = (line, column)
                state = State.CR
            case State.QUOTE, _:
                raise CsvError(
                    line, column, "expected a comma or a line end after a closing quote"
                )
            case _:
                field.append(char)
                state = State.UNQUOTED
        if ended:
            record.append("".join(field))
            field.clear()
            if width == 0:
                width = len(record)
            elif len(record) != width:
                message = f"record has {fields(len(record))}, but the first has {width}"
                raise CsvError(record_line, 1, message)
            yield record
            record = []
            state = State.RECORD_START
        if char == "\n":
            line, column = line + 1, 0


def main() -> int:
    # The bytes, decoded by hand, so that each "\r" reaches the parser, whatever the
    # platform does with line ends in text mode.
    text = sys.stdin.buffer.read().decode("utf-8")
    try:
        for record in records(text):
            # ensure_ascii=False writes "é" as itself, not as "\u00e9".
            print(json.dumps(record, ensure_ascii=False, separators=(",", ":")))
    except CsvError as err:
        print(f"csv2json: {err}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

**JavaScript**

```js
// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
import { readFileSync } from "node:fs";

// JavaScript has no enums. A frozen object of strings is the usual stand-in, though a
// misspelled State.QOUTED is undefined, not an error.
const State = Object.freeze({
  RECORD_START: "RECORD_START", // nothing of this record read yet
  FIELD_START: "FIELD_START", // just after a comma
  UNQUOTED: "UNQUOTED", // in a field that doesn't start with a quote
  QUOTED: "QUOTED", // in a field that does
  QUOTE: "QUOTE", // just after a quote in a quoted field: its end, or the first of ""
  CR: "CR", // just after a "\r" outside quotes, which needs a "\n" next
});

class CsvError extends Error {
  constructor(line, column, message) {
    super(`line ${line}, column ${column}: ${message}`);
  }
}

const fields = (n) => (n === 1 ? "1 field" : `${n} fields`);

// The records in text, one at a time. Throws a CsvError at the first error.
function* records(text) {
  let state = State.RECORD_START;
  let record = [];
  let field = "";
  let width = 0; // how many fields the first record has, once it has ended
  let line = 1;
  let column = 0; // where char is: columns count characters, from 1
  let start = [0, 0]; // where the quote that began a quoted field is, or the "\r"
  let recordLine = 1;
  // for...of goes through code points, not UTF-16 units, so "🦊" is one character. The
  // "" after them stands for the end, so that the end goes through the machine as well.
  for (const char of [...text, ""]) {
    column += 1;
    if (state === State.RECORD_START) recordLine = line;
    let ended = false; // whether char ends a record
    if (state === State.QUOTED) {
      if (char === "") {
        throw new CsvError(...start, "quoted field is never closed");
      }
      if (char === '"') state = State.QUOTE;
      else field += char;
    } else if (state === State.QUOTE && char === '"') {
      field += '"';
      state = State.QUOTED;
    } else if (state === State.CR) {
      if (char !== "\n") {
        throw new CsvError(
          ...start,
          "a carriage return without a line feed after it",
        );
      }
      ended = true;
    } else if (state === State.RECORD_START && char === "") {
      // the end, after a line end or at the start: no record
    } else if (char === '"' && state !== State.QUOTE) {
      if (state === State.UNQUOTED) {
        throw new CsvError(line, column, "a quote in an unquoted field");
      }
      start = [line, column];
      state = State.QUOTED;
    } else if (char === ",") {
      record.push(field);
      field = "";
      state = State.FIELD_START;
    } else if (char === "\n" || char === "") {
      ended = true;
    } else if (char === "\r") {
      start = [line, column];
      state = State.CR;
    } else if (state === State.QUOTE) {
      throw new CsvError(
        line,
        column,
        "expected a comma or a line end after a closing quote",
      );
    } else {
      field += char;
      state = State.UNQUOTED;
    }
    if (ended) {
      record.push(field);
      field = "";
      if (width === 0) {
        width = record.length;
      } else if (record.length !== width) {
        const message = `record has ${fields(record.length)}, but the first has ${width}`;
        throw new CsvError(recordLine, 1, message);
      }
      yield record;
      record = [];
      state = State.RECORD_START;
    }
    if (char === "\n") {
      line += 1;
      column = 0;
    }
  }
}

try {
  for (const record of records(readFileSync(0, "utf8"))) {
    console.log(JSON.stringify(record));
  }
} catch (err) {
  if (!(err instanceof CsvError)) throw err;
  console.error(`csv2json: ${err.message}`);
  process.exitCode = 1;
}
```

**TypeScript**

```ts
// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
import { readFileSync } from "node:fs";

// A union of strings, not an enum: Node can run TypeScript only by stripping out its
// types, and an enum isn't just a type. A misspelled state is a type error.
type State =
  | "recordStart" // nothing of this record read yet
  | "fieldStart" // just after a comma
  | "unquoted" // in a field that doesn't start with a quote
  | "quoted" // in a field that does
  | "quote" // just after a quote in a quoted field: its end, or the first of ""
  | "cr"; // just after a "\r" outside quotes, which needs a "\n" next

class CsvError extends Error {
  constructor(line: number, column: number, message: string) {
    super(`line ${line}, column ${column}: ${message}`);
  }
}

const fields = (n: number): string => (n === 1 ? "1 field" : `${n} fields`);

// The records in text, one at a time. Throws a CsvError at the first error.
function* records(text: string): Generator<string[]> {
  let state: State = "recordStart";
  let record: string[] = [];
  let field = "";
  let width = 0; // how many fields the first record has, once it has ended
  let line = 1;
  let column = 0; // where char is: columns count characters, from 1
  let start: [number, number] = [0, 0]; // where a quoted field's quote is, or the "\r"
  let recordLine = 1;
  // for...of goes through code points, not UTF-16 units, so "🦊" is one character. The
  // "" after them stands for the end, so that the end goes through the machine as well.
  for (const char of [...text, ""]) {
    column += 1;
    if (state === "recordStart") recordLine = line;
    let ended = false; // whether char ends a record
    switch (state) {
      case "quoted":
        if (char === "") {
          throw new CsvError(...start, "quoted field is never closed");
        }
        if (char === '"') state = "quote";
        else field += char;
        break;
      case "cr":
        if (char !== "\n") {
          throw new CsvError(
            ...start,
            "a carriage return without a line feed after it",
          );
        }
        ended = true;
        break;
      case "recordStart":
      case "fieldStart":
      case "unquoted":
      case "quote":
        if (char === '"') {
          if (state === "unquoted") {
            throw new CsvError(line, column, "a quote in an unquoted field");
          }
          if (state === "quote") field += '"';
          else start = [line, column];
          state = "quoted";
        } else if (char === ",") {
          record.push(field);
          field = "";
          state = "fieldStart";
        } else if (char === "\n" || char === "") {
          // At the very start, or just after a line end, the end makes no record.
          ended = !(char === "" && state === "recordStart");
        } else if (char === "\r") {
          start = [line, column];
          state = "cr";
        } else if (state === "quote") {
          throw new CsvError(
            line,
            column,
            "expected a comma or a line end after a closing quote",
          );
        } else {
          field += char;
          state = "unquoted";
        }
        break;
      default: {
        // Here state's type is never, unless a case above is missing.
        const missing: never = state;
        throw new Error(`no case for ${String(missing)}`);
      }
    }
    if (ended) {
      record.push(field);
      field = "";
      if (width === 0) {
        width = record.length;
      } else if (record.length !== width) {
        const message = `record has ${fields(record.length)}, but the first has ${width}`;
        throw new CsvError(recordLine, 1, message);
      }
      yield record;
      record = [];
      state = "recordStart";
    }
    if (char === "\n") {
      line += 1;
      column = 0;
    }
  }
}

try {
  for (const record of records(readFileSync(0, "utf8"))) {
    console.log(JSON.stringify(record));
  }
} catch (err) {
  if (!(err instanceof CsvError)) throw err;
  console.error(`csv2json: ${err.message}`);
  process.exitCode = 1;
}
```

**Dart**

```dart
// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
import 'dart:convert';
import 'dart:io';

enum State {
  recordStart, // nothing of this record read yet
  fieldStart, // just after a comma
  unquoted, // in a field that doesn't start with a quote
  quoted, // in a field that does
  quote, // just after a quote in a quoted field: its end, or the first of ""
  cr, // just after a "\r" outside quotes, which needs a "\n" next
}

class CsvError implements Exception {
  final int line, column;
  final String message;
  CsvError(this.line, this.column, this.message);

  @override
  String toString() => 'line $line, column $column: $message';
}

String fields(int n) => n == 1 ? '1 field' : '$n fields';

/// The records in text, one at a time. Throws a CsvError at the first error.
Iterable<List<String>> records(String text) sync* {
  var state = State.recordStart;
  var record = <String>[];
  final field = StringBuffer();
  var width = 0; // how many fields the first record has, once it has ended
  // Where char is: columns count characters, from 1.
  var (line, column) = (1, 0);
  // Where the quote that began a quoted field is, or the "\r".
  var start = (0, 0);
  var recordLine = 1;
  // A String is UTF-16, and runes are its code points, so "🦊" is one character. The
  // null after them stands for the end, so that the end goes through the machine too.
  for (final char in [...text.runes.map(String.fromCharCode), null]) {
    column++;
    if (state == State.recordStart) recordLine = line;
    var ended = false; // whether char ends a record
    // Each case is a state and a character; the first that fits is taken.
    switch ((state, char)) {
      case (State.quoted, null):
        throw CsvError(start.$1, start.$2, 'quoted field is never closed');
      case (State.quoted, '"'):
        state = State.quote;
      case (State.quoted, final String c):
        field.write(c);
      case (State.quote, '"'):
        field.write('"');
        state = State.quoted;
      case (State.cr, '\n'):
        ended = true;
      case (State.cr, _):
        throw CsvError(
          start.$1,
          start.$2,
          'a carriage return without a line feed after it',
        );
      case (State.recordStart, null):
        break;
      case (State.recordStart || State.fieldStart, '"'):
        start = (line, column);
        state = State.quoted;
      case (State.unquoted, '"'):
        throw CsvError(line, column, 'a quote in an unquoted field');
      // From here on, the state is recordStart, fieldStart, unquoted, or quote.
      case (_, ','):
        record.add(field.toString());
        field.clear();
        state = State.fieldStart;
      case (_, '\n' || null):
        ended = true;
      case (_, '\r'):
        start = (line, column);
        state = State.cr;
      case (State.quote, _):
        throw CsvError(
          line,
          column,
          'expected a comma or a line end after a closing quote',
        );
      case (_, final String c):
        field.write(c);
        state = State.unquoted;
    }
    if (ended) {
      record.add(field.toString());
      field.clear();
      if (width == 0) {
        width = record.length;
      } else if (record.length != width) {
        final message =
            'record has ${fields(record.length)}, but the first has $width';
        throw CsvError(recordLine, 1, message);
      }
      yield record;
      record = [];
      state = State.recordStart;
    }
    if (char == '\n') (line, column) = (line + 1, 0);
  }
}

Future<void> main() async {
  final text = await utf8.decodeStream(stdin);
  try {
    for (final record in records(text)) {
      print(jsonEncode(record));
    }
  } on CsvError catch (err) {
    stderr.writeln('csv2json: $err');
    exitCode = 1;
  }
}
```

**Kotlin**

```kotlin
// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
import kotlin.system.exitProcess

enum class State {
    RECORD_START, // nothing of this record read yet
    FIELD_START, // just after a comma
    UNQUOTED, // in a field that doesn't start with a quote
    QUOTED, // in a field that does
    QUOTE, // just after a quote in a quoted field: its end, or the first of ""
    CR, // just after a "\r" outside quotes, which needs a "\n" next
}

class CsvError(line: Int, column: Int, message: String) :
    Exception("line $line, column $column: $message")

fun fields(n: Int) = if (n == 1) "1 field" else "$n fields"

/** The records in text, one at a time. Throws a CsvError at the first error. */
fun records(text: String): Sequence<List<String>> = sequence {
    var state = State.RECORD_START
    var record = mutableListOf<String>()
    val field = StringBuilder()
    var width = 0 // how many fields the first record has, once it has ended
    var line = 1
    var column = 0 // where char is: columns count characters, from 1
    var start = Pair(0, 0) // where the quote that began a quoted field is, or the "\r"
    var recordLine = 1
    // A String is UTF-16, and codePoints() goes through its code points, so "🦊" is one
    // character. The null after them stands for the end, so the end goes through too.
    val chars = text.codePoints().toArray().map { Character.toString(it) } + null
    for (char in chars) {
        column++
        if (state == State.RECORD_START) recordLine = line
        var ended = false // whether char ends a record
        // The first branch whose condition holds is taken.
        when {
            state == State.QUOTED && char == null ->
                throw CsvError(start.first, start.second, "quoted field is never closed")
            state == State.QUOTED && char == "\"" -> state = State.QUOTE
            state == State.QUOTED -> field.append(char)
            state == State.QUOTE && char == "\"" -> {
                field.append('"')
                state = State.QUOTED
            }
            state == State.CR && char == "\n" -> ended = true
            state == State.CR ->
                throw CsvError(
                    start.first,
                    start.second,
                    "a carriage return without a line feed after it",
                )
            state == State.RECORD_START && char == null -> {}
            (state == State.RECORD_START || state == State.FIELD_START) && char == "\"" -> {
                start = Pair(line, column)
                state = State.QUOTED
            }
            state == State.UNQUOTED && char == "\"" ->
                throw CsvError(line, column, "a quote in an unquoted field")
            // From here on, the state is RECORD_START, FIELD_START, UNQUOTED, or QUOTE.
            char == "," -> {
                record.add(field.toString())
                field.clear()
                state = State.FIELD_START
            }
            char == "\n" || char == null -> ended = true
            char == "\r" -> {
                start = Pair(line, column)
                state = State.CR
            }
            state == State.QUOTE ->
                throw CsvError(line, column, "expected a comma or a line end after a closing quote")
            else -> {
                field.append(char)
                state = State.UNQUOTED
            }
        }
        if (ended) {
            record.add(field.toString())
            field.clear()
            if (width == 0) {
                width = record.size
            } else if (record.size != width) {
                val message = "record has ${fields(record.size)}, but the first has $width"
                throw CsvError(recordLine, 1, message)
            }
            yield(record)
            record = mutableListOf()
            state = State.RECORD_START
        }
        if (char == "\n") {
            line++
            column = 0
        }
    }
}

/** s as a JSON string, as JavaScript's JSON.stringify writes it. */
fun json(s: String): String = buildString {
    append('"')
    for (c in s) {
        when {
            c == '"' -> append("\\\"")
            c == '\\' -> append("\\\\")
            c == '\b' -> append("\\b")
            c == '\u000c' -> append("\\f")
            c == '\n' -> append("\\n")
            c == '\r' -> append("\\r")
            c == '\t' -> append("\\t")
            c < ' ' -> append("\\u%04x".format(c.code))
            else -> append(c)
        }
    }
    append('"')
}

fun main() {
    val text = System.`in`.readBytes().decodeToString()
    // A BufferedWriter writes in big pieces, and in UTF-8.
    val out = System.out.bufferedWriter()
    try {
        for (record in records(text)) {
            out.write(record.joinToString(",", "[", "]\n") { json(it) })
        }
    } catch (err: CsvError) {
        out.flush()
        System.err.println("csv2json: ${err.message}")
        exitProcess(1)
    }
    out.flush()
}
```

```text
$ csv2json < height.csv
["name","height"]
csv2json: line 2, column 8: a quote in an unquoted field
```

A state machine is what a regular expression is, written out: every regex can be turned
into one, which is how Go's and Rust's engines and exercise 22.3 run them. Writing it by
hand costs more lines, and buys three things. Each state has a name, so the code says
what it's doing. Each error is a state and a character with nowhere to go, so the
program knows exactly where it is, and can say `a quote in an unquoted field` at line 2,
column 8, rather than "no match". And the machine reads one character at a time, so it
can read a file of any size, or a socket, without holding all of it, as HTTP's readers
do in chapter 36.

Where the languages differ is in how they write the table down. Python's
`match state, char`, Rust's `match (state, char)`, and Dart's `switch` on the record
`(state, char)` read like the table, a case at a time, and Rust's compiler checks that
no case is missing. Kotlin's `when` and Go's `switch` without a value test a condition
on the state and the character in each branch, in order, and C, C++, and JavaScript do
the same with a chain of `if` and `else if`. TypeScript switches on the state. The
states are an enum in C, C++, Rust, Python, Dart, and Kotlin, and constants in Go.
JavaScript has no enums, so its states are a frozen object of strings, in which a
misspelled `State.QOUTED` is `undefined` rather than an error. TypeScript's are a union
of string literal types, in which a misspelling is a type error; its `enum` isn't
allowed under `--erasableSyntaxOnly`, which Node's type stripping needs, and tsc turns
it down with `TS1294`.

---

## 22.7 Tokens and positions

Example 03 is a calculator. It reads arithmetic, one expression on each line, with whole
numbers that fit in 64 bits and the operators `+`, `-`, `*`, `/`, `%`, `^`, and unary
`-`, with parentheses, and prints each expression with all of its parentheses written
out, and its value. At an error, it prints the line, a caret under the column, and what
went wrong, then goes on to the next line, and exits with 1 at the end.

```text
$ cat sums.txt
1 + 2 * 3
2 ^ 3 ^ 2
-2 ^ 2
2 + * 3
$ calc < sums.txt
(1 + (2 * 3)) = 7
(2 ^ (3 ^ 2)) = 512
(-(2 ^ 2)) = -4
calc: line 4, column 5: expected a number, "(", or "-", found "*"
  2 + * 3
      ^
```

**C**

```c
// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
#define _POSIX_C_SOURCE 200809L  // for getline, from POSIX
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no exceptions, so each function that can fail fills in an Error and says so.
typedef struct {
    int column;  // counted in characters, from 1
    char message[128];
} Error;

static void *checked(void *p) {
    if (p == NULL) {
        fputs("calc: out of memory\n", stderr);
        exit(1);
    }
    return p;
}

// The text is UTF-8, so a character is one to four bytes, and all but the first are
// 10xxxxxx. This is how many bytes the character that starts at text[i] has.
static size_t char_length(const char *text, size_t i) {
    size_t n = 1;
    while (((unsigned char)text[i + n] & 0xC0) == 0x80) n++;  // the '\0' ends it
    return n;
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column.
typedef enum { NUMBER_TOKEN, OPERATOR_TOKEN, END_TOKEN } Kind;  // operators include ( )

typedef struct {
    Kind kind;
    const char *text;  // in the line, not ended with a '\0'
    int length;
    int column;
    int64_t value;  // a number's value
} Token;

// Writes "the end of the line", or the token in quotes, into out.
static void describe(const Token *token, char *out, size_t size) {
    if (token->kind == END_TOKEN) {
        snprintf(out, size, "the end of the line");
    } else {
        snprintf(out, size, "\"%.*s\"", token->length, token->text);
    }
}

static bool is_digit(char c) { return '0' <= c && c <= '9'; }

// Returns the tokens, ended by an END_TOKEN, or NULL with error filled in.
static Token *tokens(const char *line, Error *error) {
    size_t count = 0, capacity = 16;
    Token *found = checked(malloc(capacity * sizeof *found));
    int column = 1;
    size_t i = 0;
    for (; line[i] != '\0'; column++) {
        if (count + 1 == capacity) {  // room for this token and the END_TOKEN
            capacity *= 2;
            found = checked(realloc(found, capacity * sizeof *found));
        }
        char c = line[i];
        if (c == ' ' || c == '\t') {
            i++;
        } else if (is_digit(c)) {
            size_t start = i;
            while (is_digit(line[i])) i++;
            // strtoll stops at the first character that isn't a digit. It would also
            // take spaces and a sign before them, but there are none.
            errno = 0;
            int64_t value = strtoll(line + start, NULL, 10);
            if (errno == ERANGE) {
                *error = (Error){.column = column, .message = "number too large for 64 bits"};
                free(found);
                return NULL;
            }
            int length = (int)(i - start);
            found[count++] = (Token){NUMBER_TOKEN, line + start, length, column, value};
            column += length - 1;
        } else if (strchr("+-*/%^()", c) != NULL) {
            found[count++] = (Token){OPERATOR_TOKEN, line + i, 1, column, 0};
            i++;
        } else {
            error->column = column;
            snprintf(error->message, sizeof error->message, "unexpected \"%.*s\"",
                     (int)char_length(line, i), line + i);
            free(found);
            return NULL;
        }
    }
    found[count] = (Token){END_TOKEN, line + i, 0, column, 0};
    return found;
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. C has no sum
// types, so a node is a struct with a kind, and a union of what each kind holds. Only
// the member for the kind may be used. column is where the operator is, for an error
// such as overflow.
typedef struct Expr Expr;
struct Expr {
    enum { NUMBER, NEGATE, BINARY } kind;
    int column;
    union {
        int64_t value;                                   // NUMBER
        Expr *operand;                                   // NEGATE
        struct { char op; Expr *left, *right; } binary;  // BINARY
    };
};

static void free_expr(Expr *expr) {
    if (expr == NULL) return;
    if (expr->kind == NEGATE) free_expr(expr->operand);
    if (expr->kind == BINARY) {
        free_expr(expr->binary.left);
        free_expr(expr->binary.right);
    }
    free(expr);
}

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2). Returns false if token
// isn't an infix operator.
static bool infix(const Token *token, int *left, int *right) {
    if (token->kind != OPERATOR_TOKEN) return false;
    switch (token->text[0]) {
        case '+': case '-': *left = 1; *right = 2; return true;
        case '*': case '/': case '%': *left = 3; *right = 4; return true;
        case '^': *left = 8; *right = 7; return true;
        default: return false;
    }
}

// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
enum { PREFIX = 5 };

typedef struct {
    const Token *tokens;
    size_t at;
    Error *error;
} Parser;

static const Token *next(Parser *p) {
    const Token *token = &p->tokens[p->at];
    if (token->kind != END_TOKEN) p->at++;
    return token;
}

static void fail(Parser *p, const Token *token, const char *expected) {
    char found[64];
    describe(token, found, sizeof found);
    p->error->column = token->column;
    snprintf(p->error->message, sizeof p->error->message, "expected %s, found %s", expected,
             found);
}

static Expr *new_expr(Expr node) {
    Expr *expr = checked(malloc(sizeof *expr));
    *expr = node;
    return expr;
}

// Parses the longest expression whose operators hold at least min_power. Returns NULL
// with the error filled in if it can't.
static Expr *expression(Parser *p, int min_power) {
    const Token *token = next(p);
    Expr *left;
    if (token->kind == NUMBER_TOKEN) {
        left = new_expr((Expr){.kind = NUMBER, .value = token->value});
    } else if (token->text[0] == '-') {
        Expr *operand = expression(p, PREFIX);
        if (operand == NULL) return NULL;
        left = new_expr((Expr){.kind = NEGATE, .column = token->column, .operand = operand});
    } else if (token->text[0] == '(') {
        left = expression(p, 0);
        if (left == NULL) return NULL;
        const Token *close = next(p);
        if (close->kind != OPERATOR_TOKEN || close->text[0] != ')') {
            fail(p, close, "\")\"");
            free_expr(left);
            return NULL;
        }
    } else {
        fail(p, token, "a number, \"(\", or \"-\"");
        return NULL;
    }
    int left_power, right_power;
    while (infix(&p->tokens[p->at], &left_power, &right_power) && left_power >= min_power) {
        const Token *op = next(p);
        Expr *right = expression(p, right_power);
        if (right == NULL) {
            free_expr(left);
            return NULL;
        }
        left = new_expr((Expr){.kind = BINARY, .column = op->column,
                               .binary = {op->text[0], left, right}});
    }
    return left;
}

// Returns the tree for line, or NULL with error filled in.
static Expr *parse(const char *line, Error *error) {
    Token *found = tokens(line, error);
    if (found == NULL) return NULL;
    Parser p = {found, 0, error};
    Expr *expr = expression(&p, 0);
    if (expr != NULL) {
        const Token *token = next(&p);
        if (token->kind != END_TOKEN) {
            fail(&p, token, "an operator");
            free_expr(expr);
            expr = NULL;
        }
    }
    free(found);
    return expr;
}

static void show(const Expr *expr) {
    switch (expr->kind) {
        case NUMBER: printf("%" PRId64, expr->value); break;
        case NEGATE:
            printf("(-");
            show(expr->operand);
            printf(")");
            break;
        case BINARY:
            printf("(");
            show(expr->binary.left);
            printf(" %c ", expr->binary.op);
            show(expr->binary.right);
            printf(")");
            break;
    }
}

static bool error_at(Error *error, int column, const char *message) {
    error->column = column;
    snprintf(error->message, sizeof error->message, "%s", message);
    return false;
}

// Signed overflow is undefined behavior in C, so the answer must be checked before it's
// made. GCC and Clang's __builtin_mul_overflow and the others work out the answer, and
// say whether it overflowed; C23 has them as ckd_mul and the others, in <stdckdint.h>.
static bool power(int64_t base, int64_t exponent, int64_t *result, int column, Error *error) {
    *result = 1;
    // Squares the base for each bit of the exponent, and stops at the first overflow.
    while (exponent > 0) {
        if (exponent % 2 == 1 && __builtin_mul_overflow(*result, base, result)) {
            return error_at(error, column, "overflow");
        }
        exponent /= 2;
        if (exponent > 0 && __builtin_mul_overflow(base, base, &base)) {
            return error_at(error, column, "overflow");
        }
    }
    return true;
}

// Works out the value of expr into *value. Returns false with error filled in if it
// can't.
static bool evaluate(const Expr *expr, int64_t *value, Error *error) {
    switch (expr->kind) {
        case NUMBER:
            *value = expr->value;
            return true;
        case NEGATE:
            if (!evaluate(expr->operand, value, error)) return false;
            if (*value == INT64_MIN) return error_at(error, expr->column, "overflow");
            *value = -*value;
            return true;
        case BINARY: break;
    }
    int64_t a, b;
    if (!evaluate(expr->binary.left, &a, error) || !evaluate(expr->binary.right, &b, error)) {
        return false;
    }
    char op = expr->binary.op;
    int column = expr->column;
    if ((op == '/' || op == '%') && b == 0) return error_at(error, column, "division by zero");
    bool overflow = false;
    switch (op) {
        case '+': overflow = __builtin_add_overflow(a, b, value); break;
        case '-': overflow = __builtin_sub_overflow(a, b, value); break;
        case '*': overflow = __builtin_mul_overflow(a, b, value); break;
        // / and % round toward zero. INT64_MIN / -1 and INT64_MIN % -1 are undefined,
        // since the quotient doesn't fit; the remainder is 0.
        case '/':
            overflow = a == INT64_MIN && b == -1;
            if (!overflow) *value = a / b;
            break;
        case '%': *value = b == -1 ? 0 : a % b; break;
        default:  // '^'
            if (b < 0) return error_at(error, column, "negative exponent");
            return power(a, b, value, column, error);
    }
    return overflow ? error_at(error, column, "overflow") : true;
}

int main(void) {
    int status = 0;
    char *line = NULL;
    size_t size = 0;
    ssize_t length;
    // getline keeps the '\n', and a '\r' before it.
    for (long number = 1; (length = getline(&line, &size, stdin)) != -1; number++) {
        if (length > 0 && line[length - 1] == '\n') line[--length] = '\0';
        if (length > 0 && line[length - 1] == '\r') line[--length] = '\0';
        if (line[strspn(line, " \t")] == '\0') continue;
        Error error;
        int64_t value;
        Expr *expr = parse(line, &error);
        if (expr != NULL && evaluate(expr, &value, &error)) {
            show(expr);
            printf(" = %" PRId64 "\n", value);
        } else {
            fprintf(stderr, "calc: line %ld, column %d: %s\n  %s\n  ", number, error.column,
                    error.message, line);
            // The caret goes under the column. A tab before it stays a tab, so that the
            // caret lines up with the text whatever a tab's width is.
            for (size_t i = 0, column = 1; column < (size_t)error.column; column++) {
                fputc(line[i] == '\t' ? '\t' : ' ', stderr);
                i += char_length(line, i);
            }
            fputs("^\n", stderr);
            status = 1;
        }
        free_expr(expr);
    }
    free(line);
    return status;
}
```

**C++**

```cpp
// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
#include <charconv>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

struct CalcError : std::runtime_error {
    int column;  // counted in characters, from 1
    CalcError(int column, const std::string& message)
        : std::runtime_error(message), column(column) {}
};

// The text is UTF-8, so a character is one to four bytes, and all but the first are
// 10xxxxxx. This is how many bytes the character that starts at text[i] has.
static std::size_t char_length(std::string_view text, std::size_t i) {
    std::size_t n = 1;
    while (i + n < text.size() && (static_cast<unsigned char>(text[i + n]) & 0xC0) == 0x80) n++;
    return n;
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column.
enum class Kind { Number, Operator, End };  // Operator includes the parentheses

struct Token {
    Kind kind;
    std::string text;
    int column;
    std::int64_t value = 0;  // a number's value

    std::string describe() const {
        return kind == Kind::End ? "the end of the line" : "\"" + text + "\"";
    }
};

static bool is_digit(char c) { return '0' <= c && c <= '9'; }

static std::vector<Token> tokens(std::string_view line) {
    std::vector<Token> found;
    int column = 1;
    for (std::size_t i = 0; i < line.size(); column++) {
        char c = line[i];
        if (c == ' ' || c == '\t') {
            i++;
        } else if (is_digit(c)) {
            std::size_t start = i;
            while (i < line.size() && is_digit(line[i])) i++;
            std::int64_t value = 0;
            // from_chars takes only the digits, with no spaces or "+", and says
            // result_out_of_range if the number doesn't fit.
            auto [end, err] = std::from_chars(line.data() + start, line.data() + i, value);
            if (err == std::errc::result_out_of_range) {
                throw CalcError(column, "number too large for 64 bits");
            }
            std::string text(line.substr(start, i - start));
            found.push_back({Kind::Number, text, column, value});
            column += static_cast<int>(i - start) - 1;
        } else if (std::string_view("+-*/%^()").find(c) != std::string_view::npos) {
            found.push_back({Kind::Operator, std::string(1, c), column});
            i++;
        } else {
            std::string text(line.substr(i, char_length(line, i)));
            throw CalcError(column, "unexpected \"" + text + "\"");
        }
    }
    found.push_back({Kind::End, "", column});
    return found;
}

// The parser turns the tokens into a tree, whose nodes are of three kinds: an Expr holds
// one of them, in a variant. A node holds the nodes under it through unique_ptrs, which
// free them when it goes. column is where the operator is, for an error such as overflow.
struct Expr;
using ExprPtr = std::unique_ptr<Expr>;
struct Number {
    std::int64_t value;
};
struct Negate {
    ExprPtr operand;
    int column;
};
struct Binary {
    char op;
    ExprPtr left, right;
    int column;
};
struct Expr {
    std::variant<Number, Negate, Binary> node;
};

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
static std::optional<std::pair<int, int>> infix(const Token& token) {
    if (token.kind != Kind::Operator) return std::nullopt;
    switch (token.text[0]) {
        case '+': case '-': return std::pair{1, 2};
        case '*': case '/': case '%': return std::pair{3, 4};
        case '^': return std::pair{8, 7};
        default: return std::nullopt;
    }
}

// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
constexpr int prefix = 5;

class Parser {
  public:
    explicit Parser(std::string_view line) : tokens_(tokens(line)) {}

    ExprPtr parse() {
        ExprPtr expr = expression(0);
        const Token& token = next();
        if (token.kind != Kind::End) {
            throw CalcError(token.column, "expected an operator, found " + token.describe());
        }
        return expr;
    }

  private:
    std::vector<Token> tokens_;
    std::size_t at_ = 0;

    const Token& next() {
        const Token& token = tokens_[at_];
        if (token.kind != Kind::End) at_++;
        return token;
    }

    // Parses the longest expression whose operators hold at least min_power.
    ExprPtr expression(int min_power) {
        const Token& token = next();
        ExprPtr left;
        if (token.kind == Kind::Number) {
            left = std::make_unique<Expr>(Number{token.value});
        } else if (token.text == "-") {
            left = std::make_unique<Expr>(Negate{expression(prefix), token.column});
        } else if (token.text == "(") {
            left = expression(0);
            const Token& close = next();
            if (close.text != ")") {
                throw CalcError(close.column, "expected \")\", found " + close.describe());
            }
        } else {
            throw CalcError(token.column,
                            "expected a number, \"(\", or \"-\", found " + token.describe());
        }
        for (;;) {
            const Token& op = tokens_[at_];
            auto powers = infix(op);
            if (!powers || powers->first < min_power) return left;
            next();
            ExprPtr right = expression(powers->second);
            left = std::make_unique<Expr>(Binary{op.text[0], std::move(left), std::move(right),
                                                 op.column});
        }
    }
};

// Calls the lambda that takes the node's type: the usual way to visit a variant.
template <typename... F>
struct Overloaded : F... {
    using F::operator()...;
};

static std::string show(const Expr& expr) {
    return std::visit(
        Overloaded{
            [](const Number& n) { return std::to_string(n.value); },
            [](const Negate& n) { return "(-" + show(*n.operand) + ")"; },
            [](const Binary& b) {
                return "(" + show(*b.left) + " " + b.op + " " + show(*b.right) + ")";
            },
        },
        expr.node);
}

// Signed overflow is undefined behavior in C++, so the answer must be checked before
// it's made. GCC and Clang's __builtin_mul_overflow and the others work out the answer,
// and say whether it overflowed.
static std::int64_t power(std::int64_t base, std::int64_t exponent, int column) {
    std::int64_t result = 1;
    // Squares the base for each bit of the exponent, and stops at the first overflow.
    while (exponent > 0) {
        if (exponent % 2 == 1 && __builtin_mul_overflow(result, base, &result)) {
            throw CalcError(column, "overflow");
        }
        exponent /= 2;
        if (exponent > 0 && __builtin_mul_overflow(base, base, &base)) {
            throw CalcError(column, "overflow");
        }
    }
    return result;
}

static std::int64_t evaluate(const Expr& expr) {
    constexpr std::int64_t min = std::numeric_limits<std::int64_t>::min();
    return std::visit(
        Overloaded{
            [](const Number& n) { return n.value; },
            [](const Negate& n) {
                std::int64_t value = evaluate(*n.operand);
                if (value == min) throw CalcError(n.column, "overflow");
                return -value;
            },
            [](const Binary& b) {
                std::int64_t x = evaluate(*b.left), y = evaluate(*b.right), result = 0;
                bool overflow = false;
                if ((b.op == '/' || b.op == '%') && y == 0) {
                    throw CalcError(b.column, "division by zero");
                }
                switch (b.op) {
                    case '+': overflow = __builtin_add_overflow(x, y, &result); break;
                    case '-': overflow = __builtin_sub_overflow(x, y, &result); break;
                    case '*': overflow = __builtin_mul_overflow(x, y, &result); break;
                    // / and % round toward zero. min / -1 and min % -1 are undefined,
                    // since the quotient doesn't fit; the remainder is 0.
                    case '/': overflow = x == min && y == -1; result = overflow ? 0 : x / y; break;
                    case '%': result = y == -1 ? 0 : x % y; break;
                    default:  // '^'
                        if (y < 0) throw CalcError(b.column, "negative exponent");
                        result = power(x, y, b.column);
                }
                if (overflow) throw CalcError(b.column, "overflow");
                return result;
            },
        },
        expr.node);
}

int main() {
    int status = 0;
    std::string line;
    // getline splits at "\n" only, and keeps a "\r" before it.
    for (int number = 1; std::getline(std::cin, line); number++) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.find_first_not_of(" \t") == std::string::npos) continue;
        try {
            ExprPtr expr = Parser(line).parse();
            // Worked out first: in one << chain, show's text would be written before
            // evaluate threw.
            std::int64_t value = evaluate(*expr);
            std::cout << show(*expr) << " = " << value << '\n';
        } catch (const CalcError& err) {
            // The caret goes under the column. A tab before it stays a tab, so that the
            // caret lines up with the text whatever a tab's width is.
            std::string pad;
            for (std::size_t i = 0; static_cast<int>(pad.size()) < err.column - 1;
                 i += char_length(line, i)) {
                pad += line[i] == '\t' ? '\t' : ' ';
            }
            std::cerr << "calc: line " << number << ", column " << err.column << ": "
                      << err.what() << "\n  " << line << "\n  " << pad << "^\n";
            status = 1;
        }
    }
    return status;
}
```

**Rust**

```rust
//! Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
//! its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
//! and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
//! round toward zero, as in C. An error is printed with its line and a caret under its
//! column, the next line is read, and at the end, the exit status is 1.
use std::io::{self, Read};
use std::process::ExitCode;

struct CalcError {
    column: usize, // counted in characters, from 1
    message: String,
}

fn error<T>(column: usize, message: impl Into<String>) -> Result<T, CalcError> {
    Err(CalcError {
        column,
        message: message.into(),
    })
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each with
// its column.
#[derive(Clone, Copy, PartialEq)]
enum Kind {
    Number(i64),
    Op(char), // an operator or a parenthesis
    End,      // after the last token
}

#[derive(Clone)]
struct Token {
    kind: Kind,
    text: String,
    column: usize,
}

fn describe(token: &Token) -> String {
    match token.kind {
        Kind::End => "the end of the line".to_string(),
        _ => format!("\"{}\"", token.text),
    }
}

fn tokens(line: &str) -> Result<Vec<Token>, CalcError> {
    // A Vec of chars, so that an index counts code points, and i + 1 is the column.
    let chars: Vec<char> = line.chars().collect();
    let mut found = Vec::new();
    let mut i = 0;
    while i < chars.len() {
        let c = chars[i];
        // is_ascii_digit, not is_numeric, which is true for "٣" and "²".
        if c == ' ' || c == '\t' {
            i += 1;
        } else if c.is_ascii_digit() {
            let start = i;
            while i < chars.len() && chars[i].is_ascii_digit() {
                i += 1;
            }
            let text: String = chars[start..i].iter().collect();
            // Only digits, so parse fails only if the number doesn't fit.
            let Ok(value) = text.parse() else {
                return error(start + 1, "number too large for 64 bits");
            };
            found.push(Token {
                kind: Kind::Number(value),
                text,
                column: start + 1,
            });
        } else if "+-*/%^()".contains(c) {
            found.push(Token {
                kind: Kind::Op(c),
                text: c.to_string(),
                column: i + 1,
            });
            i += 1;
        } else {
            return error(i + 1, format!("unexpected \"{c}\""));
        }
    }
    found.push(Token {
        kind: Kind::End,
        text: String::new(),
        column: chars.len() + 1,
    });
    Ok(found)
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. A Box holds a
// node on the heap, since an Expr can't hold another Expr inside itself.
enum Expr {
    Number(i64),
    // column is where the operator is, for an error such as overflow.
    Negate {
        operand: Box<Expr>,
        column: usize,
    },
    Binary {
        op: char,
        left: Box<Expr>,
        right: Box<Expr>,
        column: usize,
    },
}

/// How tightly each operator holds the expressions on its left and right: the higher,
/// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
/// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
fn infix(op: char) -> Option<(u8, u8)> {
    match op {
        '+' | '-' => Some((1, 2)),
        '*' | '/' | '%' => Some((3, 4)),
        '^' => Some((8, 7)),
        _ => None,
    }
}

/// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const PREFIX: u8 = 5;

struct Parser {
    tokens: Vec<Token>,
    at: usize,
}

impl Parser {
    fn next(&mut self) -> Token {
        let token = self.tokens[self.at].clone();
        if token.kind != Kind::End {
            self.at += 1;
        }
        token
    }

    /// Parses the longest expression whose operators hold at least min_power.
    fn expression(&mut self, min_power: u8) -> Result<Expr, CalcError> {
        let token = self.next();
        let mut left = match token.kind {
            Kind::Number(value) => Expr::Number(value),
            Kind::Op('-') => {
                let operand = Box::new(self.expression(PREFIX)?);
                Expr::Negate {
                    operand,
                    column: token.column,
                }
            }
            Kind::Op('(') => {
                let inside = self.expression(0)?;
                let close = self.next();
                if close.kind != Kind::Op(')') {
                    return error(
                        close.column,
                        format!("expected \")\", found {}", describe(&close)),
                    );
                }
                inside
            }
            _ => {
                let found = describe(&token);
                return error(
                    token.column,
                    format!("expected a number, \"(\", or \"-\", found {found}"),
                );
            }
        };
        loop {
            let token = &self.tokens[self.at];
            let Kind::Op(op) = token.kind else {
                return Ok(left);
            };
            let Some((left_power, right_power)) = infix(op) else {
                return Ok(left);
            };
            if left_power < min_power {
                return Ok(left);
            }
            let column = self.next().column;
            let right = Box::new(self.expression(right_power)?);
            left = Expr::Binary {
                op,
                left: Box::new(left),
                right,
                column,
            };
        }
    }

    fn parse(line: &str) -> Result<Expr, CalcError> {
        let mut parser = Parser {
            tokens: tokens(line)?,
            at: 0,
        };
        let expr = parser.expression(0)?;
        let token = parser.next();
        if token.kind != Kind::End {
            return error(
                token.column,
                format!("expected an operator, found {}", describe(&token)),
            );
        }
        Ok(expr)
    }
}

fn show(expr: &Expr) -> String {
    match expr {
        Expr::Number(value) => value.to_string(),
        Expr::Negate { operand, .. } => format!("(-{})", show(operand)),
        Expr::Binary {
            op, left, right, ..
        } => format!("({} {op} {})", show(left), show(right)),
    }
}

/// The value, or an overflow error at column if checked arithmetic found none.
fn fit(value: Option<i64>, column: usize) -> Result<i64, CalcError> {
    value.map_or_else(|| error(column, "overflow"), Ok)
}

/// Squares the base for each bit of the exponent, and stops at the first overflow.
/// i64's checked_pow takes the exponent as a u32.
fn power(mut base: i64, mut exponent: i64, column: usize) -> Result<i64, CalcError> {
    let mut result: i64 = 1;
    while exponent > 0 {
        if exponent % 2 == 1 {
            result = fit(result.checked_mul(base), column)?;
        }
        exponent /= 2;
        if exponent > 0 {
            base = fit(base.checked_mul(base), column)?;
        }
    }
    Ok(result)
}

fn evaluate(expr: &Expr) -> Result<i64, CalcError> {
    match expr {
        Expr::Number(value) => Ok(*value),
        Expr::Negate { operand, column } => fit(evaluate(operand)?.checked_neg(), *column),
        Expr::Binary {
            op,
            left,
            right,
            column,
        } => {
            let (a, b) = (evaluate(left)?, evaluate(right)?);
            let column = *column;
            if (*op == '/' || *op == '%') && b == 0 {
                return error(column, "division by zero");
            }
            // checked_ gives None where the answer doesn't fit in an i64; / and % round
            // toward zero. i64::MIN % -1 overflows, but its remainder is 0, as
            // wrapping_rem says.
            match op {
                '+' => fit(a.checked_add(b), column),
                '-' => fit(a.checked_sub(b), column),
                '*' => fit(a.checked_mul(b), column),
                '/' => fit(a.checked_div(b), column),
                '%' => Ok(a.wrapping_rem(b)),
                _ if b < 0 => error(column, "negative exponent"),
                _ => power(a, b, column),
            }
        }
    }
}

fn main() -> ExitCode {
    let mut text = String::new();
    if let Err(err) = io::stdin().read_to_string(&mut text) {
        eprintln!("calc: {err}");
        return ExitCode::FAILURE;
    }
    let mut status = ExitCode::SUCCESS;
    // lines() splits at "\n" and removes a "\r" before it.
    for (index, line) in text.lines().enumerate() {
        if line.trim_matches([' ', '\t']).is_empty() {
            continue;
        }
        match Parser::parse(line).and_then(|expr| Ok((show(&expr), evaluate(&expr)?))) {
            Ok((shown, value)) => println!("{shown} = {value}"),
            Err(err) => {
                // The caret goes under the column. A tab before it stays a tab, so that
                // the caret lines up with the text whatever a tab's width is.
                let pad: String = line
                    .chars()
                    .take(err.column - 1)
                    .map(|c| if c == '\t' { '\t' } else { ' ' })
                    .collect();
                eprintln!(
                    "calc: line {}, column {}: {}",
                    index + 1,
                    err.column,
                    err.message
                );
                eprintln!("  {line}\n  {pad}^");
                status = ExitCode::FAILURE;
            }
        }
    }
    status
}
```

**Go**

```go
// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
package main

import (
	"bufio"
	"fmt"
	"math"
	"os"
	"strconv"
	"strings"
)

type calcError struct {
	column  int // counted in characters, from 1
	message string
}

func (e *calcError) Error() string { return e.message }

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column.
type kind int

const (
	number   kind = iota
	operator      // an operator or a parenthesis
	end           // after the last token
)

type token struct {
	kind   kind
	text   string
	value  int64 // a number's value
	column int
}

func describe(t token) string {
	if t.kind == end {
		return "the end of the line"
	}
	return `"` + t.text + `"`
}

func isDigit(r rune) bool { return '0' <= r && r <= '9' }

func tokens(line string) ([]token, error) {
	// A []rune, so that an index counts code points, and i + 1 is the column.
	chars := []rune(line)
	var found []token
	for i := 0; i < len(chars); {
		c := chars[i]
		switch {
		case c == ' ' || c == '\t':
			i++
		case isDigit(c):
			start := i
			for i < len(chars) && isDigit(chars[i]) {
				i++
			}
			text := string(chars[start:i])
			// Only digits, so ParseInt fails only if the number doesn't fit.
			value, err := strconv.ParseInt(text, 10, 64)
			if err != nil {
				return nil, &calcError{start + 1, "number too large for 64 bits"}
			}
			found = append(found, token{number, text, value, start + 1})
		case strings.ContainsRune("+-*/%^()", c):
			found = append(found, token{operator, string(c), 0, i + 1})
			i++
		default:
			return nil, &calcError{i + 1, `unexpected "` + string(c) + `"`}
		}
	}
	return append(found, token{end, "", 0, len(chars) + 1}), nil
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. Go has no sum
// types: an expr is any type with an isExpr method, and these three are the only ones.
type expr interface{ isExpr() }

type numberExpr struct{ value int64 }

type negateExpr struct {
	operand expr
	column  int // where the operator is, for an error such as overflow
}

type binaryExpr struct {
	op          string
	left, right expr
	column      int
}

func (numberExpr) isExpr() {}
func (negateExpr) isExpr() {}
func (binaryExpr) isExpr() {}

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
var infix = map[string][2]int{
	"+": {1, 2}, "-": {1, 2}, "*": {3, 4}, "/": {3, 4}, "%": {3, 4}, "^": {8, 7},
}

// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const prefix = 5

type parser struct {
	tokens []token
	at     int
}

func (p *parser) next() token {
	t := p.tokens[p.at]
	if t.kind != end {
		p.at++
	}
	return t
}

// expression parses the longest expression whose operators hold at least minPower.
func (p *parser) expression(minPower int) (expr, error) {
	t := p.next()
	var left expr
	switch {
	case t.kind == number:
		left = numberExpr{t.value}
	case t.text == "-":
		operand, err := p.expression(prefix)
		if err != nil {
			return nil, err
		}
		left = negateExpr{operand, t.column}
	case t.text == "(":
		inside, err := p.expression(0)
		if err != nil {
			return nil, err
		}
		if close := p.next(); close.text != ")" {
			return nil, &calcError{close.column, `expected ")", found ` + describe(close)}
		}
		left = inside
	default:
		return nil, &calcError{t.column, `expected a number, "(", or "-", found ` + describe(t)}
	}
	for {
		t := p.tokens[p.at]
		powers, ok := infix[t.text]
		if t.kind != operator || !ok || powers[0] < minPower {
			return left, nil
		}
		p.next()
		right, err := p.expression(powers[1])
		if err != nil {
			return nil, err
		}
		left = binaryExpr{t.text, left, right, t.column}
	}
}

func parse(line string) (expr, error) {
	found, err := tokens(line)
	if err != nil {
		return nil, err
	}
	p := &parser{tokens: found}
	e, err := p.expression(0)
	if err != nil {
		return nil, err
	}
	if t := p.next(); t.kind != end {
		return nil, &calcError{t.column, "expected an operator, found " + describe(t)}
	}
	return e, nil
}

func show(e expr) string {
	switch e := e.(type) {
	case numberExpr:
		return strconv.FormatInt(e.value, 10)
	case negateExpr:
		return "(-" + show(e.operand) + ")"
	case binaryExpr:
		return "(" + show(e.left) + " " + e.op + " " + show(e.right) + ")"
	}
	panic("not an expr")
}

// Go's integers wrap around when they overflow, with no error, so each of these checks
// first that the answer fits. It returns false if it doesn't.
func apply(op string, a, b int64) (int64, bool) {
	switch op {
	case "+":
		return a + b, !(b > 0 && a > math.MaxInt64-b || b < 0 && a < math.MinInt64-b)
	case "-":
		return a - b, !(b < 0 && a > math.MaxInt64+b || b > 0 && a < math.MinInt64+b)
	case "*":
		if a == 0 || b == 0 {
			return 0, true
		}
		c := a * b
		return c, c/b == a && !(a == -1 && b == math.MinInt64) && !(b == -1 && a == math.MinInt64)
	case "/": // rounds toward zero
		return a / b, !(a == math.MinInt64 && b == -1)
	case "%": // has the sign of a, and math.MinInt64 % -1 is 0
		return a % b, true
	}
	// "^": squares the base for each bit of the exponent, and stops at the first overflow.
	result := int64(1)
	for ok := true; b > 0; b /= 2 {
		if b%2 == 1 {
			if result, ok = apply("*", result, a); !ok {
				return 0, false
			}
		}
		if b > 1 {
			if a, ok = apply("*", a, a); !ok {
				return 0, false
			}
		}
	}
	return result, true
}

func evaluate(e expr) (int64, error) {
	switch e := e.(type) {
	case numberExpr:
		return e.value, nil
	case negateExpr:
		value, err := evaluate(e.operand)
		if err != nil {
			return 0, err
		}
		if value == math.MinInt64 {
			return 0, &calcError{e.column, "overflow"}
		}
		return -value, nil
	case binaryExpr:
		a, err := evaluate(e.left)
		if err != nil {
			return 0, err
		}
		b, err := evaluate(e.right)
		if err != nil {
			return 0, err
		}
		switch {
		case (e.op == "/" || e.op == "%") && b == 0:
			return 0, &calcError{e.column, "division by zero"}
		case e.op == "^" && b < 0:
			return 0, &calcError{e.column, "negative exponent"}
		}
		value, ok := apply(e.op, a, b)
		if !ok {
			return 0, &calcError{e.column, "overflow"}
		}
		return value, nil
	}
	panic("not an expr")
}

func main() {
	status := 0
	scanner := bufio.NewScanner(os.Stdin)
	// A Scanner splits at "\n" and removes a "\r" before it.
	for n := 1; scanner.Scan(); n++ {
		line := scanner.Text()
		if strings.Trim(line, " \t") == "" {
			continue
		}
		e, err := parse(line)
		var value int64
		if err == nil {
			value, err = evaluate(e)
		}
		if err == nil {
			fmt.Printf("%s = %d\n", show(e), value)
			continue
		}
		column := err.(*calcError).column
		// The caret goes under the column. A tab before it stays a tab, so that the caret
		// lines up with the text whatever a tab's width is.
		pad := []rune(line)[:column-1]
		for i, c := range pad {
			if c != '\t' {
				pad[i] = ' '
			}
		}
		fmt.Fprintf(os.Stderr, "calc: line %d, column %d: %s\n", n, column, err)
		fmt.Fprintf(os.Stderr, "  %s\n  %s^\n", line, string(pad))
		status = 1
	}
	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "calc:", err)
		status = 1
	}
	os.Exit(status)
}
```

**Python**

```python
# Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
# its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
# and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
# round toward zero, as in C. An error is printed with its line and a caret under its
# column, the next line is read, and at the end, the exit status is 1.
import sys
from dataclasses import dataclass

MAX = 2**63 - 1
MIN = -(2**63)


class CalcError(Exception):
    def __init__(self, column: int, message: str) -> None:
        super().__init__(message)
        self.column = column  # counted in characters, from 1


# The lexer turns the text into tokens: numbers, and operators and parentheses, each with
# its column.
@dataclass(frozen=True)
class Token:
    kind: str  # "number", "end" after the last token, or the character: "+", "(", ...
    text: str
    column: int
    value: int = 0  # a number's value


def describe(token: Token) -> str:
    return "the end of the line" if token.kind == "end" else f'"{token.text}"'


def tokens(line: str) -> list[Token]:
    found: list[Token] = []
    i = 0
    while i < len(line):  # a str's indexes count code points, so i + 1 is the column
        char = line[i]
        if char in " \t":
            i += 1
        # Not char.isdigit(), which is true for "٣" and "²", and int() takes "٣".
        elif "0" <= char <= "9":
            start = i
            while i < len(line) and "0" <= line[i] <= "9":
                i += 1
            value = int(line[start:i])
            if value > MAX:
                raise CalcError(start + 1, "number too large for 64 bits")
            found.append(Token("number", line[start:i], start + 1, value))
        elif char in "+-*/%^()":
            found.append(Token(char, char, i + 1))
            i += 1
        else:
            raise CalcError(i + 1, f'unexpected "{char}"')
    found.append(Token("end", "", len(line) + 1))
    return found


# The parser turns the tokens into a tree, whose nodes are of three kinds.
@dataclass(frozen=True)
class Number:
    value: int


@dataclass(frozen=True)
class Negate:
    operand: "Expr"  # in quotes, because Expr is defined below, from these classes
    column: int  # where the operator is, for an error such as overflow


@dataclass(frozen=True)
class Binary:
    op: str
    left: "Expr"
    right: "Expr"
    column: int


type Expr = Number | Negate | Binary

# How tightly each operator holds the expressions on its left and right: the higher, the
# tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
# (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
INFIX = {"+": (1, 2), "-": (1, 2), "*": (3, 4), "/": (3, 4), "%": (3, 4), "^": (8, 7)}
# Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
PREFIX = 5


class Parser:
    def __init__(self, line: str) -> None:
        self.tokens = tokens(line)
        self.at = 0

    def next(self) -> Token:
        token = self.tokens[self.at]
        if token.kind != "end":
            self.at += 1
        return token

    def expression(self, min_power: int) -> Expr:
        """Parses the longest expression whose operators hold at least min_power."""
        token = self.next()
        left: Expr
        match token.kind:
            case "number":
                left = Number(token.value)
            case "-":
                left = Negate(self.expression(PREFIX), token.column)
            case "(":
                left = self.expression(0)
                close = self.next()
                if close.kind != ")":
                    raise CalcError(
                        close.column, f'expected ")", found {describe(close)}'
                    )
            case _:
                message = f'expected a number, "(", or "-", found {describe(token)}'
                raise CalcError(token.column, message)
        while True:
            token = self.tokens[self.at]
            powers = INFIX.get(token.kind)
            if powers is None or powers[0] < min_power:
                return left
            self.next()
            left = Binary(token.kind, left, self.expression(powers[1]), token.column)

    def parse(self) -> Expr:
        expr = self.expression(0)
        token = self.next()
        if token.kind != "end":
            raise CalcError(
                token.column, f"expected an operator, found {describe(token)}"
            )
        return expr


def show(expr: Expr) -> str:
    match expr:
        case Number(value):
            return str(value)
        case Negate(operand):
            return f"(-{show(operand)})"
        case Binary(op, left, right):
            return f"({show(left)} {op} {show(right)})"


# Python's ints have no limit, so each result is checked against 64 bits by hand.
def fit(value: int, column: int) -> int:
    if not MIN <= value <= MAX:
        raise CalcError(column, "overflow")
    return value


def power(base: int, exponent: int, column: int) -> int:
    # base ** exponent would work out 1 ** 9223372036854775807 by multiplying, and
    # 2 ** 9223372036854775807 until memory ran out. This squares the base for each bit
    # of the exponent, and stops at the first overflow.
    result = 1
    while exponent > 0:
        if exponent % 2 == 1:
            result = fit(result * base, column)
        exponent //= 2
        if exponent > 0:
            base = fit(base * base, column)
    return result


def evaluate(expr: Expr) -> int:
    match expr:
        case Number(value):
            return value
        case Negate(operand, column):
            return fit(-evaluate(operand), column)
        case Binary(op, left, right, column):
            a, b = evaluate(left), evaluate(right)
            if op in "/%" and b == 0:
                raise CalcError(column, "division by zero")
            match op:
                case "+":
                    return fit(a + b, column)
                case "-":
                    return fit(a - b, column)
                case "*":
                    return fit(a * b, column)
                # Python's // and % round toward minus infinity, so -7 // 2 is -4 and
                # -7 % 2 is 1. These round toward zero, as C's / and % do.
                case "/":
                    quotient = abs(a) // abs(b)
                    return fit(quotient if (a < 0) == (b < 0) else -quotient, column)
                case "%":
                    remainder = abs(a) % abs(b)
                    return remainder if a >= 0 else -remainder
                case _:  # "^"
                    if b < 0:
                        raise CalcError(column, "negative exponent")
                    return power(a, b, column)


def main() -> int:
    status = 0
    text = sys.stdin.read()
    lines = text.split("\n")
    if lines[-1] == "":  # the input ended with a newline, or was empty
        lines.pop()
    for number, line in enumerate(lines, 1):
        line = line.removesuffix("\r")
        if line.strip(" \t") == "":
            continue
        try:
            expr = Parser(line).parse()
            print(f"{show(expr)} = {evaluate(expr)}")
        except CalcError as err:
            # The caret goes under the column. A tab before it stays a tab, so that the
            # caret lines up with the text whatever a tab's width is.
            pad = "".join("\t" if c == "\t" else " " for c in line[: err.column - 1])
            print(f"calc: line {number}, column {err.column}: {err}", file=sys.stderr)
            print(f"  {line}\n  {pad}^", file=sys.stderr)
            status = 1
    return status


if __name__ == "__main__":
    sys.exit(main())
```

**JavaScript**

```js
// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
import { readFileSync } from "node:fs";

class CalcError extends Error {
  constructor(column, message) {
    super(message);
    this.column = column; // counted in characters, from 1
  }
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column. A token's kind is "number", "operator" (or a parenthesis), or "end",
// after the last token. A number is 64 bits, which is more than a Number can hold
// exactly, so it is a BigInt.

const MAX = 2n ** 63n - 1n;
const MIN = -(2n ** 63n);

function describe(token) {
  return token.kind === "end" ? "the end of the line" : `"${token.text}"`;
}

function tokens(line) {
  // Spread into an array of code points, so that i + 1 is the column.
  const chars = [...line];
  const found = [];
  // Only ASCII digits go to BigInt, which would also take "0x1F", "+5", and spaces
  // around the digits, and says BigInt("") is 0n.
  const isDigit = (c) => c !== undefined && c >= "0" && c <= "9";
  let i = 0;
  while (i < chars.length) {
    const c = chars[i];
    if (c === " " || c === "\t") {
      i += 1;
    } else if (isDigit(c)) {
      const start = i;
      while (isDigit(chars[i])) i += 1;
      const text = chars.slice(start, i).join("");
      const value = BigInt(text);
      if (value > MAX) {
        throw new CalcError(start + 1, "number too large for 64 bits");
      }
      found.push({ kind: "number", text, value, column: start + 1 });
    } else if ("+-*/%^()".includes(c)) {
      found.push({ kind: "operator", text: c, column: i + 1 });
      i += 1;
    } else {
      throw new CalcError(i + 1, `unexpected "${c}"`);
    }
  }
  found.push({ kind: "end", text: "", column: chars.length + 1 });
  return found;
}

// The parser turns the tokens into a tree, whose nodes are of three kinds: "number",
// "negate", and "binary". column is where the operator is, for an error such as
// overflow.

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
const INFIX = new Map([
  ["+", [1, 2]],
  ["-", [1, 2]],
  ["*", [3, 4]],
  ["/", [3, 4]],
  ["%", [3, 4]],
  ["^", [8, 7]],
]);
// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const PREFIX = 5;

class Parser {
  #tokens;
  #at = 0;

  constructor(line) {
    this.#tokens = tokens(line);
  }

  #next() {
    const token = this.#tokens[this.#at];
    if (token.kind !== "end") this.#at += 1;
    return token;
  }

  // Parses the longest expression whose operators hold at least minPower.
  #expression(minPower) {
    const token = this.#next();
    let left;
    if (token.kind === "number") {
      left = { kind: "number", value: token.value };
    } else if (token.text === "-") {
      const operand = this.#expression(PREFIX);
      left = { kind: "negate", operand, column: token.column };
    } else if (token.text === "(") {
      left = this.#expression(0);
      const close = this.#next();
      if (close.text !== ")") {
        throw new CalcError(
          close.column,
          `expected ")", found ${describe(close)}`,
        );
      }
    } else {
      const found = describe(token);
      throw new CalcError(
        token.column,
        `expected a number, "(", or "-", found ${found}`,
      );
    }
    for (;;) {
      const token = this.#tokens[this.#at];
      const powers =
        token.kind === "operator" ? INFIX.get(token.text) : undefined;
      if (powers === undefined || powers[0] < minPower) return left;
      this.#next();
      const right = this.#expression(powers[1]);
      left = {
        kind: "binary",
        op: token.text,
        left,
        right,
        column: token.column,
      };
    }
  }

  parse() {
    const expr = this.#expression(0);
    const token = this.#next();
    if (token.kind !== "end") {
      throw new CalcError(
        token.column,
        `expected an operator, found ${describe(token)}`,
      );
    }
    return expr;
  }
}

function show(expr) {
  switch (expr.kind) {
    case "number":
      return expr.value.toString();
    case "negate":
      return `(-${show(expr.operand)})`;
    case "binary":
      return `(${show(expr.left)} ${expr.op} ${show(expr.right)})`;
  }
}

// A BigInt has no limit, so each result is checked against 64 bits by hand.
function fit(value, column) {
  if (value < MIN || value > MAX) throw new CalcError(column, "overflow");
  return value;
}

// base ** exponent would give up only once the BigInt grew too large, with a
// RangeError. This squares the base for each bit of the exponent, and stops at the
// first overflow.
function power(base, exponent, column) {
  let result = 1n;
  while (exponent > 0n) {
    if (exponent % 2n === 1n) result = fit(result * base, column);
    exponent /= 2n;
    if (exponent > 0n) base = fit(base * base, column);
  }
  return result;
}

function evaluate(expr) {
  switch (expr.kind) {
    case "number":
      return expr.value;
    case "negate":
      return fit(-evaluate(expr.operand), expr.column);
    default: {
      const a = evaluate(expr.left);
      const b = evaluate(expr.right);
      const column = expr.column;
      if ((expr.op === "/" || expr.op === "%") && b === 0n) {
        throw new CalcError(column, "division by zero");
      }
      // A BigInt's / and % round toward zero, as C's do.
      switch (expr.op) {
        case "+":
          return fit(a + b, column);
        case "-":
          return fit(a - b, column);
        case "*":
          return fit(a * b, column);
        case "/":
          return fit(a / b, column);
        case "%":
          return a % b;
        default: // "^"
          if (b < 0n) throw new CalcError(column, "negative exponent");
          return power(a, b, column);
      }
    }
  }
}

let status = 0;
const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop(); // the input ended with a newline, or was empty
lines.forEach((text, index) => {
  const line = text.endsWith("\r") ? text.slice(0, -1) : text;
  if (line.replaceAll(/[ \t]/g, "") === "") return;
  try {
    const expr = new Parser(line).parse();
    console.log(`${show(expr)} = ${evaluate(expr)}`);
  } catch (err) {
    if (!(err instanceof CalcError)) throw err;
    // The caret goes under the column. A tab before it stays a tab, so that the caret
    // lines up with the text whatever a tab's width is.
    const before = [...line].slice(0, err.column - 1);
    const pad = before.map((c) => (c === "\t" ? "\t" : " ")).join("");
    console.error(
      `calc: line ${index + 1}, column ${err.column}: ${err.message}`,
    );
    console.error(`  ${line}\n  ${pad}^`);
    status = 1;
  }
});
process.exitCode = status;
```

**TypeScript**

```ts
// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
import { readFileSync } from "node:fs";

class CalcError extends Error {
  readonly column: number; // counted in characters, from 1
  constructor(column: number, message: string) {
    super(message);
    this.column = column;
  }
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column. A number is 64 bits, which is more than a number can hold exactly,
// so it is a bigint.
type Token =
  | { kind: "number"; text: string; value: bigint; column: number }
  | { kind: "operator"; text: string; column: number } // or a parenthesis
  | { kind: "end"; text: ""; column: number }; // after the last token

const MAX = 2n ** 63n - 1n;
const MIN = -(2n ** 63n);

function describe(token: Token): string {
  return token.kind === "end" ? "the end of the line" : `"${token.text}"`;
}

function tokens(line: string): Token[] {
  // Spread into an array of code points, so that i + 1 is the column.
  const chars = [...line];
  const found: Token[] = [];
  // Only ASCII digits go to BigInt, which would also take "0x1F", "+5", and spaces
  // around the digits, and says BigInt("") is 0n.
  const isDigit = (c: string | undefined) =>
    c !== undefined && c >= "0" && c <= "9";
  let i = 0;
  while (i < chars.length) {
    const c = chars[i];
    if (c === " " || c === "\t") {
      i += 1;
    } else if (isDigit(c)) {
      const start = i;
      while (isDigit(chars[i])) i += 1;
      const text = chars.slice(start, i).join("");
      const value = BigInt(text);
      if (value > MAX) {
        throw new CalcError(start + 1, "number too large for 64 bits");
      }
      found.push({ kind: "number", text, value, column: start + 1 });
    } else if ("+-*/%^()".includes(c)) {
      found.push({ kind: "operator", text: c, column: i + 1 });
      i += 1;
    } else {
      throw new CalcError(i + 1, `unexpected "${c}"`);
    }
  }
  found.push({ kind: "end", text: "", column: chars.length + 1 });
  return found;
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. column is
// where the operator is, for an error such as overflow.
type Expr =
  | { kind: "number"; value: bigint }
  | { kind: "negate"; operand: Expr; column: number }
  | { kind: "binary"; op: string; left: Expr; right: Expr; column: number };

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
const INFIX = new Map<string, [number, number]>([
  ["+", [1, 2]],
  ["-", [1, 2]],
  ["*", [3, 4]],
  ["/", [3, 4]],
  ["%", [3, 4]],
  ["^", [8, 7]],
]);
// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const PREFIX = 5;

class Parser {
  private readonly tokens: Token[];
  private at = 0;

  constructor(line: string) {
    this.tokens = tokens(line);
  }

  private next(): Token {
    const token = this.tokens[this.at];
    if (token.kind !== "end") this.at += 1;
    return token;
  }

  // Parses the longest expression whose operators hold at least minPower.
  private expression(minPower: number): Expr {
    const token = this.next();
    let left: Expr;
    if (token.kind === "number") {
      left = { kind: "number", value: token.value };
    } else if (token.text === "-") {
      const operand = this.expression(PREFIX);
      left = { kind: "negate", operand, column: token.column };
    } else if (token.text === "(") {
      left = this.expression(0);
      const close = this.next();
      if (close.text !== ")") {
        throw new CalcError(
          close.column,
          `expected ")", found ${describe(close)}`,
        );
      }
    } else {
      const found = describe(token);
      throw new CalcError(
        token.column,
        `expected a number, "(", or "-", found ${found}`,
      );
    }
    for (;;) {
      const token = this.tokens[this.at];
      const powers =
        token.kind === "operator" ? INFIX.get(token.text) : undefined;
      if (powers === undefined || powers[0] < minPower) return left;
      this.next();
      const right = this.expression(powers[1]);
      left = {
        kind: "binary",
        op: token.text,
        left,
        right,
        column: token.column,
      };
    }
  }

  parse(): Expr {
    const expr = this.expression(0);
    const token = this.next();
    if (token.kind !== "end") {
      throw new CalcError(
        token.column,
        `expected an operator, found ${describe(token)}`,
      );
    }
    return expr;
  }
}

function show(expr: Expr): string {
  switch (expr.kind) {
    case "number":
      return expr.value.toString();
    case "negate":
      return `(-${show(expr.operand)})`;
    case "binary":
      return `(${show(expr.left)} ${expr.op} ${show(expr.right)})`;
  }
}

// A bigint has no limit, so each result is checked against 64 bits by hand.
function fit(value: bigint, column: number): bigint {
  if (value < MIN || value > MAX) throw new CalcError(column, "overflow");
  return value;
}

// base ** exponent would give up only once the bigint grew too large, with a
// RangeError. This squares the base for each bit of the exponent, and stops at the
// first overflow.
function power(base: bigint, exponent: bigint, column: number): bigint {
  let result = 1n;
  while (exponent > 0n) {
    if (exponent % 2n === 1n) result = fit(result * base, column);
    exponent /= 2n;
    if (exponent > 0n) base = fit(base * base, column);
  }
  return result;
}

function evaluate(expr: Expr): bigint {
  switch (expr.kind) {
    case "number":
      return expr.value;
    case "negate":
      return fit(-evaluate(expr.operand), expr.column);
    case "binary": {
      const a = evaluate(expr.left);
      const b = evaluate(expr.right);
      const column = expr.column;
      if ((expr.op === "/" || expr.op === "%") && b === 0n) {
        throw new CalcError(column, "division by zero");
      }
      // A bigint's / and % round toward zero, as C's do.
      switch (expr.op) {
        case "+":
          return fit(a + b, column);
        case "-":
          return fit(a - b, column);
        case "*":
          return fit(a * b, column);
        case "/":
          return fit(a / b, column);
        case "%":
          return a % b;
        default: // "^"
          if (b < 0n) throw new CalcError(column, "negative exponent");
          return power(a, b, column);
      }
    }
  }
}

let status = 0;
const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop(); // the input ended with a newline, or was empty
lines.forEach((text, index) => {
  const line = text.endsWith("\r") ? text.slice(0, -1) : text;
  if (line.replaceAll(/[ \t]/g, "") === "") return;
  try {
    const expr = new Parser(line).parse();
    console.log(`${show(expr)} = ${evaluate(expr)}`);
  } catch (err) {
    if (!(err instanceof CalcError)) throw err;
    // The caret goes under the column. A tab before it stays a tab, so that the caret
    // lines up with the text whatever a tab's width is.
    const before = [...line].slice(0, err.column - 1);
    const pad = before.map((c) => (c === "\t" ? "\t" : " ")).join("");
    console.error(
      `calc: line ${index + 1}, column ${err.column}: ${err.message}`,
    );
    console.error(`  ${line}\n  ${pad}^`);
    status = 1;
  }
});
process.exitCode = status;
```

**Dart**

```dart
// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
import 'dart:convert';
import 'dart:io';

class CalcError implements Exception {
  final int column; // counted in characters, from 1
  final String message;
  CalcError(this.column, this.message);
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column.
enum Kind { number, operator, end } // operator includes the parentheses

class Token {
  final Kind kind;
  final String text;
  final int value; // a number's value
  final int column;
  Token(this.kind, this.text, this.column, [this.value = 0]);

  String describe() => kind == Kind.end ? 'the end of the line' : '"$text"';
}

bool isDigit(String c) => c.compareTo('0') >= 0 && c.compareTo('9') <= 0;

List<Token> tokens(String line) {
  // A String is UTF-16, so this splits it into code points, and i + 1 is the column.
  final chars = line.runes.map(String.fromCharCode).toList();
  final found = <Token>[];
  var i = 0;
  while (i < chars.length) {
    final c = chars[i];
    if (c == ' ' || c == '\t') {
      i++;
    } else if (isDigit(c)) {
      final start = i;
      while (i < chars.length && isDigit(chars[i])) {
        i++;
      }
      final text = chars.sublist(start, i).join();
      // tryParse gives null if the number doesn't fit, where parse would throw.
      final value = int.tryParse(text);
      if (value == null) {
        throw CalcError(start + 1, 'number too large for 64 bits');
      }
      found.add(Token(Kind.number, text, start + 1, value));
    } else if ('+-*/%^()'.contains(c)) {
      found.add(Token(Kind.operator, c, i + 1));
      i++;
    } else {
      throw CalcError(i + 1, 'unexpected "$c"');
    }
  }
  found.add(Token(Kind.end, '', chars.length + 1));
  return found;
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. A sealed
// class can have subclasses only in this file, so a switch on an Expr can know that it
// has a case for each. column is where the operator is, for an error such as overflow.
sealed class Expr {}

class Number extends Expr {
  final int value;
  Number(this.value);
}

class Negate extends Expr {
  final Expr operand;
  final int column;
  Negate(this.operand, this.column);
}

class Binary extends Expr {
  final String op;
  final Expr left, right;
  final int column;
  Binary(this.op, this.left, this.right, this.column);
}

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
const infix = {
  '+': (1, 2),
  '-': (1, 2),
  '*': (3, 4),
  '/': (3, 4),
  '%': (3, 4),
  '^': (8, 7),
};
// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const prefix = 5;

class Parser {
  final List<Token> _tokens;
  var _at = 0;

  Parser(String line) : _tokens = tokens(line);

  Token _next() {
    final token = _tokens[_at];
    if (token.kind != Kind.end) _at++;
    return token;
  }

  /// Parses the longest expression whose operators hold at least minPower.
  Expr _expression(int minPower) {
    final token = _next();
    var left = switch (token) {
      Token(kind: Kind.number, :final value) => Number(value),
      Token(text: '-') => Negate(_expression(prefix), token.column),
      Token(text: '(') => _parenthesized(),
      _ => throw CalcError(
        token.column,
        'expected a number, "(", or "-", found ${token.describe()}',
      ),
    };
    while (true) {
      final token = _tokens[_at];
      final powers = token.kind == Kind.operator ? infix[token.text] : null;
      if (powers == null || powers.$1 < minPower) return left;
      _next();
      left = Binary(token.text, left, _expression(powers.$2), token.column);
    }
  }

  Expr _parenthesized() {
    final inside = _expression(0);
    final close = _next();
    if (close.text != ')') {
      throw CalcError(close.column, 'expected ")", found ${close.describe()}');
    }
    return inside;
  }

  Expr parse() {
    final expr = _expression(0);
    final token = _next();
    if (token.kind != Kind.end) {
      throw CalcError(
        token.column,
        'expected an operator, found ${token.describe()}',
      );
    }
    return expr;
  }
}

String show(Expr expr) => switch (expr) {
  Number(:final value) => '$value',
  Negate(:final operand) => '(-${show(operand)})',
  Binary(:final op, :final left, :final right) =>
    '(${show(left)} $op ${show(right)})',
};

// Dart's ints are 64 bits, and wrap around when they overflow, with no error. So the
// arithmetic is done with BigInts, which have no limit, and each result is checked.
int fit(BigInt value, int column) {
  if (!value.isValidInt) throw CalcError(column, 'overflow');
  return value.toInt();
}

// Squares the base for each bit of the exponent, and stops at the first overflow.
int power(int base, int exponent, int column) {
  var result = 1;
  while (exponent > 0) {
    if (exponent.isOdd) {
      result = fit(BigInt.from(result) * BigInt.from(base), column);
    }
    exponent ~/= 2;
    if (exponent > 0) base = fit(BigInt.from(base) * BigInt.from(base), column);
  }
  return result;
}

int evaluate(Expr expr) {
  switch (expr) {
    case Number(:final value):
      return value;
    case Negate(:final operand, :final column):
      return fit(-BigInt.from(evaluate(operand)), column);
    case Binary(:final op, :final left, :final right, :final column):
      final (a, b) = (
        BigInt.from(evaluate(left)),
        BigInt.from(evaluate(right)),
      );
      if ((op == '/' || op == '%') && b == BigInt.zero) {
        throw CalcError(column, 'division by zero');
      }
      return switch (op) {
        '+' => fit(a + b, column),
        '-' => fit(a - b, column),
        '*' => fit(a * b, column),
        // ~/ rounds toward zero. % would give a result that is never negative, so
        // -7 % 2 would be 1; remainder has the sign of a, as C's % does.
        '/' => fit(a ~/ b, column),
        '%' => fit(a.remainder(b), column),
        _ when b.isNegative => throw CalcError(column, 'negative exponent'),
        _ => power(a.toInt(), b.toInt(), column),
      };
  }
}

Future<void> main() async {
  var status = 0;
  final lines = (await utf8.decodeStream(stdin)).split('\n');
  // The last is empty when the input ended with a newline, or was empty.
  if (lines.last == '') lines.removeLast();
  for (final (index, text) in lines.indexed) {
    final line = text.endsWith('\r')
        ? text.substring(0, text.length - 1)
        : text;
    if (line.replaceAll(RegExp(r'[ \t]'), '').isEmpty) continue;
    try {
      final expr = Parser(line).parse();
      print('${show(expr)} = ${evaluate(expr)}');
    } on CalcError catch (err) {
      // The caret goes under the column. A tab before it stays a tab, so that the
      // caret lines up with the text whatever a tab's width is.
      final pad = line.runes
          .take(err.column - 1)
          .map((c) => String.fromCharCode(c) == '\t' ? '\t' : ' ')
          .join();
      stderr.writeln(
        'calc: line ${index + 1}, column ${err.column}: ${err.message}',
      );
      stderr.writeln('  $line\n  $pad^');
      status = 1;
    }
  }
  exitCode = status;
}
```

**Kotlin**

```kotlin
// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
import kotlin.system.exitProcess

class CalcError(val column: Int, message: String) : Exception(message) // column from 1

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column.
enum class Kind {
    NUMBER,
    OPERATOR, // or a parenthesis
    END, // after the last token
}

data class Token(val kind: Kind, val text: String, val column: Int, val value: Long = 0) {
    fun describe() = if (kind == Kind.END) "the end of the line" else "\"$text\""
}

fun tokens(line: String): List<Token> {
    // A String is UTF-16, so this splits it into code points, and i + 1 is the column.
    val chars = line.codePoints().toArray().map { Character.toString(it) }
    val found = mutableListOf<Token>()
    // Not Char.isDigit(), which is true for "٣", and toLong() takes "٣" as 3.
    fun isDigit(c: String) = c.length == 1 && c[0] in '0'..'9'
    var i = 0
    while (i < chars.size) {
        val c = chars[i]
        when {
            c == " " || c == "\t" -> i++
            isDigit(c) -> {
                val start = i
                while (i < chars.size && isDigit(chars[i])) i++
                val text = chars.subList(start, i).joinToString("")
                // toLongOrNull gives null if the number doesn't fit, where toLong throws.
                val value =
                    text.toLongOrNull()
                        ?: throw CalcError(start + 1, "number too large for 64 bits")
                found.add(Token(Kind.NUMBER, text, start + 1, value))
            }
            c in listOf("+", "-", "*", "/", "%", "^", "(", ")") -> {
                found.add(Token(Kind.OPERATOR, c, i + 1))
                i++
            }
            else -> throw CalcError(i + 1, "unexpected \"$c\"")
        }
    }
    found.add(Token(Kind.END, "", chars.size + 1))
    return found
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. A sealed
// interface can have subtypes only in this module, so a when on an Expr can know that
// it has a branch for each. column is where the operator is, for an error such as
// overflow.
sealed interface Expr

data class Number(val value: Long) : Expr

data class Negate(val operand: Expr, val column: Int) : Expr

data class Binary(val op: String, val left: Expr, val right: Expr, val column: Int) : Expr

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
val INFIX =
    mapOf(
        "+" to Pair(1, 2),
        "-" to Pair(1, 2),
        "*" to Pair(3, 4),
        "/" to Pair(3, 4),
        "%" to Pair(3, 4),
        "^" to Pair(8, 7),
    )
// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const val PREFIX = 5

class Parser(line: String) {
    private val tokens = tokens(line)
    private var at = 0

    private fun next(): Token {
        val token = tokens[at]
        if (token.kind != Kind.END) at++
        return token
    }

    /** Parses the longest expression whose operators hold at least minPower. */
    private fun expression(minPower: Int): Expr {
        val token = next()
        var left =
            when {
                token.kind == Kind.NUMBER -> Number(token.value)
                token.text == "-" -> Negate(expression(PREFIX), token.column)
                token.text == "(" -> {
                    val inside = expression(0)
                    val close = next()
                    if (close.text != ")") {
                        throw CalcError(close.column, "expected \")\", found ${close.describe()}")
                    }
                    inside
                }
                else -> {
                    val found = token.describe()
                    throw CalcError(
                        token.column,
                        "expected a number, \"(\", or \"-\", found $found",
                    )
                }
            }
        while (true) {
            val operator = tokens[at]
            val powers = if (operator.kind == Kind.OPERATOR) INFIX[operator.text] else null
            if (powers == null || powers.first < minPower) return left
            next()
            left = Binary(operator.text, left, expression(powers.second), operator.column)
        }
    }

    fun parse(): Expr {
        val expr = expression(0)
        val token = next()
        if (token.kind != Kind.END) {
            throw CalcError(token.column, "expected an operator, found ${token.describe()}")
        }
        return expr
    }
}

fun show(expr: Expr): String =
    when (expr) {
        is Number -> expr.value.toString()
        is Negate -> "(-${show(expr.operand)})"
        is Binary -> "(${show(expr.left)} ${expr.op} ${show(expr.right)})"
    }

// A Long wraps around when it overflows, with no error. Math's ...Exact functions throw
// an ArithmeticException instead, which this turns into an error at the column.
fun <T> fit(column: Int, work: () -> T): T =
    try {
        work()
    } catch (_: ArithmeticException) {
        throw CalcError(column, "overflow")
    }

// Squares the base for each bit of the exponent, and stops at the first overflow.
fun power(base: Long, exponent: Long, column: Int): Long {
    var (result, b, e) = Triple(1L, base, exponent)
    while (e > 0) {
        if (e % 2 == 1L) result = fit(column) { Math.multiplyExact(result, b) }
        e /= 2
        if (e > 0) b = fit(column) { Math.multiplyExact(b, b) }
    }
    return result
}

fun evaluate(expr: Expr): Long =
    when (expr) {
        is Number -> expr.value
        is Negate -> fit(expr.column) { Math.negateExact(evaluate(expr.operand)) }
        is Binary -> {
            val (a, b, column) = Triple(evaluate(expr.left), evaluate(expr.right), expr.column)
            if ((expr.op == "/" || expr.op == "%") && b == 0L) {
                throw CalcError(column, "division by zero")
            }
            when (expr.op) {
                "+" -> fit(column) { Math.addExact(a, b) }
                "-" -> fit(column) { Math.subtractExact(a, b) }
                "*" -> fit(column) { Math.multiplyExact(a, b) }
                // / and % round toward zero. Long.MIN_VALUE / -1 overflows, with no error,
                // to Long.MIN_VALUE; divideExact throws instead.
                "/" -> fit(column) { Math.divideExact(a, b) }
                "%" -> a % b
                else ->
                    if (b < 0) throw CalcError(column, "negative exponent") else power(a, b, column)
            }
        }
    }

fun main() {
    var status = 0
    val lines = System.`in`.readBytes().decodeToString().split("\n").toMutableList()
    // The last is empty when the input ended with a newline, or was empty.
    if (lines.last() == "") lines.removeLast()
    for ((index, text) in lines.withIndex()) {
        val line = text.removeSuffix("\r")
        if (line.trim(' ', '\t').isEmpty()) continue
        try {
            val expr = Parser(line).parse()
            println("${show(expr)} = ${evaluate(expr)}")
        } catch (err: CalcError) {
            // The caret goes under the column. A tab before it stays a tab, so that the
            // caret lines up with the text whatever a tab's width is.
            val before = line.codePoints().toArray().take(err.column - 1)
            val pad = before.joinToString("") { if (it == '\t'.code) "\t" else " " }
            System.err.println("calc: line ${index + 1}, column ${err.column}: ${err.message}")
            System.err.println("  $line\n  $pad^")
            status = 1
        }
    }
    exitProcess(status)
}
```

The program has the three parts from section 22.1. The **lexer**, `tokens`, reads the
line from left to right and makes a list of tokens, each with its kind, its text, and
its column, and one more token of kind `end`, after the last, so that the parser never
runs off the end of the list. It skips spaces and tabs, reads the longest run of digits
as one number, and for anything it doesn't know, stops with `unexpected "@"`. The lexer
reads the whole line before the parser starts, so an error in the characters is reported
before an error in the grammar, wherever each is.

The lexer is also where a number is checked. It reads only `0` to `9`, not whatever the
language's `isdigit` takes: Python's `str.isdigit` is true for `٣` and for `²`, and
`int` would take the `٣`. And it checks that the number fits in 64 bits before anything
uses it: `9223372036854775807` is a number, and `9223372036854775808` is
`number too large for 64 bits`, at its first digit.

Each token keeps its column, and the column counts characters. In Python, Go with
`[]rune`, Rust with `chars`, and Kotlin with `codePoints`, an index into what the lexer
reads is a code point, and the column is the index plus one. JavaScript and TypeScript
spread the string into code points with `[...line]`, and Dart reads its `runes`, since
their strings are UTF-16, in which `🦊` is two units. C and C++ read bytes, and count a
character at each byte that isn't a continuation byte, `10xxxxxx`. And the caret has to
line up under the column on the screen: the part of the line before the column is copied
with each character turned into a space, except a tab, which stays a tab, so that
whatever a tab's width is, the caret is under the right character.

---

## 22.8 Recursive descent and precedence

The parser turns the tokens into a **tree** of three kinds of node: a number, a
negation, and a binary operation, with an operator, a left side, and a right side. The
last two keep the column of their operator, for the errors that evaluating them can
find. Each language says "one of three kinds" in its own way, as chapter 8 showed: an
`enum` in Rust, a `sealed interface` in Kotlin, a `sealed class` in Dart, a union of
dataclasses in Python, objects with a `kind` in JavaScript, and a union of their types
in TypeScript, a `std::variant` in C++, an interface in Go, and a struct with a kind and
a union in C.

A **recursive descent** parser has a function for each rule of the grammar, which calls
the functions for the rules inside it, so that `factor`, for `(`, calls `expression`,
which calls `term`, which calls `factor` again: the recursion is the stack that a
context-free language needs. Written that way, a grammar with a level for each
precedence needs a function for each level. Example 03 uses **Pratt parsing**, from
Vaughan Pratt, in 1973, which needs only one. Each infix operator has two **binding
powers**, how tightly it holds the expression on its left and the one on its right:

| Operator | Left | Right |
| --- | --- | --- |
| `+` and `-` | 1 | 2 |
| `*`, `/`, and `%` | 3 | 4 |
| `^` | 8 | 7 |
| Unary `-` | | 5 |

`expression(min_power)` reads one operand, a number, a `-` and its operand, or an
expression in parentheses, and then, as long as the next token is an operator whose left
power is at least `min_power`, takes the operator and reads its right side with
`expression(right_power)`. In `1 + 2 * 3`, after `1`, the `+` has a left power of 1, so
it's taken, and its right side is read with a minimum of 2; inside that, after `2`, the
`*` has a left power of 3, which is at least 2, so it's taken there, and `2 * 3` becomes
the right side of the `+`. In `10 - 4 - 3`, the second `-` has a left power of 1, less
than the 2 that the first `-`'s right side asks for, so the inner call stops at `4` and
the result is `(10 - 4) - 3`: a right power higher than the left makes an operator
associate to the left. `^` has them the other way round, so `2 ^ 3 ^ 2` is
`2 ^ (3 ^ 2)`, 512, and unary minus has a power of 5, below `^`, so `-2 ^ 2` is
`-(2 ^ 2)`, as in mathematics, and Python, where `-2 ** 2` is −4. A new operator is a
new row in the table, not a new function.

The tree is then printed with all of its parentheses, which shows how it was read, and
evaluated, by a function that matches on the kind of node and calls itself for the
sides. Evaluating has its own errors, `division by zero`, `negative exponent`, and
`overflow`, each at its operator's column, since the tree kept it. Each language checks
overflow in its own way, as chapter 2 showed: `checked_add` and its sisters in Rust,
which give `None` for an answer that doesn't fit, `__builtin_add_overflow` in GCC's and
Clang's C and C++, `Math.addExact` in Kotlin, `BigInt` in JavaScript, TypeScript, and
Dart, whose results are then checked against 64 bits, a check before each operation in
Go, and in Python, whose integers have no limit, a check of each result. The edges are
in test 08: `(-9223372036854775807 - 1) / -1` overflows, and
`(-9223372036854775807 - 1) % -1` is 0, although Rust's `%` panics there with
`attempt to calculate the remainder with overflow`, so the Rust version uses
`wrapping_rem`.

---

## 22.9 Errors

A parse error has three parts: where, what was expected, and what was found. Example 03
and exercise 22.2 write all three,
`line 2, column 7: expected ")", found the end of the line`, because each part answers a
question the person will ask, and "found" names the token or the character exactly, in
quotes, or as `the end of the line`, so that an invisible problem, such as a stray tab,
is visible.

How the error travels from the parser to `main` is each language's error handling, from
chapter 11. Python, JavaScript, TypeScript, Dart, Kotlin, and C++ throw an exception
that carries the column, and `main` catches it. Rust returns `Result<Expr, CalcError>`
from every parsing function, and `?` passes an error up. Go returns `(expr, error)` and
checks each call, and C returns `NULL` from each parsing function and fills in the
parser's error. A parser has many small functions, and an exception, or Rust's `?`,
keeps the error out of the way of each of them.

What to do after an error depends on what the text is. A JSON file with one error is
wrong, and exercise 22.2 stops at the first. A calculator's lines don't depend on each
other, so example 03 reports the error and goes on to the next line, and the exit status
says that something failed. A compiler goes further, and after an error skips ahead to a
token where it can start again, such as the next `;` or `}`, and keeps parsing, so that
one run reports every mistake; the hard part is not reporting the mistakes that the
first one caused.

Parsers are also where testing with random input finds the most. A parser should never
crash, whatever it reads, and its output can be checked against itself: example 03's
printed form, `((1 + 2) * 3)`, parses back to the same tree, so printing, parsing, and
printing again should give the same text. A fuzzer feeds a parser random input and keeps
whatever makes it crash or hang, and chapter 20's shrinking then cuts such an input down
to something small enough to read.

---

## 22.10 Why each language looks the way it does

- **C** has POSIX's `<regex.h>`, from 1992, with two syntaxes, basic and extended, and
  the longest-match rule. It has no `\d`, no named groups, and no replace, and `regexec`
  reads up to a `'\0'`, so searching part of a string means copying it. `strtol` and
  `strtod` report errors through `errno` and an end pointer, and `atoi`, which reports
  nothing, is older than both.
- **C++** added `<regex>` in C++11, from Boost, with ECMAScript's syntax by default and
  five others. Its implementations are slow and, in GCC, recursive, and they can't
  change without breaking their ABI, so programs that care use RE2, PCRE2, or a
  compile-time library such as CTRE. `std::from_chars`, from C++17, is the strict, fast,
  locale-free conversion that `stoi` and `strtol` aren't.
- **Rust** keeps regular expressions out of the standard library, in a crate from the
  Rust project that took RE2's design: linear time, no backreferences, and Unicode by
  default, since a Rust `str` is always UTF-8. `str::parse` is strict because `FromStr`
  is one trait for every type, and its errors say what was wrong, such as `InvalidDigit`
  or `PosOverflow`.
- **Go**'s `regexp` is RE2's design, written in Go by Russ Cox, who also wrote RE2 and
  the articles that explain it, starting with "Regular Expression Matching Can Be Simple
  And Fast". Like exercise 22.3, it runs Ken Thompson's construction, from 1968.
  `strconv` is strict and returns an error that names the function and the text,
  `strconv.Atoi: parsing " 12 ": invalid syntax`.
- **Python**'s `re` is a backtracking engine written for Python, which took Perl's
  syntax and added `(?P<name>...)`, the `P` for Python, before Perl had named groups.
  Python 3 made it Unicode-aware with its strings, which is why `\d` and `int` both take
  every script's digits, and 3.11 added atomic groups and possessive quantifiers, which
  tame backtracking by hand.
- **JavaScript**'s `RegExp` is in the language, with literals, `/[0-9]+/g`, and flags
  that change its meaning: `u` for code points rather than UTF-16 units, `g` for every
  match, with a `lastIndex` that a reused regex remembers, and `v` for set operations.
  `Number` follows the grammar of JavaScript's own numeric literals, which is why it
  takes `0x1F` and spaces, and an empty string becomes 0 by the same rule that makes
  `+""` 0.
- **TypeScript** adds types to the matches: `exec` returns `RegExpExecArray | null`, so
  the code has to handle no match, and `groups` is
  `{ [key: string]: string } | undefined`, so it has to handle a regex without names.
  But any name at all type-checks as a `string`, so a misspelled group name gives
  `undefined` at run time.
- **Dart** took JavaScript's regular expressions so that Dart could compile to
  JavaScript and use the browser's, and the VM runs a port of V8's engine. Its
  `replaceAll` takes the replacement as plain text, with `replaceAllMapped` for anything
  computed, and its `int.tryParse` returns `null` rather than throwing, as Dart prefers
  for input that may be bad.
- **Kotlin**'s `Regex` wraps Java's `java.util.regex`, a backtracking engine from Java
  1.4, with Kotlin's names, `matchEntire` and `find`, and `MatchResult.groupValues`. Its
  `toIntOrNull` is Kotlin's own, and takes any digit that Java's `Character.digit`
  knows, which is every script's.

---

## 22.11 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | `atoi` on text from outside | 0 for `"abc"` and for `"0"` alike, and −1 for 20 nines; use `strtol`, and check `errno` and that the end pointer reached the end |
| C | `strtol` without checking the end pointer | `"12abc"` and `" 12"` are 12 |
| C | `regexec` on part of a buffer | It reads to the next `'\0'`, past the part; copy the part first, or use glibc's `REG_STARTEND` |
| C++ | `std::regex` on a long line | GCC's calls itself for each character that `[^ ]*` takes, and crashes at about 33,000 |
| C++ | `std::stoi` to check a number | `"12abc"` is 12, with no error; check the `pos` it gives back, or use `from_chars` |
| C++ | `std::from_chars` on `"+5"` | An error: it takes `-` but not `+` |
| Rust | `$1x` in `replace_all` | The group named `1x`, which isn't there, so nothing; write `${1}x` |
| Rust | `\d` or `\w` in the `regex` crate | Unicode's, so `\d` matches `٣`; write `[0-9]`, or `(?-u:\d)` |
| Go | `$1x` in `ReplaceAllString` | Nothing, as in Rust; write `${1}x` |
| Go | `strconv.ParseInt(s, 0, 64)` to read decimal | `0x1F` is 31, and `1_000` is 1000; pass 10 for decimal |
| Python | `re.match` to check a whole string | It fixes only the start, so `re.match("[0-9]+", "12abc")` matches; use `fullmatch` |
| Python | `$` at the end of a pattern | It matches before a final `\n` too; use `\Z`, or `fullmatch` |
| Python | `\d`, `str.isdigit`, or `int` to check digits | All three take `٣`; `int` also takes spaces and `1_000`; use `[0-9]` |
| Python | A pattern such as `(a*)*b` on text from outside | Seconds for 26 characters, doubling with each one; rewrite it, or use `(?>...)` |
| JavaScript | `str.replace("a", "b")` | Only the first; use `replaceAll`, or a regex with `g` |
| JavaScript | One regex with `g`, reused with `test` | It starts where the last match ended: with `re = /a/g`, `re.test("a")` twice gives `true`, then `false` |
| JavaScript | `Number(text)` to check a number | `""` is 0, `" 12 "` is 12, and `"0x1F"` is 31 |
| JavaScript | `parseInt(text)` to check a number | `"12abc"` is 12, and `"1_000"` is 1 |
| TypeScript | A misspelled group name, such as `groups.methd` | It type-checks as a `string`, and is `undefined` |
| Dart | `replaceAll(re, r'$1')` | The text `$1`; use `replaceAllMapped` |
| Dart | `\d` with `unicode: true` | Still only `0` to `9`; use `\p{Nd}` for every script's digits |
| Kotlin | `$` at the end of a pattern | It matches before a final `\n`, as in Python |
| Kotlin | `toInt` or `toIntOrNull` to check digits | `"٣".toInt()` is 3 |
| Kotlin | `toDouble` to check a number | `"1.5d"` and `" 1.5 "` are 1.5 |
| All | A regular expression for something that nests | It can't count: `\(.*\)` takes `(a) + (b)` as one group; write a parser |
| All | Columns counted in bytes or UTF-16 units | The caret is in the wrong place after `é` or `🦊`; count code points |

---

## 22.12 Idiom check

**C: POSIX `regex.h`, and `strtol` with every check.** Compile with `REG_EXTENDED` once,
check `regcomp`'s result and print `regerror`'s message, and match with `regexec` and an
array of `regmatch_t`. Convert numbers with `strtol` or `strtod`, with `errno` set to 0
before and checked after, and the end pointer checked. Write a state machine as an
`enum` of states and a loop that tests the state and the character, and a parser as
functions that return `NULL` and fill in an `Error` that the caller gave them.

**C++: `<regex>` with care, and `from_chars`.** Use `std::regex` for short, trusted
text, with `regex_match` for whole strings, and RE2 or CTRE for anything long or from
outside. Convert with `std::from_chars`, and check both its `ec` and its `ptr`.
Represent a tree with `std::variant` and `std::unique_ptr`, and visit it.

**Rust: the `regex` crate, `parse`, and `Result`.** Compile each regex once, with
`LazyLock` if it's global, and use `captures` with named groups. Convert with `parse`,
which is strict, and match on its error. Represent tokens and trees with enums, return
`Result` from each parsing function, and let `?` carry the error.

**Go: `regexp.MustCompile` at package level, and `strconv`.** Compile once, at package
level, with `MustCompile`, which panics at startup on a bad pattern, and use
`FindStringSubmatch` and `SubexpIndex` for named groups. Convert with `strconv.Atoi` or
`ParseInt` with base 10, and wrap its error. Return `(value, error)` from each parsing
function.

**Python: `re.compile`, `fullmatch`, and `match` statements.** Compile patterns once at
module level, use named groups and `fullmatch`, and say `[0-9]` or `re.ASCII` when you
mean ASCII. Check text before `int`. Write tokens and trees as dataclasses, walk them
with `match`, and raise an exception with the position.

**JavaScript: literals, `u`, and named groups.** Write regular expressions as literals
with the `u` flag, use `exec` or `matchAll` with named groups, and `replaceAll` with
`g`. Check numbers with a regex before `Number`, or use `Number.isInteger` on the
result. Read text by code point with `[...text]`.

**TypeScript: unions for tokens and trees.** Describe tokens and nodes as discriminated
unions, with a `kind` field, and let `switch` narrow them, with a `never` check for
missing cases. Treat `groups` as possibly `undefined`.

**Dart: `RegExp`, `tryParse`, and sealed classes.** Use raw strings, `r'...'`, for
patterns, `firstMatch` and `namedGroup`, and `replaceAllMapped` for computed
replacements. Convert with `int.tryParse` after checking the text, and write trees as
`sealed` classes with switch expressions.

**Kotlin: `Regex`, `matchEntire`, and `when`.** Use raw strings, `"""..."""`, for
patterns, `matchEntire` for whole strings and `find` for searches, with
`groups["name"]`. Check digits as `'0'..'9'` before `toLongOrNull`, which gives `null`
rather than throwing, and write trees as `sealed interface`s with an exhaustive `when`.

---

## 22.13 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Rewrite dates](exercises/01-iso-dates/) | One regex with alternatives, a replacement function, and checking what matched before using it |
| 2 | [Check JSON](exercises/02-check-json/) | Recursive descent with exact error messages, strings with escapes, and limits on depth |
| 3 | [Match a regex](exercises/03-match-regex/) | Parsing a pattern into a tree, compiling it, and running it as an automaton that no pattern can stop |

```sh
python3 tools/check.py 22                     # your code, every language
python3 tools/check.py 22 --lang go,python    # some languages
python3 tools/check.py 22 --solutions         # the solutions, and the examples
```

Each test gives the program its input on standard input, and for exercise 22.3, its
pattern as an argument, and checks what it prints on standard output, its exit status,
and for the tests that expect an error, what it prints on standard error. The starters
print nothing and exit with 0, so they fail at once. The Rust starter for exercise 22.1
is a Cargo project that names the `regex` crate, and exercises 22.2 and 22.3 need no
library at all: they are about not using one.

**Stretch:** give example 03 variables, `x = 2 * 3` on one line and `x ^ 2` on the next,
which needs a new token, a name, a new kind of node, and a table of values that lasts
from line to line. Then have its errors underline the whole of the part that's wrong, as
Clang's do, so that under `(1 / 0)` the line is `~~^~~`, from the `1` to the `0` with
the caret at the `/`, which needs each node to keep where it starts and ends.

---

Previous: [Chapter 21: Files, JSON, and the CLI](../21-files-json-and-the-cli/) ·
Next: Chapter 23: FFI and interop (coming soon)
