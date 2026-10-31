# Exercise 12.2: Plugins

Your program passes words through a pipeline of plugins. Each line of input is one
command.

| Command | What it does |
| --- | --- |
| `list` | Prints every plugin as `USAGE: HELP`, in the order below |
| `add NAME ARGS` | Makes a plugin and adds it to the end of the pipeline |
| `pipeline` | Prints the description of each plugin in the pipeline |
| `run WORDS` | Passes the words through each plugin in turn, and prints the result |

There are four plugins:

| Usage | Help | Description |
| --- | --- | --- |
| `upper` | `make every letter uppercase` | `upper` |
| `lower` | `make every letter lowercase` | `lower` |
| `reverse` | `reverse the order of the words` | `reverse` |
| `censor WORD` | `hide WORD wherever it appears` | `censor(WORD)` |

`censor` replaces each word that matches WORD with as many `*` as the word has letters.
It matches whole words only, and ignores case, so `censor bad` hides `bad` and `BAD` but
not `badly`.

| Input | Output |
| --- | --- |
| `add reverse` | `added reverse` |
| `add censor bad` | `added censor(bad)` |
| `add upper please` | `error: upper takes no arguments` |
| `pipeline` | `pipeline: reverse \| censor(bad)` |
| `run this is not bad` | `*** not is this` |

With nothing in the pipeline, `pipeline` prints `pipeline: (empty)` and `run` prints the
words as they are. The other errors are `error: no plugin named NAME` and
`error: censor needs exactly one word`. A plugin that fails to be made is not added.
`run` prints the words separated by single spaces. Every line is a well-formed command.
Skip blank lines, and allow any number of spaces between words. The tests use ASCII
only, with lines under 1,000 characters.

Give the plugins a shared interface with `name`, `apply`, and `describe`, where
`describe` has a default that returns the name, so only `censor` writes its own. Keep
the plugins in a registry that maps each name to a way to make one, and have the main
loop find plugins only through the registry.

```sh
python3 tools/check.py 12 --lang go
```

**Things to notice:**

- **Where the default lives.** A Rust trait, a Kotlin interface, and the base classes in
  Python, C++, and Dart can give `describe` a body. In C++ that means `virtual` but not
  `= 0`. A Go interface cannot hold code, so each plugin embeds a `named` struct that
  supplies `Name` and `Describe`, and `Censor` writes its own `Describe`, which takes
  the place of the embedded one. A TypeScript interface cannot hold code either, so
  `describe?()` is optional, and the default is at the call:
  `plugin.describe?.() ?? plugin.name`. JavaScript does the same. In C, a `NULL` in the
  table of functions means "use the name".
- **In Dart, `implements` brings no code.** The Dart plugins say `extends Plugin`. A
  class that says `implements Plugin` only promises to have Plugin's members, so it must
  write `describe` too: `Missing concrete implementation of 'Plugin.describe'.`
- **A plugin with no settings needs only one value.** Kotlin makes `Upper` an `object`,
  a class with exactly one value. JavaScript and TypeScript use object literals, and Go
  uses struct values. Their registries hand out the same plugin every time, which is
  safe only because it holds no state. A `censor` plugin holds its word, so every
  `add censor` makes a new one. In JavaScript and TypeScript, a function makes it, and
  the object keeps the word in a closure.
- **One function for every plugin with no arguments.** Rust's `no_arguments::<Upper>`
  and C++'s `no_arguments<Upper>` are generic, and the compiler makes a separate copy
  for each type (chapter 13). Python passes the class itself, since calling a class
  makes an object, and Dart passes the constructor as `Upper.new`.
- **`list` keeps the order the plugins were registered in.** Python's `dict`,
  JavaScript's `Map`, Dart's map, and Kotlin's `mutableMapOf` all keep the order in
  which keys were added. A Go map has no order, so the Go solution keeps a slice, and
  the C, C++, and Rust solutions keep a list too.

**Stretch:** add a `limit N` plugin that keeps only the first N words, with the error
`limit needs a whole number`. How much of your program had to change?
