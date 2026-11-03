# Exercise 14.2: Lazy pipelines

Each line of input is a pipeline: a source of whole numbers, then stages separated by
`|`. Print the numbers that come out of the last stage, separated by spaces, or
`(empty)` if none do.

| Source | Numbers |
| --- | --- |
| `naturals` | 1, 2, 3, and so on forever |
| `primes` | 2, 3, 5, 7, and so on forever |
| `fibonacci` | 0, 1, 1, 2, 3, 5, and so on forever, each the sum of the two before it |
| `range A B` | A to B, both included, or nothing if A is greater than B |
| `repeat N` | N, forever |

| Stage | What comes out |
| --- | --- |
| `map F` | F of each number, where F is `square`, `negate`, `add N`, or `mul N` |
| `filter P` | The numbers for which P is true |
| `takewhile P` | The numbers up to the first one for which P is false, and then nothing more |
| `dropwhile P` | Nothing until the first number for which P is false, and then every number from it on |
| `take N` | The first N numbers |
| `skip N` | Every number after the first N |
| `step N` | Every Nth number, starting with the first: numbers 0, N, 2N, and so on |
| `sum`, `count` | One number, the sum or the count of what arrives. Either can only be the last stage |

A test P is `even`, `odd`, `prime`, `above N`, or `below N`. Odd means not even, so -3
is odd, and no number below 2 is prime. `above N` is true for numbers greater than N,
and `below N` for numbers less than N.

| Input | Output |
| --- | --- |
| `naturals \| map square \| filter even \| take 3` | `4 16 36` |
| `range 1 10 \| step 3` | `1 4 7 10` |
| `fibonacci \| takewhile below 100` | `0 1 1 2 3 5 8 13 21 34 55 89` |
| `primes \| dropwhile below 100 \| take 3` | `101 103 107` |
| `primes \| takewhile below 1000 \| count` | `168` |
| `range 5 1 \| sum` | `0` |
| `range 1 5 \| take 0` | `(empty)` |

Every N is a whole number, and N is at least 1 for `step` and at least 0 for `take` and
`skip`. Skip blank lines, and allow any number of spaces and tabs around words and `|`.
The tests use lines under 256 characters, and every number a pipeline makes stays below
2^53 in size.

Build each line's pipeline out of lazy stages, one per stage, before any number moves
through it, and then pull the numbers out of the end. Nothing may run ahead of what the
end asks for. `primes | filter below 10 | take 4` must print `2 3 5 7` and stop: there
is no fifth prime below 10, so a `take` that asks for one more number than it needs runs
forever. For the same reason, `naturals | filter below 1 | take 0` must print `(empty)`
without asking `filter` for anything.

```sh
python3 tools/check.py 14 --lang rust
```

**Things to notice:**

- **One type for any pipeline.** In Rust, each adapter has a type of its own, and a
  pipeline's type spells out every stage, such as `Take<Filter<RangeFrom<i64>, ...>>`.
  Here the stages come from the input, so their types aren't known when the program is
  compiled, and the Rust solution boxes each stage as a `Box<dyn Iterator<Item = i64>>`,
  chapter 13's trait objects. C++ views have the same problem, and the standard library
  has no type that holds any view, so the C++ solution makes each stage a
  `std::function` that hands out the next number as a `std::optional<long long>`. Go's
  stages are all `iter.Seq[int64]`, and in Python, JavaScript, TypeScript, Dart, and
  Kotlin every stage is an iterator, an iterable, or a sequence already.
- **Who writes the stages.** Rust has every stage built in, including `step_by`, and
  Python has them all between the built-in `map` and `filter` and `itertools`'
  `takewhile`, `dropwhile`, and `islice`, which takes, skips, and steps. Kotlin's
  sequences and Dart's iterables have all but `step`, which Kotlin writes with
  `filterIndexed` and Dart with `indexed`. JavaScript's iterator helpers have `map`,
  `filter`, `take`, and `drop`, so the solution writes `takeWhile`, `dropWhile`, and
  `step` as generators. Go's `iter` package has no stages at all, and C has nothing to
  start from.
- **`take` checks its count before it pulls.** Rust's `take`, Python's `islice`,
  JavaScript's `take`, Kotlin's `take`, and Dart's `take` all stop after the Nth number
  without asking for another, and so do the hand-written ones in Go, C++, and C. The
  endless sources depend on it.
- **Reading a pipeline twice.** A Python generator and a JavaScript iterator are used up
  once they have been read, so summing one a second time gives 0. Rust's `sum` takes the
  iterator by value, so a second `sum` doesn't compile:
  `error[E0382]: use of moved value`. A Dart `Iterable` starts again from the source
  each time it is read, so asking a Dart pipeline `isEmpty` and then joining it starts
  the pipeline twice, and the Dart solution joins it once and checks the text. Kotlin's
  sequences start again too, except one made to be read only once, such as
  `generateSequence { n }`, which throws `This sequence can be consumed only once` the
  second time.

**Stretch:** add a `scan` stage, which prints the running totals, so `range 1 5 | scan`
prints `1 3 6 10 15`. Rust's `scan`, Kotlin's `runningReduce`, and Python's
`itertools.accumulate` do this. How many lines does it take in the languages that have
no such stage?
