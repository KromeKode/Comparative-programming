# Chapter 21: Files, JSON, and the CLI

Until now, the programs in this book have read standard input and written standard
output, and left everything else to the shell and the checker. A real tool does more at
its edges. It reads options from its command line, opens files by name, replaces a file
without losing it if the power goes, reports trouble on standard error with an exit
status that a script can test, and reads and writes JSON, the format that most programs
now use to pass data to each other. This chapter writes three small tools in all nine
languages, a temperature converter, a program that removes repeated lines from files,
and a report on a JSON file of books, and runs them on the inputs that make the
languages disagree. They disagree more here than anywhere since chapter 6, because a
program's edges are bytes, and each language decides in its own way where bytes become
text, and what to do with bytes that aren't the text it expected: C, C++, and Go keep
them, Rust, Python, and Dart refuse them, and JavaScript and Kotlin quietly change them.

**Languages:** all nine · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Reading a command line: options, arguments, `--`, help, and usage errors, by hand and with each language's library | Text parsing (chapter 22), which reads harder input in the same way, one piece at a time |
| Exit status and standard error, the way Unix tools report trouble | FFI and interop (chapter 23), where programs in different languages run each other |
| Reading files as bytes or as text, and what each language does with bytes that aren't UTF-8 | Text handling in web servers (chapter 37), where the bytes come from strangers |
| Replacing a file safely: a new file, `fsync`, and a rename | SQLite from code, and transactions (chapters 34 and 35), where a database does this for you |
| Standard streams and pipes: buffering, and what happens when the reader stops reading | HTTP from sockets (chapter 36), where a closed connection is a closed pipe |
| JSON: reading it into types, checking it, writing it, and where the nine parsers disagree | REST CRUD API (chapter 38), whose requests and responses are JSON |

---

## 21.1 The concept: a program's edges

A program started from a shell gets five things from the system, and gives one back:

- **Arguments.** A list of strings, which the shell made by splitting the command line
  at spaces, taking off quotes, and expanding patterns such as `*.txt`. The first is the
  program's own name, as it was run. The program sees only the list: `temp "21.5c" 70f`
  and `temp 21.5c 70f` give it the same one, and a file named `-n` looks just like an
  option.
- **Environment variables.** Strings of the form `NAME=value`, inherited from the
  program that started it, such as `HOME`, `PATH`, and `LANG`, which names the user's
  language and the character set of their terminal.
- **Three open files**, numbered 0, 1, and 2: **standard input**, **standard output**,
  and **standard error**. The shell connects them to the terminal, to files with `<` and
  `>`, or to other programs with `|`. Standard output is for the program's results, and
  standard error is for everything else, such as error messages, warnings, and progress,
  so that `dedup *.txt > report.txt` still shows its errors on the screen, and the next
  program in a pipeline never reads them as data.
- **A current working directory**, which every file name that doesn't start with `/` is
  relative to.
- **The files it opens** by name.
- **The exit status**, which the program gives back when it ends: a number from 0 to
  255, where 0 means success and anything else means failure. A shell's `if`, `&&`,
  `||`, and `set -e` look at nothing else. Some tools give different failures different
  numbers, and this chapter's tools follow `grep` and `diff`: 1 for "found nothing" or
  "they differ", and 2 for trouble, including a command line that makes no sense. A
  program killed by a signal has no status of its own, and the shell reports 128 plus
  the signal's number, such as 141 for `SIGPIPE`, which is 13, as section 21.6 shows.

All of these are bytes. On Linux and the BSDs, an argument, an environment variable, a
file name, and a file's contents are sequences of bytes, with no encoding recorded
anywhere. "This file is UTF-8 text" is a promise about the bytes that nothing checks,
and a file from an older system, such as one in Latin-1, where `é` is the one byte
`0xe9`, breaks it. A line is a promise too: Unix ends each line with `\n`, Windows with
`\r\n`, and a file's last line may have no ending at all. Each language has to decide
where to turn bytes into strings, and what to do with bytes that break the promise, and
sections 21.3 and 21.4 show what the nine decide.

The design is Unix's, and Doug McIlroy, who proposed its pipes, summed it up: "Write
programs that do one thing and do it well. Write programs to work together. Write
programs to handle text streams, because that is a universal interface." A tool that
follows the conventions in this chapter, with results on standard output, messages on
standard error, and an exit status that means something, works with every other tool
that does.

**JSON**, JavaScript Object Notation, is the most common way for programs to pass
structured data to each other: a web API's requests and responses, a program's settings,
and npm's `package-lock.json`, from chapter 18. Douglas Crockford named it and described
it in the early 2000s, from the syntax of JavaScript's literals, and it is now RFC 8259
and ECMA-404. A JSON value is one of six kinds: `null`, `true` or `false`, a number, a
string in double quotes, an array of values in `[]`, or an object in `{}`, whose members
are pairs of a string key and a value. The grammar fits on a page, and the trouble comes
from what it leaves out. RFC 8259 says that an object's keys SHOULD be unique, not that
they must be, and that parsers differ in what they do when they aren't; it says nothing
about the order of the keys; and it lets each parser choose how big a number may be, and
how precise, with a note that only whole numbers from −(2^53 − 1) to 2^53 − 1 are safe
everywhere, since many parsers hold every number as a double. Section 21.8 shows what
the nine parsers do with each.

---

## 21.2 What each language gives you

| Language | Arguments | Reading options | Files | JSON |
| --- | --- | --- | --- | --- |
| C | `argc` and `argv`, as bytes | `getopt`, from POSIX, and `getopt_long`, from GNU, in the C library on Linux and the BSDs | `fopen` and `FILE *`, as bytes; `errno` and `strerror` say what went wrong | None: cJSON, used here, or Jansson |
| C++ | The same as C | None: CLI11 and cxxopts are common | `std::ifstream` and `std::ofstream`, and `std::filesystem` for paths, folders, and renames | None: nlohmann/json, used here, or RapidJSON |
| Rust | `std::env::args()`, as `String`s, which panics on one that isn't UTF-8; `args_os()` gives `OsString`s | None: clap, used here, is the usual one | `std::fs`, as a `String` or a `Vec<u8>`, with `io::Error` | None: serde, with serde_json, used here |
| Go | `os.Args`, as strings, which hold any bytes | `flag` | `os` and `bufio`, as `[]byte` or strings | `encoding/json` |
| Python | `sys.argv`, as `str`, with bytes that aren't UTF-8 kept as lone surrogates | `argparse` | `open`, as `str`, or as `bytes` with `"rb"`, with `OSError` | `json` |
| JavaScript | `process.argv`, as strings, with `�` for bytes that aren't UTF-8 | `util.parseArgs`, from `node:util` | `node:fs`, as a `Buffer` or a string, with an `Error` whose `code` is the system's, such as `ENOENT` | `JSON`, built into the language |
| TypeScript | JavaScript's | JavaScript's | JavaScript's | JavaScript's, whose `JSON.parse` returns `any` |
| Dart | `main`'s `List<String>`; the VM won't start if one isn't UTF-8 | None: `package:args`, used here, from the Dart team | `dart:io`'s `File`, as bytes or a `String`, with `FileSystemException` | `dart:convert` |
| Kotlin | `main`'s `Array<String>`, with `�` for bytes that aren't UTF-8 | None: Clikt, used here | The JVM's `java.io` and `java.nio.file`, and Kotlin's extensions, such as `File.readText`, with `IOException` | None: kotlinx.serialization, used here, from JetBrains |

Read down the last column and the languages split in two. Go, Python, JavaScript, and
Dart have JSON in their standard libraries, and C, C++, Rust, and Kotlin leave it to
libraries, which chapter 18's build tools download: the Rust, Kotlin, C++, and C
examples in this chapter are projects, with a `Cargo.toml`, a `build.gradle.kts`, or a
`CMakeLists.txt` that names serde_json 1.0, kotlinx.serialization 1.9, nlohmann/json
3.12, or cJSON 1.7.18, and Rust's and Kotlin's name clap 4 and Clikt 5 for options too.
Dart's `package:args` comes from pub.dev, although the Dart team writes it, so example
01's Dart version is a project too.

The libraries also differ in how you describe what you want. clap, serde, and
kotlinx.serialization read a type you declare, a struct or a class, and generate the
code for it when the program is compiled, with a derive macro or a compiler plugin, as
in chapter 15. Clikt reads the properties of a class too, but builds its parser at run
time, through Kotlin's delegated properties, and argparse, `flag`, `parseArgs`, and
`package:args` are told about each option by calls at run time. And `getopt` only splits
the command line into options and their values, and leaves the rest to the program.

---

## 21.3 Arguments and exit status

Example 01 converts temperatures, such as `21.5c`, `-40f`, or `300k`, to Celsius, or
with `--to`, to Fahrenheit or Kelvin, with one digit after the point, or with
`--digits`, as many as you ask for, up to 9. Each version reads its options the way its
language usually does: with `getopt_long` in C, a loop of its own in C++, clap in Rust,
`flag` in Go, argparse in Python, `util.parseArgs` in JavaScript and TypeScript,
`package:args` in Dart, and Clikt in Kotlin. Each checks every temperature before it
prints any, so that a mistake prints nothing on standard output.

**C**

```c
// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. getopt_long, from the C library on Linux and the BSDs, reads the options; the
// usage message, the exit status, and the checks of each value are ours.
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char USAGE[] = "usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...\n";
static const char HELP[] =
    "\n"
    "Converts temperatures, such as 21.5c, -40f, or 300k.\n"
    "\n"
    "options:\n"
    "  -h, --help           show this help and exit\n"
    "  -t, --to UNIT        the unit to convert to: c, f, or k (default: c)\n"
    "  -d, --digits DIGITS  digits after the point, 0 to 9 (default: 1)\n";

static double to_celsius(double t, char unit) {
    switch (unit) {
    case 'f': return (t - 32) * 5 / 9;
    case 'k': return t - 273.15;
    default: return t;
    }
}

static double from_celsius(double t, char unit) {
    switch (unit) {
    case 'f': return t * 9 / 5 + 32;
    case 'k': return t + 273.15;
    default: return t;
    }
}

static int is_unit(const char *text) {
    return strlen(text) == 1 && strchr("cfk", text[0]) != NULL;
}

// Prints a message and the usage line to stderr, and exits with status 2.
static void usage_error(const char *message, const char *value) {
    fprintf(stderr, "temp: %s%s\n%s", message, value, USAGE);
    exit(2);
}

// Reads a number followed by c, f, or k into *value and *unit, and returns 1, or
// returns 0 if text isn't one.
static int parse_temperature(const char *text, double *value, char *unit) {
    size_t length = strlen(text);
    if (length < 2 || !is_unit(text + length - 1)) return 0;
    *unit = text[length - 1];
    char *end;
    errno = 0;
    *value = strtod(text, &end);
    // strtod stops at the first character that isn't part of the number, which must be
    // the unit.
    return end == text + length - 1 && errno == 0 && isfinite(*value);
}

int main(int argc, char **argv) {
    static const struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {"to", required_argument, NULL, 't'},
        {"digits", required_argument, NULL, 'd'},
        {NULL, 0, NULL, 0},
    };
    char to = 'c';
    int digits = 1;
    int option;
    // "ht:d:" lists the short options; a colon means that one takes a value. For an
    // option it doesn't know, or one without its value, getopt_long prints a message,
    // such as "unrecognized option '--bogus'", and returns '?'.
    while ((option = getopt_long(argc, argv, "ht:d:", options, NULL)) != -1) {
        switch (option) {
        case 'h':
            printf("%s%s", USAGE, HELP);
            return 0;
        case 't':
            if (!is_unit(optarg)) usage_error("unknown unit: ", optarg);
            to = optarg[0];
            break;
        case 'd':
            if (strlen(optarg) != 1 || optarg[0] < '0' || optarg[0] > '9') {
                usage_error("digits must be 0 to 9, not ", optarg);
            }
            digits = optarg[0] - '0';
            break;
        default:
            fputs(USAGE, stderr);
            return 2;
        }
    }
    // getopt_long has moved the arguments that aren't options to the end, from optind.
    if (optind == argc) usage_error("no temperatures", "");

    // Check every temperature before printing any, so that a mistake prints nothing.
    double value;
    char unit;
    for (int i = optind; i < argc; i++) {
        if (!parse_temperature(argv[i], &value, &unit)) {
            fprintf(stderr, "temp: not a temperature: %s\n", argv[i]);
            return 2;
        }
    }
    for (int i = optind; i < argc; i++) {
        parse_temperature(argv[i], &value, &unit);
        printf("%s = %.*f%c\n", argv[i], digits, from_celsius(to_celsius(value, unit), to), to);
    }
    return 0;
}
```

**C++**

```cpp
// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. C++'s standard library has nothing to read options with, so this reads them
// itself, the way getopt_long does: -t f, -tf, --to f, and --to=f all work, options
// can come after the temperatures, and -- ends them.
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

const std::string_view usage = "usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...";
const std::string_view help = R"(
Converts temperatures, such as 21.5c, -40f, or 300k.

options:
  -h, --help           show this help and exit
  -t, --to UNIT        the unit to convert to: c, f, or k (default: c)
  -d, --digits DIGITS  digits after the point, 0 to 9 (default: 1))";

struct Temperature {
    std::string text;
    double value;
    char unit;
};

struct CommandLine {
    char to = 'c';
    int digits = 1;
    std::vector<std::string> temperatures;
};

double to_celsius(double t, char unit) {
    switch (unit) {
    case 'f': return (t - 32) * 5 / 9;
    case 'k': return t - 273.15;
    default: return t;
    }
}

double from_celsius(double t, char unit) {
    switch (unit) {
    case 'f': return t * 9 / 5 + 32;
    case 'k': return t + 273.15;
    default: return t;
    }
}

bool is_unit(std::string_view text) {
    return text == "c" || text == "f" || text == "k";
}

// Prints a message and the usage line to stderr, and exits with status 2.
[[noreturn]] void usage_error(std::string_view message) {
    std::cerr << "temp: " << message << '\n' << usage << '\n';
    std::exit(2);
}

CommandLine parse_command_line(const std::vector<std::string>& args) {
    CommandLine command_line;
    for (std::size_t i = 0; i < args.size(); i++) {
        const std::string& arg = args[i];
        if (arg == "--") {  // everything after -- is a temperature
            command_line.temperatures.insert(command_line.temperatures.end(),
                                             args.begin() + i + 1, args.end());
            break;
        }
        if (arg.size() < 2 || arg[0] != '-') {  // not an option; "-" alone isn't either
            command_line.temperatures.push_back(arg);
            continue;
        }
        // Split --to=f into --to and f, and -tf into -t and f.
        std::string name = arg;
        std::optional<std::string> value;
        if (arg.starts_with("--")) {
            if (auto equals = arg.find('='); equals != std::string::npos) {
                name = arg.substr(0, equals);
                value = arg.substr(equals + 1);
            }
        } else if (arg.size() > 2) {
            name = arg.substr(0, 2);
            value = arg.substr(2);
        }
        if (name == "-h" || name == "--help") {
            std::cout << usage << '\n' << help << '\n';
            std::exit(0);
        }
        if (name != "-t" && name != "--to" && name != "-d" && name != "--digits") {
            usage_error("unknown option " + arg);
        }
        if (!value) {  // the value is the next argument
            if (i + 1 == args.size()) usage_error("option " + name + " needs a value");
            value = args[++i];
        }
        if (name == "-t" || name == "--to") {
            if (!is_unit(*value)) usage_error("unknown unit: " + *value);
            command_line.to = (*value)[0];
        } else {
            if (value->size() != 1 || (*value)[0] < '0' || (*value)[0] > '9') {
                usage_error("digits must be 0 to 9, not " + *value);
            }
            command_line.digits = (*value)[0] - '0';
        }
    }
    if (command_line.temperatures.empty()) usage_error("no temperatures");
    return command_line;
}

// A number followed by c, f, or k, or nothing.
std::optional<Temperature> parse_temperature(const std::string& text) {
    if (text.size() < 2 || !is_unit(text.substr(text.size() - 1))) return std::nullopt;
    const char* first = text.data();
    const char* last = text.data() + text.size() - 1;  // the unit
    double value;
    // from_chars reads the whole number or fails; it skips no spaces and allows no +.
    auto [end, error] = std::from_chars(first, last, value);
    if (error != std::errc{} || end != last || !std::isfinite(value)) return std::nullopt;
    return Temperature{text, value, text.back()};
}

int main(int argc, char** argv) {
    CommandLine command_line = parse_command_line(std::vector<std::string>(argv + 1, argv + argc));

    // Check every temperature before printing any, so that a mistake prints nothing.
    std::vector<Temperature> temperatures;
    for (const std::string& text : command_line.temperatures) {
        std::optional<Temperature> temperature = parse_temperature(text);
        if (!temperature) {
            std::cerr << "temp: not a temperature: " << text << '\n';
            return 2;
        }
        temperatures.push_back(*temperature);
    }

    for (const Temperature& t : temperatures) {
        double converted = from_celsius(to_celsius(t.value, t.unit), command_line.to);
        std::cout << std::format("{} = {:.{}f}{}\n", t.text, converted, command_line.digits,
                                 command_line.to);
    }
}
```

**Rust**

```rust
//! Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
//! Kelvin. clap reads the options from the struct `Args`, writes --help from its doc
//! comments, and exits with status 2, after a message on stderr, when the command line
//! is wrong.
use clap::{Parser, ValueEnum};
use std::process::ExitCode;

#[derive(Clone, Copy, ValueEnum)]
enum Unit {
    C,
    F,
    K,
}

impl Unit {
    fn letter(self) -> char {
        match self {
            Unit::C => 'c',
            Unit::F => 'f',
            Unit::K => 'k',
        }
    }
}

fn to_celsius(t: f64, unit: Unit) -> f64 {
    match unit {
        Unit::C => t,
        Unit::F => (t - 32.0) * 5.0 / 9.0,
        Unit::K => t - 273.15,
    }
}

fn from_celsius(t: f64, unit: Unit) -> f64 {
    match unit {
        Unit::C => t,
        Unit::F => t * 9.0 / 5.0 + 32.0,
        Unit::K => t + 273.15,
    }
}

/// Converts temperatures, such as 21.5c, -40f, or 300k.
#[derive(Parser)]
#[command(name = "temp", bin_name = "temp")]
struct Args {
    /// The unit to convert to
    #[arg(short, long, value_enum, default_value = "c")]
    to: Unit,

    /// Digits after the point, 0 to 9
    #[arg(short, long, default_value_t = 1, value_parser = clap::value_parser!(u8).range(0..=9))]
    digits: u8,

    #[arg(required = true, value_name = "TEMPERATURE")]
    temperatures: Vec<String>,
}

/// A number followed by c, f, or k.
fn parse_temperature(text: &str) -> Option<(f64, Unit)> {
    let unit = match text.chars().last()? {
        'c' => Unit::C,
        'f' => Unit::F,
        'k' => Unit::K,
        _ => return None,
    };
    let value: f64 = text[..text.len() - 1].parse().ok()?;
    value.is_finite().then_some((value, unit))
}

fn main() -> ExitCode {
    let args = Args::parse(); // exits with status 2 if the arguments are wrong

    // Check every temperature before printing any, so that a mistake prints nothing.
    let mut temperatures = Vec::new();
    for text in &args.temperatures {
        match parse_temperature(text) {
            Some((value, unit)) => temperatures.push((text, value, unit)),
            None => {
                eprintln!("temp: not a temperature: {text}");
                return ExitCode::from(2);
            }
        }
    }

    for (text, value, unit) in temperatures {
        let converted = from_celsius(to_celsius(value, unit), args.to);
        let digits = usize::from(args.digits);
        println!("{text} = {converted:.digits$}{}", args.to.letter());
    }
    ExitCode::SUCCESS
}
```

**Go**

```go
// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. The flag package reads the options, prints -h, and exits with status 2, after
// a message on stderr, when the command line is wrong.
package main

import (
	"flag"
	"fmt"
	"math"
	"os"
	"strconv"
)

var toCelsius = map[string]func(float64) float64{
	"c": func(t float64) float64 { return t },
	"f": func(t float64) float64 { return (t - 32) * 5 / 9 },
	"k": func(t float64) float64 { return t - 273.15 },
}

var fromCelsius = map[string]func(float64) float64{
	"c": func(t float64) float64 { return t },
	"f": func(t float64) float64 { return t*9/5 + 32 },
	"k": func(t float64) float64 { return t + 273.15 },
}

type temperature struct {
	text  string
	value float64
	unit  string
}

// parseTemperature reads a number followed by c, f, or k.
func parseTemperature(text string) (temperature, bool) {
	if text == "" {
		return temperature{}, false
	}
	unit := text[len(text)-1:]
	if _, ok := toCelsius[unit]; !ok {
		return temperature{}, false
	}
	value, err := strconv.ParseFloat(text[:len(text)-1], 64)
	if err != nil || math.IsInf(value, 0) || math.IsNaN(value) {
		return temperature{}, false
	}
	return temperature{text, value, unit}, true
}

// usageError prints a message and the usage to stderr, and exits with status 2, as the
// flag package does for its own errors.
func usageError(message string) {
	fmt.Fprintln(os.Stderr, "temp:", message)
	flag.Usage()
	os.Exit(2)
}

func main() {
	// Each flag has two names, -t and -to; flag accepts one dash or two for either.
	var to string
	var digits int
	flag.StringVar(&to, "to", "c", "the unit to convert to: c, f, or k")
	flag.StringVar(&to, "t", "c", "the same as -to")
	flag.IntVar(&digits, "digits", 1, "digits after the point, 0 to 9")
	flag.IntVar(&digits, "d", 1, "the same as -digits")
	flag.Usage = func() {
		fmt.Fprintln(os.Stderr, "usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...")
		flag.PrintDefaults()
	}
	flag.Parse() // exits with status 2 if a flag is wrong

	if _, ok := fromCelsius[to]; !ok {
		usageError("unknown unit: " + to)
	}
	if digits < 0 || digits > 9 {
		usageError(fmt.Sprintf("digits must be 0 to 9, not %d", digits))
	}
	if flag.NArg() == 0 {
		usageError("no temperatures")
	}

	// Check every temperature before printing any, so that a mistake prints nothing.
	var temperatures []temperature
	for _, text := range flag.Args() {
		t, ok := parseTemperature(text)
		if !ok {
			fmt.Fprintln(os.Stderr, "temp: not a temperature:", text)
			os.Exit(2)
		}
		temperatures = append(temperatures, t)
	}

	for _, t := range temperatures {
		converted := fromCelsius[to](toCelsius[t.unit](t.value))
		fmt.Printf("%s = %.*f%s\n", t.text, digits, converted, to)
	}
}
```

**Python**

```python
# Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
# Kelvin. argparse reads the options, prints --help, and exits with status 2, after a
# message on stderr, when the command line is wrong.
import argparse
import math
import sys
from collections.abc import Callable

TO_CELSIUS: dict[str, Callable[[float], float]] = {
    "c": lambda t: t,
    "f": lambda t: (t - 32) * 5 / 9,
    "k": lambda t: t - 273.15,
}
FROM_CELSIUS: dict[str, Callable[[float], float]] = {
    "c": lambda t: t,
    "f": lambda t: t * 9 / 5 + 32,
    "k": lambda t: t + 273.15,
}


def parse_temperature(text: str) -> tuple[float, str] | None:
    """A number followed by c, f, or k, or None."""
    unit = text[-1:]
    if unit not in TO_CELSIUS:
        return None
    try:
        value = float(text[:-1])
    except ValueError:
        return None
    return (value, unit) if math.isfinite(value) else None


def main() -> int:
    parser = argparse.ArgumentParser(
        prog="temp", description="Converts temperatures, such as 21.5c, -40f, or 300k."
    )
    parser.add_argument(
        "-t",
        "--to",
        choices=["c", "f", "k"],
        default="c",
        help="the unit to convert to (default: c)",
    )
    parser.add_argument(
        "-d",
        "--digits",
        type=int,
        choices=range(10),
        default=1,
        metavar="DIGITS",
        help="digits after the point, 0 to 9 (default: 1)",
    )
    parser.add_argument("temperatures", nargs="+", metavar="TEMPERATURE")
    args = parser.parse_args()  # exits with status 2 if the arguments are wrong

    # Check every temperature before printing any, so that a mistake prints nothing.
    temperatures = []
    for text in args.temperatures:
        parsed = parse_temperature(text)
        if parsed is None:
            print(f"temp: not a temperature: {text}", file=sys.stderr)
            return 2
        temperatures.append((text, *parsed))

    for text, value, unit in temperatures:
        converted = FROM_CELSIUS[args.to](TO_CELSIUS[unit](value))
        print(f"{text} = {converted:.{args.digits}f}{args.to}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

**JavaScript**

```js
// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. util.parseArgs, built into Node, reads the options, and throws when the
// command line is wrong; the usage message and the checks of each value are ours.
import { parseArgs } from "node:util";

const USAGE = "usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...";
const HELP = `${USAGE}

Converts temperatures, such as 21.5c, -40f, or 300k.

options:
  -h, --help           show this help and exit
  -t, --to UNIT        the unit to convert to: c, f, or k (default: c)
  -d, --digits DIGITS  digits after the point, 0 to 9 (default: 1)`;

const TO_CELSIUS = {
  c: (t) => t,
  f: (t) => ((t - 32) * 5) / 9,
  k: (t) => t - 273.15,
};
const FROM_CELSIUS = {
  c: (t) => t,
  f: (t) => (t * 9) / 5 + 32,
  k: (t) => t + 273.15,
};

// Prints a message and the usage line to stderr, and exits with status 2.
function usageError(message) {
  console.error(`temp: ${message}\n${USAGE}`);
  process.exit(2);
}

// A number followed by c, f, or k, or null.
function parseTemperature(text) {
  const unit = text.slice(-1);
  const number = text.slice(0, -1);
  // Number("") is 0, so an empty number needs a check of its own.
  if (!Object.hasOwn(TO_CELSIUS, unit) || number.trim() === "") return null;
  const value = Number(number);
  return Number.isFinite(value) ? { value, unit } : null;
}

let parsed;
try {
  parsed = parseArgs({
    options: {
      help: { type: "boolean", short: "h" },
      to: { type: "string", short: "t", default: "c" },
      digits: { type: "string", short: "d", default: "1" },
    },
    allowPositionals: true,
  });
} catch (error) {
  usageError(error.message);
}
const { values, positionals } = parsed;
if (values.help) {
  console.log(HELP);
  process.exit(0);
}
const to = values.to;
if (!Object.hasOwn(FROM_CELSIUS, to)) usageError(`unknown unit: ${to}`);
const digits = Number.parseInt(values.digits, 10);
// String(digits) is values.digits only when it was a plain whole number, such as 3.
if (String(digits) !== values.digits || digits > 9) {
  usageError(`digits must be 0 to 9, not ${values.digits}`);
}
if (positionals.length === 0) usageError("no temperatures");

// Check every temperature before printing any, so that a mistake prints nothing.
const temperatures = positionals.map((text) => {
  const temperature = parseTemperature(text);
  if (temperature === null) {
    console.error(`temp: not a temperature: ${text}`);
    process.exit(2);
  }
  return { text, ...temperature };
});

for (const { text, value, unit } of temperatures) {
  const converted = FROM_CELSIUS[to](TO_CELSIUS[unit](value));
  console.log(`${text} = ${converted.toFixed(digits)}${to}`);
}
```

**TypeScript**

```ts
// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. util.parseArgs, built into Node, reads the options, and throws when the
// command line is wrong; the usage message and the checks of each value are ours.
import { parseArgs } from "node:util";

const USAGE = "usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...";
const HELP = `${USAGE}

Converts temperatures, such as 21.5c, -40f, or 300k.

options:
  -h, --help           show this help and exit
  -t, --to UNIT        the unit to convert to: c, f, or k (default: c)
  -d, --digits DIGITS  digits after the point, 0 to 9 (default: 1)`;

type Unit = "c" | "f" | "k";

// Narrows a string to a Unit, so that it can index the tables.
function isUnit(text: string): text is Unit {
  return text === "c" || text === "f" || text === "k";
}

const TO_CELSIUS: Record<Unit, (t: number) => number> = {
  c: (t) => t,
  f: (t) => ((t - 32) * 5) / 9,
  k: (t) => t - 273.15,
};
const FROM_CELSIUS: Record<Unit, (t: number) => number> = {
  c: (t) => t,
  f: (t) => (t * 9) / 5 + 32,
  k: (t) => t + 273.15,
};

// Prints a message and the usage line to stderr, and exits with status 2.
function usageError(message: string): never {
  console.error(`temp: ${message}\n${USAGE}`);
  process.exit(2);
}

// A number followed by c, f, or k, or null.
function parseTemperature(text: string): { value: number; unit: Unit } | null {
  const unit = text.slice(-1);
  const number = text.slice(0, -1);
  // Number("") is 0, so an empty number needs a check of its own.
  if (!isUnit(unit) || number.trim() === "") return null;
  const value = Number(number);
  return Number.isFinite(value) ? { value, unit } : null;
}

let parsed;
try {
  parsed = parseArgs({
    options: {
      help: { type: "boolean", short: "h" },
      to: { type: "string", short: "t", default: "c" },
      digits: { type: "string", short: "d", default: "1" },
    },
    allowPositionals: true,
  });
} catch (error) {
  usageError(error instanceof Error ? error.message : String(error));
}
const { values, positionals } = parsed;
if (values.help) {
  console.log(HELP);
  process.exit(0);
}
const to = values.to;
if (!isUnit(to)) usageError(`unknown unit: ${to}`);
const digits = Number.parseInt(values.digits, 10);
// String(digits) is values.digits only when it was a plain whole number, such as 3.
if (String(digits) !== values.digits || digits > 9) {
  usageError(`digits must be 0 to 9, not ${values.digits}`);
}
if (positionals.length === 0) usageError("no temperatures");

// Check every temperature before printing any, so that a mistake prints nothing.
const temperatures = positionals.map((text) => {
  const temperature = parseTemperature(text);
  if (temperature === null) {
    console.error(`temp: not a temperature: ${text}`);
    process.exit(2);
  }
  return { text, ...temperature };
});

for (const { text, value, unit } of temperatures) {
  const converted = FROM_CELSIUS[to](TO_CELSIUS[unit](value));
  console.log(`${text} = ${converted.toFixed(digits)}${to}`);
}
```

**Dart**

```dart
// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. package:args reads the options and writes the help from them; it throws a
// FormatException when the command line is wrong, and the exit status is ours.
import 'dart:io';

import 'package:args/args.dart';

const usage = 'usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...';
const units = ['c', 'f', 'k'];

double toCelsius(double t, String unit) => switch (unit) {
  'f' => (t - 32) * 5 / 9,
  'k' => t - 273.15,
  _ => t,
};

double fromCelsius(double t, String unit) => switch (unit) {
  'f' => t * 9 / 5 + 32,
  'k' => t + 273.15,
  _ => t,
};

/// Prints a message and the usage line to stderr, and exits with status 2.
Never usageError(String message) {
  stderr.writeln('temp: $message\n$usage');
  exit(2);
}

/// A number followed by c, f, or k, or null.
(double, String)? parseTemperature(String text) {
  if (text.isEmpty) return null;
  final unit = text.substring(text.length - 1);
  final value = double.tryParse(text.substring(0, text.length - 1));
  if (!units.contains(unit) || value == null || !value.isFinite) return null;
  return (value, unit);
}

void main(List<String> arguments) {
  final parser = ArgParser()
    ..addFlag(
      'help',
      abbr: 'h',
      negatable: false,
      help: 'show this help and exit',
    )
    ..addOption(
      'to',
      abbr: 't',
      allowed: units,
      defaultsTo: 'c',
      valueHelp: 'UNIT',
      help: 'the unit to convert to',
    )
    ..addOption(
      'digits',
      abbr: 'd',
      defaultsTo: '1',
      valueHelp: 'DIGITS',
      help: 'digits after the point, 0 to 9',
    );

  final ArgResults results;
  try {
    results = parser.parse(arguments);
  } on FormatException catch (error) {
    usageError(error.message);
  }
  if (results.flag('help')) {
    print('$usage\n\nConverts temperatures, such as 21.5c, -40f, or 300k.\n');
    print(parser.usage);
    exit(0);
  }
  final to = results.option('to')!;
  final digitsText = results.option('digits')!;
  final digits = int.tryParse(digitsText);
  // '$digits' is digitsText only when it was a plain whole number, such as 3.
  if (digits == null || digits < 0 || digits > 9 || '$digits' != digitsText) {
    usageError('digits must be 0 to 9, not $digitsText');
  }
  if (results.rest.isEmpty) usageError('no temperatures');

  // Check every temperature before printing any, so that a mistake prints nothing.
  final temperatures = <(String, double, String)>[];
  for (final text in results.rest) {
    final temperature = parseTemperature(text);
    if (temperature == null) {
      stderr.writeln('temp: not a temperature: $text');
      exit(2);
    }
    final (value, unit) = temperature;
    temperatures.add((text, value, unit));
  }

  for (final (text, value, unit) in temperatures) {
    final converted = fromCelsius(toCelsius(value, unit), to);
    print('$text = ${converted.toStringAsFixed(digits)}$to');
  }
}
```

**Kotlin**

```kotlin
// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. Clikt reads the options from the properties of the class Temp, and writes the
// help from them. It exits with status 1 when the command line is wrong, so main
// catches its errors to exit with 2, as the other languages' tools do.
import com.github.ajalt.clikt.core.CliktCommand
import com.github.ajalt.clikt.core.CliktError
import com.github.ajalt.clikt.core.Context
import com.github.ajalt.clikt.core.UsageError
import com.github.ajalt.clikt.core.parse
import com.github.ajalt.clikt.parameters.arguments.argument
import com.github.ajalt.clikt.parameters.arguments.multiple
import com.github.ajalt.clikt.parameters.options.default
import com.github.ajalt.clikt.parameters.options.option
import com.github.ajalt.clikt.parameters.types.choice
import com.github.ajalt.clikt.parameters.types.int
import com.github.ajalt.clikt.parameters.types.restrictTo
import java.util.Locale
import kotlin.system.exitProcess

fun toCelsius(t: Double, unit: Char): Double =
    when (unit) {
        'f' -> (t - 32) * 5 / 9
        'k' -> t - 273.15
        else -> t
    }

fun fromCelsius(t: Double, unit: Char): Double =
    when (unit) {
        'f' -> t * 9 / 5 + 32
        'k' -> t + 273.15
        else -> t
    }

data class Temperature(val text: String, val value: Double, val unit: Char)

// A number followed by c, f, or k, or null.
fun parseTemperature(text: String): Temperature? {
    val unit = text.lastOrNull()
    if (unit == null || unit !in "cfk") return null
    val value = text.dropLast(1).toDoubleOrNull()
    return if (value != null && value.isFinite()) Temperature(text, value, unit) else null
}

class Temp : CliktCommand(name = "temp") {
    val to by
        option("-t", "--to", metavar = "UNIT", help = "the unit to convert to")
            .choice("c", "f", "k")
            .default("c")
    val digits by
        option("-d", "--digits", metavar = "DIGITS", help = "digits after the point")
            .int()
            .restrictTo(0..9)
            .default(1)
    val texts by argument(name = "TEMPERATURE").multiple(required = true)

    override fun help(context: Context) = "Converts temperatures, such as 21.5c, -40f, or 300k."

    override fun run() {
        // Check every temperature before printing any, so that a mistake prints nothing.
        val temperatures = texts.map { text ->
            parseTemperature(text)
                ?: run {
                    System.err.println("temp: not a temperature: $text")
                    exitProcess(2)
                }
        }
        for ((text, value, unit) in temperatures) {
            val converted = fromCelsius(toCelsius(value, unit), to[0])
            // Locale.ROOT, or a German computer would print 21,5.
            println(String.format(Locale.ROOT, "%s = %.${digits}f%s", text, converted, to))
        }
    }
}

fun main(args: Array<String>) {
    val command = Temp()
    try {
        command.parse(args)
    } catch (error: CliktError) {
        // What Temp().main(args) would do, but with status 2 for a usage error.
        command.echoFormattedHelp(error)
        exitProcess(if (error is UsageError) 2 else error.statusCode)
    }
}
```

All nine accept the same command lines, and print the same results:

```console
$ temp 21.5c 70f 300k
21.5c = 21.5c
70f = 21.1c
300k = 26.9c
$ temp --to f -- 100c 37c -40c
100c = 212.0f
37c = 98.6f
-40c = -40.0f
$ temp -t k -d 2 0c 32f
0c = 273.15k
32f = 273.15k
$ temp --digits=0 --to=f 37c 22c
37c = 99f
22c = 72f
$ temp 20c 21x
temp: not a temperature: 21x
$ echo $?
2
```

### The conventions

A command line's options follow conventions, from POSIX's guidelines for the short ones
and from GNU for the long ones, which the libraries keep to different degrees:

- A short option is a dash and a letter, as in `-t f`, and its value can be joined to
  it, as in `-tf`. Short options that take no value can be grouped, so that in exercise
  21.1, `-in` is `-i -n`.
- A long option is two dashes and a word, as in `--to f` or `--to=f`.
- An argument of `--` ends the options, so that `temp -- -40c` has a temperature that
  starts with a dash.
- An argument of `-` alone isn't an option, and means standard input, or standard
  output, by convention.

Go's `flag` is the exception. It takes one dash or two for any flag, so that `-to` and
`--to` are the same, and it doesn't group letters: `temp -tk 0c` is a flag named `tk`,
and Go's version says `flag provided but not defined: -tk`. Example 01 gives each Go
flag two names, `-t` and `-to`, for the short and the long spelling.

### Help

Five of the libraries write a `--help` from the options they were given. argparse and
clap, for example:

```console
$ python3 main.py --help
usage: temp [-h] [-t {c,f,k}] [-d DIGITS] TEMPERATURE [TEMPERATURE ...]

Converts temperatures, such as 21.5c, -40f, or 300k.

positional arguments:
  TEMPERATURE

options:
  -h, --help           show this help message and exit
  -t, --to {c,f,k}     the unit to convert to (default: c)
  -d, --digits DIGITS  digits after the point, 0 to 9 (default: 1)
```

```console
$ ./temp --help
Converts temperatures, such as 21.5c, -40f, or 300k

Usage: temp [OPTIONS] <TEMPERATURE>...

Arguments:
  <TEMPERATURE>...

Options:
  -t, --to <TO>          The unit to convert to [default: c] [possible values: c, f, k]
  -d, --digits <DIGITS>  Digits after the point, 0 to 9 [default: 1]
  -h, --help             Print help
```

clap takes the description from the doc comment above the struct, without its full stop,
and the help for each option from the doc comment above its field. Clikt writes
`Usage: temp [<options>] <temperature>...`, in lowercase, and `package:args` lists the
allowed values as `[c (default), f, k]`. Go's `flag` prints each flag with its type and
default, `-d int`, and prints it to standard error, with status 0. C, C++, JavaScript,
and TypeScript have help text of their own, since `getopt_long` and `parseArgs` don't
write any, and C++ has no library at all.

### Usage errors

A command line that the program can't use is a **usage error**: the program prints what
was wrong, and how to use it, on standard error, and exits with 2. The libraries'
messages for `temp --bogus 1c`:

| Language | First line of standard error |
| --- | --- |
| C | `./temp: unrecognized option '--bogus'`, with the program's name as it was run, from `argv[0]` |
| C++ | `temp: unknown option --bogus`, written by the example |
| Rust | `error: unexpected argument '--bogus' found`, then a tip, the usage, and ``For more information, try '--help'.`` |
| Go | `flag provided but not defined: -bogus`, then the usage and every flag |
| Python | `usage: temp [-h] [-t {c,f,k}] [-d DIGITS] TEMPERATURE [TEMPERATURE ...]`, then `temp: error: unrecognized arguments: --bogus` |
| JavaScript, TypeScript | `temp: Unknown option '--bogus'. To specify a positional argument starting with a '-', place it at the end of the command after '--', as in '-- "--bogus"` |
| Dart | `temp: Could not find an option named "--bogus".` |
| Kotlin | `Usage: temp [<options>] <temperature>...`, then `Error: no such option --bogus` |

All nine exit with 2, which argparse, clap, and `flag` do by themselves. `parseArgs` and
`package:args` throw an exception instead, and Clikt exits with 1 unless `main` catches
its error, so those versions choose 2 themselves. The checker tests a usage error's
status but not its words, which is what lets each language use its own library.

### Negative numbers, and options after arguments

A negative temperature starts with a dash, so it looks like an option:

| Language | `temp -40c` |
| --- | --- |
| C | `invalid option -- '4'` |
| C++ | `temp: unknown option -40c` |
| Rust | `error: unexpected argument '-4' found`, with the tip ``to pass '-4' as a value, use '-- -4'`` |
| Go | `flag provided but not defined: -40c` |
| Python | `temp: error: the following arguments are required: TEMPERATURE` |
| JavaScript, TypeScript | `temp: Unknown option '-4'` |
| Dart | `temp: Could not find an option with short name "-4".` |
| Kotlin | `Error: no such option -4`, and `Error: missing argument <temperature>` |

`temp -- -40c` works in all nine. argparse is the only library that guesses: when a
parser has no options that look like negative numbers, an argument that does, such as
`-40`, is taken as a positional argument, so `temp -40` gets as far as the example's own
check and `temp: not a temperature: -40`. `-40c`, with its unit, doesn't look like a
number.

Options can come after the other arguments in eight of the nine, so that
`temp 21.5c 70f --to f` converts both to Fahrenheit. Go's `flag` stops at the first
argument that isn't a flag, as POSIX says, and `--to` and `f` become temperatures:

```console
$ ./temp 21.5c 70f --to f
temp: not a temperature: --to
```

GNU's `getopt_long` moves the options to the front, unless the environment variable
`POSIXLY_CORRECT` is set, when it stops as Go's does. Exercise 21.1 follows POSIX, so
that `search cat -n notes.txt` searches a file named `-n`.

### Arguments that aren't UTF-8

An argument is bytes, and a file name in Latin-1, such as `caf\351`, with the byte
`0xe9`, isn't UTF-8. The nine do four different things with it: keep the bytes, keep
them in a form that can go back to the bytes, replace them, or refuse them.

| Language | `caf\351` as an argument |
| --- | --- |
| C, C++ | The bytes, as they are |
| Rust | `std::env::args()` panics, with ``called `Result::unwrap()` on an `Err` value: "caf\xE9"``, and status 101; `std::env::args_os()` gives an `OsString` with the bytes, which `as_bytes()`, from `std::os::unix::ffi::OsStrExt`, returns |
| Go | The bytes, in a string: `"caf\xe9"` |
| Python | `'caf\udce9'`: the byte becomes a lone surrogate, U+DCE9, which isn't a character, and which `os.fsencode` turns back into the byte |
| JavaScript, TypeScript | `"caf�"`: the byte becomes U+FFFD, the replacement character, and is lost |
| Dart | The VM won't start: `Dart_NewStringFromUTF8 expects argument 'str' to be valid UTF-8.`, with status 255 |
| Kotlin | U+FFFD, as in JavaScript |

Python's trick is called `surrogateescape`, from PEP 383, in Python 3.1: each byte that
isn't part of a UTF-8 character becomes one of 128 code points that UTF-8 text can never
contain, so that the `str` can go back to the same bytes. Environment variables are
bytes too, and get the same treatment, with one more surprise: Dart's
`Platform.environment` leaves out a variable whose value isn't UTF-8, so that it looks
as if it isn't set. Reading one is `getenv` in C and C++, `std::env::var` in Rust, which
returns an error for a missing variable or one that isn't UTF-8, `os.Getenv` or
`os.LookupEnv` in Go, `os.environ.get` in Python, `process.env.NAME` in JavaScript,
`Platform.environment['NAME']` in Dart, and `System.getenv("NAME")` in Kotlin.

### Exit status

| Language | Exit with status n | What doesn't run |
| --- | --- | --- |
| C | `return n;` from `main`, or `exit(n)` | Nothing is skipped: `exit` flushes `stdio`'s buffers |
| C++ | The same, or `std::exit(n)` | `std::exit` skips the destructors of local variables |
| Rust | Return an `ExitCode` from `main`, or call `std::process::exit(n)` | `process::exit` skips `Drop`, but flushes standard output |
| Go | `os.Exit(n)` | Deferred calls: a `bufio.Writer` flushed by `defer` loses its output |
| Python | `sys.exit(n)`, as in `sys.exit(main())` | Nothing: it raises `SystemExit`, so `finally` blocks run |
| JavaScript | `process.exitCode = n`, and the program ends with n, or `process.exit(n)` at once | `process.exit` skips `finally` blocks |
| Dart | `exitCode = n`, or `exit(n)` at once | `exit` skips `finally` blocks |
| Kotlin | `exitProcess(n)` | `finally` blocks |

Each row was tested with a program that writes to standard output without a newline and
then exits with 3 from inside a `try`, or with a `defer` or a destructor waiting. Go's
is the one to watch: example 01's `os.Exit(2)` is fine, since Go's standard output has
no buffer, but exercise 21.1's Go solution writes through a `bufio.Writer`, and flushes
it by hand before each `os.Exit`. In Rust, returning an `ExitCode` from `main` lets
every value be dropped first, which is why example 01 does that rather than calling
`process::exit`.

---

## 21.4 Reading files

Example 02, `dedup`, removes repeated lines from each file named on its command line,
keeping the first of each, and rewrites the file in place. A line ends at `\n`, so a
`\r` before it stays part of the line, and a last line without a `\n` gets one. A file
that can't be read gets a message, and `dedup` goes on to the next file, and exits with
1 at the end.

**C**

```c
// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
#define _POSIX_C_SOURCE 200809L  // for mkstemp, fchmod, and fsync, from POSIX
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    const char *start;
    size_t length;  // without the '\n'
} Line;

// Reads a whole file. Returns NULL, with errno set, if it can't.
static char *read_file(const char *path, size_t *size) {
    FILE *file = fopen(path, "rb");  // "b" for bytes, which only matters on Windows
    if (file == NULL) return NULL;
    char *data = NULL;
    size_t length = 0, capacity = 0;
    for (;;) {
        if (length == capacity) {
            capacity = capacity == 0 ? 4096 : capacity * 2;
            char *bigger = realloc(data, capacity);
            if (bigger == NULL) break;  // errno is ENOMEM
            data = bigger;
        }
        size_t got = fread(data + length, 1, capacity - length, file);
        length += got;
        if (got == 0) break;  // the end of the file, or an error, such as EISDIR
    }
    int error = ferror(file) || length == capacity ? errno : 0;
    fclose(file);
    if (error != 0) {
        free(data);
        errno = error;
        return NULL;
    }
    *size = length;
    return data;
}

// Writes the lines, each with a '\n', to a new file next to path, with the same
// permissions, and renames it over path. Returns -1, with errno set, if it can't.
static int replace_file(const char *path, const Line *lines, size_t count) {
    struct stat info;
    if (stat(path, &info) != 0) return -1;
    // mkstemp makes a file with a fresh name: it puts letters where the Xs are.
    char *temporary = malloc(strlen(path) + sizeof ".XXXXXX");
    if (temporary == NULL) return -1;
    sprintf(temporary, "%s.XXXXXX", path);
    int fd = mkstemp(temporary);
    if (fd < 0) {
        free(temporary);
        return -1;
    }
    FILE *file = fdopen(fd, "wb");
    int ok = file != NULL;
    for (size_t i = 0; ok && i < count; i++) {
        ok = fwrite(lines[i].start, 1, lines[i].length, file) == lines[i].length &&
             fputc('\n', file) != EOF;
    }
    ok = ok && fflush(file) == 0;  // from the FILE's buffer to the system
    ok = ok && fsync(fd) == 0;     // onto the disk before the rename
    ok = ok && fchmod(fd, info.st_mode & 07777) == 0;  // mkstemp gave it 0600
    int error = ok ? 0 : errno;
    // fclose closes fd too; it can fail, for a write the system held back.
    if ((file != NULL ? fclose(file) : close(fd)) != 0 && error == 0) error = errno;
    if (error == 0 && rename(temporary, path) != 0) error = errno;
    if (error != 0) remove(temporary);
    free(temporary);
    errno = error;
    return error == 0 ? 0 : -1;
}

// Rewrites the file without its repeated lines, and returns how many there were, or
// -1, with errno set.
static long dedup(const char *path) {
    size_t size;
    char *data = read_file(path, &size);
    if (data == NULL) return -1;
    // Split at each '\n'; a last line without one is still a line.
    Line *lines = malloc((size + 1) * sizeof *lines);
    if (lines == NULL) {
        free(data);
        return -1;
    }
    size_t count = 0;
    for (size_t start = 0; start < size;) {
        const char *newline = memchr(data + start, '\n', size - start);
        size_t end = newline != NULL ? (size_t)(newline - data) : size;
        lines[count++] = (Line){data + start, end - start};
        start = end + 1;
    }
    // Keep each line that no line before it matches. Comparing each with all the kept
    // ones is slow for a big file; a hash table, as in chapter 25, would be faster.
    size_t kept = 0;
    for (size_t i = 0; i < count; i++) {
        int repeated = 0;
        for (size_t j = 0; j < kept && !repeated; j++) {
            repeated = lines[j].length == lines[i].length &&
                       memcmp(lines[j].start, lines[i].start, lines[i].length) == 0;
        }
        if (!repeated) lines[kept++] = lines[i];
    }
    long removed = (long)(count - kept);
    if (removed > 0 && replace_file(path, lines, kept) != 0) removed = -1;
    int error = errno;
    free(lines);
    free(data);
    errno = error;
    return removed;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs("usage: dedup FILE...\n", stderr);
        return 2;
    }
    int status = 0;
    for (int i = 1; i < argc; i++) {
        long removed = dedup(argv[i]);
        if (removed < 0) {
            // strerror turns errno into the system's message, such as
            // "No such file or directory".
            fprintf(stderr, "dedup: %s: %s\n", argv[i], strerror(errno));
            status = 1;
            continue;
        }
        printf("%s: removed %ld line%s\n", argv[i], removed, removed == 1 ? "" : "s");
    }
    return status;
}
```

**C++**

```cpp
// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

// A streams error doesn't say what went wrong, but on Linux errno does.
[[noreturn]] void throw_errno() {
    throw std::system_error(errno, std::generic_category());
}

// Rewrites the file without its repeated lines, and returns how many there were.
std::size_t dedup(const std::string& path) {
    std::vector<std::string> lines;
    {
        std::ifstream in(path, std::ios::binary);  // binary: no "\r\n" changes on Windows
        if (!in) throw_errno();
        // getline splits at each '\n' and drops it; a last line without one still counts.
        for (std::string line; std::getline(in, line);) lines.push_back(line);
        // Opening a folder works, and reading it fails, which sets badbit.
        if (in.bad()) throw_errno();
    }  // in closes here, at the end of its scope
    std::unordered_set<std::string> seen;
    std::vector<std::string> kept;
    for (const std::string& line : lines) {
        if (seen.insert(line).second) kept.push_back(line);  // second: it was new
    }
    std::size_t removed = lines.size() - kept.size();
    if (removed == 0) return 0;

    // The standard library can't make a file with a fresh name, so it's FILE.tmp.
    std::string temporary = path + ".tmp";
    try {
        {
            std::ofstream out(temporary, std::ios::binary);
            if (!out) throw_errno();
            for (const std::string& line : kept) out << line << '\n';
            // C++ has no fsync: an ofstream doesn't give the file descriptor it needs.
            out.close();
            if (!out) throw_errno();
        }
        fs::permissions(temporary, fs::status(path).permissions());
        fs::rename(temporary, path);
    } catch (...) {
        std::error_code ignored;  // remove reports into it, instead of throwing
        fs::remove(temporary, ignored);
        throw;
    }
    return removed;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: dedup FILE...\n";
        return 2;
    }
    int status = 0;
    for (int i = 1; i < argc; i++) {
        std::string path = argv[i];
        try {
            std::size_t removed = dedup(path);
            std::cout << path << ": removed " << removed << (removed == 1 ? " line\n" : " lines\n");
        } catch (const std::system_error& error) {  // fs::filesystem_error is one too
            // code().message() is the system's message, such as "No such file or directory".
            std::cerr << "dedup: " << path << ": " << error.code().message() << '\n';
            status = 1;
        }
    }
    return status;
}
```

**Rust**

```rust
//! Removes repeated lines from each file named on the command line, keeping the first of
//! each, and rewrites the file in place. It writes a new file next to the old one and
//! renames it over the old one, so that a crash halfway leaves the old file whole.
use std::collections::HashSet;
use std::fs::{self, OpenOptions};
use std::io::{self, Write};
use std::process::ExitCode;

/// Rewrites the file without its repeated lines, and returns how many there were.
fn dedup(path: &str) -> io::Result<usize> {
    // read_to_string fails if the bytes aren't UTF-8.
    let text = fs::read_to_string(path)?;
    // split_terminator leaves out the empty string after a final newline. lines() would
    // also take the "\r" off the end of each line.
    let lines: Vec<&str> = text.split_terminator('\n').collect();
    let mut seen = HashSet::new();
    // insert returns false for a line it has seen, so filter keeps the first of each.
    let kept: Vec<&str> = lines
        .iter()
        .copied()
        .filter(|line| seen.insert(*line))
        .collect();
    let removed = lines.len() - kept.len();
    if removed == 0 {
        return Ok(0);
    }

    let permissions = fs::metadata(path)?.permissions();
    let new_text: String = kept.iter().map(|line| format!("{line}\n")).collect();
    // The standard library can't make a file with a fresh name, so the new file is
    // FILE.tmp, and create_new fails if that is already there rather than writing over it.
    let temporary = format!("{path}.tmp");
    let mut file = OpenOptions::new()
        .write(true)
        .create_new(true)
        .open(&temporary)?;
    let written = file
        .write_all(new_text.as_bytes())
        .and_then(|()| file.set_permissions(permissions)) // the old file's permissions
        .and_then(|()| file.sync_all()) // onto the disk before the rename
        .and_then(|()| fs::rename(&temporary, path));
    if let Err(error) = written {
        let _ = fs::remove_file(&temporary); // an error here would hide the first one
        return Err(error);
    }
    Ok(removed)
}

fn main() -> ExitCode {
    let paths: Vec<String> = std::env::args().skip(1).collect();
    if paths.is_empty() {
        eprintln!("usage: dedup FILE...");
        return ExitCode::from(2);
    }
    let mut status = ExitCode::SUCCESS;
    for path in &paths {
        match dedup(path) {
            Ok(removed) => {
                let plural = if removed == 1 { "" } else { "s" };
                println!("{path}: removed {removed} line{plural}");
            }
            Err(error) => {
                // Such as "No such file or directory (os error 2)": no path, so it's added.
                eprintln!("dedup: {path}: {error}");
                status = ExitCode::FAILURE;
            }
        }
    }
    status
}
```

**Go**

```go
// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

// dedup rewrites the file without its repeated lines, and returns how many there were.
func dedup(path string) (int, error) {
	data, err := os.ReadFile(path) // bytes, whether or not they are UTF-8
	if err != nil {
		return 0, err
	}
	lines := strings.Split(string(data), "\n")
	if lines[len(lines)-1] == "" { // the file ended with a newline, or was empty
		lines = lines[:len(lines)-1]
	}
	seen := make(map[string]bool)
	var kept []string
	for _, line := range lines {
		if !seen[line] {
			seen[line] = true
			kept = append(kept, line)
		}
	}
	removed := len(lines) - len(kept)
	if removed == 0 {
		return 0, nil
	}

	info, err := os.Stat(path)
	if err != nil {
		return 0, err
	}
	// A new file in the same folder, so that the rename can't cross to another disk.
	// CreateTemp puts a random number where the * is.
	file, err := os.CreateTemp(filepath.Dir(path), ".dedup-*")
	if err != nil {
		return 0, err
	}
	// Removes the new file if anything below fails. After the rename there is no file
	// by that name, and Remove fails, which doesn't matter.
	defer os.Remove(file.Name())
	_, err = file.WriteString(strings.Join(kept, "\n") + "\n")
	if err == nil {
		err = file.Chmod(info.Mode()) // CreateTemp makes it readable by its owner only
	}
	if err == nil {
		err = file.Sync() // onto the disk before the rename
	}
	// Close can fail too, for a write the system held back, so its error counts.
	if closeErr := file.Close(); err == nil {
		err = closeErr
	}
	if err != nil {
		return 0, err
	}
	return removed, os.Rename(file.Name(), path)
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "usage: dedup FILE...")
		os.Exit(2)
	}
	status := 0
	for _, path := range os.Args[1:] {
		removed, err := dedup(path)
		if err != nil {
			// The error has what failed and the path, such as
			// "open notes.txt: no such file or directory".
			fmt.Fprintln(os.Stderr, "dedup:", err)
			status = 1
			continue
		}
		plural := "s"
		if removed == 1 {
			plural = ""
		}
		fmt.Printf("%s: removed %d line%s\n", path, removed, plural)
	}
	os.Exit(status)
}
```

**Python**

```python
# Removes repeated lines from each file named on the command line, keeping the first of
# each, and rewrites the file in place. It writes a new file next to the old one and
# renames it over the old one, so that a crash halfway leaves the old file whole.
import os
import shutil
import sys
import tempfile


def dedup(path: str) -> int:
    """Rewrites the file without its repeated lines, and returns how many there were."""
    # newline="" keeps "\r\n" as it is; without it, Python turns it into "\n".
    with open(path, encoding="utf-8", newline="") as file:
        text = file.read()
    lines = text.split("\n")
    if lines[-1] == "":  # the file ended with a newline, or was empty
        lines.pop()
    kept = list(dict.fromkeys(lines))  # a dict keeps the first of each, in order
    removed = len(lines) - len(kept)
    if removed == 0:
        return 0

    folder = os.path.dirname(path) or "."
    # A new file in the same folder, so that the rename can't cross to another disk.
    fd, temporary = tempfile.mkstemp(dir=folder, prefix=".dedup-")
    try:
        with open(fd, "w", encoding="utf-8", newline="") as file:
            file.write("".join(line + "\n" for line in kept))
            file.flush()
            os.fsync(file.fileno())  # onto the disk before the rename
        shutil.copymode(path, temporary)  # mkstemp makes it readable by its owner only
        os.replace(temporary, path)
    except BaseException:
        os.remove(temporary)
        raise
    return removed


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: dedup FILE...", file=sys.stderr)
        return 2
    status = 0
    for path in sys.argv[1:]:
        try:
            removed = dedup(path)
        except OSError as error:  # strerror is the system's message, without the path
            print(f"dedup: {path}: {error.strerror}", file=sys.stderr)
            status = 1
            continue
        except UnicodeDecodeError:
            print(f"dedup: {path}: not UTF-8 text", file=sys.stderr)
            status = 1
            continue
        print(f"{path}: removed {removed} line{'' if removed == 1 else 's'}")
    return status


if __name__ == "__main__":
    sys.exit(main())
```

**JavaScript**

```js
// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
import {
  closeSync,
  fchmodSync,
  fsyncSync,
  openSync,
  readFileSync,
  renameSync,
  rmSync,
  statSync,
  writeFileSync,
} from "node:fs";

// Rewrites the file without its repeated lines, and returns how many there were.
function dedup(path) {
  // "utf8" decodes the bytes; any that aren't UTF-8 become U+FFFD, without an error.
  const lines = readFileSync(path, "utf8").split("\n");
  if (lines.at(-1) === "") lines.pop(); // the file ended with a newline, or was empty
  const kept = [...new Set(lines)]; // a Set keeps the first of each, in order
  const removed = lines.length - kept.length;
  if (removed === 0) return 0;

  // Node can't make a file with a fresh name, so the new file is FILE.tmp, and "wx"
  // fails if that is already there rather than writing over it.
  const temporary = `${path}.tmp`;
  const fd = openSync(temporary, "wx");
  try {
    try {
      writeFileSync(fd, kept.map((line) => line + "\n").join(""));
      fsyncSync(fd); // onto the disk before the rename
      fchmodSync(fd, statSync(path).mode); // the old file's permissions
    } finally {
      closeSync(fd);
    }
    renameSync(temporary, path);
  } catch (error) {
    rmSync(temporary, { force: true });
    throw error;
  }
  return removed;
}

const paths = process.argv.slice(2);
if (paths.length === 0) {
  console.error("usage: dedup FILE...");
  process.exit(2);
}
for (const path of paths) {
  let removed;
  try {
    removed = dedup(path);
  } catch (error) {
    // The message has the error's code, what failed, and the path, such as
    // "ENOENT: no such file or directory, open 'notes.txt'".
    console.error(`dedup: ${error.message}`);
    process.exitCode = 1;
    continue;
  }
  console.log(`${path}: removed ${removed} line${removed === 1 ? "" : "s"}`);
}
```

**TypeScript**

```ts
// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
import {
  closeSync,
  fchmodSync,
  fsyncSync,
  openSync,
  readFileSync,
  renameSync,
  rmSync,
  statSync,
  writeFileSync,
} from "node:fs";

// Rewrites the file without its repeated lines, and returns how many there were.
function dedup(path: string): number {
  // "utf8" decodes the bytes; any that aren't UTF-8 become U+FFFD, without an error.
  const lines = readFileSync(path, "utf8").split("\n");
  if (lines.at(-1) === "") lines.pop(); // the file ended with a newline, or was empty
  const kept = [...new Set(lines)]; // a Set keeps the first of each, in order
  const removed = lines.length - kept.length;
  if (removed === 0) return 0;

  // Node can't make a file with a fresh name, so the new file is FILE.tmp, and "wx"
  // fails if that is already there rather than writing over it.
  const temporary = `${path}.tmp`;
  const fd = openSync(temporary, "wx");
  try {
    try {
      writeFileSync(fd, kept.map((line) => line + "\n").join(""));
      fsyncSync(fd); // onto the disk before the rename
      fchmodSync(fd, statSync(path).mode); // the old file's permissions
    } finally {
      closeSync(fd);
    }
    renameSync(temporary, path);
  } catch (error) {
    rmSync(temporary, { force: true });
    throw error;
  }
  return removed;
}

const paths = process.argv.slice(2);
if (paths.length === 0) {
  console.error("usage: dedup FILE...");
  process.exit(2);
}
for (const path of paths) {
  let removed: number;
  try {
    removed = dedup(path);
  } catch (error) {
    // The message has the error's code, what failed, and the path, such as
    // "ENOENT: no such file or directory, open 'notes.txt'".
    console.error(`dedup: ${error instanceof Error ? error.message : error}`);
    process.exitCode = 1;
    continue;
  }
  console.log(`${path}: removed ${removed} line${removed === 1 ? "" : "s"}`);
}
```

**Dart**

```dart
// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
import 'dart:io';

/// Rewrites the file without its repeated lines, and returns how many there were.
int dedup(String path) {
  // readAsStringSync decodes UTF-8, and throws if the bytes aren't UTF-8.
  final lines = File(path).readAsStringSync().split('\n');
  if (lines.last == '')
    lines.removeLast(); // it ended with a newline, or was empty
  final kept = lines
      .toSet()
      .toList(); // a Set keeps the first of each, in order
  final removed = lines.length - kept.length;
  if (removed == 0) return 0;

  // Dart can't make a file with a fresh name, so the new file is FILE.tmp, and
  // exclusive: true fails if that is already there rather than writing over it.
  final temporary = File('$path.tmp')..createSync(exclusive: true);
  try {
    final out = temporary.openSync(mode: FileMode.writeOnly);
    try {
      out.writeStringSync(kept.map((line) => '$line\n').join());
      out.flushSync(); // onto the disk before the rename
    } finally {
      out.closeSync();
    }
    // dart:io can't set a file's permissions, so the new file gets the usual ones.
    temporary.renameSync(path);
  } catch (_) {
    if (temporary.existsSync()) temporary.deleteSync();
    rethrow;
  }
  return removed;
}

void main(List<String> arguments) {
  if (arguments.isEmpty) {
    stderr.writeln('usage: dedup FILE...');
    exit(2);
  }
  for (final path in arguments) {
    final int removed;
    try {
      removed = dedup(path);
    } on FileSystemException catch (error) {
      // osError has the system's message, such as "No such file or directory".
      stderr.writeln(
        'dedup: $path: ${error.osError?.message ?? error.message}',
      );
      exitCode = 1;
      continue;
    }
    print('$path: removed $removed line${removed == 1 ? '' : 's'}');
  }
}
```

**Kotlin**

```kotlin
// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.nio.file.Files
import java.nio.file.StandardCopyOption
import kotlin.system.exitProcess

// Rewrites the file without its repeated lines, and returns how many there were.
fun dedup(path: String): Int {
    val file = File(path)
    // readText decodes UTF-8; bytes that aren't UTF-8 become U+FFFD, without an error.
    // split, not lines(), which would also split at a "\r".
    val lines = file.readText().split("\n").toMutableList()
    if (lines.last() == "") lines.removeLast() // it ended with a newline, or was empty
    val kept = lines.distinct() // the first of each, in order
    val removed = lines.size - kept.size
    if (removed == 0) return 0

    val target = file.toPath()
    val folder = target.toAbsolutePath().parent
    // A new file in the same folder, with a fresh name, readable by its owner only.
    val temporary = Files.createTempFile(folder, ".dedup-", null)
    try {
        FileOutputStream(temporary.toFile()).use { out ->
            out.write(kept.joinToString("") { "$it\n" }.toByteArray())
            out.fd.sync() // onto the disk before the rename
        } // use closes it, even if something threw
        Files.setPosixFilePermissions(temporary, Files.getPosixFilePermissions(target))
        Files.move(temporary, target, StandardCopyOption.ATOMIC_MOVE)
    } catch (error: Exception) {
        Files.deleteIfExists(temporary)
        throw error
    }
    return removed
}

fun main(args: Array<String>) {
    if (args.isEmpty()) {
        System.err.println("usage: dedup FILE...")
        exitProcess(2)
    }
    var status = 0
    for (path in args) {
        val removed =
            try {
                dedup(path)
            } catch (error: IOException) {
                // Such as "notes.txt (No such file or directory)", for File's errors.
                System.err.println("dedup: ${error.message}")
                status = 1
                continue
            }
        println("$path: removed $removed line${if (removed == 1) "" else "s"}")
    }
    exitProcess(status)
}
```

```console
$ cat list.txt
milk
eggs
milk
bread
eggs
$ dedup list.txt
list.txt: removed 2 lines
$ cat list.txt
milk
eggs
bread
$ dedup missing.txt list.txt
dedup: missing.txt: No such file or directory
list.txt: removed 0 lines
$ echo $?
1
```

### Whole files, and lines

Most of the versions read each file whole, which is simplest, and right for a file that
fits in memory many times over: `fs::read_to_string` in Rust, `os.ReadFile` in Go,
`file.read()` in Python, `readFileSync` in JavaScript, `readAsStringSync` in Dart, and
`readText` in Kotlin. C has no such function, and example 02's `read_file` grows a
buffer with `realloc` until `fread` returns 0. C++ has none either, short of reading
through an iterator, and its version reads a line at a time with `std::getline`, which
splits only at `\n`.

A file too big to hold is read a line at a time, and here the languages disagree about
what a line is. A file holding `one\r\ntwo\rthree\nfour`, with a Windows line ending, an
old Mac one, and no ending at the end:

| Language | A line at a time | The lines it gives |
| --- | --- | --- |
| C | `getline(&line, &size, file)`, from POSIX | `one\r\n`, `two\rthree\n`, and `four`: each with its `\n` |
| C++ | `std::getline(in, line)` | `one\r`, `two\rthree`, and `four` |
| Rust | `BufReader::new(file).lines()` | `one`, `two\rthree`, and `four`: it takes off `\n` and `\r\n` |
| Go | `bufio.NewScanner(file)` and `Scan` | The same as Rust; and a line longer than 64 KiB stops it, with `bufio.Scanner: token too long` |
| Python | `for line in file:` | `one\n`, `two\n`, `three\n`, and `four`: text mode turns `\r\n` and `\r` into `\n`, unless `open` is given `newline=""` |
| JavaScript | `readline.createInterface({ input: createReadStream(path) })`, in a `for await` loop | `one`, `two`, `three`, and `four`: a `\r` alone ends a line too |
| Dart | `file.openRead().transform(utf8.decoder).transform(const LineSplitter())`, or `readAsLinesSync` | The same as JavaScript |
| Kotlin | `File(path).useLines { lines -> ... }`, or `readText().lines()` | The same as JavaScript |

So "split into lines" is four different operations, and none of the line readers gives
back the file as it was. That is why example 02 splits at `\n` itself, in every
language, and keeps whatever comes before it: test 04 is a file from Windows, whose
`\r\n` endings must come out as they went in. Python's version opens the file with
`newline=""`, or text mode would have changed them before the program saw them.

### Errors

When a file can't be opened or read, the system says why with a number, `errno`, such as
`ENOENT`, 2, for a name that isn't there, and C's `strerror` turns the number into the
words that every Unix tool prints. The other languages wrap the number in an error type
of their own, with a message of their own:

| Language | `dedup missing.txt folder` |
| --- | --- |
| C, C++, Python, Dart | `dedup: missing.txt: No such file or directory`, and `dedup: folder: Is a directory` |
| Rust | `No such file or directory (os error 2)`, and `Is a directory (os error 21)`, after the name that the example adds |
| Go | `dedup: open missing.txt: no such file or directory`, and `dedup: read folder: is a directory`: what it was doing, and the name, in lowercase |
| JavaScript, TypeScript | `dedup: ENOENT: no such file or directory, open 'missing.txt'`, and `dedup: EISDIR: illegal operation on a directory, read`, without the name |
| Kotlin | `dedup: missing.txt (No such file or directory)`, and `dedup: folder (Is a directory)` |

Python's `OSError` has the system's words as `strerror`, without the name, and Dart's
`FileSystemException` has them as `osError.message`; C++'s `std::system_error` has them
as `code().message()`. Go's, Node's, and Java's messages already have the name, so
example 02 prints them as they are, and Node's for the folder doesn't, which is worth
knowing before a user sees `illegal operation on a directory, read` and wonders which.
Code that has to act on an error, rather than print it, should test its kind, not its
words: `errno == ENOENT` in C, `error.kind() == ErrorKind::NotFound` in Rust,
`errors.Is(err, fs.ErrNotExist)` in Go, `except FileNotFoundError` in Python,
`error.code === "ENOENT"` in JavaScript, and `NoSuchFileException`, from
`java.nio.file`, in Kotlin. Exercise 21.1's solutions do this, to print the same words
in all nine.

### Text or bytes

A file is bytes, and `readFileSync(path, "utf8")`, `read_to_string`, `readText`, and the
rest decode them as UTF-8 on the way in. Given a file in Latin-1, `caf\351` twice and
then `thé` in UTF-8, the nine versions of `dedup` do three different things:

| Language | `dedup latin1.txt` | The file afterwards |
| --- | --- | --- |
| C, C++, Go | `latin1.txt: removed 1 line` | The bytes as they were, less the repeated line |
| Rust | `dedup: latin1.txt: stream did not contain valid UTF-8`, and status 1 | Unchanged |
| Python | `dedup: latin1.txt: not UTF-8 text`, from a `UnicodeDecodeError`, and status 1 | Unchanged |
| Dart | `dedup: latin1.txt: Failed to decode data using encoding 'utf-8'`, and status 1 | Unchanged |
| JavaScript, TypeScript, Kotlin | `latin1.txt: removed 1 line` | `caf` and the bytes `ef bf bd`, U+FFFD in UTF-8: the `é` is gone, and the file is now neither Latin-1 nor what it was |

C, C++, and Go never decoded anything, and handled the file correctly without trying.
Rust, Python, and Dart refused it, which is safe and unhelpful. JavaScript and Kotlin
damaged it with no error at all, which is the one outcome a tool must avoid, since
nobody finds out until much later. A program that must take any file reads bytes, with
`fs::read`, `os.ReadFile`, `open(path, "rb")`, `readFileSync(path)` with no encoding,
`readAsBytesSync`, or `readBytes`, and searches, splits, and writes bytes, as exercise
21.1 does. A program that wants text, and an error for anything else, can ask JavaScript
for one with `new TextDecoder("utf-8", { fatal: true })`, which throws a `TypeError`,
`The encoded data was not valid for encoding utf-8`, and Kotlin with
`Charsets.UTF_8.newDecoder().decode(...)`, which throws a `MalformedInputException`.

One more default is worth knowing: Python's `open` without an `encoding` uses the
locale's, which is UTF-8 on most Linux systems and usually something else on Windows.
Every `open` of a text file in this chapter's Python says `encoding="utf-8"`, and
`python3 -X warn_default_encoding` prints an `EncodingWarning` for each one that
doesn't.

---

## 21.5 Writing files safely

The simplest way to change a file is to open it for writing, which empties it, and write
the new contents. If the program crashes, is killed, or fills the disk halfway, the old
contents are gone and the new ones aren't all there. Example 02 does what editors,
package managers, and databases do instead:

1. Write the new contents to a new file, in the same folder.
2. Flush it to the disk, with `fsync`.
3. Close it, and check that closing worked, since a write that the system held back can
   fail then.
4. Give it the old file's permissions.
5. Rename it to the old file's name.

The rename is what makes it safe. On one file system, `rename` replaces the old name in
one step, so that anyone who opens the file gets the old contents or the new, and never
a mix, and a crash leaves one or the other. The new file has to be in the same folder,
or at least on the same file system, since a rename can't move a file from one disk to
another: from `/dev/shm`, which is in memory, to a home folder, Python's `os.rename`
fails with error 18, `Invalid cross-device link`. That's why the examples don't put the
new file in `/tmp`, which is often a separate file system. The `fsync` is for a power
cut: a write puts the data in the system's memory, to be written to the disk later, and
without it the disk can get the rename before the data, and keep an empty file under the
old name. (A program that must not lose the rename itself, such as a database, then
opens the folder and fsyncs that too. None of these examples does.)

Here is what each version asks the system to do, from `strace`, which lists a program's
system calls:

| Language | The new file | Flushed with | Permissions | Renamed with |
| --- | --- | --- | --- | --- |
| C | `mkstemp`: `list.txt.jQ6F9O`, made only if no file has the name | `fsync(fd)` | `fchmod` | `rename` |
| C++ | `list.txt.tmp`, emptied if it's there | Nothing: C++ has no `fsync` | `fs::permissions` | `fs::rename` |
| Rust | `list.txt.tmp`, with `create_new(true)`, made only if no file has the name | `file.sync_all()` | `file.set_permissions` | `fs::rename` |
| Go | `os.CreateTemp`: `.dedup-4146742847` | `file.Sync()` | `file.Chmod` | `os.Rename` |
| Python | `tempfile.mkstemp`: `.dedup-r_8wn8e7` | `os.fsync` | `shutil.copymode` | `os.replace` |
| JavaScript | `list.txt.tmp`, with the flag `"wx"`, made only if no file has the name | `fsyncSync` | `fchmodSync` | `renameSync` |
| Dart | `list.txt.tmp`, with `createSync(exclusive: true)` | `flushSync()`, which calls `fsync` | Nothing: `dart:io` can't set them, and a file of `rw-r-----` came out `rw-r--r--` | `renameSync` |
| Kotlin | `Files.createTempFile`: `.dedup-17938649420884718363.tmp` | `out.fd.sync()` | `Files.setPosixFilePermissions` | `Files.move`, with `ATOMIC_MOVE` |

Four of the languages can make a file with a fresh name: `mkstemp` in C, `os.CreateTemp`
in Go, `tempfile.mkstemp` in Python, and `Files.createTempFile` in Kotlin. Each puts
random characters in the name and makes the file only if nothing has that name, readable
by its owner only, which is why each version copies the old file's permissions after.
The other four use a fixed name, `list.txt.tmp`, and that goes wrong when a crash leaves
one behind: Rust's, JavaScript's, and Dart's versions then refuse, with
`File exists (os error 17)`, `EEXIST: file already exists, open 'list.txt.tmp'`, and
`File exists`, until someone deletes it, and C++'s writes over it, which is worse if two
copies of the program run at once. C++23 adds `std::ios::noreplace`, which makes an
`ofstream` fail if the file exists, as GCC 13 shows with `-std=c++23`; C++20 has
nothing. Python uses `os.replace` rather than `os.rename`, since on Windows `rename`
fails when the new name is taken and `replace` doesn't.

Exercise 21.3 keeps a to-do list in a JSON file, and writes it in the same way, so that
a crash leaves the old list or the new one.

---

## 21.6 Standard streams and pipes

Most languages collect what a program writes in a buffer, and pass it to the system in
larger pieces, since a system call for each `printf` would be slow. When the buffer is
passed on depends on the language, and on where standard output goes. Here is
`dedup list.txt missing.txt 2>&1 | cat`, which sends both streams down one pipe, so that
the order they arrive in shows when each was written:

```console
$ dedup list.txt missing.txt 2>&1 | cat
list.txt: removed 1 line
dedup: missing.txt: No such file or directory
```

That is what seven of the versions print. C's and Python's print the two lines the other
way round. C's standard output is flushed at each newline when it goes to a terminal,
and only when its buffer is full, or at exit, when it goes to a pipe or a file; standard
error has no buffer, so the error comes out first, and the result line when the program
ends. Python's `print` does the same, unless `python3 -u` or the environment variable
`PYTHONUNBUFFERED` turns the buffer off. C++'s `std::cerr` is tied to `std::cout`, which
means that each write to `cerr` flushes `cout` first. Rust flushes standard output at
each newline wherever it goes, and Go's `os.Stdout` has no buffer at all, so that each
`fmt.Println` is a system call of its own; exercise 21.1's Go solution adds a
`bufio.Writer`, to make fewer. In a program with a buffer of its own, flush it before
writing an error, as exercise 21.1's solutions do, so that a terminal shows the lines in
the order they happened.

The same choice, line by line or in blocks, explains why a pipeline sometimes seems
stuck: `tail -f log | grep error | python3 filter.py` shows nothing until `grep`'s and
Python's buffers fill, since neither is writing to a terminal. `grep --line-buffered`,
`python3 -u`, and `stdbuf -oL`, for C programs, flush at each line instead.

Writing to standard error is `fprintf(stderr, ...)` in C, `std::cerr << ...` in C++,
`eprintln!` in Rust, `fmt.Fprintln(os.Stderr, ...)` in Go, `print(..., file=sys.stderr)`
in Python, `console.error` in JavaScript, `stderr.writeln` in Dart, and
`System.err.println` in Kotlin.

### When the reader stops reading

`head -1` reads one line and exits. The program writing into the pipe doesn't know,
until its next write fails: the system sends it the signal `SIGPIPE`, whose default
action is to end the program quietly, which is how Unix stops the earlier programs of a
pipeline once a later one has what it wants. A program that counts to 100000, one number
on each line, piped into `head -1`:

| Language | `count \| head -1` |
| --- | --- |
| C, C++, Go | Killed by `SIGPIPE`, with no message: the shell reports status 141 |
| Rust | Panics, with `failed printing to stdout: Broken pipe (os error 32)`, and status 101 |
| Python | A traceback ending in `BrokenPipeError: [Errno 32] Broken pipe`, and status 1 |
| JavaScript, Dart, Kotlin | No signal and no error: each counts to 100000 and exits with 0, and Kotlin's `System.out.checkError()` is then true |

The runtimes of Rust, Python, Node, Dart, and the JVM ignore `SIGPIPE`, so that a write
to a closed pipe returns an error, `EPIPE`, instead of killing the program; that's what
a server wants, when a client hangs up. Rust's `println!` panics on any error, and
Python raises one, and both print an alarming message for something normal. Node, Dart,
and the JVM drop the error, which is worse: a program that writes forever, as `yes`
does, never stops. Each of the three was still running when a 20-second timeout killed
it. Go ignores `SIGPIPE` for other files, but dies of it when the write was to standard
output or standard error, as the C program does.

The fixes: in Rust, write with `writeln!(out, ...)` and stop when it returns an error,
as exercise 21.2's solution does. In Python, catch `BrokenPipeError` around `main`, and
exit; Python's documentation for its `signal` module shows how to do it without a second
error when the buffer is flushed at exit. In Node, write with `fs.writeSync(1, text)`,
which throws an error whose `code` is `EPIPE`, where `console.log` reported nothing. In
Dart, `await stdout.flush()` now and then, which throws a `FileSystemException`,
`OS Error: Broken pipe`, once the reader is gone. In Kotlin, test
`System.out.checkError()` now and then. Each of these stopped the endless writer above
at once.

---

## 21.7 JSON

Example 03 reads a JSON file of books, and prints a report on them, as JSON: how many
there are, the oldest, the books with each tag, and the books with none. Each book needs
a title, an author, and a year, which must be a whole number, and may have a list of
tags; other fields are ignored.

**C**

```c
// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. cJSON reads JSON into a
// tree of cJSON nodes, and this checks each node's type itself. cJSON's printer indents
// with tabs, so this writes the report itself too.
#include <cJSON.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *title;
    const char *author;
    int year;
    const cJSON *tags;  // an array of strings, or NULL for none
} Book;

// Reads a whole file into a string. Returns NULL, with errno set, if it can't.
static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) return NULL;
    char *text = NULL;
    size_t length = 0, capacity = 0;
    for (;;) {
        if (length + 1 >= capacity) {  // room for the '\0' at the end
            capacity = capacity == 0 ? 4096 : capacity * 2;
            char *bigger = realloc(text, capacity);
            if (bigger == NULL) break;
            text = bigger;
        }
        size_t got = fread(text + length, 1, capacity - length - 1, file);
        length += got;
        if (got == 0) break;
    }
    int error = ferror(file) || length + 1 >= capacity ? errno : 0;
    fclose(file);
    if (error != 0) {
        free(text);
        errno = error;
        return NULL;
    }
    text[length] = '\0';
    return text;
}

// Fills in book from a node, or returns a message saying what is wrong with it.
static const char *book_from_json(const cJSON *json, Book *book) {
    const cJSON *title = cJSON_GetObjectItemCaseSensitive(json, "title");
    const cJSON *author = cJSON_GetObjectItemCaseSensitive(json, "author");
    const cJSON *year = cJSON_GetObjectItemCaseSensitive(json, "year");
    const cJSON *tags = cJSON_GetObjectItemCaseSensitive(json, "tags");
    // A missing key gives NULL, and cJSON_IsString(NULL) is false. valueint is the
    // number cut to an int, so it differs from valuedouble for 1965.5.
    if (!cJSON_IsObject(json) || !cJSON_IsString(title) || !cJSON_IsString(author) ||
        !cJSON_IsNumber(year) || year->valuedouble != year->valueint) {
        return "each book needs a title, an author, and a year";
    }
    if (tags != NULL) {
        int ok = cJSON_IsArray(tags);
        const cJSON *tag;
        cJSON_ArrayForEach(tag, tags) ok = ok && cJSON_IsString(tag);
        if (!ok) return "tags must be a list of strings";
    }
    *book = (Book){title->valuestring, author->valuestring, year->valueint, tags};
    return NULL;
}

// Writes text as a JSON string: in quotes, with " and \ and control characters escaped,
// and everything else, UTF-8 included, as it is.
static void print_string(const char *text) {
    putchar('"');
    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
        switch (*p) {
        case '"': fputs("\\\"", stdout); break;
        case '\\': fputs("\\\\", stdout); break;
        case '\n': fputs("\\n", stdout); break;
        case '\r': fputs("\\r", stdout); break;
        case '\t': fputs("\\t", stdout); break;
        default:
            if (*p < 0x20) printf("\\u%04x", *p);
            else putchar(*p);
        }
    }
    putchar('"');
}

// Writes an array of strings, one to a line, the way JSON.stringify(value, null, 2)
// does; indent is the indent of the line the array starts on.
static void print_strings(const char **items, size_t count, const char *indent) {
    if (count == 0) {
        fputs("[]", stdout);
        return;
    }
    puts("[");
    for (size_t i = 0; i < count; i++) {
        printf("%s  ", indent);
        print_string(items[i]);
        puts(i + 1 < count ? "," : "");
    }
    printf("%s]", indent);
}

static int has_tag(const Book *book, const char *tag) {
    const cJSON *item;
    cJSON_ArrayForEach(item, book->tags) {
        if (strcmp(item->valuestring, tag) == 0) return 1;
    }
    return 0;
}

static int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static void print_report(const Book *books, size_t count) {
    // Every title and tag fits in arrays as long as the list of books and its tags.
    size_t tag_count = 0, all_tags = 0;
    for (size_t i = 0; i < count; i++) all_tags += (size_t)cJSON_GetArraySize(books[i].tags);
    const char **tags = malloc((all_tags + 1) * sizeof *tags);
    const char **titles = malloc((count + 1) * sizeof *titles);
    if (tags == NULL || titles == NULL) {
        fputs("books: out of memory\n", stderr);
        exit(1);
    }

    const Book *oldest = NULL;  // the first of the oldest, when two have the same year
    for (size_t i = 0; i < count; i++) {
        if (oldest == NULL || books[i].year < oldest->year) oldest = &books[i];
        const cJSON *tag;
        cJSON_ArrayForEach(tag, books[i].tags) {
            int seen = 0;
            for (size_t j = 0; j < tag_count && !seen; j++) {
                seen = strcmp(tags[j], tag->valuestring) == 0;
            }
            if (!seen) tags[tag_count++] = tag->valuestring;
        }
    }
    qsort(tags, tag_count, sizeof *tags, compare_strings);  // strcmp: by byte

    printf("{\n  \"count\": %zu,\n  \"oldest\": ", count);
    if (oldest == NULL) {
        fputs("null", stdout);
    } else {
        size_t size = strlen(oldest->title) + strlen(oldest->author) + sizeof ", by ";
        char *text = malloc(size);
        if (text == NULL) exit(1);
        snprintf(text, size, "%s, by %s", oldest->title, oldest->author);
        print_string(text);
        free(text);
    }
    fputs(",\n  \"tags\": ", stdout);
    if (tag_count == 0) fputs("{}", stdout);
    for (size_t t = 0; t < tag_count; t++) {
        fputs(t == 0 ? "{\n    " : ",\n    ", stdout);
        print_string(tags[t]);
        fputs(": ", stdout);
        size_t n = 0;
        for (size_t i = 0; i < count; i++) {
            if (has_tag(&books[i], tags[t])) titles[n++] = books[i].title;
        }
        print_strings(titles, n, "    ");
        if (t + 1 == tag_count) fputs("\n  }", stdout);
    }
    fputs(",\n  \"untagged\": ", stdout);
    size_t n = 0;
    for (size_t i = 0; i < count; i++) {
        if (cJSON_GetArraySize(books[i].tags) == 0) titles[n++] = books[i].title;
    }
    print_strings(titles, n, "  ");
    puts("\n}");
    free(tags);
    free(titles);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fputs("usage: books FILE\n", stderr);
        return 2;
    }
    const char *path = argv[1];
    char *text = read_file(path);
    if (text == NULL) {
        fprintf(stderr, "books: %s: %s\n", path, strerror(errno));
        return 1;
    }
    // The 1 says that nothing may follow the JSON value; cJSON_Parse(text) would
    // read "{} and more" as {}, and stop.
    cJSON *json = cJSON_ParseWithOpts(text, NULL, 1);
    free(text);
    if (json == NULL) {
        fprintf(stderr, "books: %s: not valid JSON\n", path);
        return 1;
    }
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, "books");
    size_t count = 0;
    Book *books = malloc(((size_t)cJSON_GetArraySize(list) + 1) * sizeof *books);
    const char *problem = cJSON_IsArray(list) ? NULL : "expected an object with a list of books";
    if (books == NULL) problem = "out of memory";
    const cJSON *item;
    cJSON_ArrayForEach(item, list) {  // a loop over the array's nodes, which are a list
        if (problem == NULL) problem = book_from_json(item, &books[count++]);
    }
    if (problem == NULL) print_report(books, count);
    else fprintf(stderr, "books: %s: %s\n", path, problem);
    free(books);
    cJSON_Delete(json);  // frees every node, and the strings the books point to
    return problem == NULL ? 0 : 1;
}
```

**C++**

```cpp
// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. nlohmann::json holds
// any JSON value, and get<Book>() turns one into a Book with the function from_json
// below, which checks each field.
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using nlohmann::json;

struct Book {
    std::string title;
    std::string author;
    int year;
    std::vector<std::string> tags;
};

// get<Book>() finds this function by the type of its second argument. at() throws
// out_of_range when a key is missing, and get_to throws type_error for a wrong type.
void from_json(const json& j, Book& book) {
    j.at("title").get_to(book.title);
    j.at("author").get_to(book.author);
    // get_to would turn true into 1, and 1965.5 into 1965, so year is checked first.
    const json& year = j.at("year");
    if (!year.is_number_integer()) {
        throw json::type_error::create(302, "year must be a whole number, not " + year.dump(),
                                       &year);
    }
    year.get_to(book.year);
    book.tags = j.value("tags", std::vector<std::string>{});  // the default if missing
}

// ordered_json keeps its keys in the order they went in; json would sort them.
nlohmann::ordered_json report(const std::vector<Book>& books) {
    const Book* oldest = nullptr;  // the first of the oldest, when two have the same year
    std::map<std::string, std::vector<std::string>> tags;  // keeps its keys in order
    std::vector<std::string> untagged;
    for (const Book& book : books) {
        if (oldest == nullptr || book.year < oldest->year) oldest = &book;
        for (const std::string& tag : book.tags) tags[tag].push_back(book.title);
        if (book.tags.empty()) untagged.push_back(book.title);
    }
    nlohmann::ordered_json result;
    result["count"] = books.size();
    // Not oldest ? ... : nullptr, whose type would be std::string, from a null pointer.
    result["oldest"] = nullptr;  // written as null
    if (oldest) result["oldest"] = oldest->title + ", by " + oldest->author;
    result["tags"] = tags;  // a map becomes an object, and a vector an array
    result["untagged"] = untagged;
    return result;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: books FILE\n";
        return 2;
    }
    std::string path = argv[1];
    std::ifstream in(path);
    if (!in) {
        std::cerr << "books: " << path << ": " << std::strerror(errno) << '\n';
        return 1;
    }
    std::vector<Book> books;
    try {
        books = json::parse(in).at("books").get<std::vector<Book>>();
    } catch (const json::exception& error) {
        // Such as "[json.exception.out_of_range.403] key 'year' not found".
        std::cerr << "books: " << path << ": " << error.what() << '\n';
        return 1;
    }
    std::cout << report(books).dump(2) << '\n';  // 2: indent by two spaces
}
```

**Rust**

```rust
//! Reads a JSON file of books and prints a report on them, as JSON: how many there are,
//! the oldest, the books with each tag, and the books with none. serde's derive writes
//! the code that turns JSON into a `Library` and a `Report` into JSON, checking every
//! field's type on the way in.
use serde::{Deserialize, Serialize};
use std::collections::BTreeMap;
use std::process::ExitCode;

#[derive(Deserialize)]
struct Library {
    books: Vec<Book>,
}

#[derive(Deserialize)]
struct Book {
    title: String,
    author: String,
    year: i64,
    #[serde(default)] // an empty Vec when tags is missing
    tags: Vec<String>,
}

// The fields are written in this order, under these names.
#[derive(Serialize)]
struct Report<'a> {
    count: usize,
    oldest: Option<String>,                // None is written as null
    tags: BTreeMap<&'a str, Vec<&'a str>>, // a BTreeMap keeps its keys in order
    untagged: Vec<&'a str>,
}

fn report(books: &[Book]) -> Report<'_> {
    let mut tags: BTreeMap<&str, Vec<&str>> = BTreeMap::new();
    for book in books {
        for tag in &book.tags {
            tags.entry(tag).or_default().push(&book.title);
        }
    }
    Report {
        count: books.len(),
        // min_by_key gives the first of the oldest, when two have the same year.
        oldest: books
            .iter()
            .min_by_key(|book| book.year)
            .map(|book| format!("{}, by {}", book.title, book.author)),
        tags,
        untagged: books
            .iter()
            .filter(|book| book.tags.is_empty())
            .map(|book| book.title.as_str())
            .collect(),
    }
}

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().collect();
    let [_, path] = args.as_slice() else {
        eprintln!("usage: books FILE");
        return ExitCode::from(2);
    };
    let text = match std::fs::read_to_string(path) {
        Ok(text) => text,
        Err(error) => {
            eprintln!("books: {path}: {error}");
            return ExitCode::FAILURE;
        }
    };
    // from_str's type, Library, comes from the variable's: serde reads the JSON into it.
    let library: Library = match serde_json::from_str(&text) {
        Ok(library) => library,
        Err(error) => {
            // Such as "missing field `year` at line 1 column 49".
            eprintln!("books: {path}: {error}");
            return ExitCode::FAILURE;
        }
    };
    // to_string_pretty can only fail for a map whose keys aren't strings.
    let json = serde_json::to_string_pretty(&report(&library.books)).expect("a report");
    println!("{json}");
    ExitCode::SUCCESS
}
```

**Go**

```go
// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. encoding/json fills in
// structs, by the names in their fields' tags, and checks each field's type.
package main

import (
	"encoding/json"
	"fmt"
	"os"
)

// A field that is missing from the JSON keeps its zero value, so a book with no year
// would have year 0. Pointers tell the two apart: they stay nil when it is missing.
type bookJSON struct {
	Title  *string  `json:"title"`
	Author *string  `json:"author"`
	Year   *int     `json:"year"`
	Tags   []string `json:"tags"` // missing or null: a nil slice, which is no tags
}

type libraryJSON struct {
	Books []bookJSON `json:"books"`
}

type book struct {
	title, author string
	year          int
	tags          []string
}

// The report's fields are written in this order, under these names.
type report struct {
	Count    int                 `json:"count"`
	Oldest   *string             `json:"oldest"`   // nil is written as null
	Tags     map[string][]string `json:"tags"`     // written in order of key
	Untagged []string            `json:"untagged"` // a nil slice would be null, not []
}

func readBooks(path string) ([]book, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var library libraryJSON
	if err := json.Unmarshal(data, &library); err != nil {
		return nil, fmt.Errorf("%s: %w", path, err)
	}
	if library.Books == nil {
		return nil, fmt.Errorf("%s: expected an object with a list of books", path)
	}
	books := make([]book, 0, len(library.Books))
	for _, b := range library.Books {
		if b.Title == nil || b.Author == nil || b.Year == nil {
			return nil, fmt.Errorf("%s: each book needs a title, an author, and a year", path)
		}
		books = append(books, book{*b.Title, *b.Author, *b.Year, b.Tags})
	}
	return books, nil
}

func makeReport(books []book) report {
	r := report{Count: len(books), Tags: map[string][]string{}, Untagged: []string{}}
	oldest := -1 // the first of the oldest, when two have the same year
	for i, b := range books {
		if oldest < 0 || b.year < books[oldest].year {
			oldest = i
		}
		for _, tag := range b.tags {
			r.Tags[tag] = append(r.Tags[tag], b.title)
		}
		if len(b.tags) == 0 {
			r.Untagged = append(r.Untagged, b.title)
		}
	}
	if oldest >= 0 {
		description := books[oldest].title + ", by " + books[oldest].author
		r.Oldest = &description
	}
	return r
}

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintln(os.Stderr, "usage: books FILE")
		os.Exit(2)
	}
	books, err := readBooks(os.Args[1])
	if err != nil {
		fmt.Fprintln(os.Stderr, "books:", err)
		os.Exit(1)
	}
	encoder := json.NewEncoder(os.Stdout)
	encoder.SetIndent("", "  ")
	// Without this, & < and > are written as \u0026 \u003c and \u003e, so that the
	// JSON is safe to put in a web page.
	encoder.SetEscapeHTML(false)
	if err := encoder.Encode(makeReport(books)); err != nil {
		fmt.Fprintln(os.Stderr, "books:", err)
		os.Exit(1)
	}
}
```

**Python**

```python
# Reads a JSON file of books and prints a report on them, as JSON: how many there are,
# the oldest, the books with each tag, and the books with none. json.load gives dicts
# and lists, and match checks that each book has the fields it needs.
import json
import sys
from dataclasses import dataclass


@dataclass
class Book:
    title: str
    author: str
    year: int
    tags: list[str]


def book_from_json(value: object) -> Book:
    match value:
        # A dict pattern matches a dict with these keys, and **rest takes any others.
        # bool is a kind of int in Python, so int(year) matches true too, unless the
        # guard stops it.
        case {
            "title": str(title),
            "author": str(author),
            "year": int(year),
            **rest,
        } if not isinstance(year, bool):
            tags = rest.get("tags", [])
            if not isinstance(tags, list) or not all(isinstance(t, str) for t in tags):
                raise ValueError(f"{title}: tags must be a list of strings")
            return Book(title, author, year, tags)
        case _:
            raise ValueError("each book needs a title, an author, and a year")


def read_books(path: str) -> list[Book]:
    with open(path, encoding="utf-8") as file:
        data = json.load(
            file
        )  # raises json.JSONDecodeError, a ValueError, if it isn't JSON
    match data:
        case {"books": list(books)}:
            return [book_from_json(book) for book in books]
        case _:
            raise ValueError("expected an object with a list of books")


def report(books: list[Book]) -> dict[str, object]:
    # min gives the first of the oldest when two have the same year.
    oldest = min(books, key=lambda book: book.year, default=None)
    tags: dict[str, list[str]] = {}
    for book in books:
        for tag in book.tags:
            tags.setdefault(tag, []).append(book.title)
    return {
        "count": len(books),
        # json.dumps writes None as null.
        "oldest": f"{oldest.title}, by {oldest.author}" if oldest else None,
        "tags": dict(sorted(tags.items())),  # a dict keeps the order its keys went in
        "untagged": [book.title for book in books if not book.tags],
    }


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: books FILE", file=sys.stderr)
        return 2
    path = sys.argv[1]
    try:
        books = read_books(path)
    except OSError as error:
        print(f"books: {path}: {error.strerror}", file=sys.stderr)
        return 1
    except ValueError as error:
        print(f"books: {path}: {error}", file=sys.stderr)
        return 1
    # ensure_ascii=False writes "ñ" as it is, instead of as "\u00f1".
    print(json.dumps(report(books), indent=2, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

**JavaScript**

```js
// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. JSON.parse gives plain
// objects and arrays, and the checks that each book has the fields it needs are ours.
import { readFileSync } from "node:fs";

// Returns the book, or throws if value isn't one.
function bookFromJson(value) {
  if (typeof value !== "object" || value === null) {
    throw new Error("each book must be an object");
  }
  // The default applies when tags is missing, but not when it is null.
  const { title, author, year, tags = [] } = value;
  if (
    typeof title !== "string" ||
    typeof author !== "string" ||
    !Number.isInteger(year)
  ) {
    throw new Error("each book needs a title, an author, and a year");
  }
  if (!Array.isArray(tags) || !tags.every((tag) => typeof tag === "string")) {
    throw new Error(`${title}: tags must be a list of strings`);
  }
  return { title, author, year, tags };
}

function readBooks(text) {
  const data = JSON.parse(text); // throws a SyntaxError if it isn't JSON
  if (!Array.isArray(data?.books)) {
    throw new Error("expected an object with a list of books");
  }
  return data.books.map(bookFromJson);
}

function report(books) {
  let oldest = null; // the first of the oldest, when two have the same year
  for (const book of books) {
    if (oldest === null || book.year < oldest.year) oldest = book;
  }
  const tags = new Map();
  for (const book of books) {
    for (const tag of book.tags) {
      if (!tags.has(tag)) tags.set(tag, []);
      tags.get(tag).push(book.title);
    }
  }
  const sortedTags = [...tags].sort(([a], [b]) => (a < b ? -1 : a > b ? 1 : 0));
  return {
    count: books.length,
    oldest: oldest === null ? null : `${oldest.title}, by ${oldest.author}`,
    // An object keeps its keys in the order they went in, unless they look like array
    // indexes, such as "7": those come first, in order of their value.
    tags: Object.fromEntries(sortedTags),
    untagged: books
      .filter((book) => book.tags.length === 0)
      .map((book) => book.title),
  };
}

if (process.argv.length !== 3) {
  console.error("usage: books FILE");
  process.exit(2);
}
const path = process.argv[2];
let text;
try {
  text = readFileSync(path, "utf8");
} catch (error) {
  // Such as "ENOENT: no such file or directory, open 'books.json'", with the path.
  console.error(`books: ${error.message}`);
  process.exit(1);
}
let books;
try {
  books = readBooks(text);
} catch (error) {
  console.error(`books: ${path}: ${error.message}`);
  process.exit(1);
}
console.log(JSON.stringify(report(books), null, 2));
```

**TypeScript**

```ts
// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. JSON.parse returns any,
// which the type checker trusts with no checks at all, so this treats it as unknown,
// and checks that each book has the fields it needs before calling it a Book.
import { readFileSync } from "node:fs";

interface Book {
  title: string;
  author: string;
  year: number;
  tags: string[];
}

interface Report {
  count: number;
  oldest: string | null;
  tags: Record<string, string[]>;
  untagged: string[];
}

// Returns the book, or throws if value isn't one.
function bookFromJson(value: unknown): Book {
  if (typeof value !== "object" || value === null) {
    throw new Error("each book must be an object");
  }
  const { title, author, year, tags = [] } = value as Record<string, unknown>;
  if (
    typeof title !== "string" ||
    typeof author !== "string" ||
    !Number.isInteger(year)
  ) {
    throw new Error("each book needs a title, an author, and a year");
  }
  if (!Array.isArray(tags) || !tags.every((tag) => typeof tag === "string")) {
    throw new Error(`${title}: tags must be a list of strings`);
  }
  // Number.isInteger doesn't narrow unknown to number, so year needs a cast.
  return { title, author, year: year as number, tags };
}

function readBooks(text: string): Book[] {
  const data: unknown = JSON.parse(text);
  const books = (data as { books?: unknown } | null)?.books;
  if (!Array.isArray(books)) {
    throw new Error("expected an object with a list of books");
  }
  return books.map(bookFromJson);
}

function report(books: Book[]): Report {
  let oldest: Book | null = null; // the first of the oldest, when two have the same year
  for (const book of books) {
    if (oldest === null || book.year < oldest.year) oldest = book;
  }
  const tags = new Map<string, string[]>();
  for (const book of books) {
    for (const tag of book.tags) {
      const titles = tags.get(tag) ?? [];
      titles.push(book.title);
      tags.set(tag, titles);
    }
  }
  const sortedTags = [...tags].sort(([a], [b]) => (a < b ? -1 : a > b ? 1 : 0));
  return {
    count: books.length,
    oldest: oldest === null ? null : `${oldest.title}, by ${oldest.author}`,
    // An object keeps its keys in the order they went in, unless they look like array
    // indexes, such as "7": those come first, in order of their value.
    tags: Object.fromEntries(sortedTags),
    untagged: books
      .filter((book) => book.tags.length === 0)
      .map((book) => book.title),
  };
}

if (process.argv.length !== 3) {
  console.error("usage: books FILE");
  process.exit(2);
}
const path = process.argv[2];
let text: string;
try {
  text = readFileSync(path, "utf8");
} catch (error) {
  // Such as "ENOENT: no such file or directory, open 'books.json'", with the path.
  console.error(`books: ${(error as Error).message}`);
  process.exit(1);
}
let books: Book[];
try {
  books = readBooks(text);
} catch (error) {
  console.error(`books: ${path}: ${(error as Error).message}`);
  process.exit(1);
}
console.log(JSON.stringify(report(books), null, 2));
```

**Dart**

```dart
// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. jsonDecode gives maps
// and lists, and a pattern checks that each book has the fields it needs.
import 'dart:collection';
import 'dart:convert';
import 'dart:io';

class Book {
  final String title, author;
  final int year;
  final List<String> tags;

  Book(this.title, this.author, this.year, this.tags);
}

/// Returns the book, or throws a FormatException if json isn't one.
Book bookFromJson(Object? json) {
  // A map pattern matches a map with these keys, and any others; String title matches
  // only a String, and names it title.
  if (json
      case final Map<String, Object?> map &&
          {'title': String title, 'author': String author, 'year': int year}) {
    final tags = map['tags'] ?? const <Object?>[]; // missing, or null: no tags
    if (tags is! List || tags.any((tag) => tag is! String)) {
      throw FormatException('$title: tags must be a list of strings');
    }
    return Book(title, author, year, List<String>.from(tags));
  }
  throw const FormatException('each book needs a title, an author, and a year');
}

List<Book> readBooks(String text) {
  final data = jsonDecode(text); // throws a FormatException if it isn't JSON
  if (data case {'books': List<Object?> books}) {
    return books.map(bookFromJson).toList();
  }
  throw const FormatException('expected an object with a list of books');
}

Map<String, Object?> report(List<Book> books) {
  Book? oldest; // the first of the oldest, when two have the same year
  for (final book in books) {
    if (oldest == null || book.year < oldest.year) oldest = book;
  }
  // A SplayTreeMap keeps its keys in order; a plain map, in the order they went in.
  final tags = SplayTreeMap<String, List<String>>();
  for (final book in books) {
    for (final tag in book.tags) {
      tags.putIfAbsent(tag, () => []).add(book.title);
    }
  }
  return {
    'count': books.length,
    'oldest': oldest == null ? null : '${oldest.title}, by ${oldest.author}',
    'tags': tags,
    'untagged': [
      for (final book in books)
        if (book.tags.isEmpty) book.title,
    ],
  };
}

void main(List<String> arguments) {
  if (arguments.length != 1) {
    stderr.writeln('usage: books FILE');
    exit(2);
  }
  final path = arguments[0];
  final List<Book> books;
  try {
    books = readBooks(File(path).readAsStringSync());
  } on FileSystemException catch (error) {
    stderr.writeln('books: $path: ${error.osError?.message ?? error.message}');
    exit(1);
  } on FormatException catch (error) {
    stderr.writeln('books: $path: ${error.message}');
    exit(1);
  }
  print(const JsonEncoder.withIndent('  ').convert(report(books)));
}
```

**Kotlin**

```kotlin
// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. The serialization
// plugin writes the code that turns JSON into a Library and a Report into JSON,
// checking every field's type on the way in.
import java.io.File
import java.io.IOException
import kotlin.system.exitProcess
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.KSerializer
import kotlinx.serialization.Serializable
import kotlinx.serialization.SerializationException
import kotlinx.serialization.descriptors.PrimitiveKind
import kotlinx.serialization.descriptors.PrimitiveSerialDescriptor
import kotlinx.serialization.encoding.Decoder
import kotlinx.serialization.encoding.Encoder
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonDecoder
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.intOrNull

// kotlinx.serialization reads a number in quotes, such as "1965", as a number, and has
// no setting to stop it. This reads the JSON value itself, and refuses a string.
object StrictIntSerializer : KSerializer<Int> {
    override val descriptor = PrimitiveSerialDescriptor("StrictInt", PrimitiveKind.INT)

    override fun serialize(encoder: Encoder, value: Int) = encoder.encodeInt(value)

    override fun deserialize(decoder: Decoder): Int {
        val element = (decoder as JsonDecoder).decodeJsonElement()
        val number = (element as? JsonPrimitive)?.takeIf { !it.isString }?.intOrNull
        return number ?: throw SerializationException("expected a whole number, not $element")
    }
}

@Serializable data class Library(val books: List<Book>)

@Serializable
data class Book(
    val title: String,
    val author: String,
    @Serializable(with = StrictIntSerializer::class) val year: Int,
    val tags: List<String> = emptyList(), // used when tags is missing
)

// The fields are written in this order, under these names.
@Serializable
data class Report(
    val count: Int,
    val oldest: String?, // null is written as null
    val tags: Map<String, List<String>>,
    val untagged: List<String>,
)

@OptIn(ExperimentalSerializationApi::class) // for prettyPrintIndent
val json = Json {
    ignoreUnknownKeys = true // by default, a field Book doesn't have is an error
    prettyPrint = true
    prettyPrintIndent = "  " // four spaces, by default
}

fun report(books: List<Book>): Report {
    val oldest = books.minByOrNull { it.year } // the first of the oldest
    val tags = sortedMapOf<String, MutableList<String>>() // keeps its keys in order
    for (book in books) {
        for (tag in book.tags) tags.getOrPut(tag) { mutableListOf() }.add(book.title)
    }
    return Report(
        count = books.size,
        oldest = oldest?.let { "${it.title}, by ${it.author}" },
        tags = tags,
        untagged = books.filter { it.tags.isEmpty() }.map { it.title },
    )
}

fun main(args: Array<String>) {
    if (args.size != 1) {
        System.err.println("usage: books FILE")
        exitProcess(2)
    }
    val path = args[0]
    val library =
        try {
            json.decodeFromString<Library>(File(path).readText())
        } catch (error: IOException) {
            System.err.println("books: ${error.message}") // the path and the reason
            exitProcess(1)
        } catch (error: SerializationException) {
            // The message is several lines, with the JSON around the problem.
            System.err.println("books: $path: ${error.message?.lines()?.first()}")
            exitProcess(1)
        }
    println(json.encodeToString(report(library.books)))
}
```

Given this `books.json`:

```json
{
  "books": [
    {"title": "Pride & Prejudice", "author": "Jane Austen", "year": 1813,
     "tags": ["classic", "romance"]},
    {"title": "Dune", "author": "Frank Herbert", "year": 1965,
     "tags": ["sf", "classic"]},
    {"title": "Cien años de soledad", "author": "Gabriel García Márquez", "year": 1967},
    {"title": "Kindred", "author": "Octavia E. Butler", "year": 1979, "tags": ["sf"]},
    {"title": "Beloved", "author": "Toni Morrison", "year": 1987, "tags": ["classic"]}
  ]
}
```

all nine print the same report, with the same spaces:

```json
{
  "count": 5,
  "oldest": "Pride & Prejudice, by Jane Austen",
  "tags": {
    "classic": [
      "Pride & Prejudice",
      "Dune",
      "Beloved"
    ],
    "romance": [
      "Pride & Prejudice"
    ],
    "sf": [
      "Dune",
      "Kindred"
    ]
  },
  "untagged": [
    "Cien años de soledad"
  ]
}
```

### Two ways to read JSON

There are two ways to read JSON, and example 03 uses both. A parser can turn the text
into a **tree** of values, and leave the program to walk it, checking each value's type
as it goes. Python's `json.load` gives `dict`s, `list`s, `str`s, `int`s, `float`s,
`bool`s, and `None`; JavaScript's `JSON.parse` gives objects, arrays, strings, numbers,
booleans, and `null`; Dart's `jsonDecode` gives `Map<String, dynamic>` and
`List<dynamic>`; and cJSON gives a tree of `cJSON` nodes, each with a type to test. Or a
library can **decode into types**: you declare a struct or a class with the fields you
expect, and the library fills one in, checking each field's type as it does. serde's
`#[derive(Deserialize)]` does it in Rust, struct tags in Go, `@Serializable` in Kotlin,
and in C++ a function, `from_json`, which nlohmann's `get<Book>()` finds by the type of
its argument. Each of those libraries has a tree too, for JSON whose shape isn't known
in advance: serde_json's `Value`, Go's `any`, kotlinx's `JsonElement`, and nlohmann's
`json`. Exercise 21.2 uses the trees, since a diff has to take any JSON at all.

With a tree, the checks are the program's job, and in JavaScript and C they are a good
part of example 03. Python's `match` and Dart's patterns check a whole book in one
statement: `{"title": str(title), "author": str(author), "year": int(year), **rest}`
matches a `dict` with those three keys, whose values have those types, and names them.
Python's needs a guard, `if not isinstance(year, bool)`, since `bool` is a kind of `int`
in Python, so that `int(year)` would match a JSON `true`. Dart's needs none, since a
Dart `bool` isn't an `int`. TypeScript's `JSON.parse` returns `any`, which the type
checker trusts with no checks at all, so example 03's TypeScript takes it as `unknown`,
which it must check before using, and only calls it a `Book` after.

Decoding into types is shorter, and it checks the types, but what it checks, and what it
says, differs:

| Language | `"year"` missing | `"year": "1965"` | `{books: []}`, which isn't JSON |
| --- | --- | --- | --- |
| C++ | `[json.exception.out_of_range.403] key 'year' not found` | `[json.exception.type_error.302] type must be number, but is string` | `[json.exception.parse_error.101] parse error at line 1, column 2: syntax error while parsing object key - invalid literal; last read: '{b'; expected string literal` |
| Rust | ``missing field `year` at line 1 column 55`` | `invalid type: string "1965", expected i64 at line 1 column 70` | `key must be a string at line 1 column 2` |
| Go | Nothing: the field keeps its zero value, so example 03 uses pointers, and checks for `nil` | `json: cannot unmarshal string into Go struct field bookJSON.books.year of type int` | `invalid character 'b' looking for beginning of object key string` |
| Kotlin | `Field 'year' is required for type with serial name 'Book', but it was missing at path: $.books[0]` | Nothing: kotlinx reads `"1965"` as 1965, so example 03 adds a serializer that refuses it | `Unexpected JSON token at offset 1: Expected quotation mark '"', but had 'b' instead at path: $` |

The other five write their own messages for the first two, and their parsers' for the
third: `Expecting property name enclosed in double quotes: line 1 column 2 (char 1)` in
Python, `Expected property name or '}' in JSON at position 1 (line 1 column 2)` in
JavaScript, `Unexpected character` in Dart, and from C, which cJSON gives no reason for,
`not valid JSON`.

Go's silence about a missing field is by design: `json.Unmarshal` fills in what the JSON
has and leaves the rest, so that a struct can hold defaults before it's decoded.
kotlinx's acceptance of `"1965"` isn't a setting: kotlinx.serialization reads a number
in quotes as a number, and `"true"` as `true`, with no option to stop it, so example 03
has a `StrictIntSerializer` that looks at the JSON value itself. nlohmann's `get_to` is
loose in the other direction: it turns `true` into 1 and `1965.5` into 1965, so example
03's C++ checks `is_number_integer()` before it, and tests 11 and 12 make sure that all
nine refuse both.

**Missing fields, and fields nobody asked for.** `tags` is optional, and each language
says so in its own way: `#[serde(default)]` in Rust, a default value, `= emptyList()`,
in Kotlin, `j.value("tags", ...)` in C++, `rest.get("tags", [])` in Python, a default in
the destructuring, `tags = []`, in JavaScript, and `?? const []` in Dart. Go needs
nothing, since a missing slice is `nil`, which has length 0. Test 04 gives the file an
`"owner"` and each book `"pages"`, which example 03 doesn't know about: serde and Go
ignore a key that the type has no field for, kotlinx refuses it unless
`ignoreUnknownKeys = true`, and the tree-based versions never look. A program that wants
to be strict can say so with `#[serde(deny_unknown_fields)]` in Rust and a
`json.Decoder`'s `DisallowUnknownFields()` in Go, which exercise 21.3's solutions use.

### Writing JSON

The report has to come out with its keys in a set order, two spaces for each level, `ñ`
and `á` as they are, `null` when there are no books, and `[]` and `{}` for an empty list
and an empty set of tags. What each version needed for that:

| Language | How it writes the report |
| --- | --- |
| C | By hand: cJSON's `cJSON_Print` indents with tabs, and has no setting for spaces |
| C++ | `nlohmann::ordered_json`, which keeps keys in the order they went in, and `dump(2)`; `json` would sort them |
| Rust | `#[derive(Serialize)]` on a struct, whose fields are written in the order they're declared, and `serde_json::to_string_pretty`, which indents by two |
| Go | A struct, with tags that name its fields, and a `json.Encoder` with `SetIndent("", "  ")` and `SetEscapeHTML(false)`; `Untagged` starts as `[]string{}`, since a `nil` slice is written as `null` |
| Python | `json.dumps(report, indent=2, ensure_ascii=False)`; without `ensure_ascii=False`, `ñ` is written `\u00f1` |
| JavaScript, TypeScript | `JSON.stringify(report, null, 2)` |
| Dart | `JsonEncoder.withIndent('  ').convert(report)` |
| Kotlin | A `@Serializable` data class, and `Json { prettyPrint = true; prettyPrintIndent = "  " }`, since the default indent is four spaces |

Go's `SetEscapeHTML(false)` is for the `&` in "Pride & Prejudice": without it,
`json.Marshal` and an `Encoder` write `&`, `<`, and `>` as `\u0026`, `\u003c`, and
`\u003e`, so that JSON can go inside a web page's `<script>` without ending it. The JSON
means the same either way, but the output wouldn't match the other eight.

---

## 21.8 JSON's sharp edges

Section 21.1 listed what JSON's grammar leaves open. Here is what the nine parsers do
with each, from small probe programs that read a file and print what they got, and from
example 03.

**Numbers.** JSON has one kind of number, and the languages have two or more:

| Language | `9007199254740993`, as any JSON | `1e400` | `NaN`, which isn't JSON |
| --- | --- | --- | --- |
| C (cJSON) | A `double`: 9007199254740992 | `inf` | Refused |
| C++ (nlohmann) | Exact, as a 64-bit integer | Refused | Refused |
| Rust (serde_json) | Exact | Refused: `number out of range` | Refused |
| Go (`any`) | A `float64`: 9007199254740992 | Refused | Refused |
| Python | Exact, as an `int` | `inf`, a `float` | `nan`, unless `parse_constant` says no |
| JavaScript | 9007199254740992 | `Infinity`, which `JSON.stringify` writes as `null` | Refused |
| Dart | Exact, as an `int` | `Infinity` | Refused |
| Kotlin (`JsonElement`) | Exact: the text is kept as it was | The text, kept | Accepted, as a value that isn't a string |

2^53 + 1 is the first whole number that a double can't hold, so a parser that keeps
every number as a double, as JavaScript does, quietly gives the one before it. Go's
`any` does the same, unless the `Decoder` is told `UseNumber()`, which keeps the text.
Twitter's API, for one, sent each tweet's id twice, as a number and as a string,
`id_str`, for this reason. Whether `1e3` is a whole number depends on whom you ask, too:
as example 03's year, C's, JavaScript's, and Kotlin's checks take it as 1000, and
Rust's, Go's, Python's, Dart's, and C++'s refuse it, since their parsers read it as a
floating-point number.

**Keys.** RFC 8259 leaves both the order of keys and the same key twice to the parser:

| Language | `{"a": 1, "a": 2}` | `{"b": [], "10": 1, "9": 2}`, read and written back |
| --- | --- | --- |
| C (cJSON) | Keeps both; `cJSON_GetObjectItem` finds the first | In the file's order |
| C++ (nlohmann) | 2, the last | `json` sorts them: `"10"`, `"9"`, `"b"`; `ordered_json` keeps the file's order |
| Rust | `Value`: 2; a derived struct refuses it, with ``duplicate field `tags` `` | `Value` sorts them, as C++'s `json` does |
| Go | 2 | A map has no order, and `json.Marshal` sorts its keys |
| Python | 2 | In the file's order |
| JavaScript | 2 | `"9"`, `"10"`, `"b"`: keys that look like array indexes come first, in order of their numbers |
| Dart | 2 | In the file's order |
| Kotlin | 2 | In the file's order |

So a program that reads a file and writes it back may change the order of its keys, and
a program that compares two files by their text may find differences that aren't there;
exercise 21.2 compares the values instead. JavaScript's order catches programs that sort
keys and then build an object from them, as example 03 does with its tags: given the
tags `"9"`, `"10"`, and `"b"`, the other eight versions list `"10"` first, as the sort
put it, and JavaScript's and TypeScript's list `"9"` first.

**Null and missing.** `"tags": null` is the same as no `tags` in Go, whose `nil` slice
is both, and in Dart, whose `??` example 03 uses for both. The other seven refuse it, as
a list that isn't one.

**Text that is almost JSON.** Parsers differ on what they'll take that the grammar
doesn't allow:

- A byte order mark, the bytes `ef bb bf`, which some Windows editors put at the start
  of a UTF-8 file. nlohmann and cJSON skip it, Python's `json.loads` skips it in `bytes`
  but not in a `str`, and Dart's `readAsStringSync` takes it off before `jsonDecode`
  sees it. JavaScript, Go, Rust, and Kotlin refuse the file. RFC 8259 says a parser MAY
  ignore one.
- Something after the value, as in `{"a": 1} {"a": 2}`. `cJSON_Parse` reads the first
  value and stops, which is why example 03's C uses
  `cJSON_ParseWithOpts(text, NULL, 1)`, and a Go `json.Decoder` reads one value and
  leaves the rest for the next call. The others' functions for a whole text refuse it.
- `NaN` and `Infinity`, JavaScript's names for numbers that JSON can't write. Python and
  Kotlin accept them.
- `01`, and a tab inside a string. cJSON and Kotlin accept both, and kotlinx's
  `parseToJsonElement` takes any word outside quotes, such as `hello` or `tru`, as a
  value.

**Writing values that JSON doesn't have.** Each language has values with no JSON
spelling, and handles them in its own way. `NaN`: Python writes `NaN`, which isn't JSON,
unless `allow_nan=False`; JavaScript and serde_json write `null`; Go's `json.Marshal`
returns an error, `json: unsupported value: NaN`, and Dart's `jsonEncode` throws.
JavaScript's `JSON.stringify` writes a `Map` and a `Set` as `{}`, with nothing in them,
and leaves out a property whose value is `undefined`. Python's `json.dumps` writes a
tuple as an array, and raises `TypeError: Object of type set is not JSON serializable`
for a set. Dart's `jsonEncode` throws
`Converting object to an encodable object failed: Instance of 'Point'` for a class
without a `toJson` method. And kotlinx.serialization leaves out a property whose value
is its default, so that `Task(1, "milk")`, with `done: Boolean = false`, is written
`{"id":1,"text":"milk"}`, unless the `Json` has `encodeDefaults = true`; a program that
reads the file with another language then finds no `done`.

---

## 21.9 Why each language looks the way it does

- **C** came with Unix, and its edges are Unix's: `argc` and `argv`, `errno`, file
  descriptors, and `stdio`'s buffers, which flush at newlines only for a terminal
  because a terminal was where a person waited. `getopt` dates from early Unix, and GNU
  added `getopt_long`, with its long options and its habit of finding options anywhere
  on the command line, which the BSDs copied. C has no JSON in its standard library, and
  cJSON, one C file and a header, is popular because it is that small.
- **C++** has `std::filesystem`, since C++17, from Boost, but no `fsync`, no file with a
  fresh name until C++23's `noreplace`, and no options library, which it leaves to
  libraries such as CLI11. nlohmann/json is a single header that makes JSON feel like a
  built-in type, at some cost in speed; simdjson, which reads JSON with the processor's
  vector instructions, is the choice when speed matters.
- **Rust** guarantees that a `String` is UTF-8, so it can't hand you an argument or a
  file that isn't as a `String`. It gives you `OsString` and `Vec<u8>` for those, and
  the `String` functions fail, or, for `args()`, panic, rather than guess. serde, from
  2014, separates the data model from the format: one
  `#[derive(Serialize, Deserialize)]` works with JSON, TOML, YAML, and dozens of other
  formats, each a separate crate, which is why the standard library doesn't need one.
- **Go**'s strings are bytes with no promise about them, so Go reads arguments and files
  without decoding, and decodes UTF-8 only when asked, as when you range over a string.
  `flag` treats every flag the same, a dash and a name, whether the name is a letter or
  a word, and stops at the first argument that isn't a flag, as POSIX says.
  `encoding/json`, in the standard library since Go 1.0, in 2012, chose zero values for
  missing fields and leniency for unknown ones, to suit servers whose messages grow new
  fields.
- **Python** 3 made `str` Unicode, and had to decide what to do with the bytes at the
  edges that aren't; PEP 383, in Python 3.1, kept them as surrogates so that a file name
  can always go back to the bytes it came from. argparse, since Python 2.7 and 3.2,
  replaced `optparse` and `getopt`, which are still there. The `json` module, from 2.6,
  began as the simplejson library, and reads and writes `NaN` and `Infinity`, which
  JavaScript has and JSON doesn't, unless told not to.
- **JavaScript** got `JSON.parse` and `JSON.stringify` in ES5, in 2009, built on
  Crockford's json2.js; JSON is JavaScript's own syntax, so the language reads it with
  no types to fill. Node has synchronous and asynchronous versions of every file
  function, `readFileSync` and `readFile`, since a server shouldn't stop for a disk.
  `util.parseArgs` came in Node 18, after years in which programs used packages from
  npm, such as commander and yargs.
- **TypeScript** checks types at compile time and erases them, so it can't check JSON
  that arrives at run time; `JSON.parse` returns `any` for that reason. Libraries such
  as zod declare a schema once, check data against it at run time, and give TypeScript
  the type, as serde's derive does for Rust.
- **Dart**'s strings are UTF-16, as JavaScript's are, but where Node replaces the bytes
  it can't decode, Dart's `utf8` decoder throws a `FormatException`, unless it is given
  `allowMalformed: true`; `dart:io`'s file functions mirror Node's, with `Sync`
  versions, and `dart:convert` treats JSON as one codec among several, beside `utf8` and
  `base64`. JSON in Flutter apps is usually decoded into classes with
  `json_serializable`, which generates `fromJson` and `toJson`, since Flutter doesn't
  allow the reflection that would do it at run time.
- **Kotlin** runs on the JVM, whose readers and `String` constructor replace the bytes
  they can't decode, as Java's always have, and whose `PrintStream` never throws, so
  that `System.out.println` can't fail. kotlinx.serialization uses a compiler plugin
  rather than reflection, which Java's JSON libraries, such as Jackson and Gson, use, so
  that the same code runs on the JVM, in JavaScript, and in native code, under Kotlin
  Multiplatform.

---

## 21.10 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | `printf` to standard output, then `fprintf(stderr, ...)`, in a pipe | The error comes out first; `fflush(stdout)` before writing to `stderr` |
| C | `cJSON_Parse` on a file | It reads the first value and ignores the rest, so `{} and more` is `{}`; use `cJSON_ParseWithOpts(text, NULL, 1)` |
| C | Printing a number with `cJSON_Print` | 9007199254740991 comes out as `9.00719925474099e+15`, which is 9007199254740990 |
| C++ | `get<int>()` or `get_to` on a JSON value | `true` becomes 1, and `1965.5` becomes 1965; check `is_number_integer()` first |
| C++ | `nlohmann::json` for output in a set order | It sorts the keys; use `ordered_json` |
| C++ | `std::ofstream` to a temporary file with a fixed name | It empties a file that is already there; C++20 has no exclusive create, and no `fsync` |
| Rust | `std::env::args()` with a file name that isn't UTF-8 | A panic; use `args_os()` |
| Rust | `println!` into a pipe that closes | A panic, `failed printing to stdout: Broken pipe`; use `writeln!` and handle the error |
| Rust | `BufRead::lines()` to copy a file | It takes off each `\r\n`, so a Windows file comes back with `\n` |
| Go | `os.Exit` with output in a `bufio.Writer` | Deferred calls don't run, and the output is lost; flush first |
| Go | Options after the arguments, with `flag` | They become arguments: `temp 21.5c --to f` reads `--to` as a temperature |
| Go | A missing field in `json.Unmarshal` | No error: the field keeps its zero value; use pointers, or check after |
| Go | A `nil` slice in a struct for JSON | Written as `null`, not `[]` |
| Go | `json.Marshal` of text with `&`, `<`, or `>` | `\u0026`, `\u003c`, and `\u003e`; use an `Encoder` with `SetEscapeHTML(false)` |
| Go | `bufio.Scanner` on long lines | A line over 64 KiB stops it with `token too long`; give it a bigger buffer with `Buffer`, or use `bufio.Reader` |
| Python | `open(path)` without `encoding` | The locale's encoding, which isn't UTF-8 on Windows; say `encoding="utf-8"` |
| Python | Reading a file in text mode to rewrite it | `\r\n` becomes `\n`; use `newline=""` |
| Python | `isinstance(value, int)` to check JSON | `True` passes; check `bool` first |
| Python | `json.dumps` with non-ASCII text | `é` comes out as `\u00e9`; use `ensure_ascii=False` |
| Python | `json.dumps` of a `float('nan')` | `NaN`, which isn't JSON, and other parsers refuse; use `allow_nan=False` to get an error |
| JavaScript | `readFileSync(path, "utf8")` on a file that isn't UTF-8 | Each bad byte becomes U+FFFD, with no error, and writing it back destroys it |
| JavaScript | A JSON number above 2^53 | Rounded to the nearest double: 9007199254740993 is 9007199254740992 |
| JavaScript | `JSON.stringify` of a `Map` or a `Set` | `{}`; convert it with `Object.fromEntries` or `[...set]` first |
| JavaScript | Sorting keys, then building an object from them | Keys that look like numbers go first, whatever the sort said |
| JavaScript | `node prog.js \| head -1`, when `prog.js` writes forever | It never stops: the broken pipe gives no signal and no exception |
| TypeScript | Trusting the type of `JSON.parse(text) as Book` | Nothing is checked; take `unknown` and check each field |
| Dart | A file, an argument, or an environment variable that isn't UTF-8 | A `FileSystemException`, a VM that won't start, or a variable that seems not to be set |
| Dart | Rewriting a file through a new one | `dart:io` can't copy the permissions, so a private file becomes readable by others |
| Kotlin | `File.readText()` on a file that isn't UTF-8 | Each bad byte becomes U+FFFD, with no error |
| Kotlin | Printing `é` with `LANG` unset or `C` | The JVM writes `?`; set `-Dstdout.encoding=UTF-8`, as this chapter's Gradle builds do |
| Kotlin | `@Serializable` with a default value | The property is left out of the JSON when it has its default, unless `encodeDefaults = true` |
| Kotlin | A JSON number in quotes, `"1965"`, for an `Int` | kotlinx reads it as 1965, with no setting to refuse it |
| Kotlin | Clikt's `main(args)` on a bad command line | Status 1, not 2; catch `CliktError` and choose |
| All | Writing a file in place | A crash leaves half a file; write a new one and rename it |
| All | Error messages on standard output | They become data for the next program in the pipeline, and `> file` hides them |

---

## 21.11 Idiom check

**C: `getopt_long`, `strerror`, and checked returns.** Read options with `getopt_long`,
return 2 for a usage error, and print each failure as `prog: name: strerror(errno)`.
Check every call that can fail, `fclose` included, and replace files with `mkstemp`,
`fsync`, and `rename`. For JSON, use a library, and check every node's type before using
its value.

**C++: `std::filesystem` and a JSON library.** Use `std::filesystem` for paths, sizes,
renames, and folders, with its `std::error_code` overloads where an error is expected.
Use CLI11 or cxxopts for options in a real program, and nlohmann/json, with
`ordered_json` when the order of keys matters and an explicit check of each number's
type.

**Rust: clap, serde, and `io::Result`.** Derive the command line with clap, and the data
with serde, and return `io::Result` or `Result<_, Box<dyn Error>>` from functions that
touch files, so that `?` passes errors up. Use `args_os` and `PathBuf` where names may
not be UTF-8, `writeln!` on a locked standard output for many lines, and return
`ExitCode` from `main`.

**Go: `flag`, `os`, and `encoding/json` with structs.** Use `flag`, and accept that its
options come first. Read with `os.ReadFile`, or `bufio.Reader` for long lines, and wrap
errors with `fmt.Errorf("...: %w", err)`. Decode into structs, with pointers or a check
afterwards for required fields and `DisallowUnknownFields` where strictness matters, and
write through an `Encoder`, with `SetEscapeHTML(false)` for anything that isn't going
into a web page.

**Python: argparse, `pathlib`, and explicit encodings.** Use argparse, and
`sys.exit(main())`. Open text with `encoding="utf-8"`, and bytes with `"rb"`, and use
`pathlib.Path` for paths. Decode JSON into dataclasses with `match`, or with a library
such as pydantic for bigger schemas, and write it with `ensure_ascii=False`.

**JavaScript: `node:fs`, `parseArgs`, and checked JSON.** Use the synchronous functions
in a command-line tool and the promise ones, `node:fs/promises`, in a server. Read with
`parseArgs`, set `process.exitCode` rather than calling `process.exit`, and check every
value from `JSON.parse` before trusting it.

**TypeScript: `unknown`, and a schema.** Treat `JSON.parse`'s result as `unknown`, and
narrow it with checks, or declare the shape once with a schema library such as zod and
get both the check and the type from it.

**Dart: `package:args`, `dart:io`, and `fromJson`.** Use `package:args`, set `exitCode`,
and catch `FileSystemException`. Give each data class a `fromJson` constructor, checked
with patterns, and a `toJson` method, or generate both with `json_serializable`.

**Kotlin: Clikt, `java.nio.file`, and `@Serializable`.** Use Clikt for options,
`java.nio.file.Files` for reading, writing, and atomic moves, and kotlinx.serialization
with `@Serializable` data classes, with `ignoreUnknownKeys` and `encodeDefaults` set to
what the format needs. Read bytes, not `readText`, when a file may not be UTF-8.

---

## 21.12 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Search files](exercises/01-search-files/) | Options by hand, bytes rather than text, the system's error messages, and exit statuses, in a small `grep` |
| 2 | [Compare JSON files](exercises/02-compare-json/) | Walking a JSON tree, comparing values by kind, and writing JSON exactly |
| 3 | [A to-do list](exercises/03-todo-list/) | Decoding JSON strictly into types, writing it back, and replacing a file safely |

```sh
python3 tools/check.py 21                     # your code, every language
python3 tools/check.py 21 --lang go,python    # some languages
python3 tools/check.py 21 --solutions         # the solutions, and the examples
```

Each test gives the program its arguments and its files, in an empty folder of its own,
and checks what it prints on standard output, its exit status, and for some tests, what
it prints on standard error, and the files it leaves behind, which must be exactly the
ones the test expects. The starters print nothing and exit with 0, so they fail at once.
The Rust, Kotlin, C++, and C starters for exercises 21.2 and 21.3 come with build files
that download serde_json, kotlinx.serialization, nlohmann/json, and cJSON, and exercise
21.1 needs no library at all.

**Stretch:** give `dedup` a `-n` option, which prints what it would remove without
changing any file, and a `-` argument, which reads standard input and writes standard
output, in two of the languages: one whose library reads the options, and one where you
read them yourself. Then pipe a file of a million lines through each into `head -1`, and
make both stop quietly.

---

Previous: [Chapter 20: Debugging and profiling](../20-debugging-and-profiling/) ·
Next: [Chapter 22: Text parsing](../22-text-parsing/)
