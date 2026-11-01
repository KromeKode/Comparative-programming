# Exercise 10.1: Files that always close

Your program keeps a pretend disk of text files in memory. Each file is a list of
lines, and each line is one word. Every line of input is a command:

| Command | Effect |
| --- | --- |
| `write NAME WORD...` | Open NAME for writing, write each word as a line, close it, and print `wrote N lines`. There may be no words at all. |
| `cat NAME` | Open NAME for reading, print each line with two spaces in front, and close it. |
| `copy FROM TO` | Open FROM for reading, open TO for writing, copy FROM's lines into TO one at a time, close both, and print `copied N lines`. |
| `lock NAME` | Make NAME read-only and print `locked NAME`. This opens nothing. |

Write `1 line` for one line and `N lines` for any other count. Opening a file prints `open NAME`
and closing it prints `close NAME`. Files close in the reverse of the order they
opened, and a command's own message (`wrote`, `copied`, or an error) comes after the
closes. When the input ends, print `files still open: N`.

The disk has two limits, as a real one does:

- **At most 2 files open at once.** Opening a third fails with `error: too many open
  files`. A correct program never prints this: `copy` needs only two. If you see it,
  a file that should have closed did not.
- **At most 8 lines on the disk, in all files together.** Writing a line that would
  make 9 fails with `error: disk full`. The lines written before it stay.

Opening a file for writing creates it if it does not exist and empties it if it does,
so its old lines no longer count toward the 8. When a command fails, close every file
it opened, print the error, and carry on with the next command.

| Problem | Error |
| --- | --- |
| Not one of the four commands | `error: unknown command "zap"` |
| `write` with no name | `error: write needs a file name` |
| Too few or too many names | `error: cat takes 1 name, got 2` or `error: copy takes 2 names, got 1` |
| `copy` with FROM and TO the same | `error: cannot copy "a.txt" onto itself` |
| Reading or locking a file that does not exist | `error: no file "ghost.txt"` |
| Writing to a locked file | `error: "rules.txt" is read-only` |
| A ninth line on the disk | `error: disk full` |
| A third open file | `error: too many open files` |

A failed open prints nothing and opens nothing. When opening, check the limit of 2
first, then whether the file is locked or missing. `copy` checks for a self-copy before
it opens anything. Skip blank lines, and allow any number of spaces between the parts
of a line. The tests use ASCII only, with at most 20 file names, and names and words
under 32 characters.

| Input | Output |
| --- | --- |
| `write notes.txt draft` | `open notes.txt` |
| | `close notes.txt` |
| | `wrote 1 line` |
| `write rules.txt be kind` | `open rules.txt` |
| | `close rules.txt` |
| | `wrote 2 lines` |
| `lock rules.txt` | `locked rules.txt` |
| `copy notes.txt rules.txt` | `open notes.txt` |
| | `close notes.txt` |
| | `error: "rules.txt" is read-only` |
| | `files still open: 0` |

```sh
python3 tools/check.py 10 --lang c
```

**Things to notice:**

- **Put open and close in one place, and let the language call close.** Make a handle
  type whose open prints `open` and whose close prints `close`, then use your language's
  tool for "run this however the block ends": a destructor in C++, `Drop` in Rust,
  `defer` in Go, a `with` block in Python, `try`/`finally` in JavaScript, TypeScript,
  and Dart, and `use` in Kotlin. Each of them closes in the reverse order for free.
- **C has to do it by hand.** Write every function that opens two files with one exit
  at the bottom: a `close_target:` label that closes the second file, then a
  `close_source:` label that closes the first. A failure jumps to the label for what is
  open so far.
- **In Rust, a handle that closes the file on drop must reach the disk.** If it holds
  `&mut Disk`, two handles cannot be open at once. Hold `&Disk` instead, and keep the
  parts that change in a `Cell` and a `RefCell`. The method that opens a file then
  returns `Handle<'_>`, which tells the reader that the handle borrows the disk.
- **A leak shows up far from the bug.** Forget one close on an error path and nothing
  goes wrong there. Two commands later, `copy` fails with `too many open files`. Test
  7 is built to show that.
- **Copy one line at a time.** When the disk fills up partway through a `copy`, the
  lines already copied stay in TO, and both files still have to close.

**Stretch:** add `append NAME WORD...`, which opens NAME for writing without emptying
it. Then add `move FROM TO`, which copies and then deletes FROM. What should happen
to TO if the disk fills up halfway through a move?
