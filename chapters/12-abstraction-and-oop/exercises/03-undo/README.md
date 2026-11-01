# Exercise 12.3: Undo and redo

Your program edits a document: a list of lines, which starts out empty. Each line of
input is one command.

| Command | What it does |
| --- | --- |
| `add TEXT` | Adds TEXT as the new last line |
| `delete N` | Deletes line N |
| `replace N TEXT` | Replaces line N with TEXT |
| `undo` | Undoes the newest edit that has not been undone |
| `redo` | Does again the newest edit that was undone |
| `show` | Prints each line as `N: TEXT`, or `(empty)` if there are none |

Lines are numbered from 1. TEXT is the rest of the line, with its words joined by single
spaces. An edit prints nothing when it works. `undo` and `redo` print what they did, as
`undo: ` or `redo: ` followed by `add "TEXT"`, `delete line N`, or
`replace line N with "TEXT"`.

| Input | Output |
| --- | --- |
| `add first line` | |
| `add second line` | |
| `replace 1 new first line` | |
| `delete 5` | `error: no line 5` |
| `show` | `1: new first line` |
| | `2: second line` |
| `undo` | `undo: replace line 1 with "new first line"` |
| `undo` | `undo: add "second line"` |
| `show` | `1: first line` |
| `redo` | `redo: add "second line"` |

An edit that fails changes nothing and cannot be undone. A new edit that works clears
what could have been redone. The other errors are `error: nothing to undo` and
`error: nothing to redo`. Every line is a well-formed command, and N is a whole number.
Skip blank lines, and allow any number of spaces between words. The tests use ASCII
only, with lines under 256 characters.

Make each edit an object, a command, that can apply itself and undo itself. Keep two
stacks: the edits that can be undone, and the edits that can be redone. The editor must
never check which kind of edit it is undoing.

```sh
python3 tools/check.py 12 --lang go
```

**Things to notice:**

- **A command remembers what it needs to undo itself.** `Delete` keeps the line it
  removed and `Replace` keeps the line it replaced, and both learn it in `apply`. Rust
  moves the strings out with `std::mem::take` and `std::mem::replace` instead of copying
  them, and C++ uses `std::exchange`. Kotlin's `lines.set` returns the old value.
- **Record an edit only after it works.** The editor calls `apply` first, and pushes the
  command only if `apply` succeeds. With those two steps the other way round, a failed
  `delete 9` would be undone later, and undoing it would change the document. Redo
  cannot fail, because undo has put the document back as it was.
- **An interface with no code.** `Command` shares no code, so it is a TypeScript or Go
  `interface`, a Kotlin `interface`, a Rust trait, a Python `ABC`, a C++ class with only
  `= 0` functions, and a Dart `abstract interface class`, which code in other files can
  implement but not extend. JavaScript needs nothing: any object with `apply`, `undo`,
  and `describe` will do.
- **Who owns a command.** In C++ each stack holds `std::unique_ptr<Command>`, and
  `std::move` passes a command from one stack to the other. Rust moves the
  `Box<dyn Command>` by value. In C each command is `calloc`ed, and freed when its edit
  fails, when the redo stack is cleared, or when the program ends. The languages with a
  garbage collector drop a command when no stack holds it any more.

**Stretch:** add `begin` and `end`, which group the edits between them so that one
`undo` undoes them all. A group can be a command that holds a list of commands.
