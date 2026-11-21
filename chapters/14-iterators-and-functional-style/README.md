# Chapter 14: Iterators and functional style

Chapter 7 looped over arrays, lists, and maps, and chapter 5 passed functions around as
values. This chapter puts the two together. An **iterator** hands out the items of a
collection one at a time, and every language's `for` loop is built on one, so an
iterator for your own type makes the `for` loop work on it too. Chain iterators
together, with a function at each step, and you get a **pipeline**, such as
`scores.filter(passed).map(name)`, which says what to compute and leaves the looping to
the library. That is the **functional style**, from languages such as Lisp and Haskell.
A pipeline can be **eager**, finishing each step before the next one starts, or
**lazy**, pulling one item at a time through every step, and only a lazy one can work on
a source that never ends. **Generators** make iterators easy to write, as functions that
pause at each `yield`. The nine languages agree on the ideas, and differ on who takes
each step, on which pipelines are lazy, and on whether generators exist at all.

**Languages:** all nine · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| What a `for` loop asks for in each language, and how to make your own types work with it | Linked lists, stacks, and queues (chapter 26), trees and heaps (chapter 27) |
| Pipelines: map, filter, fold, and the rest, and what each language calls them | Files, JSON, and the CLI (chapter 21), text parsing (chapter 22), SQL fundamentals (chapter 32) |
| Lazy and eager pipelines, and sources that never end | Concurrency and parallelism (chapter 16), complexity and benchmarking (chapter 24) |
| Generators, and the difference between pull and push iterators | Concurrency and parallelism (chapter 16), graphs (chapter 28) |
| When a loop reads better than a pipeline | Every chapter from here on |

---

## 14.1 The concept: one item at a time

An **iterable** is something you can loop over: a list, a map, a range of numbers, a
tree. An **iterator** is one loop's place in it. It knows how far it has got, and hands
out the next item when asked. One list can have several iterators at once, each at its
own place, and an iterator is used up when it reaches the end. A `for` loop asks the
iterable for a new iterator, then asks the iterator for items until there are none left.

There are two ways for the items to move, and both appear in this chapter:

```text
 Pull: the loop asks for each item            Push: the source hands items to the loop

   loop  --- next? --->  iterator               source  --- yield(3) --->  loop body
   loop  <--- 3 -------  iterator               source  <-- keep going --  loop body
   loop  --- next? --->  iterator               source  --- yield(4) --->  loop body
   loop  <--- done ----  iterator               source  <-- stop --------  loop body
```

- **Pull.** The loop calls a method, such as `next`, each time it wants an item, and the
  iterator keeps its place between calls. C++, Rust, Python, JavaScript, TypeScript,
  Dart, and Kotlin iterators work this way. The caller is in charge: it can stop, wait,
  or pull from two iterators in turn.
- **Push.** The source runs its own loop and calls a function with each item, and the
  function's answer says whether to go on. Go's iterators work this way, and so do the
  visitor functions in this chapter's C and C++. A push iterator is easy to write, since
  it is an ordinary loop, but its caller can only say "stop".

A pipeline is built from a few steps that every language has in some form. **Map**
changes each item, **filter** keeps the items that pass a test, and **fold**, also
called **reduce**, combines all the items into one value, such as a sum. Around them are
`take`, `sort`, `any`, `all`, `max`, and others. Each step takes a function, which is
why the functional style needs the closures of chapter 5.

Order matters as soon as a step does work. Take the squares of 1 to 5, keep the even
ones, and stop at two of them. An **eager** pipeline squares all five numbers, then
tests all five squares, and then takes two. A **lazy** one squares 1, tests 1, squares
2, tests 4, and so on, and stops at the second even square, so 5 is never squared. Lazy
steps do no more work than the end of the pipeline asks for, and they need no list in
between, which is what makes an endless source, such as "all the natural numbers",
usable. Section 14.5 shows both.

A **generator** is a function that hands out items with `yield` and pauses after each
one, with its variables and its place in the code kept, until the next item is asked
for. It turns an ordinary loop, or a recursion, into an iterator. Section 14.6 shows
them, and what the languages without them do instead.

---

## 14.2 What each language gives you

| Language | What a `for` loop asks for | One step | Pipelines | Lazy? | Generators |
| --- | --- | --- | --- | --- | --- |
| C | Nothing: loops count indexes or move pointers | A function of your own, such as `bool next(struct iter *, int *out)` | None; write the loop | Only by putting every step in one loop | None; pass a function to call with each item |
| C++ | `begin()` and `end()` | `*it` reads, `++it` moves, and `it == end` checks | `<algorithm>`, and views in `<ranges>` (C++20) | Views are; algorithms are not | `std::generator` in C++23, not yet in GCC 13 |
| Rust | `IntoIterator` | `next()` returns `Option<T>` | `Iterator` adapters, such as `map` and `filter`, and consumers, such as `sum` and `collect` | Yes | Not yet: `gen` blocks are experimental |
| Go | A slice, map, channel, integer, or iterator function, `iter.Seq` (Go 1.23) | The iterator calls `yield(v)`, which returns `false` to stop; `iter.Pull` gives a `next` | None in the standard library: loops, with the `slices` and `maps` packages | An `iter.Seq` is | An `iter.Seq` is written like one already |
| Python | `__iter__`, which returns an iterator | `__next__`, which raises `StopIteration` at the end | Comprehensions, `map`, `filter`, `sum`, `itertools`, and `functools.reduce` | Iterators and generator expressions are; list comprehensions are not | `yield` and `yield from` |
| JavaScript | `[Symbol.iterator]()` | `next()` returns `{ value, done }` | Array methods; iterator helpers (ES2025) | Iterator helpers are; array methods are not | `function*`, `yield`, and `yield*` |
| TypeScript | The same, typed `Iterable<T>` | The same, typed `Iterator<T>`; built-in iterators are `IteratorObject<T>` | The same | The same | The same, typed `Generator<T>` |
| Dart | An `Iterable`, whose `iterator` getter gives an `Iterator` | `moveNext()` steps and says whether there is an item, and `current` is that item | `Iterable` methods, such as `map`, `where`, and `fold` | Yes, lists included | `sync*`, `yield`, and `yield*` |
| Kotlin | An `operator fun iterator()` | `hasNext()` and `next()` | Collection functions, such as `map`, `filter`, and `sumOf` | On a `Sequence`; list functions are not | `sequence { }`, with `yield` and `yieldAll` |

Two columns explain most of the rest. "One step" says who is in charge: where it is a
`next` method, the loop pulls, and writing an iterator means keeping its place in
fields, while Go's push iterators keep their place in an ordinary loop. "Lazy?" says
what a pipeline costs. Where the usual collection methods are eager, as with
JavaScript's arrays and Kotlin's lists, laziness is a separate type that you switch to,
and where they are lazy, as in Rust and Dart, a pipeline does nothing until something
reads it.

---

## 14.3 The iterator protocol

Each version has a `Ring`, a buffer that keeps the last three items pushed into it,
oldest first. Pushing 1 to 5 leaves 3, 4, and 5, and a `for` loop over the ring visits
them in that order, twice. Then each program steps through the ring by hand, the way the
loop does, and finally takes one item by hand and gives the rest to a loop.

**C**

```c
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// A ring keeps the last CAPACITY items pushed into it, oldest first.
#define CAPACITY 3

struct ring {
    int items[CAPACITY];
    size_t start, len;
};

static void ring_push(struct ring *ring, int item) {
    ring->items[(ring->start + ring->len) % CAPACITY] = item;
    if (ring->len < CAPACITY) {
        ring->len++;
    } else {
        ring->start = (ring->start + 1) % CAPACITY;  // the oldest item is overwritten
    }
}

// C has no iterator protocol, so this is the usual shape: a struct that
// remembers the position, and a function that hands out the next item.
struct ring_iter {
    const struct ring *ring;
    size_t index;
};

static struct ring_iter ring_iter(const struct ring *ring) {
    return (struct ring_iter){ring, 0};
}

// Stores the next item in *out and returns true, or returns false at the end.
static bool ring_next(struct ring_iter *it, int *out) {
    if (it->index == it->ring->len) return false;
    *out = it->ring->items[(it->ring->start + it->index) % CAPACITY];
    it->index++;
    return true;
}

static void print_all(const char *label, struct ring_iter it) {
    printf("%s:", label);
    int item = 0;
    while (ring_next(&it, &item)) printf(" %d", item);
    printf("\n");
}

int main(void) {
    struct ring ring = {0};
    for (int i = 1; i <= 5; i++) ring_push(&ring, i);

    print_all("loop", ring_iter(&ring));
    print_all("again", ring_iter(&ring));  // a new iterator starts at the beginning

    struct ring_iter it = ring_iter(&ring);
    int item = 0;
    printf("by hand:");
    for (const char *sep = " "; ring_next(&it, &item); sep = ", then ") {
        printf("%s%d", sep, item);
    }
    printf(", then nothing\n");

    it = ring_iter(&ring);
    ring_next(&it, &item);
    printf("resumed: took %d, then the loop got", item);
    while (ring_next(&it, &item)) printf(" %d", item);
    printf("\n");
    return 0;
}
```

**C++**

```cpp
#include <array>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>

// A ring keeps the last three items pushed into it, oldest first.
class Ring {
public:
    // A C++ iterator is a position. A range-for needs only *, ++, and ==, but
    // these two type names and post-increment also make it a std::input_iterator.
    class Iterator {
    public:
        using value_type = int;
        using difference_type = std::ptrdiff_t;

        Iterator() = default;
        Iterator(const Ring* ring, std::size_t index) : ring_(ring), index_(index) {}

        int operator*() const {
            return ring_->items_[(ring_->start_ + index_) % ring_->items_.size()];
        }

        Iterator& operator++() {
            index_++;
            return *this;
        }

        void operator++(int) { ++*this; }

        bool operator==(const Iterator& other) const { return index_ == other.index_; }

    private:
        const Ring* ring_ = nullptr;
        std::size_t index_ = 0;
    };

    void push(int item) {
        items_[(start_ + len_) % items_.size()] = item;
        if (len_ < items_.size()) {
            len_++;
        } else {
            start_ = (start_ + 1) % items_.size();  // the oldest item is overwritten
        }
    }

    // begin and end are what a range-for calls.
    Iterator begin() const { return {this, 0}; }
    Iterator end() const { return {this, len_}; }

private:
    std::array<int, 3> items_{};
    std::size_t start_ = 0, len_ = 0;
};

static_assert(std::ranges::input_range<Ring>);  // so std::ranges algorithms accept it

int main() {
    Ring ring;
    for (int i = 1; i <= 5; i++) ring.push(i);

    std::cout << "loop:";
    for (int item : ring) std::cout << " " << item;
    std::cout << "\nagain:";
    for (int item : ring) std::cout << " " << item;  // begin() starts a new position
    std::cout << "\n";

    std::cout << "by hand:";
    const char* sep = " ";
    for (auto it = ring.begin(); it != ring.end(); ++it, sep = ", then ") {
        std::cout << sep << *it;
    }
    std::cout << ", then nothing\n";

    // Moving the position forward by hand, then looping from there.
    auto it = ring.begin();
    std::cout << "resumed: took " << *it << ", then the loop got";
    ++it;
    for (int item : std::ranges::subrange(it, ring.end())) std::cout << " " << item;
    std::cout << "\n";
}
```

**Rust**

```rust
// A ring keeps the last three items pushed into it, oldest first.
struct Ring {
    items: [i32; 3],
    start: usize,
    len: usize,
}

impl Ring {
    fn new() -> Self {
        Ring {
            items: [0; 3],
            start: 0,
            len: 0,
        }
    }

    fn push(&mut self, item: i32) {
        self.items[(self.start + self.len) % 3] = item;
        if self.len < 3 {
            self.len += 1;
        } else {
            self.start = (self.start + 1) % 3; // the oldest item is overwritten
        }
    }

    fn iter(&self) -> RingIter<'_> {
        RingIter {
            ring: self,
            index: 0,
        }
    }
}

// The iterator borrows the ring, so the ring can't change while it is in use.
struct RingIter<'a> {
    ring: &'a Ring,
    index: usize,
}

// One required method: next returns Some(item), or None at the end. Every
// other Iterator method, such as map, filter, and sum, comes with it.
impl Iterator for RingIter<'_> {
    type Item = i32;

    fn next(&mut self) -> Option<i32> {
        if self.index == self.ring.len {
            return None;
        }
        let item = self.ring.items[(self.ring.start + self.index) % 3];
        self.index += 1;
        Some(item)
    }
}

// IntoIterator is what a for loop calls, so this makes `for item in &ring` work.
impl<'a> IntoIterator for &'a Ring {
    type Item = i32;
    type IntoIter = RingIter<'a>;

    fn into_iter(self) -> RingIter<'a> {
        self.iter()
    }
}

fn main() {
    let mut ring = Ring::new();
    for i in 1..=5 {
        ring.push(i);
    }

    print!("loop:");
    for item in &ring {
        print!(" {item}");
    }
    let again: Vec<String> = ring.iter().map(|item| item.to_string()).collect();
    println!("\nagain: {}", again.join(" ")); // each iter() starts at the beginning

    let mut it = ring.iter();
    print!("by hand:");
    let mut sep = " ";
    while let Some(item) = it.next() {
        print!("{sep}{item}");
        sep = ", then ";
    }
    println!(", then nothing");

    let mut it = ring.iter();
    let first = it.next().unwrap();
    print!("resumed: took {first}, then the loop got");
    for item in it {
        print!(" {item}"); // the loop carries on from where next left off
    }
    println!();
}
```

**Go**

```go
package main

import (
	"fmt"
	"iter"
)

// A Ring keeps the last three items pushed into it, oldest first.
type Ring struct {
	items      [3]int
	start, len int
}

func (r *Ring) Push(item int) {
	r.items[(r.start+r.len)%3] = item
	if r.len < 3 {
		r.len++
	} else {
		r.start = (r.start + 1) % 3 // the oldest item is overwritten
	}
}

// All returns a push iterator (Go 1.23): a function that calls yield with
// each item, and stops early if yield returns false. A for loop can range
// over it, and the loop body becomes yield.
func (r *Ring) All() iter.Seq[int] {
	return func(yield func(int) bool) {
		for i := range r.len {
			if !yield(r.items[(r.start+i)%3]) {
				return
			}
		}
	}
}

func main() {
	var ring Ring
	for i := 1; i <= 5; i++ {
		ring.Push(i)
	}

	fmt.Print("loop:")
	for item := range ring.All() {
		fmt.Print(" ", item)
	}
	fmt.Print("\nagain:")
	for item := range ring.All() { // each call to All starts again
		fmt.Print(" ", item)
	}
	fmt.Println()

	// iter.Pull turns a push iterator into next and stop functions.
	next, stop := iter.Pull(ring.All())
	fmt.Print("by hand:")
	sep := " "
	for item, ok := next(); ok; item, ok = next() {
		fmt.Print(sep, item)
		sep = ", then "
	}
	fmt.Println(", then nothing")
	stop()

	// A range loop always starts a push iterator from the beginning, so
	// carrying on after a next means calling next.
	next, stop = iter.Pull(ring.All())
	defer stop()
	first, _ := next()
	fmt.Printf("resumed: took %d, then the loop got", first)
	for item, ok := next(); ok; item, ok = next() {
		fmt.Print(" ", item)
	}
	fmt.Println()
}
```

**Python**

```python
from typing import Self


class Ring:
    """Keeps the last three items pushed into it, oldest first."""

    def __init__(self) -> None:
        self._items = [0, 0, 0]
        self._start = 0
        self._len = 0

    def push(self, item: int) -> None:
        self._items[(self._start + self._len) % 3] = item
        if self._len < 3:
            self._len += 1
        else:
            self._start = (self._start + 1) % 3  # the oldest item is overwritten

    # A for loop calls iter(ring), which calls __iter__ for a new iterator.
    def __iter__(self) -> "RingIterator":
        return RingIterator(self._items, self._start, self._len)


class RingIterator:
    def __init__(self, items: list[int], start: int, length: int) -> None:
        self._items = items
        self._start = start
        self._len = length
        self._index = 0

    # next(it) calls __next__, which raises StopIteration at the end.
    def __next__(self) -> int:
        if self._index == self._len:
            raise StopIteration
        item = self._items[(self._start + self._index) % 3]
        self._index += 1
        return item

    # An iterator is iterable too: it returns itself, so a for loop over it
    # carries on from where it is.
    def __iter__(self) -> Self:
        return self


def main() -> None:
    ring = Ring()
    for i in range(1, 6):
        ring.push(i)

    print("loop:", end="")
    for item in ring:
        print(f" {item}", end="")
    print()
    print("again:", " ".join(str(item) for item in ring))  # a new iterator each time

    it = iter(ring)
    parts = []
    while True:
        try:
            parts.append(str(next(it)))
        except StopIteration:
            break
    print("by hand:", ", then ".join(parts) + ", then nothing")

    it = iter(ring)
    first = next(it)
    print(f"resumed: took {first}, then the loop got", end="")
    for item in it:  # the loop carries on from where next left off
        print(f" {item}", end="")
    print()


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// A ring keeps the last three items pushed into it, oldest first.
class Ring {
  #items = [0, 0, 0];
  #start = 0;
  #len = 0;

  push(item) {
    this.#items[(this.#start + this.#len) % 3] = item;
    if (this.#len < 3) {
      this.#len++;
    } else {
      this.#start = (this.#start + 1) % 3; // the oldest item is overwritten
    }
  }

  // for...of calls the method named Symbol.iterator to get an iterator: an
  // object whose next() returns { value, done }.
  [Symbol.iterator]() {
    const items = this.#items;
    const start = this.#start;
    const len = this.#len;
    let index = 0;
    return {
      next() {
        if (index === len) return { value: undefined, done: true };
        return { value: items[(start + index++) % 3], done: false };
      },
      // An iterator is iterable too: it returns itself, so a loop over it
      // carries on from where it is.
      [Symbol.iterator]() {
        return this;
      },
    };
  }
}

const ring = new Ring();
for (let i = 1; i <= 5; i++) ring.push(i);

let line = "loop:";
for (const item of ring) line += ` ${item}`;
console.log(line);
console.log(`again: ${[...ring].join(" ")}`); // spreading asks for a new iterator too

let it = ring[Symbol.iterator]();
const parts = [];
for (let step = it.next(); !step.done; step = it.next()) parts.push(step.value);
console.log(`by hand: ${parts.join(", then ")}, then nothing`);

it = ring[Symbol.iterator]();
const first = it.next().value;
line = `resumed: took ${first}, then the loop got`;
for (const item of it) line += ` ${item}`; // carries on from where next left off
console.log(line);
```

**TypeScript**

```ts
// A ring keeps the last three items pushed into it, oldest first.
class Ring implements Iterable<number> {
  #items = [0, 0, 0];
  #start = 0;
  #len = 0;

  push(item: number): void {
    this.#items[(this.#start + this.#len) % 3] = item;
    if (this.#len < 3) {
      this.#len++;
    } else {
      this.#start = (this.#start + 1) % 3; // the oldest item is overwritten
    }
  }

  // for...of calls the method named Symbol.iterator to get an iterator: an
  // object whose next() returns { value, done }.
  [Symbol.iterator](): IterableIterator<number> {
    const items = this.#items;
    const start = this.#start;
    const len = this.#len;
    let index = 0;
    return {
      next(): IteratorResult<number> {
        if (index === len) return { value: undefined, done: true };
        return { value: items[(start + index++) % 3], done: false };
      },
      // An iterator is iterable too: it returns itself, so a loop over it
      // carries on from where it is.
      [Symbol.iterator]() {
        return this;
      },
    };
  }
}

const ring = new Ring();
for (let i = 1; i <= 5; i++) ring.push(i);

let line = "loop:";
for (const item of ring) line += ` ${item}`;
console.log(line);
console.log(`again: ${[...ring].join(" ")}`); // spreading asks for a new iterator too

let it = ring[Symbol.iterator]();
const parts: number[] = [];
for (let step = it.next(); !step.done; step = it.next()) parts.push(step.value);
console.log(`by hand: ${parts.join(", then ")}, then nothing`);

it = ring[Symbol.iterator]();
const first = it.next().value;
line = `resumed: took ${first}, then the loop got`;
for (const item of it) line += ` ${item}`; // carries on from where next left off
console.log(line);
```

**Dart**

```dart
// A ring keeps the last three items pushed into it, oldest first. Extending
// Iterable means writing only the iterator getter; for-in, map, where, join,
// and the rest come with it.
class Ring extends Iterable<int> {
  final _items = [0, 0, 0];
  var _start = 0;
  var _len = 0;

  void push(int item) {
    _items[(_start + _len) % 3] = item;
    if (_len < 3) {
      _len++;
    } else {
      _start = (_start + 1) % 3; // the oldest item is overwritten
    }
  }

  @override
  Iterator<int> get iterator => _RingIterator(_items, _start, _len);
}

// A Dart iterator has two members: moveNext() steps forward and says whether
// there is an item, and current is that item.
class _RingIterator implements Iterator<int> {
  final List<int> _items;
  final int _start;
  final int _len;
  var _index = -1;

  _RingIterator(this._items, this._start, this._len);

  @override
  bool moveNext() => ++_index < _len;

  @override
  int get current => _items[(_start + _index) % 3];
}

void main() {
  final ring = Ring();
  for (var i = 1; i <= 5; i++) {
    ring.push(i);
  }

  var line = 'loop:';
  for (final item in ring) {
    line += ' $item';
  }
  print(line);
  print('again: ${ring.join(' ')}'); // join asks for a new iterator too

  var it = ring.iterator;
  final parts = <int>[];
  while (it.moveNext()) {
    parts.add(it.current);
  }
  print('by hand: ${parts.join(', then ')}, then nothing');

  // for-in takes an Iterable, not an Iterator, so carrying on from where an
  // iterator is means calling moveNext.
  it = ring.iterator;
  it.moveNext();
  line = 'resumed: took ${it.current}, then the loop got';
  while (it.moveNext()) {
    line += ' ${it.current}';
  }
  print(line);
}
```

**Kotlin**

```kotlin
// A ring keeps the last three items pushed into it, oldest first.
class Ring : Iterable<Int> {
    private val items = IntArray(3)
    private var start = 0
    private var len = 0

    fun push(item: Int) {
        items[(start + len) % 3] = item
        if (len < 3) {
            len++
        } else {
            start = (start + 1) % 3 // the oldest item is overwritten
        }
    }

    // A for loop calls iterator(). A Kotlin iterator has hasNext() and next().
    override fun iterator(): Iterator<Int> =
        object : Iterator<Int> {
            private var index = 0

            override fun hasNext() = index < len

            override fun next(): Int {
                if (!hasNext()) throw NoSuchElementException()
                return items[(start + index++) % 3]
            }
        }
}

fun main() {
    val ring = Ring()
    for (i in 1..5) ring.push(i)

    print("loop:")
    for (item in ring) print(" $item")
    println("\nagain: ${ring.joinToString(" ")}") // joinToString asks for a new iterator

    var it = ring.iterator()
    val parts = mutableListOf<Int>()
    while (it.hasNext()) parts.add(it.next())
    println("by hand: ${parts.joinToString(", then ")}, then nothing")

    it = ring.iterator()
    print("resumed: took ${it.next()}, then the loop got")
    for (item in it) print(" $item") // a for loop accepts an Iterator too, and carries on
    println()
}
```

All nine print the same lines:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
loop: 3 4 5
again: 3 4 5
by hand: 3, then 4, then 5, then nothing
resumed: took 3, then the loop got 4 5
```

**C: a struct and a next function.** C's loops know nothing about collections, so the
usual shape is a small struct that remembers the place, `struct ring_iter`, and a
function that hands out the next item, `ring_next`. It returns `false` at the end, and
otherwise stores the item through a pointer and returns `true`, which is how C returns
two things. A new iterator starts at the beginning because it is a new struct, and
resuming needs nothing special: the `while` loop carries on with the same struct.
Nothing stops the ring from changing while an iterator is in use.

**C++: an iterator is a position.** A C++ iterator works like a pointer into the
collection: `*it` reads the item at the position, `++it` moves on, and comparing with
`end()` says whether there are more. A range-`for` loop is shorthand for exactly that
loop, with `begin()` and `end()` taken from the ring. The loop itself needs only `*`,
`++`, and `!=`, which C++20 works out from `==`. The two type names, `value_type` and
`difference_type`, and the post-increment, `it++`, make `Ring::Iterator` a
`std::input_iterator`, which the standard algorithms and views ask for, and
`static_assert(std::ranges::input_range<Ring>)` checks that when the program compiles.
Because an iterator is a position, and not a stream, resuming means making a range from
where `it` is to the end: `std::ranges::subrange(it, ring.end())`.

**Rust: one method, and the rest for free.** The `Iterator` trait has one required
method, `next`, which returns `Some(item)`, or `None` at the end, and dozens of provided
ones, such as `map`, `filter`, and `sum`, which all work through `next`. `RingIter<'a>`
borrows the ring, so the borrow checker won't let the ring change while an iterator is
alive (chapter 10). A `for` loop calls `into_iter` on what it is given, from the
`IntoIterator` trait, so `impl IntoIterator for &Ring` makes `for item in &ring` work.
Without one, `for item in ring` stops with `error[E0277]: 'Ring' is not an iterator`.
Every iterator implements `IntoIterator` too, by returning itself, so `for item in it`
carries on from where `next` left off.

**Go: the iterator pushes.** Since Go 1.23, a `for range` loop can run over a function.
`All` returns an `iter.Seq[int]`, which is a `func(yield func(int) bool)`: a function
that calls `yield` with each item and stops when `yield` returns `false`. The loop body
becomes `yield`, and a `break` in the body makes `yield` return `false`. So `All` is a
push iterator: it runs its own loop, and the `for` loop never asks for anything. To step
by hand, `iter.Pull` turns a push iterator into two functions. `next` returns an item
and a `bool`, as a map lookup does, and `stop` "must be called when the caller is no
longer interested in next values", as the documentation puts it, unless `next` has
already reported the end. A `range` loop always starts the function from the beginning,
so carrying on after a `next` means calling `next`.

**Python: `__iter__` and `__next__`.** A `for` loop calls `iter(ring)`, which calls
`ring.__iter__()`, and then calls `next` on what it got until `__next__` raises
`StopIteration`. So there are two classes. `Ring` is iterable, and `RingIterator` is the
iterator. A list is iterable but not an iterator: `next([1, 2])` raises
`TypeError: 'list' object is not an iterator`. An iterator's own `__iter__` returns
itself, so that a `for` loop over it carries on from where it is, and the standard
library's iterators all follow that rule. `StopIteration` is an exception, but the loop
catches it, so no one else sees it. Section 14.6 shows the short way to write
`__iter__`, as a generator.

**JavaScript and TypeScript: `Symbol.iterator` and `{ value, done }`.** `for...of`,
spreading with `...`, and destructuring all call the method named `Symbol.iterator`, a
built-in symbol, so it can't clash with a method of your own. It returns an object with
a `next()` method, and each `next()` returns `{ value, done }`. That object also has a
`Symbol.iterator` method that returns itself, which makes it iterable as well, so
`for (const item of it)` carries on. Without it, the loop throws
`TypeError: it is not iterable`. TypeScript types the parts as `Iterable<number>`,
`IterableIterator<number>`, and `IteratorResult<number>`. An iterator written as a plain
object has `next` and nothing else, and none of the helper methods of section 14.5.
`Iterator.from(it)` wraps it in one that has them, and so does writing it as a class
that `extends Iterator`, or as a generator (section 14.6).

**Dart: `moveNext` and `current`.** A Dart iterator has two members: `moveNext()` steps
forward and returns whether there is an item, and `current` is that item. `Ring` extends
`Iterable<int>` and writes only the `iterator` getter, and the rest of `Iterable`, such
as `map`, `where`, and `join`, comes with it, built on that iterator. A `for-in` loop
takes an `Iterable`, never an `Iterator`, and `for (final x in it)` is refused with
`The type 'Iterator<int>' used in the 'for' loop must implement 'Iterable'`. So resuming
is a `while (it.moveNext())` loop.

**Kotlin: `hasNext` and `next`.** A Kotlin iterator has `hasNext()` and `next()`, and
`next()` throws `NoSuchElementException` at the end. A `for` loop needs only an
`operator fun iterator()`, so a class with that function works in a loop without
implementing anything, but implementing `Iterable<Int>` also brings `joinToString`,
`map`, `filter`, and the rest. The standard library gives every `Iterator` an
`operator fun iterator()` that returns the iterator itself, which is why
`for (item in it)` carries on.

---

## 14.4 Pipelines

Each version answers seven questions about six test scores: who passed, with 70 or more;
the total of the passes; the best score; whether everyone passed; whether anyone scored
above 95; the top three; and which letters the names use.

**C**

```c
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct score {
    const char *name;
    int points;
};

static const struct score scores[] = {{"ada", 91}, {"bob", 78}, {"cy", 85},
                                      {"dee", 62}, {"eve", 99}, {"finn", 54}};
enum { COUNT = sizeof scores / sizeof scores[0] };

static const char *yes_no(bool value) { return value ? "yes" : "no"; }

static int by_points_descending(const void *a, const void *b) {
    const struct score *x = a, *y = b;
    return (y->points > x->points) - (y->points < x->points);
}

// C has no map, filter, or reduce, so each question is a loop.
int main(void) {
    printf("passed:");
    const char *sep = " ";
    int total = 0;
    for (size_t i = 0; i < COUNT; i++) {
        if (scores[i].points < 70) continue;
        printf("%s%s", sep, scores[i].name);
        sep = ", ";
        total += scores[i].points;
    }
    printf("\ntotal of the passes: %d\n", total);

    const struct score *best = &scores[0];
    bool everyone = true, anyone = false;
    for (size_t i = 0; i < COUNT; i++) {
        if (scores[i].points > best->points) best = &scores[i];
        if (scores[i].points < 70) everyone = false;
        if (scores[i].points > 95) anyone = true;
    }
    printf("best: %s with %d\n", best->name, best->points);
    printf("everyone passed: %s\n", yes_no(everyone));
    printf("anyone above 95: %s\n", yes_no(anyone));

    struct score ranked[COUNT];
    memcpy(ranked, scores, sizeof scores);
    qsort(ranked, COUNT, sizeof ranked[0], by_points_descending);
    printf("top three:");
    for (size_t i = 0; i < 3; i++) {
        printf("%s %zu. %s %d", i ? "," : "", i + 1, ranked[i].name, ranked[i].points);
    }

    bool seen[26] = {false};
    for (size_t i = 0; i < COUNT; i++) {
        for (const char *c = scores[i].name; *c; c++) seen[*c - 'a'] = true;
    }
    printf("\nletters used:");
    for (int i = 0; i < 26; i++) {
        if (seen[i]) printf(" %c", 'a' + i);
    }
    printf("\n");
    return 0;
}
```

**C++**

```cpp
#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <ranges>
#include <set>
#include <string>
#include <vector>

struct Score {
    std::string name;
    int points;
};

// C++20 has no join for strings, so this writes one.
std::string join(std::ranges::input_range auto&& items, const std::string& sep) {
    std::string text;
    for (const auto& item : items) text += (text.empty() ? "" : sep) + item;
    return text;
}

const char* yes_no(bool value) { return value ? "yes" : "no"; }

int main() {
    const std::vector<Score> scores{{"ada", 91}, {"bob", 78}, {"cy", 85},
                                    {"dee", 62}, {"eve", 99}, {"finn", 54}};
    auto passed = [](const Score& score) { return score.points >= 70; };

    // Views are lazy: nothing is filtered until join loops over the result.
    std::cout << "passed: "
              << join(scores | std::views::filter(passed) | std::views::transform(&Score::name),
                      ", ")
              << "\n";

    auto passes = scores | std::views::filter(passed) | std::views::transform(&Score::points);
    std::cout << "total of the passes: " << std::accumulate(passes.begin(), passes.end(), 0)
              << "\n";

    // &Score::points is a projection: compare scores by their points.
    const Score& best = std::ranges::max(scores, {}, &Score::points);
    std::cout << "best: " << best.name << " with " << best.points << "\n";
    std::cout << "everyone passed: " << yes_no(std::ranges::all_of(scores, passed)) << "\n";
    std::cout << "anyone above 95: "
              << yes_no(std::ranges::any_of(scores, [](int p) { return p > 95; }, &Score::points))
              << "\n";

    std::vector<Score> ranked = scores;
    std::ranges::sort(ranked, std::greater{}, &Score::points);
    std::vector<std::string> top;
    for (const Score& score : ranked | std::views::take(3)) {
        top.push_back(std::to_string(top.size() + 1) + ". " + score.name + " " +
                      std::to_string(score.points));
    }
    std::cout << "top three: " << join(top, ", ") << "\n";

    std::set<char> letters;
    for (char letter : scores | std::views::transform(&Score::name) | std::views::join) {
        letters.insert(letter);
    }
    auto as_text = [](char letter) { return std::string(1, letter); };
    std::cout << "letters used: " << join(letters | std::views::transform(as_text), " ") << "\n";
}
```

**Rust**

```rust
use std::cmp::Reverse;
use std::collections::BTreeSet;

fn yes_no(value: bool) -> &'static str {
    if value { "yes" } else { "no" }
}

fn main() {
    let scores = [
        ("ada", 91),
        ("bob", 78),
        ("cy", 85),
        ("dee", 62),
        ("eve", 99),
        ("finn", 54),
    ];

    // iter() lends the items, filter keeps some, map changes each one, and
    // collect gathers the result. Nothing runs until collect asks.
    let passed: Vec<&str> = scores
        .iter()
        .filter(|&&(_, points)| points >= 70)
        .map(|&(name, _)| name)
        .collect();
    println!("passed: {}", passed.join(", "));

    let total: i32 = scores
        .iter()
        .map(|&(_, points)| points)
        .filter(|&p| p >= 70)
        .sum();
    println!("total of the passes: {total}");

    let &(name, points) = scores.iter().max_by_key(|&&(_, points)| points).unwrap();
    println!("best: {name} with {points}");
    println!(
        "everyone passed: {}",
        yes_no(scores.iter().all(|&(_, p)| p >= 70))
    );
    println!(
        "anyone above 95: {}",
        yes_no(scores.iter().any(|&(_, p)| p > 95))
    );

    let mut ranked = scores.to_vec();
    ranked.sort_by_key(|&(_, points)| Reverse(points));
    let top: Vec<String> = ranked
        .iter()
        .take(3)
        .enumerate()
        .map(|(i, (name, points))| format!("{}. {name} {points}", i + 1))
        .collect();
    println!("top three: {}", top.join(", "));

    // flat_map turns each name into its letters, and a BTreeSet keeps one of
    // each, in order.
    let letters: BTreeSet<char> = scores.iter().flat_map(|(name, _)| name.chars()).collect();
    let letters: Vec<String> = letters.iter().map(|c| c.to_string()).collect();
    println!("letters used: {}", letters.join(" "));
}
```

**Go**

```go
package main

import (
	"cmp"
	"fmt"
	"maps"
	"slices"
	"strings"
)

type Score struct {
	Name   string
	Points int
}

func yesNo(value bool) string {
	if value {
		return "yes"
	}
	return "no"
}

// Go's standard library has no map, filter, or reduce. Loops are the usual
// style, with the slices and maps packages for sorting, searching, and keys.
func main() {
	scores := []Score{{"ada", 91}, {"bob", 78}, {"cy", 85}, {"dee", 62}, {"eve", 99}, {"finn", 54}}

	var passed []string
	total := 0
	for _, score := range scores {
		if score.Points >= 70 {
			passed = append(passed, score.Name)
			total += score.Points
		}
	}
	fmt.Println("passed:", strings.Join(passed, ", "))
	fmt.Println("total of the passes:", total)

	byPoints := func(a, b Score) int { return cmp.Compare(a.Points, b.Points) }
	best := slices.MaxFunc(scores, byPoints)
	fmt.Printf("best: %s with %d\n", best.Name, best.Points)
	failed := slices.ContainsFunc(scores, func(s Score) bool { return s.Points < 70 })
	fmt.Println("everyone passed:", yesNo(!failed))
	high := slices.ContainsFunc(scores, func(s Score) bool { return s.Points > 95 })
	fmt.Println("anyone above 95:", yesNo(high))

	ranked := slices.Clone(scores)
	slices.SortFunc(ranked, func(a, b Score) int { return byPoints(b, a) })
	var top []string
	for i, score := range ranked[:3] {
		top = append(top, fmt.Sprintf("%d. %s %d", i+1, score.Name, score.Points))
	}
	fmt.Println("top three:", strings.Join(top, ", "))

	seen := map[string]bool{}
	for _, score := range scores {
		for _, letter := range strings.Split(score.Name, "") {
			seen[letter] = true
		}
	}
	// maps.Keys returns an iterator, and slices.Sorted collects it in order.
	fmt.Println("letters used:", strings.Join(slices.Sorted(maps.Keys(seen)), " "))
}
```

**Python**

```python
from typing import NamedTuple


class Score(NamedTuple):
    name: str
    points: int


SCORES = [
    Score("ada", 91),
    Score("bob", 78),
    Score("cy", 85),
    Score("dee", 62),
    Score("eve", 99),
    Score("finn", 54),
]


def yes_no(value: bool) -> str:
    return "yes" if value else "no"


def main() -> None:
    # A list comprehension is a map and a filter in one expression.
    passed = [score.name for score in SCORES if score.points >= 70]
    print("passed:", ", ".join(passed))

    # Without the brackets it is a generator expression, which sum pulls from
    # one item at a time, and no list is built.
    print("total of the passes:", sum(s.points for s in SCORES if s.points >= 70))

    best = max(SCORES, key=lambda score: score.points)
    print(f"best: {best.name} with {best.points}")
    print("everyone passed:", yes_no(all(s.points >= 70 for s in SCORES)))
    print("anyone above 95:", yes_no(any(s.points > 95 for s in SCORES)))

    ranked = sorted(SCORES, key=lambda score: score.points, reverse=True)
    top = [f"{i}. {s.name} {s.points}" for i, s in enumerate(ranked[:3], start=1)]
    print("top three:", ", ".join(top))

    # Two for clauses flatten the names into letters, and {...} keeps one of each.
    letters = sorted({letter for score in SCORES for letter in score.name})
    print("letters used:", " ".join(letters))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const scores = [
  { name: "ada", points: 91 },
  { name: "bob", points: 78 },
  { name: "cy", points: 85 },
  { name: "dee", points: 62 },
  { name: "eve", points: 99 },
  { name: "finn", points: 54 },
];

const yesNo = (value) => (value ? "yes" : "no");

// Array methods are eager: filter builds a new array, then map builds another.
const passes = scores.filter((score) => score.points >= 70);
console.log(`passed: ${passes.map((score) => score.name).join(", ")}`);

// There is no sum, so reduce folds the array into one value, starting at 0.
const total = passes.reduce((sum, score) => sum + score.points, 0);
console.log(`total of the passes: ${total}`);

// There is no max by a key either, so reduce keeps the better of each pair.
const best = scores.reduce((a, b) => (b.points > a.points ? b : a));
console.log(`best: ${best.name} with ${best.points}`);
console.log(`everyone passed: ${yesNo(scores.every((s) => s.points >= 70))}`);
console.log(`anyone above 95: ${yesNo(scores.some((s) => s.points > 95))}`);

// toSorted returns a sorted copy; sort would change scores itself.
const top = scores
  .toSorted((a, b) => b.points - a.points)
  .slice(0, 3)
  .map((score, i) => `${i + 1}. ${score.name} ${score.points}`);
console.log(`top three: ${top.join(", ")}`);

// flatMap turns each name into its letters, and a Set keeps one of each.
const letters = [...new Set(scores.flatMap((score) => [...score.name]))].sort();
console.log(`letters used: ${letters.join(" ")}`);
```

**TypeScript**

```ts
type Score = { readonly name: string; readonly points: number };

const scores: readonly Score[] = [
  { name: "ada", points: 91 },
  { name: "bob", points: 78 },
  { name: "cy", points: 85 },
  { name: "dee", points: 62 },
  { name: "eve", points: 99 },
  { name: "finn", points: 54 },
];

const yesNo = (value: boolean) => (value ? "yes" : "no");

// Array methods are eager: filter builds a new array, then map builds another.
const passes = scores.filter((score) => score.points >= 70);
console.log(`passed: ${passes.map((score) => score.name).join(", ")}`);

// There is no sum, so reduce folds the array into one value, starting at 0.
const total = passes.reduce((sum, score) => sum + score.points, 0);
console.log(`total of the passes: ${total}`);

// There is no max by a key either, so reduce keeps the better of each pair.
const best = scores.reduce((a, b) => (b.points > a.points ? b : a));
console.log(`best: ${best.name} with ${best.points}`);
console.log(`everyone passed: ${yesNo(scores.every((s) => s.points >= 70))}`);
console.log(`anyone above 95: ${yesNo(scores.some((s) => s.points > 95))}`);

// toSorted returns a sorted copy; sort would change scores itself.
const top = scores
  .toSorted((a, b) => b.points - a.points)
  .slice(0, 3)
  .map((score, i) => `${i + 1}. ${score.name} ${score.points}`);
console.log(`top three: ${top.join(", ")}`);

// flatMap turns each name into its letters, and a Set keeps one of each.
const letters = [...new Set(scores.flatMap((score) => [...score.name]))].sort();
console.log(`letters used: ${letters.join(" ")}`);
```

**Dart**

```dart
typedef Score = ({String name, int points});

const List<Score> scores = [
  (name: 'ada', points: 91),
  (name: 'bob', points: 78),
  (name: 'cy', points: 85),
  (name: 'dee', points: 62),
  (name: 'eve', points: 99),
  (name: 'finn', points: 54),
];

String yesNo(bool value) => value ? 'yes' : 'no';

void main() {
  // where and map are lazy: they return an Iterable that does the work only
  // when something, here join, loops over it.
  final passes = scores.where((score) => score.points >= 70);
  print('passed: ${passes.map((score) => score.name).join(', ')}');

  // fold combines the items into one value, starting at 0.
  final total = passes.fold(0, (sum, score) => sum + score.points);
  print('total of the passes: $total');

  // reduce is fold without a starting value: it starts from the first item.
  final best = scores.reduce((a, b) => b.points > a.points ? b : a);
  print('best: ${best.name} with ${best.points}');
  print('everyone passed: ${yesNo(scores.every((s) => s.points >= 70))}');
  print('anyone above 95: ${yesNo(scores.any((s) => s.points > 95))}');

  // sort works in place, so sort a copy. indexed pairs each item with its index.
  final ranked = [...scores]..sort((a, b) => b.points.compareTo(a.points));
  final top = ranked
      .take(3)
      .indexed
      .map((entry) => '${entry.$1 + 1}. ${entry.$2.name} ${entry.$2.points}');
  print('top three: ${top.join(', ')}');

  // expand turns each name into its letters, and a Set keeps one of each.
  final letters =
      scores.expand((score) => score.name.split('')).toSet().toList()..sort();
  print('letters used: ${letters.join(' ')}');
}
```

**Kotlin**

```kotlin
data class Score(val name: String, val points: Int)

val scores =
    listOf(
        Score("ada", 91),
        Score("bob", 78),
        Score("cy", 85),
        Score("dee", 62),
        Score("eve", 99),
        Score("finn", 54),
    )

fun yesNo(value: Boolean) = if (value) "yes" else "no"

fun main() {
    // List operations are eager: filter builds a new list right away.
    val passes = scores.filter { it.points >= 70 }
    println("passed: ${passes.joinToString(", ") { it.name }}")
    println("total of the passes: ${passes.sumOf { it.points }}")

    val best = scores.maxBy { it.points }
    println("best: ${best.name} with ${best.points}")
    println("everyone passed: ${yesNo(scores.all { it.points >= 70 })}")
    println("anyone above 95: ${yesNo(scores.any { it.points > 95 })}")

    // withIndex pairs each item with its index, and (i, score) takes it apart.
    val top =
        scores
            .sortedByDescending { it.points }
            .take(3)
            .withIndex()
            .joinToString(", ") { (i, score) ->
                "${i + 1}. ${score.name} ${score.points}"
            }
    println("top three: $top")

    // flatMap turns each name into its letters, and a sorted set keeps one of each.
    val letters = scores.flatMap { it.name.toList() }.toSortedSet()
    println("letters used: ${letters.joinToString(" ")}")
}
```

All nine print the same lines:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
passed: ada, bob, cy, eve
total of the passes: 353
best: eve with 99
everyone passed: no
anyone above 95: yes
top three: 1. eve 99, 2. ada 91, 3. cy 85
letters used: a b c d e f i n o v y
```

The steps have different names in different languages, and C has none of them:

| Language | Keep some | Change each | Combine into one | Flatten | All, any | Number the items |
| --- | --- | --- | --- | --- | --- | --- |
| C++ | `views::filter` | `views::transform` | `std::accumulate`, `ranges::fold_left` (C++23) | `views::join` | `ranges::all_of`, `ranges::any_of` | `views::enumerate` (C++23) |
| Rust | `filter` | `map` | `fold`, `reduce`, `sum` | `flat_map`, `flatten` | `all`, `any` | `enumerate` |
| Go | A loop | A loop | A loop | A loop | `slices.ContainsFunc` | `range` gives the index |
| Python | `filter`, or `if` in a comprehension | `map`, or a comprehension | `sum`, `functools.reduce` | Two `for` clauses, `itertools.chain.from_iterable` | `all`, `any` | `enumerate` |
| JavaScript | `filter` | `map` | `reduce` | `flatMap`, `flat` | `every`, `some` | `map((x, i) => ...)`, `entries()` |
| Dart | `where` | `map` | `fold`, `reduce` | `expand` | `every`, `any` | `indexed` |
| Kotlin | `filter` | `map` | `fold`, `reduce`, `sumOf` | `flatMap`, `flatten` | `all`, `any` | `withIndex`, `mapIndexed` |

The names mostly agree. C++ calls map `transform`, Dart calls filter `where` and flat
map `expand`, and the step that combines everything is `fold`, `reduce`, or
`accumulate`. `fold` and `accumulate` take a starting value, and `reduce` starts from
the first item, so `reduce` on an empty list has nothing to return. JavaScript then
throws `TypeError: Reduce of empty array with no initial value`, Dart throws
`Bad state: No element`, and Rust's `reduce` returns an `Option`.

**C: a loop for each question.** C has no closures to pass to a `filter` (chapter 5), so
each question is a loop. One loop can also answer several questions at once, which a
pipeline can't: the second loop finds the best score, whether everyone passed, and
whether anyone scored above 95, in one pass. `qsort` sorts a copy, and an array of 26
`bool`s is the set of letters.

**C++: algorithms, views, and projections.** C++ has two libraries for this. The
algorithms in `<algorithm>`, such as `std::ranges::sort`, `std::ranges::all_of`, and
`std::ranges::max`, work on a whole range and return an answer. The views in `<ranges>`
(C++20), such as `std::views::filter` and `std::views::transform`, are lazy:
`scores | std::views::filter(passed)` makes a view that does nothing until something
loops over it, and `|` chains views. Many algorithms take a **projection** as their last
argument, a function applied to each item first:
`std::ranges::max(scores, {}, &Score::points)` compares the scores by their points, and
`std::ranges::sort(ranked, std::greater{}, &Score::points)` sorts by points, highest
first. A pointer to a member, such as `&Score::points`, works as a function that reads
that member. C++20 leaves gaps that the other languages don't have. There is no `join`
for strings, so the example writes one, `std::accumulate` in `<numeric>` takes two
iterators rather than a range, and no function collects a view into a vector. C++23 adds
`std::ranges::fold_left`, `std::views::enumerate`, and `std::ranges::to<std::vector>()`,
and GCC 13 has the first two, but not `to`. `std::views::join` flattens the names into
one view of letters, and a `std::set` keeps one of each, sorted.

**Rust: adapters, then a consumer.** `scores.iter()` lends the items, and each
**adapter**, such as `filter` and `map`, wraps the iterator before it in a new one.
Nothing happens until a **consumer**, such as `collect`, `sum`, `max_by_key`, or `all`,
pulls the items out. The closures' patterns follow the borrowing. `iter()` hands out a
`&(&str, i32)`, and `filter` passes its closure a reference to each item, so the pattern
there is `&&(_, points)`, while `map` gets the item itself, `&(name, _)`. `collect` and
`sum` can build more than one type, so they need to be told which: `let total: i32` does
that, and without it the compiler stops with `error[E0283]: type annotations needed`.
`collect` builds whatever the type says, here a `Vec<&str>`, and later a
`BTreeSet<char>`, which keeps one of each letter, in order. Ties need care: `max_by_key`
returns the last of several equal items, and `min_by_key` the first. `sort_by_key` with
`Reverse` sorts highest first.

**Go: loops, with help from `slices` and `maps`.** Go's standard library has no `map`,
`filter`, or `reduce`, so most of the questions are `for` loops, and the first loop
answers two of them. The `slices` package has the searching and sorting.
`slices.MaxFunc` finds the best score with a compare function, `slices.ContainsFunc`
answers "anyone above 95?", and "everyone passed?" is "nobody failed", so it is
`!slices.ContainsFunc` with the opposite test. `slices.Clone` makes a copy for
`slices.SortFunc` to sort in place. `maps.Keys` returns an iterator over a map's keys,
and `slices.Sorted` collects it into a sorted slice, which is how Go turns a set, a
`map[string]bool`, into ordered output.

**Python: comprehensions.** A list comprehension,
`[score.name for score in SCORES if score.points >= 70]`, is a map and a filter in one
expression, and it is the usual Python way to write both. Without the brackets,
`(s.points for s in SCORES if ...)` is a **generator expression**, which builds no list
and hands its items one at a time to whatever reads it, here `sum`. As the only argument
to a function, it needs no parentheses of its own. `max` takes a `key` function, and so
do `min` and `sorted`. `enumerate(..., start=1)` numbers the items from 1, two `for`
clauses in one comprehension flatten the names into letters, and braces make a set
comprehension. Python has `functools.reduce`, but `sum`, `max`, `all`, `any`, and
`str.join` cover most of its uses.

**JavaScript and TypeScript: array methods.** `filter`, `map`, `every`, `some`, and
`flatMap` are methods of every array, and each one builds a new array at once. There is
no `sum` or "max by", so `reduce` does both.
`reduce((sum, score) => sum + score.points, 0)` folds from 0, and
`reduce((a, b) => (b.points > a.points ? b : a))`, with no starting value, starts from
the first score. `toSorted` (ES2023) returns a sorted copy, while `sort` sorts the array
itself, and without a compare function both compare the items as text, so
`[10, 9, 1].sort()` gives `[1, 10, 9]`. `[...score.name]` spreads a name into its
letters, and a `Set` keeps one of each. The TypeScript version is the same code with a
type for `Score`, and `readonly` on the type and the array, so that nothing changes the
scores by accident.

**Dart: `Iterable` methods, lazy already.** `where` is Dart's filter and `expand` its
flat map. Both, and `map`, return a lazy `Iterable` that does the work only when
something loops over it, here `join` and `fold`. `fold` starts from a value and `reduce`
from the first item. `sort` sorts a list in place and returns nothing, so the cascade
`..sort()` sorts the copy and gives back the list itself. `indexed` pairs each item with
its index as a record, `(int, Score)`, whose fields are `$1` and `$2`. Section 14.5
shows the catch with lazy `map`: it does its work again each time the result is read.

**Kotlin: collection functions, eager by default.** Kotlin's list functions build a new
list at each step, so `scores.filter { it.points >= 70 }` is a new list straight away.
`sumOf`, `maxBy`, `sortedByDescending`, and `joinToString` with a transform cover what
the other languages do with a `reduce` or an extra `map`. `maxBy` throws
`NoSuchElementException` on an empty list, and `maxByOrNull` returns `null` instead.
`withIndex()` pairs each item with its index, and the lambda `{ (i, score) -> ... }`
takes the pair apart. Section 14.5 switches to the lazy versions, on a `Sequence`.

---

## 14.5 Lazy and eager

`square` and `even` print a line each time they run, so the output shows the order of
the work. The eager version runs each step on all five numbers before the next step
starts. The lazy version takes each number through both steps before it starts the next
one, and stops when it has two even squares. The last line uses a source that never
ends, all the natural numbers, which only a lazy pipeline can read from.

**C**

```c
#include <stdbool.h>
#include <stdio.h>

// Both functions say when they run, so the output shows the order of the work.
static int square(int n) {
    printf("  square %d\n", n);
    return n * n;
}

static bool even(int x) {
    printf("  even? %d\n", x);
    return x % 2 == 0;
}

int main(void) {
    // Eager: each step finishes the whole list before the next step starts.
    printf("eager:\n");
    int squares[5], evens[5];
    int found = 0;
    for (int i = 0; i < 5; i++) squares[i] = square(i + 1);
    for (int i = 0; i < 5; i++) {
        if (even(squares[i])) evens[found++] = squares[i];
    }
    printf("  first two: %d %d\n", evens[0], evens[1]);

    // Lazy: one loop takes each number through every step, and stops as soon
    // as it has two. In C, lazy means writing the steps into one loop.
    printf("lazy:\n");
    int first_two[2];
    found = 0;
    for (int n = 1; n <= 5 && found < 2; n++) {
        int x = square(n);
        if (even(x)) first_two[found++] = x;
    }
    printf("  first two: %d %d\n", first_two[0], first_two[1]);

    // With no end to the numbers, only the lazy way can work.
    printf("endless:");
    found = 0;
    for (int n = 1; found < 3; n++) {
        int x = n * n;
        if (x % 2 == 0 && x > 50) {
            printf(" %d", x);
            found++;
        }
    }
    printf("\n");
    return 0;
}
```

**C++**

```cpp
#include <algorithm>
#include <iostream>
#include <iterator>
#include <ranges>
#include <vector>

// Both functions say when they run, so the output shows the order of the work.
int square(int n) {
    std::cout << "  square " << n << "\n";
    return n * n;
}

bool even(int x) {
    std::cout << "  even? " << x << "\n";
    return x % 2 == 0;
}

void print_first_two(const std::vector<int>& items) {
    std::cout << "  first two: " << items[0] << " " << items[1] << "\n";
}

int main() {
    const std::vector numbers{1, 2, 3, 4, 5};

    // Eager: each algorithm finishes the whole vector before the next starts.
    std::cout << "eager:\n";
    std::vector<int> squares, evens;
    std::ranges::transform(numbers, std::back_inserter(squares), square);
    std::ranges::copy_if(squares, std::back_inserter(evens), even);
    print_first_two(evens);

    // Lazy: a view does nothing until the loop asks it for items. The output
    // also shows two costs of C++20 views (section 14.5): reading an item
    // through filter calls square again, and take steps one item too far.
    std::cout << "lazy:\n";
    std::vector<int> first_two;
    for (int x : numbers | std::views::transform(square) | std::views::filter(even) |
                     std::views::take(2)) {
        first_two.push_back(x);
    }
    print_first_two(first_two);

    // iota(1) counts up forever, so only a lazy pipeline can use it.
    std::cout << "endless:";
    auto big_even = [](int x) { return x % 2 == 0 && x > 50; };
    for (int x : std::views::iota(1) | std::views::transform([](int n) { return n * n; }) |
                     std::views::filter(big_even) | std::views::take(3)) {
        std::cout << " " << x;
    }
    std::cout << "\n";
}
```

**Rust**

```rust
// Both functions say when they run, so the output shows the order of the work.
fn square(n: i32) -> i32 {
    println!("  square {n}");
    n * n
}

fn even(x: &i32) -> bool {
    println!("  even? {x}");
    x % 2 == 0
}

fn main() {
    // Eager: collect finishes each step before the next one starts.
    println!("eager:");
    let squares: Vec<i32> = (1..=5).map(square).collect();
    let evens: Vec<i32> = squares.into_iter().filter(even).collect();
    println!("  first two: {} {}", evens[0], evens[1]);

    // Lazy: map and filter only describe the work. collect pulls items through
    // one at a time, and take stops pulling after two.
    println!("lazy:");
    let first_two: Vec<i32> = (1..=5).map(square).filter(even).take(2).collect();
    println!("  first two: {} {}", first_two[0], first_two[1]);

    // 1.. counts up forever, so only a lazy pipeline can use it.
    let endless: Vec<String> = (1..)
        .map(|n| n * n)
        .filter(|x| x % 2 == 0 && *x > 50)
        .take(3)
        .map(|x: i32| x.to_string())
        .collect();
    println!("endless: {}", endless.join(" "));
}
```

**Go**

```go
package main

import (
	"fmt"
	"iter"
	"slices"
)

// Both functions say when they run, so the output shows the order of the work.
func square(n int) int {
	fmt.Println("  square", n)
	return n * n
}

func even(x int) bool {
	fmt.Println("  even?", x)
	return x%2 == 0
}

// Go's iter package has no Map, Filter, or Take, but each is a few lines:
// a push iterator that wraps another one.
func Map[T, U any](seq iter.Seq[T], f func(T) U) iter.Seq[U] {
	return func(yield func(U) bool) {
		for item := range seq {
			if !yield(f(item)) {
				return
			}
		}
	}
}

func Filter[T any](seq iter.Seq[T], keep func(T) bool) iter.Seq[T] {
	return func(yield func(T) bool) {
		for item := range seq {
			if keep(item) && !yield(item) {
				return
			}
		}
	}
}

func Take[T any](seq iter.Seq[T], n int) iter.Seq[T] {
	return func(yield func(T) bool) {
		if n <= 0 {
			return
		}
		count := 0
		for item := range seq {
			if !yield(item) {
				return
			}
			count++
			if count == n {
				return // leaving the loop tells seq to stop
			}
		}
	}
}

// Naturals counts up forever. It stops only when its caller does.
func Naturals() iter.Seq[int] {
	return func(yield func(int) bool) {
		for n := 1; yield(n); n++ {
		}
	}
}

func main() {
	numbers := []int{1, 2, 3, 4, 5}

	// Eager: each loop finishes the whole slice before the next one starts.
	fmt.Println("eager:")
	var squares, evens []int
	for _, n := range numbers {
		squares = append(squares, square(n))
	}
	for _, x := range squares {
		if even(x) {
			evens = append(evens, x)
		}
	}
	fmt.Println("  first two:", evens[0], evens[1])

	// Lazy: the adapters only describe the work, and slices.Collect pulls
	// each number through all of them before it asks for the next.
	fmt.Println("lazy:")
	firstTwo := slices.Collect(Take(Filter(Map(slices.Values(numbers), square), even), 2))
	fmt.Println("  first two:", firstTwo[0], firstTwo[1])

	bigEven := func(x int) bool { return x%2 == 0 && x > 50 }
	squared := Map(Naturals(), func(n int) int { return n * n })
	fmt.Print("endless:")
	for x := range Take(Filter(squared, bigEven), 3) {
		fmt.Print(" ", x)
	}
	fmt.Println()
}
```

**Python**

```python
from itertools import count, islice


# Both functions say when they run, so the output shows the order of the work.
def square(n: int) -> int:
    print(f"  square {n}")
    return n * n


def even(x: int) -> bool:
    print(f"  even? {x}")
    return x % 2 == 0


def main() -> None:
    numbers = [1, 2, 3, 4, 5]

    # Eager: a list comprehension builds the whole list before the next line.
    print("eager:")
    squares = [square(n) for n in numbers]
    evens = [x for x in squares if even(x)]
    print("  first two:", *evens[:2])

    # Lazy: generator expressions, in parentheses, describe the work. islice
    # pulls each number through both of them, and stops after two.
    print("lazy:")
    lazy_squares = (square(n) for n in numbers)
    lazy_evens = (x for x in lazy_squares if even(x))
    print("  first two:", *islice(lazy_evens, 2))

    # count(1) counts up forever, so only a lazy pipeline can use it.
    big_evens = (x for x in (n * n for n in count(1)) if x % 2 == 0 and x > 50)
    print("endless:", *islice(big_evens, 3))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// Both functions say when they run, so the output shows the order of the work.
function square(n) {
  console.log(`  square ${n}`);
  return n * n;
}

function even(x) {
  console.log(`  even? ${x}`);
  return x % 2 === 0;
}

const numbers = [1, 2, 3, 4, 5];

// Eager: each array method builds a whole new array before the next one runs.
console.log("eager:");
const evens = numbers.map(square).filter(even);
console.log(`  first two: ${evens.slice(0, 2).join(" ")}`);

// Lazy: values() gives an iterator, and iterator helpers (ES2025) such as
// map, filter, and take pull one item at a time through every step.
console.log("lazy:");
const firstTwo = numbers.values().map(square).filter(even).take(2).toArray();
console.log(`  first two: ${firstTwo.join(" ")}`);

// A generator that counts up forever, so only a lazy pipeline can use it.
function* naturals() {
  for (let n = 1; ; n++) yield n;
}

const endless = naturals()
  .map((n) => n * n)
  .filter((x) => x % 2 === 0 && x > 50)
  .take(3)
  .toArray();
console.log(`endless: ${endless.join(" ")}`);
```

**TypeScript**

```ts
// Both functions say when they run, so the output shows the order of the work.
function square(n: number): number {
  console.log(`  square ${n}`);
  return n * n;
}

function even(x: number): boolean {
  console.log(`  even? ${x}`);
  return x % 2 === 0;
}

const numbers = [1, 2, 3, 4, 5];

// Eager: each array method builds a whole new array before the next one runs.
console.log("eager:");
const evens = numbers.map(square).filter(even);
console.log(`  first two: ${evens.slice(0, 2).join(" ")}`);

// Lazy: values() gives an iterator, and iterator helpers (ES2025) such as
// map, filter, and take pull one item at a time through every step.
console.log("lazy:");
const firstTwo = numbers.values().map(square).filter(even).take(2).toArray();
console.log(`  first two: ${firstTwo.join(" ")}`);

// A generator that counts up forever, so only a lazy pipeline can use it.
function* naturals(): Generator<number> {
  for (let n = 1; ; n++) yield n;
}

const endless = naturals()
  .map((n) => n * n)
  .filter((x) => x % 2 === 0 && x > 50)
  .take(3)
  .toArray();
console.log(`endless: ${endless.join(" ")}`);
```

**Dart**

```dart
// Both functions say when they run, so the output shows the order of the work.
int square(int n) {
  print('  square $n');
  return n * n;
}

bool even(int x) {
  print('  even? $x');
  return x % 2 == 0;
}

// A generator that counts up forever, so only a lazy pipeline can use it.
Iterable<int> naturals() sync* {
  for (var n = 1; ; n++) {
    yield n;
  }
}

void main() {
  final numbers = [1, 2, 3, 4, 5];

  // Eager: toList runs each step over the whole list before the next starts.
  print('eager:');
  final squares = numbers.map(square).toList();
  final evens = squares.where(even).toList();
  print('  first two: ${evens.take(2).join(' ')}');

  // Lazy: map, where, and take on an Iterable are lazy already. toList pulls
  // each number through every step, and take stops after two.
  print('lazy:');
  final firstTwo = numbers.map(square).where(even).take(2).toList();
  print('  first two: ${firstTwo.join(' ')}');

  final endless = naturals()
      .map((n) => n * n)
      .where((x) => x % 2 == 0 && x > 50)
      .take(3);
  print('endless: ${endless.join(' ')}');
}
```

**Kotlin**

```kotlin
// Both functions say when they run, so the output shows the order of the work.
fun square(n: Int): Int {
    println("  square $n")
    return n * n
}

fun even(x: Int): Boolean {
    println("  even? $x")
    return x % 2 == 0
}

fun main() {
    val numbers = listOf(1, 2, 3, 4, 5)

    // Eager: List operations build a whole new list at each step.
    println("eager:")
    val evens = numbers.map(::square).filter(::even)
    println("  first two: ${evens.take(2).joinToString(" ")}")

    // Lazy: asSequence switches to Sequence operations, which pull one item at
    // a time through every step. toList starts the pulling.
    println("lazy:")
    val firstTwo = numbers.asSequence().map(::square).filter(::even).take(2).toList()
    println("  first two: ${firstTwo.joinToString(" ")}")

    // generateSequence counts up forever, so only a lazy pipeline can use it.
    val endless =
        generateSequence(1) { it + 1 }.map { it * it }.filter { it % 2 == 0 && it > 50 }.take(3)
    println("endless: ${endless.joinToString(" ")}")
}
```

Eight of the nine print the same lines. C++ differs in the lazy part:

*C, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
eager:
  square 1
  square 2
  square 3
  square 4
  square 5
  even? 1
  even? 4
  even? 9
  even? 16
  even? 25
  first two: 4 16
lazy:
  square 1
  even? 1
  square 2
  even? 4
  square 3
  even? 9
  square 4
  even? 16
  first two: 4 16
endless: 64 100 144
```

*C++:*

```text
eager:
  square 1
  square 2
  square 3
  square 4
  square 5
  even? 1
  even? 4
  even? 9
  even? 16
  even? 25
  first two: 4 16
lazy:
  square 1
  even? 1
  square 2
  even? 4
  square 2
  square 3
  even? 9
  square 4
  even? 16
  square 4
  square 5
  even? 25
  first two: 4 16
endless: 64 100 144
```

In the eager version, `square` runs five times, then `even` five times, and the result
holds every even square, of which the program uses two. In the lazy version, each number
goes through `square` and then `even` before the next one starts, and the work stops at
4, since 16 is the second even square. 5 is never squared. A lazy pipeline does less
work when only part of the result is needed, and keeps no list between the steps, which
matters when the source is large, slow, or endless.

**C: laziness is one loop.** The eager version fills an array at each step. In C, lazy
means writing every step into one loop, with a condition that stops it, here
`found < 2`. Exercise 14.2's C solution shows the other way, with a chain of structs,
each one pulling from the one before.

**C++: views, with two costs.** `std::ranges::transform` and `std::ranges::copy_if` are
eager, and each one fills a new vector. Views are lazy, and `std::views::iota(1)` counts
up forever. The lazy output differs from the other languages in two places. First,
`square 2` and `square 4` appear twice. `filter` calls `square` to test an item and
doesn't keep the result, so when the loop reads the item, `square` runs again. Second,
after the second even square come `square 5` and `even? 25`. The range-`for` loop moves
its iterator on after every item, the last one included, and moving `take`'s iterator
moves the `filter` iterator inside it, which looks for the next even square before
`take` notices that it has enough. On an endless source, that search may never end.
`std::views::iota(1) | std::views::filter(small) | std::views::take(2)`, with `small`
true for numbers below 3, prints 1 and 2 and then runs forever, looking for a third
number below 3. With `std::views::iota(1, INT_MAX)` it does finish, after calling
`small` 2,147,483,646 times. Exercise 14.2 has a test for this. There is a third
surprise: a `filter_view` remembers where its first item is once it has found it, so
reading the view changes it, and a `const` filter view can't be read at all. A function
that takes `const auto& items` and loops over a filter view is refused with
`passing 'const std::ranges::filter_view<...>' as 'this' argument discards qualifiers`.
Pass views by value, or as `auto&&`.

**Rust: adapters are lazy.** Every adapter is lazy, and `collect` is what pulls the
items through, so the eager version needs a `collect` after each step. An adapter whose
result is thrown away does nothing, and the compiler says so:
`warning: unused 'Map' that must be used`, with
`note: iterators are lazy and do nothing unless consumed`. `take(2)` checks its count
before it asks for another item, so the lazy version stops after `even? 16`. `1..` is a
range with no end.

**Go: hand-written adapters over push iterators.** Go has no adapters, so the example
writes `Map`, `Filter`, and `Take` as generic functions from one `iter.Seq[T]` to
another, and `Naturals` as a source that never ends. Each adapter ranges over its input
and passes items on with `yield`. `Take` returns as soon as it has handed out its last
item, without asking for another, and returning ends its `range` loop, which makes the
`yield` inside `Filter` return `false`, which ends `Filter`, and so on back to
`Naturals`. `slices.Values` turns a slice into an iterator, and `slices.Collect` gathers
an iterator into a slice.

**Python: generator expressions and `itertools`.** A list comprehension is eager, and a
generator expression, in parentheses, is lazy. `islice(lazy_evens, 2)` takes the first
two items of any iterator, and `itertools.count(1)` counts up forever. Python 3's `map`,
`filter`, `zip`, and `range` are lazy too, so `map(print, items)` on its own prints
nothing: it returns a `map` object that nothing reads.

**JavaScript and TypeScript: arrays are eager, iterators are lazy.** Array methods are
eager. `numbers.values()` returns an iterator over the array, and the iterator helpers,
added in ES2025, give iterators `map`, `filter`, `take`, `drop`, `flatMap`, `reduce`,
`toArray`, and a few more, all of them lazy. Generator objects have the helpers too, so
`naturals().map(...)` works. There is no `takeWhile`, `zip`, or `chunk` among them yet.

**Dart: `Iterable` is lazy already, and does the work again.** `map`, `where`, and
`take` on an `Iterable` are lazy, so the lazy version is the eager one without the
`toList()` calls. The catch is that a lazy `Iterable` keeps the work, not the results,
and does the work again each time it is read. After
`final squares = [1, 2, 3].map(square)`, each `squares.toList()` calls `square` three
more times. Call `toList()` once when the results are used more than once. `naturals` is
a `sync*` generator, from section 14.6.

**Kotlin: lists are eager, sequences are lazy.** `numbers.map(::square)` builds a new
list at once, while `asSequence()` switches to `Sequence`, whose functions are lazy, and
`toList()` at the end pulls the items through. Like a Dart `Iterable`, a sequence does
its work again each time it is read. Unlike Rust, Kotlin says nothing about a sequence
that nothing reads: `listOf(1, 2).asSequence().map { println(it) }` compiles without a
warning, and prints nothing. `generateSequence(1) { it + 1 }` counts up forever.

---

## 14.6 Generators

Each version has two generators: the Fibonacci numbers, which go on forever, and the
values of a binary search tree in order, 1 to 7. The last line reads the tree only up to
3, and the visits to 5, 6, and 7 never happen.

**C**

```c
#include <stdbool.h>
#include <stdio.h>

// C has no generators, so these are written the other way round: they call
// visit with each value, and stop when visit returns false. context carries
// whatever state the visitor needs.
typedef bool (*visit_fn)(long value, void *context);

static void fibonacci(visit_fn visit, void *context) {
    long a = 0, b = 1;
    while (visit(a, context)) {
        long next = a + b;
        a = b;
        b = next;
    }
}

struct node {
    int value;
    const struct node *left, *right;
};

// Returns whether to keep going, so a false from visit stops every level of
// the recursion at once.
static bool in_order(const struct node *node, visit_fn visit, void *context) {
    return !node || (in_order(node->left, visit, context) && visit(node->value, context) &&
                     in_order(node->right, visit, context));
}

static bool print_some(long value, void *context) {
    int *left = context;  // how many more to print
    if (*left == 0) return false;
    printf(" %ld", value);
    (*left)--;
    return true;
}

static bool print_up_to(long value, void *context) {
    const long *limit = context;
    if (value > *limit) return false;
    printf(" %ld", value);
    return true;
}

int main(void) {
    const struct node one = {1, NULL, NULL}, three = {3, NULL, NULL};
    const struct node five = {5, NULL, NULL}, seven = {7, NULL, NULL};
    const struct node two = {2, &one, &three}, six = {6, &five, &seven};
    const struct node tree = {4, &two, &six};

    int left = 10;
    printf("fibonacci:");
    fibonacci(print_some, &left);

    left = 7;
    printf("\nin order:");
    in_order(&tree, print_some, &left);

    long limit = 3;  // print_up_to returns false at 4, so 5, 6, and 7 are never visited
    printf("\nup to 3:");
    in_order(&tree, print_up_to, &limit);
    printf("\n");
    return 0;
}
```

**C++**

```cpp
#include <iostream>

// C++20 has coroutines but no generator type to use them with; C++23 adds
// std::generator. So these are written the other way round: they call visit
// with each value, and stop when visit returns false.
template <typename Visit>
void fibonacci(Visit visit) {
    long a = 0, b = 1;
    while (visit(a)) {
        long next = a + b;
        a = b;
        b = next;
    }
}

struct Node {
    int value;
    const Node* left = nullptr;
    const Node* right = nullptr;
};

// Returns whether to keep going, so a false from visit stops every level of
// the recursion at once.
template <typename Visit>
bool in_order(const Node* node, Visit& visit) {
    return !node || (in_order(node->left, visit) && visit(node->value) &&
                     in_order(node->right, visit));
}

int main() {
    const Node one{1}, three{3}, five{5}, seven{7};
    const Node two{2, &one, &three}, six{6, &five, &seven};
    const Node tree{4, &two, &six};

    int count = 0;
    std::cout << "fibonacci:";
    fibonacci([&](long value) {
        if (count++ == 10) return false;
        std::cout << " " << value;
        return true;
    });

    auto print_all = [](int value) {
        std::cout << " " << value;
        return true;
    };
    std::cout << "\nin order:";
    in_order(&tree, print_all);

    // Returning false at the first value over 3 means 5, 6, and 7 are never visited.
    auto print_up_to_3 = [](int value) {
        if (value > 3) return false;
        std::cout << " " << value;
        return true;
    };
    std::cout << "\nup to 3:";
    in_order(&tree, print_up_to_3);
    std::cout << "\n";
}
```

**Rust**

```rust
// Rust has no generators yet (gen blocks are unstable). from_fn makes an
// iterator from a closure that keeps its own state between calls.
fn fibonacci() -> impl Iterator<Item = u64> {
    let (mut a, mut b) = (0, 1);
    std::iter::from_fn(move || {
        let value = a;
        (a, b) = (b, a + b);
        Some(value)
    })
}

struct Node {
    value: i32,
    left: Option<Box<Node>>,
    right: Option<Box<Node>>,
}

fn node(value: i32, left: Option<Box<Node>>, right: Option<Box<Node>>) -> Option<Box<Node>> {
    Some(Box::new(Node { value, left, right }))
}

fn leaf(value: i32) -> Option<Box<Node>> {
    node(value, None, None)
}

// Without a generator to keep the path down the tree, the iterator keeps it
// itself: a stack of the nodes whose value hasn't been handed out yet.
struct InOrder<'a> {
    stack: Vec<&'a Node>,
}

impl<'a> InOrder<'a> {
    fn new(root: &'a Option<Box<Node>>) -> Self {
        let mut it = InOrder { stack: Vec::new() };
        it.push_left(root.as_deref());
        it
    }

    fn push_left(&mut self, mut node: Option<&'a Node>) {
        while let Some(n) = node {
            self.stack.push(n);
            node = n.left.as_deref();
        }
    }
}

impl Iterator for InOrder<'_> {
    type Item = i32;

    fn next(&mut self) -> Option<i32> {
        let node = self.stack.pop()?;
        self.push_left(node.right.as_deref());
        Some(node.value)
    }
}

fn line(items: impl Iterator<Item = impl ToString>) -> String {
    items
        .map(|item| item.to_string())
        .collect::<Vec<_>>()
        .join(" ")
}

fn main() {
    let tree = node(4, node(2, leaf(1), leaf(3)), node(6, leaf(5), leaf(7)));
    println!("fibonacci: {}", line(fibonacci().take(10)));
    println!("in order: {}", line(InOrder::new(&tree)));
    // take_while stops asking at the first value over 3, so 5, 6, and 7 are
    // never visited.
    println!(
        "up to 3: {}",
        line(InOrder::new(&tree).take_while(|&v| v <= 3))
    );
}
```

**Go**

```go
package main

import (
	"fmt"
	"iter"
	"strconv"
	"strings"
)

// A Go iterator is already written like a generator: the function calls
// yield for each value, and yield returns false when the loop is done.
func Fibonacci() iter.Seq[int] {
	return func(yield func(int) bool) {
		a, b := 0, 1
		for yield(a) {
			a, b = b, a+b
		}
	}
}

type Node struct {
	Value       int
	Left, Right *Node
}

// walk reports whether to keep going, so a false from yield stops every
// level of the recursion at once.
func (n *Node) walk(yield func(int) bool) bool {
	return n == nil || n.Left.walk(yield) && yield(n.Value) && n.Right.walk(yield)
}

func (n *Node) InOrder() iter.Seq[int] {
	return func(yield func(int) bool) { n.walk(yield) }
}

func main() {
	tree := &Node{4, &Node{2, &Node{1, nil, nil}, &Node{3, nil, nil}},
		&Node{6, &Node{5, nil, nil}, &Node{7, nil, nil}}}

	var fibonacci []string
	for x := range Fibonacci() {
		if len(fibonacci) == 10 {
			break
		}
		fibonacci = append(fibonacci, strconv.Itoa(x))
	}
	fmt.Println("fibonacci:", strings.Join(fibonacci, " "))

	fmt.Print("in order:")
	for value := range tree.InOrder() {
		fmt.Print(" ", value)
	}

	// break makes yield return false, so 5, 6, and 7 are never visited.
	fmt.Print("\nup to 3:")
	for value := range tree.InOrder() {
		if value > 3 {
			break
		}
		fmt.Print(" ", value)
	}
	fmt.Println()
}
```

**Python**

```python
from collections.abc import Iterator
from dataclasses import dataclass
from itertools import islice, takewhile


# A function with yield in it is a generator function. Calling it runs none of
# the body: it returns a generator, and each next() runs the body to the next
# yield, where it pauses with all its variables kept.
def fibonacci() -> Iterator[int]:
    a, b = 0, 1
    while True:
        yield a
        a, b = b, a + b


@dataclass
class Node:
    value: int
    left: "Node | None" = None
    right: "Node | None" = None


# yield from hands on everything another generator yields. Each recursive call
# pauses inside its own frame, so the generator keeps the path down the tree.
def in_order(node: Node | None) -> Iterator[int]:
    if node is None:
        return
    yield from in_order(node.left)
    yield node.value
    yield from in_order(node.right)


def main() -> None:
    tree = Node(4, Node(2, Node(1), Node(3)), Node(6, Node(5), Node(7)))
    print("fibonacci:", *islice(fibonacci(), 10))
    print("in order:", *in_order(tree))
    # takewhile stops asking at the first value over 3, so 5, 6, and 7 are
    # never visited.
    print("up to 3:", *takewhile(lambda value: value <= 3, in_order(tree)))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// function* declares a generator function. Calling it runs none of the body:
// it returns a generator, and each next() runs the body to the next yield,
// where it pauses with all its variables kept.
function* fibonacci() {
  let [a, b] = [0, 1];
  for (;;) {
    yield a;
    [a, b] = [b, a + b];
  }
}

// yield* hands on everything another generator yields. Each recursive call
// pauses inside its own frame, so the generator keeps the path down the tree.
function* inOrder(node) {
  if (node === null) return;
  yield* inOrder(node.left);
  yield node.value;
  yield* inOrder(node.right);
}

const leaf = (value) => ({ value, left: null, right: null });
const tree = {
  value: 4,
  left: { value: 2, left: leaf(1), right: leaf(3) },
  right: { value: 6, left: leaf(5), right: leaf(7) },
};

console.log(`fibonacci: ${fibonacci().take(10).toArray().join(" ")}`);
console.log(`in order: ${[...inOrder(tree)].join(" ")}`);

// break stops asking at the first value over 3, so 5, 6, and 7 are never
// visited. There is no takeWhile helper, so the loop does it.
const upTo3 = [];
for (const value of inOrder(tree)) {
  if (value > 3) break;
  upTo3.push(value);
}
console.log(`up to 3: ${upTo3.join(" ")}`);
```

**TypeScript**

```ts
// function* declares a generator function. Calling it runs none of the body:
// it returns a generator, and each next() runs the body to the next yield,
// where it pauses with all its variables kept.
function* fibonacci(): Generator<number> {
  let [a, b] = [0, 1];
  for (;;) {
    yield a;
    [a, b] = [b, a + b];
  }
}

type Tree = { value: number; left: Tree | null; right: Tree | null };

// yield* hands on everything another generator yields. Each recursive call
// pauses inside its own frame, so the generator keeps the path down the tree.
function* inOrder(node: Tree | null): Generator<number> {
  if (node === null) return;
  yield* inOrder(node.left);
  yield node.value;
  yield* inOrder(node.right);
}

const leaf = (value: number): Tree => ({ value, left: null, right: null });
const tree: Tree = {
  value: 4,
  left: { value: 2, left: leaf(1), right: leaf(3) },
  right: { value: 6, left: leaf(5), right: leaf(7) },
};

console.log(`fibonacci: ${fibonacci().take(10).toArray().join(" ")}`);
console.log(`in order: ${[...inOrder(tree)].join(" ")}`);

// break stops asking at the first value over 3, so 5, 6, and 7 are never
// visited. There is no takeWhile helper, so the loop does it.
const upTo3: number[] = [];
for (const value of inOrder(tree)) {
  if (value > 3) break;
  upTo3.push(value);
}
console.log(`up to 3: ${upTo3.join(" ")}`);
```

**Dart**

```dart
// sync* makes a generator function: it returns an Iterable, and the body runs
// only as far as the next yield each time the loop asks for an item.
Iterable<int> fibonacci() sync* {
  var (a, b) = (0, 1);
  while (true) {
    yield a;
    (a, b) = (b, a + b);
  }
}

class Node {
  final int value;
  final Node? left, right;

  const Node(this.value, [this.left, this.right]);
}

// yield* hands on everything another Iterable yields. Each recursive call
// pauses inside its own frame, so the generator keeps the path down the tree.
Iterable<int> inOrder(Node? node) sync* {
  if (node == null) return;
  yield* inOrder(node.left);
  yield node.value;
  yield* inOrder(node.right);
}

void main() {
  const tree = Node(4, Node(2, Node(1), Node(3)), Node(6, Node(5), Node(7)));
  print('fibonacci: ${fibonacci().take(10).join(' ')}');
  print('in order: ${inOrder(tree).join(' ')}');
  // takeWhile stops asking at the first value over 3, so 5, 6, and 7 are
  // never visited.
  print('up to 3: ${inOrder(tree).takeWhile((value) => value <= 3).join(' ')}');
}
```

**Kotlin**

```kotlin
// sequence { } builds a Sequence from a block that calls yield. The block is
// a coroutine: it runs only as far as the next yield each time the loop asks
// for an item, and pauses there with its variables kept.
fun fibonacci(): Sequence<Int> = sequence {
    var a = 0
    var b = 1
    while (true) {
        yield(a)
        val next = a + b
        a = b
        b = next
    }
}

data class Node(val value: Int, val left: Node? = null, val right: Node? = null)

// yieldAll hands on everything another sequence yields. Each recursive call
// pauses inside its own block, so the sequence keeps the path down the tree.
fun inOrder(node: Node?): Sequence<Int> = sequence {
    if (node != null) {
        yieldAll(inOrder(node.left))
        yield(node.value)
        yieldAll(inOrder(node.right))
    }
}

fun main() {
    val tree = Node(4, Node(2, Node(1), Node(3)), Node(6, Node(5), Node(7)))
    println("fibonacci: ${fibonacci().take(10).joinToString(" ")}")
    println("in order: ${inOrder(tree).joinToString(" ")}")
    // takeWhile stops asking at the first value over 3, so 5, 6, and 7 are
    // never visited.
    println("up to 3: ${inOrder(tree).takeWhile { it <= 3 }.joinToString(" ")}")
}
```

All nine print the same lines:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
fibonacci: 0 1 1 2 3 5 8 13 21 34
in order: 1 2 3 4 5 6 7
up to 3: 1 2 3
```

Compare `fibonacci` with `RingIterator` in section 14.3. The state that a hand-written
iterator keeps in fields lives in a generator's local variables, and its loop is an
ordinary loop. Recursion is where generators help most. An in-order walk is three lines
of recursion, but as a hand-written iterator it needs a stack of the nodes still to
visit, as the Rust version shows, because the place in the recursion is what the
iterator has to keep between calls.

**Python: `yield` and `yield from`.** A function with `yield` anywhere in it is a
generator function. Calling it runs none of the body: it returns a generator, and each
`next` runs the body up to the next `yield`. `yield from in_order(node.left)` passes on
everything that another generator yields, so each item comes up through every level of
the recursion. `takewhile` stops asking at the first value over 3, and `islice` after
ten Fibonacci numbers. A generator is an iterator, so it can be read only once.

**JavaScript and TypeScript: `function*`.** `function*` declares a generator function,
and `yield*` passes on another generator's items. A generator is an iterator with the
helper methods, so `fibonacci().take(10).toArray()` works, but there is no `takeWhile`,
so the example uses `break`. Leaving a `for...of` loop early calls the generator's
`return` method, which runs any `finally` block inside the generator, so a generator can
clean up after itself.

**Dart: `sync*`.** A function marked `sync*` returns an `Iterable`, and its body runs
only as far as the next `yield` each time a loop asks for an item. `yield*` passes on
another `Iterable`'s items. Dart also has `async*` functions, which return a `Stream`
(chapter 16).

**Kotlin: `sequence { }`.** `sequence { }` builds a `Sequence` from a block that calls
`yield` and `yieldAll`. The block is a coroutine, Kotlin's way to write code that pauses
(chapter 16), and here it pauses only at a `yield`, until the next item is asked for.

**Go: push iterators are generators already.** An `iter.Seq` calls `yield` with each
value, which is a generator's shape with no new syntax. The difference is in what
happens at each `yield`: a generator returns to its caller and pauses, while Go's
`yield` calls the loop body, which runs and returns. The recursion needs one change.
`walk` returns whether to keep going, so that when `yield` returns `false`, every level
returns at once. An iterator that ignores the `false` and calls `yield` again panics
with `range function continued iteration after function for loop body returned false`,
and `go vet` doesn't catch it.

**Rust: no generators yet.** `gen` blocks, Rust's generators, are experimental, and
using one is `error[E0658]: gen blocks are experimental`. The 2024 edition reserves
`gen` as a keyword for them. `std::iter::from_fn` makes an iterator from a closure that
keeps its own state, which is enough for `fibonacci`. The tree walk has no shortcut like
that. `InOrder` keeps a stack of the nodes whose values it hasn't handed out yet.
`push_left` pushes the path down to the leftmost node, and each `next` pops a node and
pushes the path down from its right child. It is the iterator that the recursion would
make if it could pause.

**C++: coroutines, but no generator.** C++20 added coroutines to the language, but no
generator type to use them with, and C++23's `std::generator` isn't in GCC 13, which
this book's checks use. So the example turns the problem round: `fibonacci` and
`in_order` call a `visit` function with each value, and stop when it returns `false`.
That is a push iterator, like Go's, with a template parameter for the visitor.

**C: a function pointer and a context.** C does the same with a function pointer,
`visit_fn`, and a `void *context` for whatever state the visitor needs. `print_some`
keeps the count of values still to print behind its context pointer, and `print_up_to`
keeps the limit there.

---

## 14.7 Loops or pipelines?

Pipelines don't always read better than loops. A pipeline reads well when each step is a
small idea with a name: keep the passes, take their names, join them. Each step can be
read and tested alone, and nothing outside the pipeline changes. A loop reads better in
other cases:

- **Several answers in one pass.** The C version of section 14.4 finds the best score,
  whether everyone passed, and whether anyone scored above 95 in one loop. Three
  pipelines read the scores three times.
- **Steps that depend on what came before**, such as a running total that resets, or a
  parser that changes state. Pipelines can do it, with `scan` or a `fold` that carries a
  tuple, but the loop is usually plainer.
- **Side effects.** A step that prints, writes a file, or changes a variable runs when
  the pipeline is read, which in a lazy pipeline may be later, more than once (Dart and
  Kotlin), or never. Rust's `for_each` and JavaScript's `forEach` exist, but a `for`
  loop says the same thing more plainly.
- **Errors.** A `?` or a `return` inside a closure leaves the closure, not the function
  around it. Rust has an answer for the common case: collecting an iterator of `Result`s
  into a `Result<Vec<_>, _>` stops at the first error, so
  `["1", "x", "3"].iter().map(|s| s.parse::<i32>()).collect::<Result<Vec<i32>, _>>()` is
  `Err(ParseIntError { kind: InvalidDigit })`. In the other languages, a loop is the
  simple way.

Speed is the other question. Rust's adapters are generic structs and C++'s views are
templates, so the compiler sees the whole pipeline and can usually compile it to the
same code as a hand-written loop. In Python, JavaScript, and Dart, each step calls a
function for each item. Kotlin's list functions are `inline`, so the lambda's code is
copied into a loop, but a sequence's functions call it for each item. Wherever a step is
eager, it builds a whole new list, which costs time and memory on a large input. Chapter
24 shows how to measure the difference. Debugging is a smaller cost: a pipeline is one
expression, which is harder to step through, and Rust's `inspect` and Kotlin's `onEach`
help by running a function on each item as it passes.

---

## 14.8 Why each language looks the way it does

- **C** has no closures, so it has no pipelines, and its loops count indexes or move
  pointers. That pointer loop, from `p = a` to `a + n`, is where C++'s iterators came
  from.
- **C++** got iterators with Alexander Stepanov's Standard Template Library, accepted
  into the draft standard in 1994. An iterator generalizes a pointer, so one `std::find`
  works on arrays, lists, and anything else with the same operations. Passing pairs of
  iterators made pipelines awkward, and Eric Niebler's range-v3 library led to the
  ranges and views of C++20. C++23 filled gaps, with `std::ranges::to`, `fold_left`,
  `zip`, `enumerate`, and `std::generator`.
- **Rust** built its `Iterator` trait for laziness, and for what it calls zero-cost
  abstractions: adapters are generic structs, so a whole pipeline can compile like a
  plain loop. Coroutines have been in the compiler for years, as the machinery behind
  `async`, but a stable syntax for generators is still being designed, and the 2024
  edition reserved `gen` for it.
- **Go** has one loop, `for`, and for most of its life `range` worked only on built-in
  types. Go code wrote iterators in several incompatible ways, such as callbacks, `Next`
  methods like `bufio.Scanner`'s `Scan`, and channels, until Go 1.23, in 2024,
  standardized on push functions that `range` can loop over, with `iter.Pull` for code
  that needs a `next`. The `slices` and `maps` packages now return and take iterators,
  but there is still no `Map` or `Filter`, and plain loops remain the usual Go style.
- **Python** added the iterator protocol and generators in Python 2.2 (2001), generator
  expressions in 2.4, and `yield from` in 3.3. List comprehensions came from Haskell.
  Python 3 made `map`, `filter`, `zip`, and `range` lazy, and moved `reduce` into
  `functools`. Guido van Rossum had argued for that in 2005, writing that for a `reduce`
  call with a non-trivial function, "I need to grab pen and paper to diagram what's
  actually being fed into that function".
- **JavaScript** had eager `map`, `filter`, and `reduce` on arrays from ES5 (2009).
  ES2015 added the iterator protocol, `for...of`, and generators, and the iterator
  helpers arrived in ES2025, so lazy pipelines are new to the language.
- **TypeScript** types all of this. TypeScript 5.6 added `IteratorObject`, the type of
  the built-in iterators, which have the helper methods, while `Iterator` describes
  anything with a `next`.
- **Dart** made `Iterable`'s `map`, `where`, and the rest lazy, and `List` inherits
  them, so pipelines on lists are lazy too, unlike JavaScript's and Kotlin's.
- **Kotlin** made its collection functions eager, and laziness a separate type,
  `Sequence`, much as Java 8 (2014) added streams beside its collections. `sequence { }`
  is built on coroutines, which became stable in Kotlin 1.3 (2018).

---

## 14.9 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | Changing a collection while a hand-written iterator walks it | Nothing checks, and the iterator reads whatever is there |
| C++ | `push_back` on a vector inside a range-`for` over it | Undefined behavior: the vector may move, and AddressSanitizer reports `heap-use-after-free` |
| C++ | `take` after `filter` on an endless view | `take` steps one item too far, and the search for that item can run forever |
| C++ | Expensive work in `transform` before a `filter` | It runs twice for every item that passes |
| C++ | Taking a filter view as `const auto&` | Refused: reading a `filter_view` changes it; pass views by value |
| Rust | An adapter with no consumer | Nothing runs, and the compiler warns `unused 'Map' that must be used` |
| Rust | `max_by_key` with ties | Returns the last of the equal items, where Python's `max` and Kotlin's `maxBy` return the first |
| Rust | `let total = items.sum();` | `error[E0283]: type annotations needed`; write `let total: i32` |
| Rust | Changing a `Vec` inside a `for` loop over it | `error[E0502]`: the loop has borrowed it |
| Go | An iterator that ignores `false` from `yield` | Panics with `range function continued iteration after function for loop body returned false` |
| Go | `iter.Pull` without `stop` | The documentation says `stop` must be called unless `next` reached the end; write `defer stop()` |
| Python | Reading a generator twice | The second read gets nothing: `list(squares)` is `[]` |
| Python | `map(print, items)` on its own | Prints nothing, because `map` is lazy |
| Python | Adding to a dictionary while looping over it | `RuntimeError: dictionary changed size during iteration`, while a list being appended to loops over the new items too |
| JavaScript | `reduce` with no starting value, on an empty array | `TypeError: Reduce of empty array with no initial value` |
| JavaScript | `["1", "2", "3"].map(parseInt)` | `[1, NaN, NaN]`: `map` passes the index too, and `parseInt` takes it as the base |
| JavaScript | `sort()` on numbers with no compare function | Sorts them as text: `[10, 9, 1]` becomes `[1, 10, 9]` |
| TypeScript | `map` on a value typed `Iterator<T>` | `Property 'map' does not exist`; type it `IteratorObject<T>`, or wrap it with `Iterator.from` |
| Dart | Reading a mapped `Iterable` more than once | The function runs again each time; call `toList()` once |
| Dart | `for (x in iterator)` | Refused: `for-in` needs an `Iterable` |
| Dart | Adding to a list inside a `for-in` over it | `Concurrent modification during iteration` |
| Kotlin | Long chains of list functions on a large list | Each step builds a new list; switch to `asSequence()` |
| Kotlin | A `Sequence` that nothing reads | Nothing runs, and there is no warning |
| Kotlin | Adding to a list inside a `for` loop over it | `ConcurrentModificationException` |

---

## 14.10 Idiom check

**C: a next function, or a visitor.** Write an iterator as a struct with a
`bool next(struct iter *, T *out)` function, or turn it round and take a function
pointer and a `void *` context to call with each item. Say in a comment how long any
pointer you hand out stays valid. Where the work allows it, answer several questions in
one loop.

**C++: algorithms with projections, views for pipelines.** Use the `std::ranges`
algorithms, with projections such as `&Score::points`, for answers about a whole range,
and views for lazy pipelines. Give your own iterators what `std::input_iterator` asks
for, so that the algorithms and views accept them, and check with a `static_assert`.
Pass views by value, keep expensive work out of a `transform` that comes before a
`filter`, and be careful with `take` after `filter` on an endless view.

**Rust: iterator chains.** Chains of adapters are idiomatic and fast. Implement
`Iterator` for your iterator types and `IntoIterator` for `&YourType`, return
`impl Iterator<Item = T>` from functions that make iterators, and use `collect` into a
`Result` to stop at the first error. Prefer a `for` loop to `for_each` for side effects.

**Go: loops, and iterators for your own types.** Write plain loops for pipelines. Give
your collection types methods that return an `iter.Seq`, named `All`, `Keys`, `Values`,
or `Backward` as the standard library's are, and always return when `yield` returns
`false`. Use the `slices` and `maps` packages for sorting, searching, and collecting,
and write generic `Map` and `Filter` helpers only where a program uses them a lot.

**Python: comprehensions and generators.** Use a list comprehension to build a list, a
generator expression to feed `sum`, `any`, `max`, or `str.join`, and `itertools` for the
rest. Write `__iter__` as a generator, rather than writing an iterator class. Remember
that a generator can be read once.

**JavaScript: array methods for arrays, iterators for the rest.** Use array methods on
arrays, and iterator helpers and generators for large, lazy, or endless sources. Give
`reduce` a starting value and `sort` a compare function, and prefer `toSorted` to
sorting an array that something else uses.

**TypeScript: the same, with iterable types.** Take `Iterable<T>` for input that you
only loop over, so that arrays, sets, maps, and generators are all accepted, and type a
generator's result as `Generator<T>`. Use `IteratorObject<T>` where the code needs the
helper methods.

**Dart: `Iterable` methods, and `toList` when the results are reused.** Chain `map`,
`where`, and the rest, and call `toList()` once when the results are read more than once
or the work is expensive. Write `sync*` functions for your own sequences, and extend
`Iterable` for collection types, which then get every `Iterable` method.

**Kotlin: list functions for small lists, sequences for the rest.** Use the list
functions on ordinary collections, and switch to `asSequence()` for long chains on large
inputs, for endless sources, or when only the first few results are needed. Build
sequences with `sequence { }` and `generateSequence`, and implement `Iterable` for your
own collection types.

---

## 14.11 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Windows and chunks](exercises/01-windows-and-chunks/) | Writing adapters that take any iterator and pull one item at a time |
| 2 | [Lazy pipelines](exercises/02-lazy-pipelines/) | Building a pipeline from stages read as the program runs, over sources that never end |
| 3 | [Grade report](exercises/03-grade-report/) | Grouping, sorting by two keys, and pipelines over records |

```sh
python3 tools/check.py 14                     # your answers, every language
python3 tools/check.py 14 --lang rust,go      # some languages
python3 tools/check.py 14 --solutions         # the reference answers
```

**Stretch:** rewrite exercise 3 in one language with loops only, and in another with
pipelines only. Then add a median score to each subject's line in both. Which version
was easier to change, and which one would you rather read in a year?

---

Previous: [Chapter 13: Generics and polymorphism](../13-generics-and-polymorphism/) ·
Next: Chapter 15: Macros and metaprogramming (coming soon)
