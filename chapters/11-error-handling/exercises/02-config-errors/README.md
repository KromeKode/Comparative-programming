# Exercise 11.2: A config loader with typed errors

The input holds one or more config files. A line that starts with `== ` begins a new
file, and the rest of that line is the file's name. Load each file, and print either
its settings or every problem in it. At the end, print how many files loaded and how
many problems of each kind there were.

Inside a file, line numbers count from 1 at the line after the `==` line, blank lines
and comments included. A line that is empty or only spaces is skipped, and so is one
that starts with `#` after any spaces. Every other line must be `key = value`: split
it at the first `=` and trim spaces from both parts. There are four keys:

| Key | Allowed values | Required | Default |
| --- | --- | --- | --- |
| `name` | Any text that is not empty | yes | |
| `port` | Digits only, from 1 to 65535 | yes | |
| `debug` | `true` or `false` | no | `false` |
| `timeout` | Digits only, from 1 to 3600 | no | `30` |

There are five kinds of problem:

| Kind | Example |
| --- | --- |
| syntax | `line 2: expected "key = value"`, for a line with no `=` or nothing before it |
| unknown key | `line 3: unknown key "colour"`. Keys are case-sensitive, so `Port` is unknown |
| duplicate key | `line 6: "port" is already set on line 2` |
| bad value | `line 4: port: "80x" is not a whole number`, `line 4: port: 70000 is not between 1 and 65535`, `line 5: debug: "yes" is not true or false`, or `line 1: name: must not be empty` |
| missing key | `missing key "name"` |

The first line that names a key sets it, even when its value is bad, so a later line
with the same key is a duplicate and its value is not checked, and a key with a bad
value is not also missing. A number that is out of range is printed as it was
written, so `00000` stays `00000`. List the problems in line order, then the missing
keys, `name` before `port`.

For a file with no problems, print `NAME: ok` and the four settings, with numbers
written without leading zeros. Otherwise print `NAME: 1 error` or `NAME: N errors` and
each problem, indented by two spaces. Then print the summary.

Input:

```text
== app.conf
name = shop
port = 8080
== broken.conf
port = 80x
colour = blue
port = 81
```

Output:

```text
app.conf: ok
  name = shop
  port = 8080
  debug = false
  timeout = 30
broken.conf: 4 errors
  line 1: port: "80x" is not a whole number
  line 2: unknown key "colour"
  line 3: "port" is already set on line 1
  missing key "name"
files loaded: 1 of 2
syntax: 0, unknown key: 1, duplicate key: 1, bad value: 1, missing key: 1
```

Write `load` as a function that takes a file's lines and returns either the settings
or a list of errors, and give the errors a type with one case per kind. The summary
must count the errors by their type, never by looking at their message text.

```sh
python3 tools/check.py 11 --lang kotlin
```

**Things to notice:**

- **These errors are values, not exceptions.** `load` keeps going after a problem, so
  it collects the problems in a list and returns it. Rust returns
  `Result<Config, Vec<ConfigError>>`, C++ a `std::variant` of the config and a vector
  of error variants, Dart and Kotlin a sealed `Loaded` or `Failed` result, and
  TypeScript a tagged union. Exceptions still fit inside: several solutions have the
  value parsers throw, and `load` catches each one and adds it to the list.
- **Some languages can bundle several errors into one.** Go's `errors.Join` returns a
  single `error` that holds them all, and its `Unwrap() []error` method hands them
  back. Python raises an `ExceptionGroup`, and JavaScript throws an `AggregateError`,
  whose `exceptions` or `errors` field is the list. The reference solutions use all
  three.
- **One error can carry another.** A bad value has two parts: where it is (the line and
  the key) and what is wrong with it (the value parser's own error). Keep the parser's
  error inside the bad value error, as Go's `BadValueError` does with an `Unwrap`
  method, instead of flattening both into one string too early.
- **Let the compiler find every place a new kind must go.** Rust's `match`, Kotlin's
  `when`, Dart's `switch`, TypeScript's `switch` in a function that must return a
  string, C++'s `std::visit`, and C's `switch` under `-Wall -Werror` all refuse to build
  when they leave out a kind. Go's type switch does not check, and neither do the
  `isinstance` and `instanceof` tests of Python and JavaScript.

**Stretch:** add a second mode that stops at the first problem in each file. Which mode
is more useful to the person fixing the file, and which is easier to write?
