# Exercise 15.3: A macro expander

Write a small version of C's preprocessor. Each line of input is a directive, which
defines or removes a macro and prints nothing, or a line of text, which is printed with
its macros expanded.

| Line | What it does |
| --- | --- |
| `#define NAME body` | Defines a macro that is replaced by its body |
| `#define NAME(a, b) body` | Defines a macro that takes arguments, when the `(` comes right after the name |
| `#undef NAME` | Removes the macro, if there is one |
| Anything else | Prints the line, with its macros expanded |

For example:

```text
#define LIMIT 100
#define SQUARE(x) x * x
#define SHOW(e) #e = e
LIMIT and SQUARE(1 + 2)
SHOW(SQUARE(LIMIT))
```

prints:

```text
100 and 1 + 2 * 1 + 2
"SQUARE(LIMIT)" = 100 * 100
```

The rules are:

- **Words.** A word is a run of letters, digits, and underscores, and a name is a word
  that doesn't start with a digit. Only whole words are replaced, so `LIMITS`, `_LIMIT`,
  and `2LIMIT` are left alone. Text in double quotes is copied as it is, up to the next
  quote or the end of the line, and has no escapes.
- **Directives.** The first word of a line, after removing spaces from both ends,
  decides what the line is. A body is everything after the name or the `)`, with spaces
  removed from both ends, and can be empty. Defining a name again replaces its macro.
  `#define PAIR (x) x` has a space before the `(`, so it defines `PAIR` as `(x) x`.
- **Calls.** A macro that takes arguments is replaced only when the next character after
  its name that isn't a space is `(`. Otherwise its name is left alone. The arguments
  run up to the matching `)`, and are split at each comma that isn't inside parentheses
  or quotes, with spaces removed from both ends. `NOW()` and `NOW( )` have no arguments.
- **Parameters.** In a copy of the body, each parameter is replaced by its argument,
  with the argument's macros expanded first. A `#` followed by a parameter, with or
  without spaces between, is replaced by the argument as it was written, in double
  quotes. A `#` that isn't followed by a parameter stays, and so does a parameter inside
  quotes.
- **Rescanning.** The result is expanded again, which is how one macro uses another.
  Inside a macro's expansion, its own name isn't expanded again, even through other
  macros. So `#define LOOP LOOP + 1` turns `LOOP` into `LOOP + 1`, and with `A` defined
  as `B` and `B` as `A`, `A` turns into `B` and back into `A`, and stops.

A line with a problem prints one line instead, and the lines after it carry on, with
every macro defined so far.

| Problem | Output |
| --- | --- |
| The wrong number of arguments | `error: SQUARE takes 1 argument, got 2` |
| A call with no `)` | `error: unterminated call to SQUARE` |
| A `#define` with no name, no `)` after its parameters, or a parameter that isn't a name or is used twice | `error: bad #define` |
| A `#undef` of something that isn't a name | `error: bad #undef` |

The tests use lines under 1000 characters, fewer than 100 macros, and fewer than 10
parameters for each.

```sh
python3 tools/check.py 15 --lang rust
```

**Things to notice:**

- **An argument is expanded only if it is used.** In `QUOTE(SQUARE(1, 2))`, with
  `#define QUOTE(x) #x`, the only use of `x` is with `#`, so the argument is never
  expanded, and the line prints `"SQUARE(1, 2)"` instead of an error. C's preprocessor
  does the same. The solutions expand each argument the first time its parameter appears
  in the body without `#`, and keep the result for its other uses.
- **Where this differs from C.** This expander rescans each expansion by itself. C's
  rescan carries on into the rest of the line, so with `#define DOUBLE(x) ((x) + (x))`
  and `#define CALL DOUBLE`, `cc -E` turns `CALL(7)` into `((7) + (7))`, and this
  expander prints `DOUBLE(7)`. C also has `##`, `__VA_ARGS__`, and `#if`, and squeezes
  the spaces between words, so it prints `a  b` as `a b`.
- **The set of macros being expanded.** Each expansion needs the names of the macros it
  is inside, plus its own. Python writes `disabled | {word}`, Kotlin `disabled + word`,
  Dart `{...disabled, word}`, and JavaScript and TypeScript
  `new Set([...disabled, word])`, and each makes a new set. C++ copies a `std::set` and
  adds to the copy, and Go copies a map with `maps.Copy`. C uses a linked list whose
  links are on the stack: each call that expands a macro makes a link that points to the
  list it was given, and the link goes away when the call returns. Rust's set holds
  names borrowed from the keys of the map of macros, so the lifetime `'m` in
  `HashSet<&'m str>` says the set can't outlive the borrow of the expander.
- **Errors from deep inside.** An error can be found many calls deep, and has to reach
  the loop over the lines. Python, C++, JavaScript, TypeScript, Dart, and Kotlin throw
  an exception. Rust returns a `Result`, and `?` passes an error up. Go returns an
  `error` that each caller checks. C returns `false` and leaves the message in a global,
  `error`.
- **Indexes into text.** Every solution walks the text by index. Rust's indexes count
  bytes, and slicing a `str` in the middle of a character such as `é` panics with
  `end byte index 4 is not a char boundary`, so its `char_end` steps over a whole
  character. C, C++, and Go copy bytes one at a time, which puts the bytes of a
  character back together in the output. JavaScript, TypeScript, Dart, and Kotlin count
  UTF-16 code units, and Python counts code points.

**Stretch:** add `##`, which pastes the words on either side of it into one, after the
parameters are replaced, so that `#define CAT(a, b) a ## b` turns `CAT(x, 1)` into `x1`.
Then make the rescan carry on into the rest of the line, as C's does, so that `CALL(7)`
above prints `((7) + (7))`. Which of the tests change?
