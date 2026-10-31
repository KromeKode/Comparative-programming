# Exercise 8.2: Optional config values

Read a small configuration file from stdin and print the three settings it controls. A
setting that the file never sets is different from one set to its default value, and
the output shows which is which.

Each line of the file is one of these:

| Line | Meaning |
| --- | --- |
| Empty, or only spaces and tabs | Nothing; skip it |
| Starts with `#`, after any spaces | A comment; skip it |
| `key = value` | A setting. Split at the first `=`, then trim spaces and tabs from the key and from the value |
| Anything else, that is, a line with no `=` | Print `line N: expected key = value` |

Line numbers count every line from 1, blank lines and comments included. There are
three settings:

| Key | Allowed values | Printed when not set |
| --- | --- | --- |
| `host` | Any text | `host: localhost (default)` |
| `port` | Digits only, from 1 to 65535 | `port: 8080 (default)` |
| `timeout` | Digits only, from 1 to 86400 (seconds) | `timeout: none` |

- A key that is not one of these three prints `line N: unknown key "colour"`. Keys are
  compared exactly, so `Port` is unknown too.
- A value that is not allowed prints `line N: bad port "abc"` (or `bad timeout`) and
  leaves the setting as it was.
- An empty value, as in `host =`, clears the setting, so it is not set any more.
- When a key appears more than once, the last line with an allowed value wins.
- A `#` after a value is part of the value, so `port = 80 # web` is a bad port.

Print the messages as you read the lines. Then print the three settings, in the order
`host`, `port`, `timeout`. A setting that was set prints its value and nothing else,
even when it equals the default, so `port = 8080` prints `port: 8080`. A timeout prints
with an `s` after it: `timeout: 30s`.

Input:

```text
# web server
host = example.com
port = 80.5
timeout = 30
```

Output:

```text
line 3: bad port "80.5"
host: example.com
port: 8080 (default)
timeout: 30s
```

```sh
python3 tools/check.py 08 --lang go
```

**Things to notice:**

- **Keep "not set" apart from the default.** If `port` started out as 8080, the program
  could not tell `port = 8080` from a file that never mentions the port, and those
  print differently. Store `Option<u32>`, `std::optional<int>`, `int?`, `Int?`,
  `number | null`, or `int | None`, and turn "not set" into the default only when you
  print, with `unwrap_or`, `value_or`, `??`, `?:`, or `is None`.
- **A sentinel works only while no real value can equal it.** C and Go have no
  optional type, so their solutions use 0 to mean "not set", because 0 is not an
  allowed port or timeout, and an empty string for the host. If `timeout = 0` were
  allowed to mean "never time out", that trick would break.
- **Number parsers accept more than digits.** `+80` passes Rust's `parse`, Python's
  `int`, Go's `Atoi`, JavaScript's `Number`, Dart's `int.tryParse`, Kotlin's
  `toIntOrNull`, and C's `strtol`; only C++'s `from_chars` refuses it. Python's `int`
  also accepts `8_0`, and Dart's `int.tryParse` accepts `0x50`. Check that every
  character is a digit first, and then parse.
- **Some numbers are too big.** `99999999999999999999` makes Rust's `parse`, Go's
  `Atoi`, Kotlin's `toIntOrNull`, Dart's `int.tryParse`, and C++'s `from_chars` fail,
  which is the right answer here. Python's `int` has no limit, and JavaScript's
  `Number` gives `100000000000000000000`, so those two reject it with the range check.

**Stretch:** add a fourth setting, `debug`, which takes `true` or `false` and prints
`debug: false (default)` when it is not set. Does your type for "not set" still work
when the value itself is a `bool`?
