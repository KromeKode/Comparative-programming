# Exercise 18.1: A stock module

Write a module called `stock` that keeps count of how many of each item a shop has. The
main program is already written: it reads commands, checks them, and prints the answers,
but it leaves the counting to your module. The module's counts must be private, so that
the main program can only reach them through the functions the module gives it.

Each line of input is a command. For example:

```text
add apples 5
add pears 2
take apples 3
take pears 4
count apples
count plums
list
sell apples 1
```

prints:

```text
added 5 apples, now 5
added 2 pears, now 2
took 3 apples, now 2
error: line 4: only 2 pears in stock
apples: 2
plums: 0
stock: apples 2, pears 2
error: line 8: expected add, take, count, or list
```

The main program does all of this already, so you don't need to change it. What it needs
from your module is:

- **Adding.** Add an amount of an item, which may be one it has never seen.
- **Taking.** Take an amount of an item away if there are that many, and say whether
  there were. If there aren't, nothing changes.
- **Counting.** How many of an item there are, which is 0 for an item it has never seen.
- **Listing.** The items there are any of, with their counts, in order of their names,
  comparing character codes, so capitals come before lower case and digits before both.
  An item whose count has gone back to 0 isn't listed.

These are the names and types that each main program uses:

| Language | Files to write | The module |
| --- | --- | --- |
| C | `stock.h`, `stock.c` | `typedef struct Stock Stock;` with `Stock *stock_new(void)`, `void stock_free(Stock *)`, `bool stock_add(Stock *, const char *item, long long amount)`, which returns false only if there is no memory, `bool stock_take(Stock *, const char *item, long long amount)`, `long long stock_count(const Stock *, const char *item)`, and `void stock_each(const Stock *, void (*visit)(const char *item, long long count, void *context), void *context)`, which calls `visit` for each item in order |
| C++ | `stock.hpp`, `stock.cpp` | A class `stock::Stock` with `void add(const std::string& item, long long amount)`, `bool take(const std::string& item, long long amount)`, `long long count(const std::string& item) const`, and `std::vector<std::pair<std::string, long long>> items() const` |
| Rust | `stock.rs` | A struct `Stock` with `Stock::new()`, `add(&mut self, item: &str, amount: u64)`, `take(&mut self, item: &str, amount: u64) -> bool`, `count(&self, item: &str) -> u64`, and `items(&self) -> Vec<(&str, u64)>` |
| Go | `stock/stock.go` | A package `stock` with `New() *Stock`, the methods `Add(item string, amount int64)`, `Take(item string, amount int64) bool`, `Count(item string) int64`, and `Items() []Item`, and `type Item struct { Name string; Count int64 }` |
| Python | `stock.py` | A class `Stock` with `add(item: str, amount: int) -> None`, `take(item: str, amount: int) -> bool`, `count(item: str) -> int`, and `items() -> list[tuple[str, int]]` |
| JavaScript and TypeScript | `stock.js` or `stock.ts` | An exported class `Stock` with `add(item, amount)`, `take(item, amount)`, which returns a boolean, `count(item)`, and `items()`, which returns `[name, count]` pairs |
| Dart | `stock.dart` | A class `Stock` with `void add(String item, int amount)`, `bool take(String item, int amount)`, `int count(String item)`, and `List<(String, int)> items()` |
| Kotlin | `stock.kt` | A package `stock` with a class `Stock` that has `add(item: String, amount: Long)`, `take(item: String, amount: Long): Boolean`, `count(item: String): Long`, and `items(): List<Pair<String, Long>>` |

The starter's module files hold only a comment, so the starter doesn't build until you
write them. The checker builds every source file in the language's folder together, and
follows the imports.

Keep the counts private in the strongest way the language has: a struct that only
`stock.c` defines in C, a `private` member in C++, a field without `pub` in Rust, a
field that starts with a lower-case letter in Go, a field that starts with `_` in Python
and Dart, a `#` field in JavaScript and TypeScript, and a `private` property in Kotlin.
A main program that tried to read the counts directly shouldn't build, or in Python,
should be plainly breaking the module's rules.

The tests use ASCII names, lines under 1,000 characters, and amounts that keep every
count below 10^15.

```sh
python3 tools/check.py 18 --lang c
```

**Things to notice:**

- **How private is private?** In C, `main.c` sees only `typedef struct Stock Stock;`, a
  type without its insides, so it can't read a field, or even know how big a `Stock` is:
  it can't make one except through `stock_new`, which is why there is a `stock_free`. In
  C++, the private member is in the header for every file to see, and the compiler stops
  any use of it outside the class, but `main.cpp` still has to be compiled again
  whenever it changes. Rust, Go, Kotlin, and Dart stop it when they compile, and
  JavaScript as soon as it loads the file, which is a syntax error. TypeScript's `#` is
  stopped by `tsc` and by JavaScript too. Python's `_counts` is only an agreement:
  `stock._counts` works, mypy says nothing, and Ruff reports it only if its `SLF` rules
  are on.
- **Sorted or not.** C++'s `std::map`, Rust's `BTreeMap`, and Kotlin's `sortedMapOf`
  keep their keys in order, so `items` only has to skip the zeros. Go's maps have no
  order at all, and Python's, JavaScript's, and Dart's keep the order of insertion, so
  those `items` sort. The C solution keeps a sorted array and finds names with a binary
  search.
- **Handing the items out.** C has no list type to return, so `stock_each` calls back to
  the main program for each item, with a `void *context` that the callback can use for
  its own state. The others return a new list of pairs, which the caller can't use to
  change the stock.
- **Go's `New`.** A Go struct can be made without calling any function, and the zero
  `Stock{}` would have a nil map, which panics when written to. That's why the package
  has a `New`, and why the field is unexported: other packages can't make a `Stock` with
  a map of their own.

**Stretch:** change how the C module stores its counts, from the sorted array to a hash
table, without touching `main.c`. Then do the same in C++, moving the map out of the
header with the "pointer to implementation" idiom: a `private` member that is a
`std::unique_ptr` to a struct that only `stock.cpp` defines. Why would a big project
care that `main.cpp` no longer has to be compiled again when the stock's insides change?
