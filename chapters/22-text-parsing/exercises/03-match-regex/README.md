# Exercise 22.3: Match a regex

Write `rematch`, which prints each line of stdin that has a match for a regular
expression, as `grep` does, with a matcher of your own. Don't hand the pattern to your
language's regular expressions: on three of the tests, most of them take far longer than
the tests allow, and two of them crash.

```text
$ rematch 'colou?r' < notes.txt
The colour of the sky
The color of the sea
A coloured pencil
$ rematch 'a)b' < notes.txt
rematch: column 2: unmatched ")"
```

**The syntax** is a small part of what the libraries take. A pattern is characters, not
bytes, and each of these is an atom:

- A character that isn't one of `\ . [ ( ) * + ? | ^ $`, which matches itself. So `]`,
  `{`, `}`, and `-` are ordinary characters outside brackets.
- `.`, which matches any one character.
- Brackets, `[` and `]` around characters, which match any one of them, or, with a `^`
  straight after the `[`, any one character that isn't one of them. A character, a `-`,
  and another character are a range, which takes in every character from the first to
  the second, by code point. A `-` that can't make a range, as in `[-a]` and `[a-]`, is
  a `-`. Inside brackets, only `\`, `]`, `-`, and a `^` straight after the `[` mean
  anything, and `.`, `*`, `(`, `[`, `|`, `$`, and the rest are ordinary characters.
- `(` and `)` around a pattern, which match what the pattern does.
- `\` and one of `\ . [ ] ( ) * + ? | ^ $ -`, which matches that character, inside
  brackets or out.

An atom can have one `*` (any number of times, none included), `+` (once or more), or
`?` (once or not at all) after it. Atoms one after another match one after another, and
`|` between two patterns matches what either does. It binds less tightly than anything
else, so `^ab|cd$` is `(^ab)|(cd$)`. Either side of a `|` can be empty, and so can the
whole pattern, which matches every line. `^` matches at the start of the line and `$` at
its end, wherever they are in the pattern, so `(^| )cat` finds `cat` at the start or
after a space, and `a^b` matches nothing.

**Lines.** A line ends at `\n`, and a `\r` before it is an ordinary character, so `b$`
doesn't match `ab\r`. The last line may have no `\n` after it. A line with a match is
printed as it was, with a `\n` after it. `rematch` exits with 0 if it printed a line,
and with 1 if it didn't.

**Errors.** With no pattern, or more than one, `rematch` prints `usage: rematch PATTERN`
on stderr. With a pattern it can't read, it prints `rematch: column C: MESSAGE` on
stderr, where columns count characters from 1, and prints nothing on stdout. Either way
it exits with 2. It reads the pattern from left to right and stops at the first error it
finds, so in `(a[b` that's the `[`, at column 3: the brackets come to the end of the
pattern before the group does.

| Where | Message |
| --- | --- |
| At a `*`, `+`, or `?` at the start, or after `(`, `\|`, `^`, `$`, or another `*`, `+`, or `?` | `nothing to repeat` |
| At a `(` with no `)` after it | `unclosed "("` |
| At a `)` with no `(` before it | `unmatched ")"` |
| At a `[` with no `]` after it | `unclosed "["` |
| At the `[` of `[]` or `[^]` | `empty brackets` |
| At the first character of a range such as `z-a` | `range out of order` |
| At a `\` before a character that isn't one of the fourteen | `invalid escape` |
| At a `\` at the very end | `trailing backslash` |

So `(?:a)`, `a*?`, `\d`, and `\n` are errors, and so is `[]a]`, which in POSIX is a set
of two characters.

**The matcher.** The easy matcher to write tries one way through the pattern and, when
that fails, goes back and tries the next. That's backtracking, and it's how Python's,
JavaScript's, Dart's, and Java's work. On `(a*)*b` and a line of a's with no `b`, it
tries every way of sharing the a's out among the turns of the outer `*` before it gives
up, and with each a there are twice as many. Tests 26 to 28 are made of patterns like
that, on lines long enough that backtracking takes minutes, or years.

Instead, turn the pattern into a tree, as the parser in Example 3 does, and the tree
into a program for a machine with a few instructions: read a character if it's in a set,
go to two places at once, jump, check for the start or the end of the line, and match.
Then run every path through the program together, one character at a time: before each
character, gather the instructions that read a character and that the paths so far can
reach without reading one, and keep those that take this character. A match can start
anywhere, so the program's first instruction joins in at every position. That's Ken
Thompson's construction, and it reads each character once, so a line takes time in
proportion to its length times the pattern's.

The tests give each program its input on stdin, and the pattern as its argument, and
check stdout, stderr, and the exit status:

```sh
python3 tools/check.py 22 --lang python
```

**Things to notice:**

- **The libraries blow up.** Python's `re` takes 8 seconds for `(a*)*b` on 26 a's, and
  four times that for each two a's more; JavaScript's takes 6 seconds on 28 a's, and
  Dart's 10 on 26. Python's and JavaScript's are still at tests 26 and 27 after two
  minutes, and Python's takes 45 seconds on test 28. Java's, which Kotlin's `Regex` is,
  finds no `b` in 26 a's at once, but takes 10 seconds on test 26's line of 30,000 a's,
  throws `StackOverflowError` on test 27's, and takes 14 seconds on test 28. GCC's
  `std::regex` crashes on tests 26 and 27, and is still at test 28 after a minute. Go's
  `regexp` and Rust's `regex` crate take under two milliseconds for each, because they
  promise time in proportion to the length of the text, as this exercise does, and leave
  out what can't be done that way, such as backreferences.
- **Loops that read nothing.** In `(a*)*` and `()*`, the inner part can match without
  reading a character, so a path can come back around to the same instruction without
  moving on, forever. Keep the instructions a path has reached at this position, and
  don't follow one twice.
- **What a character is.** A `.` is one character, so `^.$` matches `🦊`, which is four
  bytes in UTF-8 and two units in UTF-16. A range compares code points, and in
  JavaScript, Dart, and Kotlin, comparing strings compares UTF-16 units: `🦊`, U+1F98A,
  starts with the unit U+D83E, so it comes before `～`, U+FF5E, and test 25's `[～-🦊]`
  would be `range out of order`. Python compares code points.
- **Kotlin's arguments.** The JVM decodes the command line with the locale's encoding,
  and with `LANG` unset, a pattern of `é🦊` arrives as six U+FFFD characters, `�`. The
  tests pass anyway because `check.py` is Python, which runs with a UTF-8 locale when
  it's given none, and passes that on to what it runs. Outside it, run Kotlin with
  `LANG=C.UTF-8`.
- **Long lines.** Test 29 has a line of 100,001 characters, and Go's `bufio.Scanner`
  stops at 64 KiB with `bufio.Scanner: token too long`. Read all of stdin and split it,
  or give the scanner a bigger buffer.

**Stretch:** add `-o`, which prints only the part of each line that matched, as
`grep -o` does. Take the match that starts first, and of those, the longest, as POSIX
does: each path has to remember where it started. Then add counted repeats, `a{2,4}`,
which a compiler can write out as `aaa?a?`.
