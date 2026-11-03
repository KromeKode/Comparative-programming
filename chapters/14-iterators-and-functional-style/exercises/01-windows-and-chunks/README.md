# Exercise 14.1: Windows and chunks

Each line of input is a command followed by whole numbers. Print the groups that the
command makes from the numbers, each group in square brackets, with a space between
groups.

| Command | Groups |
| --- | --- |
| `windows K` | Every K numbers in a row, moving along one number at a time |
| `chunks K` | The numbers K at a time, so the last chunk can be shorter |
| `runs` | Each stretch of equal numbers that sit next to each other |
| `sums K` | The sum of each window of K numbers, printed without brackets |

| Input | Output |
| --- | --- |
| `windows 3 1 2 3 4 5` | `[1 2 3] [2 3 4] [3 4 5]` |
| `chunks 2 1 2 3 4 5` | `[1 2] [3 4] [5]` |
| `runs 1 1 2 2 2 1` | `[1 1] [2 2 2] [1]` |
| `sums 3 1 2 3 4 5` | `6 9 12` |
| `windows 4 1 2 3` | `(none)` |
| `chunks 3` | `(none)` |

Print `(none)` when a command makes no groups: a window needs K numbers, and no command
makes a group from no numbers. K is at least 1, and `runs` has no K. Skip blank lines,
and allow any number of spaces between words. The tests use lines under 256 characters,
and numbers and sums below 2^53 in size, so JavaScript's numbers hold them exactly.

Write `windows`, `chunks`, and `runs` as adapters. Each one takes any iterator of
numbers, in whatever form your language uses, and gives back an iterator of groups. It
pulls the numbers one at a time, as it needs them, and keeps only what it must: K
numbers for a window or a chunk, and the current run. `sums` is then `windows` followed
by a sum of each window. Don't use your library's own windows or chunks, which are
listed below.

```sh
python3 tools/check.py 14 --lang rust
```

**Things to notice:**

- **Four ways to write an adapter.** Python, JavaScript, and TypeScript write each
  adapter as a generator, Dart as a `sync*` function, and Kotlin with the `sequence { }`
  builder: a loop over the input that calls `yield`, which the language turns into an
  iterator. Go returns an `iter.Seq[[]int64]`, a function that loops over the input and
  calls `yield` itself. Rust writes a struct for each adapter and implements
  `Iterator`'s one required method, `next`. The C++ and C solutions also write a class
  or struct that holds its place in the input, with a `next` that hands out the next
  group.
- **Adapters that chain.** Rust's solution has a trait, `Groups`, whose methods have
  default bodies, and `impl<I: Iterator<Item = i64>> Groups for I {}` gives them to
  every iterator of numbers, so `numbers.windows(3)` reads like a standard adapter.
  Dart's `extension Groups on Iterable<int>` and Kotlin's extension functions on
  `Sequence<Long>` do the same. The other solutions call their adapters as functions, as
  in `windows(numbers, k)`.
- **A window is handed out as a copy.** The adapter keeps changing its buffer as numbers
  arrive, so each window it hands out is a new list. Rust can't avoid the copy: an
  iterator's items must be able to outlive the next call to `next`, and lending out its
  own buffer is refused, with the note
  `you can't create an 'Iterator' that borrows each 'Item' from itself`. The slice
  method `windows` can hand out borrowed slices because they borrow the slice, not the
  iterator. The C solution does hand out a pointer into its buffer, and its comment says
  that the group is good only until the next call.
- **Runs need to look ahead.** A run ends when a different number arrives, and that
  number starts the next run. Rust's `Peekable` has `next_if_eq`, which takes the next
  item only if it equals the one given. A C++ iterator is a position, so `*it_` reads
  the next number without moving past it. The C source is a plain struct, so a copy of
  it reads ahead, and the real one moves on only when the number joins the run. The
  other solutions keep the run they are building and hand it out when a different number
  turns up, or at the end.
- **The library versions.** Rust's slices have `windows`, `chunks`, and `chunk_by`,
  which makes runs. Kotlin has `windowed` and `chunked` for lists and sequences. Python
  has `itertools.batched` (3.12 and later) for chunks, `itertools.pairwise` for windows
  of two, and `itertools.groupby` for runs. Go 1.23 added `slices.Chunk`, and C++23 adds
  `std::views::slide`, `std::views::chunk`, and `std::views::chunk_by`. Dart's core
  library has none of them, though `package:collection` has `slices` for chunks, and
  JavaScript has none.

**Stretch:** give `windows` a step, so that `windows 3 2 1 2 3 4 5 6 7` prints
`[1 2 3] [3 4 5] [5 6 7]`, as Kotlin's `windowed(3, 2)` does. With a step of K, windows
are almost chunks. What is the difference, and what does Kotlin's
`partialWindows = true` change?
