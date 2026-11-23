# Exercise 22.2: Check JSON

Write `jsoncheck`, which reads one JSON text on stdin, checks it strictly, and prints it
again in one compact form, or says exactly where it went wrong. Write the parser
yourself: your language's JSON library, if it has one, is lenient in ways this exercise
isn't, and it doesn't keep numbers as they were written.

```text
$ jsoncheck < book.json
{"title":"Dune","year":1965,"tags":["science fiction","desert"],"rating":4.5,"series":null}
$ jsoncheck < broken.json
jsoncheck: line 3, column 14: expected a value, found ","
```

**The grammar** is RFC 8259's. A JSON text is one value, with whitespace before and
after it, and whitespace is space, tab, `\n`, and `\r`, and nothing else. A value is one
of these:

- An object, `{` and `}` around members separated by commas, where a member is a string,
  a colon, and a value.
- An array, `[` and `]` around values separated by commas.
- A string, `"` and `"` around characters, with no characters below U+0020 in it. A `\`
  starts an escape: `\"`, `\\`, `\/`, `\b`, `\f`, `\n`, `\r`, `\t`, or `\u` and four hex
  digits, in either case.
- A number: an optional `-`, then `0` or a digit from `1` to `9` and any digits after
  it, then optionally `.` and one or more digits, then optionally `e` or `E`, an
  optional `+` or `-`, and one or more digits. The digits are ASCII's.
- `true`, `false`, or `null`, in small letters.

So there are no trailing commas, no comments, no single quotes, no `NaN`, and no `+1`,
`.5`, `1.`, or `01`.

**Three more rules.** RFC 8259 leaves these to the parser, and `jsoncheck` makes each
one an error:

- **Duplicate keys.** No object may have the same key twice, after escapes are undone,
  so `{"é": 1, "\u00e9": 2}` is an error. Two objects may each have the same key.
- **Unpaired surrogates.** A character above U+FFFF is written as two `\u` escapes, a
  high surrogate from `\ud800` to `\udbff` and then a low one from `\udc00` to `\udfff`,
  so `"\ud83e\udd8a"` is `"🦊"`. A high surrogate without a low one straight after it is
  an error, and so is a low one on its own.
- **Depth.** Arrays and objects can be nested 1000 deep, and no deeper: the outermost
  `[` or `{` is at depth 1.

**Output.** A valid text is printed on one line, with a `\n` after it, and no whitespace
in it. Each object's keys are in the order they came. Each number is written exactly as
it was, so `1E+2` stays `1E+2` and `1e400` stays `1e400`. Each string is written again
from its characters: `"` and `\` are `\"` and `\\`; backspace, form feed, newline,
carriage return, and tab are `\b`, `\f`, `\n`, `\r`, and `\t`; the other characters
below U+0020 are `\u` and four hex digits, in lowercase; and every other character is
written as it is, in UTF-8. So `"\/"` becomes `"/"`, and `"\u00e9"` becomes `"é"`.

**Errors.** At the first error, `jsoncheck` prints
`jsoncheck: line L, column C: MESSAGE` on stderr, prints nothing on stdout, and exits
with 1. Lines end at `\n`, and columns count characters, not bytes, from 1. The end of
the input is just after its last character. Where a message has `X`, it is the character
found there, written as a JSON string as above, as in `"]"`, `"\""`, or `"\u0001"`, or
it is `the end of the input`.

| Where | Message |
| --- | --- |
| Where a value should start | `expected a value, found X` |
| After `{` | `expected a string key or "}", found X` |
| After `,` in an object | `expected a string key, found X` |
| After a key | `expected ":", found X` |
| After a value in an array | `expected "," or "]", found X` |
| After a value in an object | `expected "," or "}", found X` |
| After the whole value | `expected the end of the input, found X` |
| Where a number needs a digit | `expected a digit, found X` |
| At the `"` of a string with no end | `string is never closed` |
| At a character below U+0020 in a string | `control character in a string` |
| At the `\` of an escape that isn't one of the above | `invalid escape` |
| At the `\` of an unpaired surrogate | `unpaired surrogate` |
| At the `"` of a key that the object has already | `duplicate key K`, with `K` written as a JSON string |
| At the 1001st `[` or `{` | `nesting deeper than 1000` |

A word that isn't `true`, `false`, or `null`, such as `tru` or `True`, is
`expected a value`, at its first letter. A number ends where the grammar says it does,
so in `[01]` the number is `0`, and the `1` after it is the error. A string with a `\`
at the very end of the input is `string is never closed`.

The input is UTF-8. A byte order mark at its start is a character, U+FEFF, which isn't
whitespace, so it is an error too.

The tests give each program its input on stdin, and check stdout, stderr, and the exit
status:

```sh
python3 tools/check.py 22 --lang python
```

**Things to notice:**

- **The libraries let more through.** Python's `json.loads` takes `NaN` and `-Infinity`,
  a lone `"\ud83e"`, and `{"a": 1, "a": 2}`, which it reads as `{"a": 2}`. JavaScript's
  `JSON.parse` turns down `NaN`, but takes the other two, and so does Go's
  `encoding/json`, which turns the lone surrogate into U+FFFD, `�`, without a word. Each
  library chose a reading for what RFC 8259 leaves open.
- **Recursion has a limit.** A parser with a function for each kind of value calls
  itself once for each level of nesting, or twice: `value` calls `array`, which calls
  `value`. Python stops at a depth of 1000 calls unless told otherwise with
  `sys.setrecursionlimit`, so a parser written that way fails at about 500 levels, and
  test 8 is 1000 deep. Python's own `json.loads` manages 1000, but not 10000, and Node's
  `JSON.parse` manages a million.
- **What a column counts.** Test 39 has `🦊` and `é` before the error. In Python, a `str`
  is code points, and an index counts them. JavaScript, Dart, and Kotlin strings are
  UTF-16, where `🦊` is two units, so they need `[...text]`, `runes`, or `codePoints()`.
  Rust's `chars()` and Go's `[]rune` are code points too. In C and C++ the text is
  bytes, and a character is a byte that isn't `10xxxxxx` and the bytes of that form
  after it.
- **A byte order mark.** Dart's `utf8.decode` drops a byte order mark at the start of
  its input, so test 44, which has one, would pass by accident: the solution has to
  check the bytes itself. Python's `"utf-8"`, Node's `readFileSync(0, "utf8")`, and
  Kotlin's `decodeToString` keep it.

**Stretch:** make the parser handle any depth, without recursion: keep a stack of the
arrays and objects that are open, with what each expects next. Then add `--pretty`,
which prints the value with two spaces of indent for each level, as
`JSON.stringify(value, null, 2)` does.
