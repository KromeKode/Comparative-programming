# Exercise 21.1: Search files

Write `search`, a small `grep`: it prints the lines that contain a pattern, from each
file named on the command line, or from stdin.

```text
usage: search [-icnv] PATTERN [FILE...]
```

**Arguments.** The options come first, then the pattern, then the files.

- `-i` ignores case: `A` to `Z` match `a` to `z`. Every other byte must match exactly.
- `-c` prints how many lines were selected, instead of the lines.
- `-n` puts each line's number, from 1, and a colon before it. With `-c`, it does
  nothing.
- `-v` selects the lines that don't contain the pattern.

Options can be grouped, so `-in` is `-i -n`. An argument of `--` ends the options, so
that a pattern can start with `-`. The options end at the first argument that isn't one,
which is the pattern: `search cat -n notes.txt` searches a file named `-n`. An argument
of `-` alone isn't an option: as a pattern it is the text `-`, and as a file it is
stdin. The pattern is plain text, not a regular expression, and an empty pattern is in
every line. With no files, `search` reads stdin.

**Lines.** A file is bytes, which may not be UTF-8. A line ends at each `\n`, and a last
line without one is a line too. Print each selected line as it is, byte for byte, with a
`\n` after it.

**Output.** With one file, `search` prints the lines alone. With more than one, each
output line starts with the file's name and a colon, and stdin's name is
`(standard input)`. `-n` puts the number after the name:

```text
$ search -n cat poem.txt notes.txt
poem.txt:2:on little cat feet.
notes.txt:1:Ada: bring the cat
```

`-c` prints a count for each file, with the name before it when there is more than one
file:

```text
$ search -c cat poem.txt notes.txt
poem.txt:1
notes.txt:1
```

**Problems.** A file that can't be read gets a message on stderr, and `search` goes on
to the next file. The command line is checked before any file is read.

| Problem | stderr |
| --- | --- |
| A file that isn't there | `search: NAME: No such file or directory` |
| A folder | `search: NAME: Is a directory` |
| Any other file that can't be read | `search: NAME: ` and the system's message |
| An option that isn't one of the four | `search: unknown option -x`, then the usage line |
| No pattern | `search: no pattern`, then the usage line |

**Exit status.** `search` exits with 0 if it selected a line, 1 if it selected none, and
2 if anything went wrong, even if it selected lines too. These are `grep`'s, so a shell
script can write `if search -c TODO notes.txt > /dev/null; then`.

The tests give each program its arguments and files, and check stdout, stderr, and the
exit status. Each test's stdout and exit status are what GNU grep gives, with `-F` for
plain text, when `POSIXLY_CORRECT` is set and the locale is `C`, except for test 14's:
`-x` is one of grep's options.

```sh
python3 tools/check.py 21 --lang go
```

**Things to notice:**

- **Bytes, not text.** Test 16 has a file in Latin-1, where `é` is the one byte `0xe9`,
  which isn't UTF-8. Reading it as text fails in some languages and quietly changes it
  in others. Python's `open(name)` raises `UnicodeDecodeError` when it reads that byte,
  Rust's `read_to_string` returns an error, and Dart's `readAsStringSync` throws one.
  Node's `readFileSync(name, "utf8")` and Kotlin's `readText` turn the byte into
  `U+FFFD`, `�`, with no error. Go, C, and C++ don't decode at all. Read bytes, search
  bytes, and write bytes.
- **The system's messages.** C has `strerror(errno)`, which gives the messages in the
  table, and Python's `OSError` has them as `strerror`. The others wrap the system's
  error code in a type of their own, with a message of their own: Go's starts with a
  small letter, Rust's ends with `(os error 2)`, Node's is
  `ENOENT: no such file or directory, open 'missing.txt'`, and Java's
  `NoSuchFileException` has only the path. Each needs a few lines that turn the error
  back into the system's words.
- **Where the options end.** GNU's `getopt`, and so GNU grep, looks for options after
  the pattern too, which is how `grep cat -n notes.txt` works. POSIX says that options
  end at the first argument that isn't one, and that is what `search` does. Python's
  `argparse`, Rust's clap, and Node's `parseArgs` follow GNU; Go's `flag` follows POSIX.
- **Order of output.** When stdout and stderr both go to a terminal, a line written to
  each comes out in the order they were written only if stdout was flushed in between.
  Go's `os.Stdout` has no buffer, and Rust's and Java's, which Kotlin uses, flush at
  least at each newline. C's and Python's flush at each newline on a terminal, and only
  when the buffer is full in a pipe. A buffer of your own, which the solutions use for
  speed, needs flushing before an error.

**Stretch:** add `-r`, which searches each folder named on the command line, and every
file in it and in the folders below it, in order of path. A listing of a folder comes in
no particular order, so the paths need sorting.
