# Exercise 18.2: Build order

A build tool has to build each module after the modules it imports, and when one module
changes, rebuild it and everything that depends on it. Given the modules of a program
and what each one imports, print the order to build them in, and what to rebuild after a
change.

Each line of input is either a module, written `NAME: IMPORTS`, with the names it
imports separated by spaces, or a question, `rebuild NAME`. For example:

```text
app: ui net
ui: core
net: core log
core:
log:
rebuild core
rebuild log
rebuild app
```

prints:

```text
build order: core, log, net, ui, app
rebuild core: core, net, ui, app
rebuild log: log, net, app
rebuild app: app
```

The rules are:

- **Names.** A name is one or more of the letters A to Z in either case, the digits,
  `_`, `.`, `/`, and `-`, so `lib/str_utils` and `v2.0-beta` are names. Spaces and tabs
  around the names and the colon don't matter, and a module that imports the same name
  twice imports it once. Blank lines are skipped, but they still count as lines.
- **Build order.** A module is ready to build once every module it imports is built. Of
  the ready modules, build the first in alphabetical order, comparing character codes,
  so capitals come first. Then look again, and keep going until every module is built.
  This is Kahn's algorithm, with the ready modules kept in order. Print them all on one
  line after `build order: `, separated by commas, or `build order: nothing to build` if
  there are no modules.
- **Rebuilds.** After the build order, answer each `rebuild` question in the order they
  were asked: the module itself, then every module that imports it, directly or through
  other modules, all in build order. A question about a module that isn't defined prints
  `error: line N: no module called NAME`, and the program goes on to the next question.
- **Order of lines.** A module can import a module defined on a later line, and a
  question can ask about one too, since the answers come after all the input is read.

Some problems stop the program after one line of output, with nothing printed before it.
The program reads all the lines first, stopping at the first line that is wrong:

| Problem | Output |
| --- | --- |
| A line with a colon whose module name, or one of whose imports, isn't a name | `error: line N: a module is NAME: IMPORTS` |
| A module defined a second time | `error: line N: NAME is already defined on line M` |
| A line without a colon that isn't blank, and isn't `rebuild` and one name | `error: line N: expected NAME: IMPORTS or rebuild NAME` |

Then it checks each module's imports, taking the modules in the order they were defined
and each one's imports in the order they were written:

| Problem | Output |
| --- | --- |
| A module imports a name that no line defines | `error: line N: NAME imports OTHER, which isn't defined`, where N is the line that defines NAME |

And last, the build itself:

| Problem | Output |
| --- | --- |
| Kahn's algorithm stops before every module is built | `error: import cycle: a -> b -> c -> a` |

When the build stops, every module left over imports at least one other module left
over, so there is a cycle among them. To find the one to print, start at the left-over
module that comes first in alphabetical order, and keep going to the first of its
imports, in the order they were written, that is also left over. Sooner or later that
comes back to a module already on the path, and the cycle is the path from that module
on, with the module again at the end. A module that imports itself is the cycle
`a -> a`.

```sh
python3 tools/check.py 18 --lang rust
```

**Things to notice:**

- **A heap for the ready modules.** Taking the smallest of a changing set of names is
  the job of a priority queue. C++ has `std::priority_queue`, which gives the largest
  first unless it is given `std::greater<>`, Rust has `BinaryHeap`, which needs
  `Reverse` for the same reason, and Kotlin uses Java's `PriorityQueue`, which gives the
  smallest first. Python has `heapq`, a set of functions that keep a plain list in heap
  order, and Go has `container/heap`, which works on any type with five methods that the
  program writes itself. JavaScript has no heap at all, so its solution looks through
  the ready modules for the first, as the C solution does. Dart's `SplayTreeSet` is a
  sorted set, whose `first` is the smallest.
- **Remembering the order of definition.** The undefined-import check goes through the
  modules in the order they were defined. Python's dicts, JavaScript's `Map`, Dart's map
  literals, and Kotlin's `mutableMapOf` keep that order on their own. C++'s `std::map`
  and Rust's `BTreeMap` keep keys sorted, and Go's maps have no order, so those
  solutions keep a list of names as well.
- **One pass for a rebuild.** Every module that imports a changed module comes after it
  in the build order. So walking the build order once, and marking each module that
  imports one already marked, finds them all in the order to print, without a search
  through the graph.
- **What real tools do.** `make` compares the times files were last changed: a target
  older than any of its inputs is built again. Cargo does too, so `touch src/lib.rs`
  rebuilds the crate and everything that depends on it. Go and Gradle record a hash of
  each input's contents instead, so a file saved without changes, or a branch switched
  away and back, doesn't start a rebuild: after `touch`, `go build` compiles nothing,
  and Gradle reports its compile tasks `UP-TO-DATE`. Cargo and Go build modules that
  don't depend on each other at the same time, on different cores, and `make -j` and
  `gradle --parallel` do when asked.

**Stretch:** a build tool with several cores can build every ready module at once. Print
the build in rounds instead: round 1 is every module with no imports, round 2 every
module whose imports were all in round 1, and so on. How many rounds does the sample
take, and how many cores would it need to finish in that many?
