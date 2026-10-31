# Chapter 7: Built-in collections

Almost every program keeps several values together: the lines of a file, the users
signed in, the price of each item in a shop. All nine languages offer the same few
shapes for this: a **list** you can index and grow, a **map** from keys to values, a
**set** of values with no repeats, and a **tuple** that groups a fixed number of values.
What differs is underneath. Some languages copy a list when you assign it and others
share it. Some maps remember the order you added keys and others scramble it on
purpose. And a missing key can give you a default, `null`, an exception, or a new entry
you did not ask for. This chapter compares those choices.

**Languages:** all 9 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| How arrays, growable lists, hash maps, and sets work inside, and what each operation costs | Complexity and benchmarking (chapter 23), hashing (chapter 24) |
| Each language's names for them | Every chapter |
| Which assignments copy a collection and which share it | The memory model and ownership (chapters 9 and 10) |
| What a missing key or a bad index does | Error handling (chapter 11) |
| Which maps and sets keep order, and which values can be keys | Data structures (chapters 24 to 27), JSON (chapter 20) |
| Tuples, and using them as keys | Graphs and grids (chapter 27), dynamic programming (chapter 29) |

---

## 7.1 The concept: four shapes and what they cost

An **array** is a row of values of one type, side by side in memory. Because every
value has the same size, the address of item `i` is the start plus `i` times that size,
so reading or writing any item takes the same short time however long the array is.
That is what "constant time" means, written O(1).

A **growable list** (a vector, slice, `ArrayList`, or JavaScript array) is an array with
spare room at the end. It keeps a **length**, how many items it holds, and a
**capacity**, how many fit before it must grow. Appending writes into the spare room.
When the room runs out, the list allocates a bigger array, usually about twice the size,
and copies everything across. Copying is slow, but it happens rarely enough that
appending costs O(1) on average, which is called **amortized** constant time. Inserting
or removing at the front is different: every later item has to move over by one, so it
costs O(n) for a list of n items.

A **hash map** stores key-value pairs in an array of slots. A **hash function** turns
each key into a number, and that number picks the slot. Finding a key means hashing it
and looking in one slot, or a few if two keys landed in the same place, so lookups,
inserts, and deletes take O(1) on average. The order of the slots follows the hash
values, not the order you added the keys. A **hash set** is a hash map with keys and no
values.

A **sorted map** keeps its keys in order, usually in a balanced tree. Each operation
costs O(log n), a little slower than a hash map, but you can walk the keys in order and
ask for the first key after a given one.

| Operation | Growable list | Hash map or set | Sorted map or set |
| --- | --- | --- | --- |
| Read or write item `i` | O(1) | | |
| Append at the end | O(1) amortized | | |
| Insert or remove at the front | O(n) | | |
| Find a value or key | O(n), a scan | O(1) on average | O(log n) |
| Insert or delete a key | | O(1) on average | O(log n) |
| Walk in order | Order of the items | Unspecified, or insertion order (see 7.5) | Sorted by key |

A **tuple** is a fixed number of values that may have different types, such as a name
and an age. It is not a container you add to. It is one value made of a few parts.

---

## 7.2 What each language calls them

| Language | Fixed-size array | Growable list | Hash map | Sorted map | Hash set | Sorted set | Tuple |
| --- | --- | --- | --- | --- | --- | --- | --- |
| C | `int a[5]` | None | None | None | None | None | A `struct` |
| C++ | `std::array<int, 5>` | `std::vector<int>` | `std::unordered_map` | `std::map` | `std::unordered_set` | `std::set` | `std::pair`, `std::tuple` |
| Rust | `[i32; 5]` | `Vec<i32>` | `HashMap` | `BTreeMap` | `HashSet` | `BTreeSet` | `(i32, &str)` |
| Go | `[5]int` | `[]int`, a slice | `map[string]int` | None | None; use `map[T]bool` or `map[T]struct{}` | None | A `struct` |
| Python | None; `tuple` is a fixed sequence | `list` | `dict` | None in the standard library | `set` | None | `tuple` |
| JavaScript | None; typed arrays such as `Int32Array` are fixed | `Array` | `Map`, or a plain object | None | `Set` | None | An array |
| TypeScript | Tuple types, such as `[number, number]` | `number[]` | `Map<K, V>`, or `Record<K, V>` | None | `Set<T>` | None | `[number, string]` |
| Dart | `List.filled(5, 0)`, which has a fixed length | `List<int>` | `Map`; `HashMap` in `dart:collection` | `SplayTreeMap` | `Set`; `HashSet` | `SplayTreeSet` | Records, such as `(int, String)` |
| Kotlin | `IntArray(5)`, `Array<T>` | `MutableList<T>` | `mutableMapOf()`, `HashMap` | `sortedMapOf()` | `mutableSetOf()`, `HashSet` | `sortedSetOf()` | `Pair`, `Triple`, data classes |

C has arrays and nothing else. Every growable list, map, or set in C is code you write
or a library you add, which is why the C examples in this chapter are the longest. C++
and Rust put the collections in their standard libraries as generic types. Go builds
slices and maps into the language itself, with `[]T` and `map[K]V` syntax, and has no
set. The garbage-collected languages make lists and maps part of everyday syntax:
`[1, 2]` and `{"a": 1}` in Python, `[1, 2]` in JavaScript and Dart, and
`listOf(1, 2)` and `mapOf("a" to 1)` in Kotlin.

**Read-only lists in Kotlin.** Kotlin separates the interface that reads, `List`, from
the one that changes, `MutableList`. `listOf` returns a `List`, so `add` does not
compile. A `List` is a read-only *view*, though, not a frozen list: a `List` variable
pointing at a `MutableList` shows every change made through the other variable.
TypeScript's `readonly number[]` and C++'s `const std::vector<int>&` are views in the
same way.

---

## 7.3 Arrays and lists

The program below makes a list of five numbers, then appends, inserts, removes,
searches, sorts, slices, and reverses it. C has no growable array, so its version
builds one: a pointer, a length, and a capacity that doubles when it runs out, which is
what the other eight do inside. The code is in
[`examples/01-lists/`](examples/01-lists/).

**C**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no growable array, so here is a small one: a pointer to the items, how many
// there are, and how many fit before the memory has to grow.
typedef struct {
    int *items;
    size_t length;
    size_t capacity;
} IntList;

static void reserve(IntList *list, size_t needed) {
    if (needed <= list->capacity) {
        return;
    }
    size_t capacity = list->capacity == 0 ? 4 : list->capacity * 2;  // double it
    while (capacity < needed) {
        capacity *= 2;
    }
    int *items = realloc(list->items, capacity * sizeof *items);
    if (items == NULL) {
        perror("realloc");
        exit(1);
    }
    list->items = items;
    list->capacity = capacity;
}

static void append(IntList *list, int value) {
    reserve(list, list->length + 1);
    list->items[list->length++] = value;
}

static void insert_at(IntList *list, size_t index, int value) {
    reserve(list, list->length + 1);
    // Move everything from index onward one place to the right.
    memmove(&list->items[index + 1], &list->items[index],
            (list->length - index) * sizeof *list->items);
    list->items[index] = value;
    list->length++;
}

static int remove_at(IntList *list, size_t index) {
    int removed = list->items[index];
    memmove(&list->items[index], &list->items[index + 1],
            (list->length - index - 1) * sizeof *list->items);
    list->length--;
    return removed;
}

// Searching is a loop. Returns the index of the first `value`, or -1 if it is missing.
static long index_of(const IntList *list, int value) {
    for (size_t i = 0; i < list->length; i++) {
        if (list->items[i] == value) {
            return (long)i;
        }
    }
    return -1;
}

static void print_items(const char *label, const int *items, size_t count) {
    printf("%s: [", label);
    for (size_t i = 0; i < count; i++) {
        printf(i > 0 ? ", %d" : "%d", items[i]);
    }
    printf("]\n");
}

static int compare_ints(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void) {
    IntList numbers = {NULL, 0, 0};
    const int start[] = {3, 1, 4, 1, 5};
    for (size_t i = 0; i < 5; i++) {
        append(&numbers, start[i]);
    }
    print_items("numbers", numbers.items, numbers.length);
    printf("length: %zu\n", numbers.length);
    printf("first: %d, last: %d\n", numbers.items[0], numbers.items[numbers.length - 1]);

    append(&numbers, 9);
    print_items("append 9", numbers.items, numbers.length);
    insert_at(&numbers, 1, 2);
    print_items("insert 2 at index 1", numbers.items, numbers.length);
    int removed = remove_at(&numbers, 0);
    char label[64];
    snprintf(label, sizeof label, "remove index 0 (was %d)", removed);
    print_items(label, numbers.items, numbers.length);

    printf("contains 4: %s\n", index_of(&numbers, 4) >= 0 ? "yes" : "no");
    printf("index of 1: %ld\n", index_of(&numbers, 1));

    qsort(numbers.items, numbers.length, sizeof *numbers.items, compare_ints);
    print_items("sorted", numbers.items, numbers.length);
    print_items("slice 1..4", numbers.items + 1, 3);  // a pointer and a count, no copy

    for (size_t i = 0, j = numbers.length - 1; i < j; i++, j--) {
        int swap = numbers.items[i];
        numbers.items[i] = numbers.items[j];
        numbers.items[j] = swap;
    }
    print_items("reversed", numbers.items, numbers.length);

    free(numbers.items);
    return 0;
}
```

**C++**

```cpp
#include <algorithm>
#include <iostream>
#include <span>
#include <string>
#include <vector>

// std::span is a view of some elements that live elsewhere: a pointer and a count.
void print(const std::string& label, std::span<const int> items) {
    std::cout << label << ": [";
    for (std::size_t i = 0; i < items.size(); ++i) {
        std::cout << (i > 0 ? ", " : "") << items[i];
    }
    std::cout << "]\n";
}

int main() {
    std::vector<int> numbers = {3, 1, 4, 1, 5};
    print("numbers", numbers);
    std::cout << "length: " << numbers.size() << '\n';
    std::cout << "first: " << numbers.front() << ", last: " << numbers.back() << '\n';

    numbers.push_back(9);
    print("append 9", numbers);
    numbers.insert(numbers.begin() + 1, 2);
    print("insert 2 at index 1", numbers);
    int removed = numbers.front();
    numbers.erase(numbers.begin());
    print("remove index 0 (was " + std::to_string(removed) + ")", numbers);

    bool found = std::ranges::find(numbers, 4) != numbers.end();
    std::cout << "contains 4: " << (found ? "yes" : "no") << '\n';
    auto first_one = std::ranges::find(numbers, 1) - numbers.begin();
    std::cout << "index of 1: " << first_one << '\n';

    std::ranges::sort(numbers);
    print("sorted", numbers);
    print("slice 1..4", std::span(numbers).subspan(1, 3));  // a view, no copy

    std::ranges::reverse(numbers);
    print("reversed", numbers);
}
```

**Rust**

```rust
fn main() {
    let mut numbers = vec![3, 1, 4, 1, 5];
    println!("numbers: {numbers:?}"); // {:?} prints a Vec as [3, 1, 4, 1, 5]
    println!("length: {}", numbers.len());
    // Indexing panics on an empty Vec; first() and last() return an Option instead.
    let (first, last) = (numbers[0], numbers[numbers.len() - 1]);
    println!("first: {first}, last: {last}");

    numbers.push(9);
    println!("append 9: {numbers:?}");
    numbers.insert(1, 2);
    println!("insert 2 at index 1: {numbers:?}");
    let removed = numbers.remove(0);
    println!("remove index 0 (was {removed}): {numbers:?}");

    let has_four = if numbers.contains(&4) { "yes" } else { "no" };
    println!("contains 4: {has_four}");
    let first_one = numbers.iter().position(|&n| n == 1).unwrap();
    println!("index of 1: {first_one}");

    numbers.sort();
    println!("sorted: {numbers:?}");
    println!("slice 1..4: {:?}", &numbers[1..4]); // a borrowed slice, no copy

    numbers.reverse();
    println!("reversed: {numbers:?}");
}
```

**Go**

```go
package main

import (
	"fmt"
	"slices"
	"strings"
)

// fmt.Println prints a slice as [3 1 4], so this helper adds the commas.
func show(items []int) string {
	parts := make([]string, len(items))
	for i, n := range items {
		parts[i] = fmt.Sprint(n)
	}
	return "[" + strings.Join(parts, ", ") + "]"
}

func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func main() {
	numbers := []int{3, 1, 4, 1, 5}
	fmt.Println("numbers:", show(numbers))
	fmt.Println("length:", len(numbers))
	fmt.Printf("first: %d, last: %d\n", numbers[0], numbers[len(numbers)-1])

	// append returns the slice, which may now point at a new, larger array.
	numbers = append(numbers, 9)
	fmt.Println("append 9:", show(numbers))
	numbers = slices.Insert(numbers, 1, 2)
	fmt.Println("insert 2 at index 1:", show(numbers))
	removed := numbers[0]
	numbers = slices.Delete(numbers, 0, 1)
	fmt.Printf("remove index 0 (was %d): %s\n", removed, show(numbers))

	fmt.Println("contains 4:", yesNo(slices.Contains(numbers, 4)))
	fmt.Println("index of 1:", slices.Index(numbers, 1))

	slices.Sort(numbers)
	fmt.Println("sorted:", show(numbers))
	fmt.Println("slice 1..4:", show(numbers[1:4])) // shares the array, no copy

	slices.Reverse(numbers)
	fmt.Println("reversed:", show(numbers))
}
```

**Python**

```python
def main() -> None:
    numbers = [3, 1, 4, 1, 5]
    print("numbers:", numbers)  # a list prints as [3, 1, 4, 1, 5]
    print("length:", len(numbers))
    print(f"first: {numbers[0]}, last: {numbers[-1]}")  # -1 counts from the end

    numbers.append(9)
    print("append 9:", numbers)
    numbers.insert(1, 2)
    print("insert 2 at index 1:", numbers)
    removed = numbers.pop(0)
    print(f"remove index 0 (was {removed}):", numbers)

    print("contains 4:", "yes" if 4 in numbers else "no")
    print("index of 1:", numbers.index(1))  # ValueError if it is missing

    numbers.sort()
    print("sorted:", numbers)
    print("slice 1..4:", numbers[1:4])  # a new list, copied

    numbers.reverse()
    print("reversed:", numbers)


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// console.log prints an array as [ 3, 1, 4 ], so this helper matches the other languages.
const show = (items) => `[${items.join(", ")}]`;

const numbers = [3, 1, 4, 1, 5];
console.log("numbers:", show(numbers));
console.log("length:", numbers.length);
console.log(`first: ${numbers[0]}, last: ${numbers.at(-1)}`); // at(-1) counts from the end

numbers.push(9);
console.log("append 9:", show(numbers));
numbers.splice(1, 0, 2); // at index 1, remove 0 items, insert 2
console.log("insert 2 at index 1:", show(numbers));
const removed = numbers.shift();
console.log(`remove index 0 (was ${removed}):`, show(numbers));

console.log("contains 4:", numbers.includes(4) ? "yes" : "no");
console.log("index of 1:", numbers.indexOf(1)); // -1 if it is missing

numbers.sort((a, b) => a - b); // without a comparator, sort compares as strings
console.log("sorted:", show(numbers));
console.log("slice 1..4:", show(numbers.slice(1, 4))); // a new array, copied

numbers.reverse();
console.log("reversed:", show(numbers));
```

**TypeScript**

```ts
const show = (items: readonly number[]): string => `[${items.join(", ")}]`;

const numbers: number[] = [3, 1, 4, 1, 5];
console.log("numbers:", show(numbers));
console.log("length:", numbers.length);
// at() returns number | undefined, because the array might be empty.
console.log(`first: ${numbers[0]}, last: ${numbers.at(-1)}`);

numbers.push(9);
console.log("append 9:", show(numbers));
numbers.splice(1, 0, 2);
console.log("insert 2 at index 1:", show(numbers));
const removed = numbers.shift();
console.log(`remove index 0 (was ${removed}):`, show(numbers));

console.log("contains 4:", numbers.includes(4) ? "yes" : "no");
console.log("index of 1:", numbers.indexOf(1));

numbers.sort((a, b) => a - b);
console.log("sorted:", show(numbers));
console.log("slice 1..4:", show(numbers.slice(1, 4)));

numbers.reverse();
console.log("reversed:", show(numbers));
```

**Dart**

```dart
void main() {
  final numbers = [3, 1, 4, 1, 5]; // a List<int>, which can grow
  print('numbers: $numbers'); // a List prints as [3, 1, 4, 1, 5]
  print('length: ${numbers.length}');
  print('first: ${numbers.first}, last: ${numbers.last}');

  numbers.add(9);
  print('append 9: $numbers');
  numbers.insert(1, 2);
  print('insert 2 at index 1: $numbers');
  final removed = numbers.removeAt(0);
  print('remove index 0 (was $removed): $numbers');

  print('contains 4: ${numbers.contains(4) ? 'yes' : 'no'}');
  print('index of 1: ${numbers.indexOf(1)}'); // -1 if it is missing

  numbers.sort();
  print('sorted: $numbers');
  print('slice 1..4: ${numbers.sublist(1, 4)}'); // a new list, copied

  final reversed = numbers.reversed.toList(); // reversed is a lazy view
  print('reversed: $reversed');
}
```

**Kotlin**

```kotlin
fun main() {
    // listOf makes a read-only List; mutableListOf makes one you can change.
    val numbers = mutableListOf(3, 1, 4, 1, 5)
    println("numbers: $numbers") // a List prints as [3, 1, 4, 1, 5]
    println("length: ${numbers.size}")
    println("first: ${numbers.first()}, last: ${numbers.last()}")

    numbers.add(9)
    println("append 9: $numbers")
    numbers.add(1, 2) // add(index, value) inserts
    println("insert 2 at index 1: $numbers")
    val removed = numbers.removeAt(0)
    println("remove index 0 (was $removed): $numbers")

    println("contains 4: ${if (4 in numbers) "yes" else "no"}")
    println("index of 1: ${numbers.indexOf(1)}") // -1 if it is missing

    numbers.sort()
    println("sorted: $numbers")
    println("slice 1..4: ${numbers.subList(1, 4)}") // a view of the same list, no copy

    numbers.reverse()
    println("reversed: $numbers")
}
```

Output, identical in all nine:

```text
numbers: [3, 1, 4, 1, 5]
length: 5
first: 3, last: 5
append 9: [3, 1, 4, 1, 5, 9]
insert 2 at index 1: [3, 2, 1, 4, 1, 5, 9]
remove index 0 (was 3): [2, 1, 4, 1, 5, 9]
contains 4: yes
index of 1: 1
sorted: [1, 1, 2, 4, 5, 9]
slice 1..4: [1, 2, 4]
reversed: [9, 5, 4, 2, 1, 1]
```

| Language | Append | Insert at `i` | Remove at `i` | Find | Sort |
| --- | --- | --- | --- | --- | --- |
| C | By hand: grow, then write | By hand, with `memmove` | By hand, with `memmove` | A loop | `qsort` with a comparison function |
| C++ | `v.push_back(x)` | `v.insert(v.begin() + i, x)` | `v.erase(v.begin() + i)` | `std::ranges::find` | `std::ranges::sort(v)` |
| Rust | `v.push(x)` | `v.insert(i, x)` | `v.remove(i)` | `v.contains(&x)`, `iter().position` | `v.sort()` |
| Go | `s = append(s, x)` | `s = slices.Insert(s, i, x)` | `s = slices.Delete(s, i, i+1)` | `slices.Contains`, `slices.Index` | `slices.Sort(s)` |
| Python | `a.append(x)` | `a.insert(i, x)` | `a.pop(i)` | `x in a`, `a.index(x)` | `a.sort()`, or `sorted(a)` for a copy |
| JavaScript, TypeScript | `a.push(x)` | `a.splice(i, 0, x)` | `a.splice(i, 1)` | `a.includes(x)`, `a.indexOf(x)` | `a.sort((x, y) => x - y)` |
| Dart | `a.add(x)` | `a.insert(i, x)` | `a.removeAt(i)` | `a.contains(x)`, `a.indexOf(x)` | `a.sort()` |
| Kotlin | `a.add(x)` | `a.add(i, x)` | `a.removeAt(i)` | `x in a`, `a.indexOf(x)` | `a.sort()`, or `a.sorted()` for a copy |

**Go's `append` returns the slice.** A slice is a small value: a pointer to an array, a
length, and a capacity. When `append` runs out of room, it makes a bigger array and
returns a slice that points at it, so you must always write `s = append(s, x)`. Forget
the assignment and the new item is lost.

**JavaScript sorts as strings.** Without a comparison function, `sort` turns every item
into a string and compares those, so `[10, 9, 1].sort()` gives `[1, 10, 9]`. Always
pass `(a, b) => a - b` for numbers.

**Slices that share and slices that copy.** Rust's `&v[1..4]`, Go's `s[1:4]`, C++'s
`std::span`, and C's pointer plus count are views of the same memory. Kotlin's
`subList` is a view too. Python's `a[1:4]`, JavaScript's `slice`, and Dart's `sublist`
make a new list.

### How a list grows

Each language picks its own growth rule. Here is the capacity after each time it grew,
while appending one `int` at a time to an empty list:

| Language | Capacities |
| --- | --- |
| C++ (GCC's library) | 1, 2, 4, 8, 16, 32, 64, 128 |
| Rust | 4, 8, 16, 32, 64, 128 |
| Go | 1, 2, 4, 8, … 256, 512, then 848, 1280, 1792, 2560: about 1.25 times for large slices |
| Python (CPython 3.13) | 4, 8, 16, 24, 32, 40, 52, 64, 76, 92, 108: about 1.125 times plus a little |
| Kotlin on the JVM | `ArrayList` starts at 10 and grows by half each time |

None of this changes the cost per append, which stays O(1) on average, but it explains
why a list can use more memory than its items need. When you know the final size,
reserve it up front: `v.reserve(n)` in C++, `Vec::with_capacity(n)` in Rust,
`make([]int, 0, n)` in Go, and `ArrayList(n)` in Kotlin.

### An index that is out of range

| Language | `a[10]` on a list of 5 |
| --- | --- |
| C | Undefined behavior: it reads whatever memory is there |
| C++ | `v[10]` is undefined behavior; `v.at(10)` throws `std::out_of_range`: `vector::_M_range_check: __n (which is 10) >= this->size() (which is 5)` |
| Rust | Panic: `index out of bounds: the len is 5 but the index is 10`; `v.get(10)` returns `None` |
| Go | Panic: `index out of range [10] with length 5` |
| Python | `IndexError: list index out of range` |
| JavaScript | `undefined`, with no error |
| TypeScript | `undefined` at run time, though the type says `number`, unless you turn on `--noUncheckedIndexedAccess` |
| Dart | `RangeError (length): Invalid value: Not in inclusive range 0..4: 10` |
| Kotlin | `IndexOutOfBoundsException: Index 10 out of bounds for length 5`; `getOrNull(10)` returns `null` |

TypeScript's choice is a trade-off. With `--noUncheckedIndexedAccess`, `a[10]` has type
`number | undefined`, which is correct but means checking every index, even in a loop
that cannot go wrong. Without it, `const x: number = xs[10]` compiles and
`x.toFixed(1)` fails at run time with `TypeError: Cannot read properties of undefined`.
The book's checker leaves the flag off, as `--strict` does.

### Printing a list

| Language | `print([1, 2, 3])` shows |
| --- | --- |
| C, C++ | Nothing built in; write a loop |
| Rust | `[1, 2, 3]` with `{:?}` |
| Go | `[1 2 3]`, with no commas |
| Python, Dart, Kotlin | `[1, 2, 3]` |
| JavaScript, TypeScript | `[ 1, 2, 3 ]` from `console.log`, but `1,2,3` inside a template string |

Kotlin's *arrays* do not print their items: `println(intArrayOf(1, 2))` shows something
like `[I@1b6d3586`, the type and an identity hash. Call `contentToString()` or convert
to a list.

---

## 7.4 Copy or share?

When you write `b = a` for a list, there are two possibilities. The language can copy
the items, so `a` and `b` are independent, or it can copy only a reference, so both
names point at the same list. The program below tries four things: assigning and then
changing `b`, changing a copy, passing the list to a function that changes it, and
copying a list of lists. The code is in
[`examples/02-copy-or-share/`](examples/02-copy-or-share/).

**C**

```c
#include <stdio.h>
#include <string.h>

static void print_items(const int *items, size_t count) {
    printf("[");
    for (size_t i = 0; i < count; i++) {
        printf(i > 0 ? ", %d" : "%d", items[i]);
    }
    printf("]");
}

// An array parameter is really a pointer, so the function changes the caller's array.
static void set_first(int items[]) { items[0] = -1; }

int main(void) {
    int a[] = {1, 2, 3};

    // An array cannot be assigned, so "b = a" in C means a pointer to the same items.
    int *b = a;
    b[0] = 99;
    printf("b = a, then b[0] = 99: a is ");
    print_items(a, 3);
    printf("\n");

    int c[3];
    memcpy(c, a, sizeof a);  // copies the bytes of all three items
    c[0] = 0;
    printf("c = copy of a, then c[0] = 0: a is ");
    print_items(a, 3);
    printf("\n");

    set_first(a);
    printf("set_first(a) sets a[0] = -1: a is ");
    print_items(a, 3);
    printf("\n");

    // A 2D array is one block of memory, so memcpy copies the rows too.
    int grid[2][2] = {{0, 0}, {0, 0}};
    int g[2][2];
    memcpy(g, grid, sizeof grid);
    g[0][0] = 1;
    printf("g = copy of grid, then g[0][0] = 1: grid is [");
    for (size_t row = 0; row < 2; row++) {
        printf(row > 0 ? ", " : "");
        print_items(grid[row], 2);
    }
    printf("]\n");
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <string>
#include <vector>

std::string show(const std::vector<int>& items) {
    std::string text = "[";
    for (std::size_t i = 0; i < items.size(); ++i) {
        text += (i > 0 ? ", " : "") + std::to_string(items[i]);
    }
    return text + "]";
}

// A reference parameter (&) refers to the caller's vector. Without the &, the
// function would get its own copy.
void set_first(std::vector<int>& items) { items[0] = -1; }

int main() {
    std::vector<int> a = {1, 2, 3};

    std::vector<int> b = a;  // copies every element
    b[0] = 99;
    std::cout << "b = a, then b[0] = 99: a is " << show(a) << '\n';

    auto c = a;  // also a copy: C++ needs no separate copy call
    c[0] = 0;
    std::cout << "c = copy of a, then c[0] = 0: a is " << show(a) << '\n';

    set_first(a);
    std::cout << "set_first(a) sets a[0] = -1: a is " << show(a) << '\n';

    std::vector<std::vector<int>> grid = {{0, 0}, {0, 0}};
    auto g = grid;  // copies the outer vector and every row in it
    g[0][0] = 1;
    std::cout << "g = copy of grid, then g[0][0] = 1: grid is [" << show(grid[0]) << ", "
              << show(grid[1]) << "]\n";
}
```

**Rust**

```rust
// &mut [i32] borrows the caller's items and may change them.
fn set_first(items: &mut [i32]) {
    items[0] = -1;
}

fn main() {
    let mut a = vec![1, 2, 3];

    // `let b = a;` would move the Vec into b, and using `a` afterwards would not
    // compile. To share it, borrow it.
    let b = &mut a;
    b[0] = 99;
    println!("b = a, then b[0] = 99: a is {a:?}");

    let mut c = a.clone(); // an explicit copy
    c[0] = 0;
    println!("c = copy of a, then c[0] = 0: a is {a:?}");

    set_first(&mut a);
    println!("set_first(a) sets a[0] = -1: a is {a:?}");

    let grid = vec![vec![0, 0], vec![0, 0]];
    let mut g = grid.clone(); // clone() copies every row too
    g[0][0] = 1;
    println!("g = copy of grid, then g[0][0] = 1: grid is {grid:?}");
}
```

**Go**

```go
package main

import (
	"fmt"
	"slices"
	"strings"
)

func show(items []int) string {
	parts := make([]string, len(items))
	for i, n := range items {
		parts[i] = fmt.Sprint(n)
	}
	return "[" + strings.Join(parts, ", ") + "]"
}

// A slice is a small value (pointer, length, capacity) that refers to an array, so
// the function changes the caller's items.
func setFirst(items []int) {
	items[0] = -1
}

func main() {
	a := []int{1, 2, 3}

	b := a // copies the slice header, not the items
	b[0] = 99
	fmt.Println("b = a, then b[0] = 99: a is", show(a))

	c := slices.Clone(a)
	c[0] = 0
	fmt.Println("c = copy of a, then c[0] = 0: a is", show(a))

	setFirst(a)
	fmt.Println("set_first(a) sets a[0] = -1: a is", show(a))

	grid := [][]int{{0, 0}, {0, 0}}
	g := slices.Clone(grid) // copies the outer slice; the rows are still shared
	g[0][0] = 1
	fmt.Printf("g = copy of grid, then g[0][0] = 1: grid is [%s, %s]\n", show(grid[0]), show(grid[1]))
}
```

**Python**

```python
def set_first(items: list[int]) -> None:
    items[0] = -1  # the function gets the same list object, not a copy


def main() -> None:
    a = [1, 2, 3]

    b = a  # a second name for the same list
    b[0] = 99
    print("b = a, then b[0] = 99: a is", a)

    c = a.copy()  # or list(a), or a[:]
    c[0] = 0
    print("c = copy of a, then c[0] = 0: a is", a)

    set_first(a)
    print("set_first(a) sets a[0] = -1: a is", a)

    grid = [[0, 0], [0, 0]]
    g = grid.copy()  # a new outer list holding the same two rows
    g[0][0] = 1
    print("g = copy of grid, then g[0][0] = 1: grid is", grid)


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const show = (value) =>
  Array.isArray(value) ? `[${value.map(show).join(", ")}]` : `${value}`;

// The function gets a reference to the same array.
function setFirst(items) {
  items[0] = -1;
}

const a = [1, 2, 3];

const b = a; // another reference to the same array
b[0] = 99;
console.log("b = a, then b[0] = 99: a is", show(a));

const c = [...a]; // spread the items into a new array
c[0] = 0;
console.log("c = copy of a, then c[0] = 0: a is", show(a));

setFirst(a);
console.log("set_first(a) sets a[0] = -1: a is", show(a));

const grid = [
  [0, 0],
  [0, 0],
];
const g = [...grid]; // a new outer array holding the same two rows
g[0][0] = 1;
console.log("g = copy of grid, then g[0][0] = 1: grid is", show(grid));
```

**TypeScript**

```ts
const show = (value: number | number[] | number[][]): string =>
  Array.isArray(value) ? `[${value.map(show).join(", ")}]` : `${value}`;

function setFirst(items: number[]): void {
  items[0] = -1;
}

const a = [1, 2, 3];

const b = a;
b[0] = 99;
console.log("b = a, then b[0] = 99: a is", show(a));

const c = [...a];
c[0] = 0;
console.log("c = copy of a, then c[0] = 0: a is", show(a));

setFirst(a);
console.log("set_first(a) sets a[0] = -1: a is", show(a));

// `const` stops reassigning grid, not changing it; readonly number[][] would stop both.
const grid = [
  [0, 0],
  [0, 0],
];
const g = [...grid];
g[0][0] = 1;
console.log("g = copy of grid, then g[0][0] = 1: grid is", show(grid));
```

**Dart**

```dart
// The function gets a reference to the same list.
void setFirst(List<int> items) {
  items[0] = -1;
}

void main() {
  final a = [1, 2, 3];

  final b = a; // another reference to the same list
  b[0] = 99;
  print('b = a, then b[0] = 99: a is $a');

  final c = [...a]; // or List.of(a)
  c[0] = 0;
  print('c = copy of a, then c[0] = 0: a is $a');

  setFirst(a);
  print('set_first(a) sets a[0] = -1: a is $a');

  final grid = [
    [0, 0],
    [0, 0],
  ];
  final g = [...grid]; // a new outer list holding the same two rows
  g[0][0] = 1;
  print('g = copy of grid, then g[0][0] = 1: grid is $grid');
}
```

**Kotlin**

```kotlin
// The function gets a reference to the same list.
fun setFirst(items: MutableList<Int>) {
    items[0] = -1
}

fun main() {
    val a = mutableListOf(1, 2, 3)

    val b = a // another reference to the same list
    b[0] = 99
    println("b = a, then b[0] = 99: a is $a")

    val c = a.toMutableList() // a new list with the same items
    c[0] = 0
    println("c = copy of a, then c[0] = 0: a is $a")

    setFirst(a)
    println("set_first(a) sets a[0] = -1: a is $a")

    val grid = listOf(mutableListOf(0, 0), mutableListOf(0, 0))
    val g = grid.toList() // a new outer list holding the same two rows
    g[0][0] = 1
    println("g = copy of grid, then g[0][0] = 1: grid is $grid")
}
```

Output:

*C and Rust:*

```text
b = a, then b[0] = 99: a is [99, 2, 3]
c = copy of a, then c[0] = 0: a is [99, 2, 3]
set_first(a) sets a[0] = -1: a is [-1, 2, 3]
g = copy of grid, then g[0][0] = 1: grid is [[0, 0], [0, 0]]
```

*C++:*

```text
b = a, then b[0] = 99: a is [1, 2, 3]
c = copy of a, then c[0] = 0: a is [1, 2, 3]
set_first(a) sets a[0] = -1: a is [-1, 2, 3]
g = copy of grid, then g[0][0] = 1: grid is [[0, 0], [0, 0]]
```

*Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
b = a, then b[0] = 99: a is [99, 2, 3]
c = copy of a, then c[0] = 0: a is [99, 2, 3]
set_first(a) sets a[0] = -1: a is [-1, 2, 3]
g = copy of grid, then g[0][0] = 1: grid is [[1, 0], [0, 0]]
```

| Language | `b = a` | Copy the items | Deep copy |
| --- | --- | --- | --- |
| C | Arrays cannot be assigned; a pointer shares | `memcpy` | By hand, for anything that holds pointers |
| C++ | **Copies** every element | `b = a` | `b = a` copies nested vectors too |
| Rust | **Moves**: `a` can no longer be used | `a.clone()` | `clone()` copies nested `Vec`s too |
| Go | Shares the items (copies the slice header) | `slices.Clone(a)` | By hand |
| Python | Shares | `a.copy()`, `list(a)`, `a[:]` | `copy.deepcopy(a)` |
| JavaScript, TypeScript | Shares | `[...a]`, `a.slice()` | `structuredClone(a)` |
| Dart | Shares | `[...a]`, `List.of(a)` | By hand |
| Kotlin | Shares | `a.toMutableList()` | By hand |

**C++ copies.** A `std::vector` is a value, like an `int`: assignment copies every
element, and so does passing one to a function by value. That is why C++ functions take
`const std::vector<int>&` to read a vector and `std::vector<int>&` to change it, and why
the first two lines of the C++ output differ from the rest.

**Rust moves.** `let b = a;` moves the `Vec` into `b`, and using `a` afterwards is an
error: `E0382: borrow of moved value: 'a'`. Exactly one variable owns the list at a
time, so changing it through one name can never surprise another. To share, borrow it
with `&mut a`, as the example does. To copy, call `clone()`. Chapter 10 explains moves
and borrows.

**Shallow copies share the rows.** In Go, Python, JavaScript, TypeScript, Dart, and
Kotlin, a list of lists holds references to the inner lists, and a plain copy copies
only those references. Changing `g[0][0]` changed `grid[0][0]`. The same trap appears
when building a grid:

| Language | Looks like a 3-by-3 grid | What it is |
| --- | --- | --- |
| Python | `[[0] * 3] * 3` | One row, three times: after `g[0][0] = 1`, it is `[[1, 0, 0], [1, 0, 0], [1, 0, 0]]` |
| JavaScript | `Array(3).fill([])` | One array, three times |
| Dart | `List.filled(3, <int>[])` | One list, three times |

Build each row separately instead: `[[0] * 3 for _ in range(3)]` in Python,
`Array.from({ length: 3 }, () => [])` in JavaScript, and
`List.generate(3, (_) => <int>[])` in Dart.

**Go's two kinds of array.** A Go *array*, such as `[3]int`, is a value: assigning it
copies all three items, and two arrays compare with `==`. A *slice*, `[]int`, shares.
Slices are what Go programs use almost everywhere, and they have one more trap. Two
slices made by `append` from the same slice can share the spare room at its end:

```go
s := make([]int, 3, 10) // length 3, room for 10
a := append(s, 1)
b := append(s, 2)       // writes into the same spare slot as a
fmt.Println(a[3], b[3]) // 2 2
```

---

## 7.5 Maps

The program below keeps the stock of a small shop in a map, reads a key that is there
and one that is not, updates a value, removes a key, and prints the map after each
change. The C version is an array of key-value pairs searched one by one, which is fine
for a handful of keys; chapter 24 builds a real hash table. The code is in
[`examples/03-maps/`](examples/03-maps/).

**C**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no map. For a handful of keys, an array of pairs searched from the start is
// enough; chapter 24 builds a hash table, which stays fast with millions of keys.
typedef struct {
    const char *key;
    int value;
} Entry;

typedef struct {
    Entry entries[16];
    size_t count;
} Map;

// Returns the position of `key` in the array, or -1 if it is not there.
static long index_of(const Map *map, const char *key) {
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            return (long)i;
        }
    }
    return -1;
}

// Returns a pointer to the value stored for `key`, or NULL if there is none.
static int *find(Map *map, const char *key) {
    long i = index_of(map, key);
    return i >= 0 ? &map->entries[i].value : NULL;
}

static void put(Map *map, const char *key, int value) {
    int *existing = find(map, key);
    if (existing != NULL) {
        *existing = value;
    } else if (map->count < sizeof map->entries / sizeof map->entries[0]) {
        map->entries[map->count++] = (Entry){key, value};
    }
}

static void remove_key(Map *map, const char *key) {
    long i = index_of(map, key);
    if (i >= 0) {
        map->entries[i] = map->entries[--map->count];  // move the last entry into the gap
    }
}

static int compare_keys(const void *a, const void *b) {
    return strcmp(((const Entry *)a)->key, ((const Entry *)b)->key);
}

// Prints the entries sorted by key, since removing one changed their order.
static void print_map(const char *label, const Map *map) {
    Map sorted = *map;
    qsort(sorted.entries, sorted.count, sizeof sorted.entries[0], compare_keys);
    printf("%s:", label);
    for (size_t i = 0; i < sorted.count; i++) {
        printf(" %s=%d", sorted.entries[i].key, sorted.entries[i].value);
    }
    printf("\n");
}

int main(void) {
    Map stock = {.count = 0};
    put(&stock, "apple", 3);
    put(&stock, "banana", 5);
    print_map("stock", &stock);

    put(&stock, "cherry", 7);
    print_map("add cherry", &stock);

    printf("banana: %d\n", *find(&stock, "banana"));
    int *durian = find(&stock, "durian");
    printf("durian: %d%s\n", durian != NULL ? *durian : 0, durian != NULL ? "" : " (missing)");

    *find(&stock, "apple") += 2;
    print_map("apple += 2", &stock);
    remove_key(&stock, "banana");
    print_map("remove banana", &stock);

    printf("has cherry: %s\n", find(&stock, "cherry") != NULL ? "yes" : "no");
    printf("size: %zu\n", stock.count);
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <map>
#include <string>

// std::map keeps its keys sorted. std::unordered_map is a hash table: usually faster,
// but its order is unspecified.
void print(const std::string& label, const std::map<std::string, int>& stock) {
    std::cout << label << ':';
    for (const auto& [key, value] : stock) {
        std::cout << ' ' << key << '=' << value;
    }
    std::cout << '\n';
}

int main() {
    std::map<std::string, int> stock = {{"apple", 3}, {"banana", 5}};
    print("stock", stock);

    stock["cherry"] = 7;
    print("add cherry", stock);

    std::cout << "banana: " << stock.at("banana") << '\n';  // at() throws if missing
    // stock["durian"] would add durian with the value 0. find() only looks.
    auto durian = stock.find("durian");
    if (durian == stock.end()) {
        std::cout << "durian: 0 (missing)\n";
    }

    stock["apple"] += 2;
    print("apple += 2", stock);
    stock.erase("banana");
    print("remove banana", stock);

    std::cout << "has cherry: " << (stock.contains("cherry") ? "yes" : "no") << '\n';
    std::cout << "size: " << stock.size() << '\n';
}
```

**Rust**

```rust
use std::collections::HashMap;

// A HashMap's order changes from run to run, so print the keys sorted. A BTreeMap
// keeps them sorted all the time.
fn print(label: &str, stock: &HashMap<&str, i32>) {
    let mut keys: Vec<_> = stock.keys().collect();
    keys.sort();
    print!("{label}:");
    for key in keys {
        print!(" {key}={}", stock[key]);
    }
    println!();
}

fn main() {
    let mut stock = HashMap::from([("apple", 3), ("banana", 5)]);
    print("stock", &stock);

    stock.insert("cherry", 7);
    print("add cherry", &stock);

    println!("banana: {}", stock["banana"]); // indexing panics if the key is missing
    match stock.get("durian") {
        Some(count) => println!("durian: {count}"),
        None => println!("durian: 0 (missing)"),
    }

    *stock.entry("apple").or_insert(0) += 2;
    print("apple += 2", &stock);
    stock.remove("banana");
    print("remove banana", &stock);

    let has = stock.contains_key("cherry");
    println!("has cherry: {}", if has { "yes" } else { "no" });
    println!("size: {}", stock.len());
}
```

**Go**

```go
package main

import (
	"fmt"
	"maps"
	"slices"
)

// Go randomizes the order of a range over a map on purpose, so sort the keys.
func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func printStock(label string, stock map[string]int) {
	fmt.Print(label, ":")
	for _, key := range slices.Sorted(maps.Keys(stock)) {
		fmt.Printf(" %s=%d", key, stock[key])
	}
	fmt.Println()
}

func main() {
	stock := map[string]int{"apple": 3, "banana": 5}
	printStock("stock", stock)

	stock["cherry"] = 7
	printStock("add cherry", stock)

	fmt.Println("banana:", stock["banana"])
	// A missing key gives the zero value. The second result says whether it was there.
	count, ok := stock["durian"]
	if !ok {
		fmt.Printf("durian: %d (missing)\n", count)
	}

	stock["apple"] += 2
	printStock("apple += 2", stock)
	delete(stock, "banana")
	printStock("remove banana", stock)

	_, has := stock["cherry"]
	fmt.Println("has cherry:", yesNo(has))
	fmt.Println("size:", len(stock))
}
```

**Python**

```python
def show(stock: dict[str, int]) -> str:
    # A dict remembers the order its keys were added in.
    return " ".join(f"{key}={value}" for key, value in stock.items())


def main() -> None:
    stock = {"apple": 3, "banana": 5}
    print("stock:", show(stock))

    stock["cherry"] = 7
    print("add cherry:", show(stock))

    print("banana:", stock["banana"])  # KeyError if the key is missing
    if "durian" not in stock:
        print(f"durian: {stock.get('durian', 0)} (missing)")

    stock["apple"] += 2
    print("apple += 2:", show(stock))
    del stock["banana"]
    print("remove banana:", show(stock))

    print("has cherry:", "yes" if "cherry" in stock else "no")
    print("size:", len(stock))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// A Map remembers the order its keys were added in.
const show = (stock) =>
  [...stock].map(([key, value]) => `${key}=${value}`).join(" ");

const stock = new Map([
  ["apple", 3],
  ["banana", 5],
]);
console.log("stock:", show(stock));

stock.set("cherry", 7);
console.log("add cherry:", show(stock));

console.log("banana:", stock.get("banana"));
if (!stock.has("durian")) {
  console.log(`durian: ${stock.get("durian") ?? 0} (missing)`); // get gives undefined
}

stock.set("apple", stock.get("apple") + 2);
console.log("apple += 2:", show(stock));
stock.delete("banana");
console.log("remove banana:", show(stock));

console.log("has cherry:", stock.has("cherry") ? "yes" : "no");
console.log("size:", stock.size);
```

**TypeScript**

```ts
const show = (stock: Map<string, number>): string =>
  [...stock].map(([key, value]) => `${key}=${value}`).join(" ");

const stock = new Map<string, number>([
  ["apple", 3],
  ["banana", 5],
]);
console.log("stock:", show(stock));

stock.set("cherry", 7);
console.log("add cherry:", show(stock));

console.log("banana:", stock.get("banana"));
if (!stock.has("durian")) {
  console.log(`durian: ${stock.get("durian") ?? 0} (missing)`);
}

// get() returns number | undefined, even right after has(), so give a default.
stock.set("apple", (stock.get("apple") ?? 0) + 2);
console.log("apple += 2:", show(stock));
stock.delete("banana");
console.log("remove banana:", show(stock));

console.log("has cherry:", stock.has("cherry") ? "yes" : "no");
console.log("size:", stock.size);
```

**Dart**

```dart
// A map literal makes a LinkedHashMap, which remembers the order keys were added in.
String show(Map<String, int> stock) =>
    stock.entries.map((entry) => '${entry.key}=${entry.value}').join(' ');

void main() {
  final stock = {'apple': 3, 'banana': 5};
  print('stock: ${show(stock)}');

  stock['cherry'] = 7;
  print('add cherry: ${show(stock)}');

  print('banana: ${stock['banana']}');
  if (!stock.containsKey('durian')) {
    // [] gives null for a missing key, and ?? swaps in a default.
    print('durian: ${stock['durian'] ?? 0} (missing)');
  }

  stock.update('apple', (count) => count + 2);
  print('apple += 2: ${show(stock)}');
  stock.remove('banana');
  print('remove banana: ${show(stock)}');

  print('has cherry: ${stock.containsKey('cherry') ? 'yes' : 'no'}');
  print('size: ${stock.length}');
}
```

**Kotlin**

```kotlin
// mutableMapOf makes a LinkedHashMap, which remembers the order keys were added in.
fun show(stock: Map<String, Int>) = stock.entries.joinToString(" ") { "${it.key}=${it.value}" }

fun main() {
    val stock = mutableMapOf("apple" to 3, "banana" to 5)
    println("stock: ${show(stock)}")

    stock["cherry"] = 7
    println("add cherry: ${show(stock)}")

    println("banana: ${stock["banana"]}")
    if ("durian" !in stock) {
        println("durian: ${stock["durian"] ?: 0} (missing)") // [] gives null if missing
    }

    stock["apple"] = stock.getValue("apple") + 2 // getValue throws if missing
    println("apple += 2: ${show(stock)}")
    stock.remove("banana")
    println("remove banana: ${show(stock)}")

    println("has cherry: ${if ("cherry" in stock) "yes" else "no"}")
    println("size: ${stock.size}")
}
```

Output, identical in all nine:

```text
stock: apple=3 banana=5
add cherry: apple=3 banana=5 cherry=7
banana: 5
durian: 0 (missing)
apple += 2: apple=5 banana=5 cherry=7
remove banana: apple=5 cherry=7
has cherry: yes
size: 2
```

### A key that is not there

Reading a missing key is where maps differ most:

| Language | `m["durian"]` | Safe lookup |
| --- | --- | --- |
| C++ | **Adds** `durian` with the value 0, and returns it | `m.find(k)`, `m.contains(k)` (C++20); `m.at(k)` throws `std::out_of_range` with the message `map::at` |
| Rust | Panic: `no entry found for key` | `m.get(k)` returns an `Option` |
| Go | The zero value, 0, and no entry is added | `v, ok := m[k]` |
| Python | `KeyError: 'durian'` | `m.get(k)`, `m.get(k, 0)`, `k in m` |
| JavaScript, TypeScript | `m.get(k)` returns `undefined` | `m.has(k)`; `m.get(k) ?? 0` |
| Dart | `null`, so the type of `m[k]` is `int?` | `m[k] ?? 0`, `m.containsKey(k)` |
| Kotlin | `null`, so the type is `Int?` | `m[k] ?: 0`, `m.getOrDefault(k, 0)`; `m.getValue(k)` throws `NoSuchElementException: Key durian is missing in the map.` |

C++'s `operator[]` returns a reference to the value, so that `m[k] = v` and `m[k] += 1`
work, and it needs something to refer to. If the key is missing, it inserts one with a
default value. That is handy for counting, as in `counts[word]++`, and a surprise
everywhere else: a function that only meant to look can grow the map, and `[]` cannot
be used on a `const` map at all.

Go's zero values make counting just as short, `counts[word]++`, without the surprise,
because reading never inserts. Writing to a map that was declared but never made does
panic: `var m map[string]int` followed by `m["a"] = 1` stops with
`assignment to entry in nil map`. Make it first with `map[string]int{}` or
`make(map[string]int)`.

### Which maps keep order

| Language | Default map walks in | Notes |
| --- | --- | --- |
| C++ | Sorted order for `std::map`; unspecified for `std::unordered_map` | |
| Rust | A different order on each run for `HashMap` | Each `HashMap` gets random hashing keys, which defends against inputs built to collide; `BTreeMap` is sorted |
| Go | A different order from run to run | Go randomizes it on purpose, so that no program comes to depend on it |
| Python | Insertion order | Guaranteed since Python 3.7. Deleting a key and adding it again moves it to the end |
| JavaScript, TypeScript | Insertion order for `Map` | A plain object lists integer-like keys first, in number order: `Object.keys({b: 1, 10: 2, a: 3, 2: 4})` is `["2", "10", "b", "a"]` |
| Dart | Insertion order for a map literal, which is a `LinkedHashMap` | `HashMap` makes no promise; `SplayTreeMap` is sorted |
| Kotlin | Insertion order for `mutableMapOf`, a `LinkedHashMap` | `hashMapOf` makes no promise; `sortedMapOf` is sorted |

Sets follow their maps: Python's `set` does **not** keep insertion order, even though
`dict` does. A set of strings can print in a different order each time you run the
program, because Python randomizes string hashes per process. Small integers hash to
themselves, so `{3, 1, 2}` happens to print as `{1, 2, 3}`, which can make it look
sorted when it is not. JavaScript's `Set`, Dart's set literals, and Kotlin's
`mutableSetOf` keep insertion order.

Go's `fmt.Println(m)` prints a map with its keys sorted, `map[apple:1 banana:2]`, even
though a `for range` loop over the same map does not. The example sorts the keys with
`slices.Sorted(maps.Keys(m))` instead, which works in any loop.

### What can be a key

A hash map needs keys that hash and compare consistently, and a key that changes after
it goes into the map breaks both.

| Language | A list as a key | Other rules |
| --- | --- | --- |
| C++ | `std::map<std::vector<int>, int>` works, since vectors compare with `<` | `std::unordered_map` needs a hash function for any key type the library does not know |
| Rust | `Vec<i32>` works: it implements `Hash` and `Eq` | `f64` is not allowed, because `NaN != NaN` breaks `Eq`: `the trait bound 'f64: Eq' is not satisfied`, `'f64: Hash'` |
| Go | Compile error: `invalid map key type []int` | Arrays and structs of comparable fields are fine |
| Python | `TypeError: unhashable type: 'list'` | Use a `tuple` |
| JavaScript, TypeScript | Allowed, but compared by identity: a second `[1, 2]` is a different key | A plain object turns keys into strings, so `o[[1, 2]]` uses the key `"1,2"` |
| Dart | Allowed, but compared by identity, so `{[1, 2]: 'x'}[[1, 2]]` is `null` | Records compare by value, so `(1, 2)` works |
| Kotlin | Allowed, and compared by contents | Changing a list after using it as a key loses the entry |

**Maps or objects in JavaScript.** Plain objects were JavaScript's maps for its first 20
years, and JSON still uses them. A `Map` is better for data whose keys come from
outside: any value can be a key, it keeps insertion order for every key, `size` is
instant, and no key can collide with a built-in property such as `constructor` or
`__proto__`.

---

## 7.6 Sets and tuples

The program below takes the union, intersection, and difference of two sets, removes
repeats from a list, and uses pairs of numbers as set members to count the distinct
points on a path. The C version stores a set of small numbers as the bits of a 64-bit
integer, and points as a `struct`. The code is in
[`examples/04-sets-and-tuples/`](examples/04-sets-and-tuples/).

**C**

```c
#include <stdint.h>
#include <stdio.h>

// C has no set. For small whole numbers, one bit per number does the job: bit n of a
// 64-bit integer says whether n is in the set, and the bitwise operators do the rest.
typedef uint64_t Set;

static Set with(Set set, int n) { return set | (UINT64_C(1) << n); }
static int has(Set set, int n) { return (set >> n) & 1; }

static void print_set(const char *label, Set set) {
    printf("%s:", label);
    for (int n = 0; n < 64; n++) {
        if (has(set, n)) {
            printf(" %d", n);
        }
    }
    printf("\n");
}

// A struct groups two values, like a tuple. C cannot compare structs with ==.
typedef struct {
    int x, y;
} Point;

static int same_point(Point p, Point q) { return p.x == q.x && p.y == q.y; }

static int contains_point(const Point *points, size_t count, Point p) {
    for (size_t i = 0; i < count; i++) {
        if (same_point(points[i], p)) {
            return 1;
        }
    }
    return 0;
}

int main(void) {
    Set a = with(with(with(with(0, 1), 2), 3), 4);
    Set b = with(with(with(0, 3), 4), 5);
    print_set("a", a);
    print_set("b", b);
    print_set("union", a | b);
    print_set("intersection", a & b);
    print_set("a minus b", a & ~b);
    printf("3 in a: %s\n", has(a, 3) ? "yes" : "no");

    const int values[] = {3, 1, 3, 2, 1};
    Set unique = 0;
    for (size_t i = 0; i < 5; i++) {
        unique = with(unique, values[i]);
    }
    print_set("unique of 3 1 3 2 1, sorted", unique);

    const Point path[] = {{0, 0}, {1, 0}, {0, 0}, {1, 1}};
    Point visited[4];
    size_t count = 0;
    for (size_t i = 0; i < 4; i++) {
        if (!contains_point(visited, count, path[i])) {
            visited[count++] = path[i];
        }
    }
    printf("visited 4 points, %zu distinct\n", count);
    Point target = {1, 0};
    printf("(1, 0) visited: %s\n", contains_point(visited, count, target) ? "yes" : "no");
    return 0;
}
```

**C++**

```cpp
#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>

// std::set keeps its items sorted; std::unordered_set is a hash set.
void print(const std::string& label, const std::set<int>& items) {
    std::cout << label << ':';
    for (int n : items) {
        std::cout << ' ' << n;
    }
    std::cout << '\n';
}

int main() {
    const std::set<int> a = {1, 2, 3, 4};
    const std::set<int> b = {3, 4, 5};
    print("a", a);
    print("b", b);

    // The set algorithms walk two sorted ranges and write the result somewhere.
    std::set<int> all, both, only_a;
    std::ranges::set_union(a, b, std::inserter(all, all.end()));
    std::ranges::set_intersection(a, b, std::inserter(both, both.end()));
    std::ranges::set_difference(a, b, std::inserter(only_a, only_a.end()));
    print("union", all);
    print("intersection", both);
    print("a minus b", only_a);
    std::cout << "3 in a: " << (a.contains(3) ? "yes" : "no") << '\n';

    const std::vector<int> values = {3, 1, 3, 2, 1};
    print("unique of 3 1 3 2 1, sorted", std::set<int>(values.begin(), values.end()));

    // std::pair is a two-item tuple. Pairs compare item by item, so a set can sort them.
    const std::vector<std::pair<int, int>> path = {{0, 0}, {1, 0}, {0, 0}, {1, 1}};
    const std::set<std::pair<int, int>> visited(path.begin(), path.end());
    std::cout << "visited " << path.size() << " points, " << visited.size() << " distinct\n";
    std::cout << "(1, 0) visited: " << (visited.contains({1, 0}) ? "yes" : "no") << '\n';
}
```

**Rust**

```rust
use std::collections::HashSet;

// A HashSet has no order, so sort a copy of the items to print them.
fn print(label: &str, items: &HashSet<i32>) {
    let mut sorted: Vec<_> = items.iter().collect();
    sorted.sort();
    print!("{label}:");
    for n in sorted {
        print!(" {n}");
    }
    println!();
}

fn main() {
    let a = HashSet::from([1, 2, 3, 4]);
    let b = HashSet::from([3, 4, 5]);
    print("a", &a);
    print("b", &b);
    print("union", &(&a | &b)); // the operators work on references to two sets
    print("intersection", &(&a & &b));
    print("a minus b", &(&a - &b));
    println!("3 in a: {}", if a.contains(&3) { "yes" } else { "no" });

    let unique: HashSet<i32> = [3, 1, 3, 2, 1].into_iter().collect();
    print("unique of 3 1 3 2 1, sorted", &unique);

    // Tuples can be set members and map keys, because they hash and compare item by item.
    let path = [(0, 0), (1, 0), (0, 0), (1, 1)];
    let visited: HashSet<(i32, i32)> = path.into_iter().collect();
    println!("visited {} points, {} distinct", path.len(), visited.len());
    let seen = visited.contains(&(1, 0));
    println!("(1, 0) visited: {}", if seen { "yes" } else { "no" });
}
```

**Go**

```go
package main

import (
	"fmt"
	"maps"
	"slices"
)

// Go has no set type. A map whose values are all true does the job.
type set map[int]bool

func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func printSet(label string, items set) {
	fmt.Print(label, ":")
	for _, n := range slices.Sorted(maps.Keys(items)) {
		fmt.Print(" ", n)
	}
	fmt.Println()
}

// A struct with comparable fields can be a map key, so it works like a tuple.
type point struct{ x, y int }

func main() {
	a := set{1: true, 2: true, 3: true, 4: true}
	b := set{3: true, 4: true, 5: true}
	printSet("a", a)
	printSet("b", b)

	union, intersection, difference := set{}, set{}, set{}
	for n := range a {
		union[n] = true
		if b[n] { // a missing key gives false
			intersection[n] = true
		} else {
			difference[n] = true
		}
	}
	for n := range b {
		union[n] = true
	}
	printSet("union", union)
	printSet("intersection", intersection)
	printSet("a minus b", difference)
	fmt.Println("3 in a:", yesNo(a[3]))

	unique := set{}
	for _, n := range []int{3, 1, 3, 2, 1} {
		unique[n] = true
	}
	printSet("unique of 3 1 3 2 1, sorted", unique)

	path := []point{{0, 0}, {1, 0}, {0, 0}, {1, 1}}
	visited := map[point]bool{}
	for _, p := range path {
		visited[p] = true
	}
	fmt.Printf("visited %d points, %d distinct\n", len(path), len(visited))
	fmt.Println("(1, 0) visited:", yesNo(visited[point{1, 0}]))
}
```

**Python**

```python
def show(items: set[int]) -> str:
    return " ".join(str(n) for n in sorted(items))  # a set has no order


def main() -> None:
    a = {1, 2, 3, 4}
    b = {3, 4, 5}
    print("a:", show(a))
    print("b:", show(b))
    print("union:", show(a | b))
    print("intersection:", show(a & b))
    print("a minus b:", show(a - b))
    print("3 in a:", "yes" if 3 in a else "no")

    print("unique of 3 1 3 2 1, sorted:", show(set([3, 1, 3, 2, 1])))

    # Tuples can be set members and dict keys because they cannot change.
    path = [(0, 0), (1, 0), (0, 0), (1, 1)]
    visited = set(path)
    print(f"visited {len(path)} points, {len(visited)} distinct")
    print("(1, 0) visited:", "yes" if (1, 0) in visited else "no")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// A Set keeps insertion order, so sort to print in number order.
const show = (items) => [...items].sort((x, y) => x - y).join(" ");

const a = new Set([1, 2, 3, 4]);
const b = new Set([3, 4, 5]);
console.log("a:", show(a));
console.log("b:", show(b));
console.log("union:", show(a.union(b))); // these three methods arrived in ES2025
console.log("intersection:", show(a.intersection(b)));
console.log("a minus b:", show(a.difference(b)));
console.log("3 in a:", a.has(3) ? "yes" : "no");

console.log("unique of 3 1 3 2 1, sorted:", show(new Set([3, 1, 3, 2, 1])));

// Two arrays are equal only if they are the same object, so [0, 0] and [0, 0] would be
// two different members. Use a string such as "0,0" as the key instead.
const path = [
  [0, 0],
  [1, 0],
  [0, 0],
  [1, 1],
];
const visited = new Set(path.map(([x, y]) => `${x},${y}`));
console.log(`visited ${path.length} points, ${visited.size} distinct`);
console.log("(1, 0) visited:", visited.has("1,0") ? "yes" : "no");
```

**TypeScript**

```ts
const show = (items: Iterable<number>): string =>
  [...items].sort((x, y) => x - y).join(" ");

const a = new Set([1, 2, 3, 4]);
const b = new Set([3, 4, 5]);
console.log("a:", show(a));
console.log("b:", show(b));
console.log("union:", show(a.union(b)));
console.log("intersection:", show(a.intersection(b)));
console.log("a minus b:", show(a.difference(b)));
console.log("3 in a:", a.has(3) ? "yes" : "no");

console.log("unique of 3 1 3 2 1, sorted:", show(new Set([3, 1, 3, 2, 1])));

// A tuple type: exactly two numbers. At run time it is still an array.
type Point = [number, number];
const path: Point[] = [
  [0, 0],
  [1, 0],
  [0, 0],
  [1, 1],
];
const visited = new Set(path.map(([x, y]) => `${x},${y}`));
console.log(`visited ${path.length} points, ${visited.size} distinct`);
console.log("(1, 0) visited:", visited.has("1,0") ? "yes" : "no");
```

**Dart**

```dart
// A set literal makes a LinkedHashSet, which keeps insertion order.
String show(Set<int> items) => (items.toList()..sort()).join(' ');

void main() {
  final a = {1, 2, 3, 4};
  final b = {3, 4, 5};
  print('a: ${show(a)}');
  print('b: ${show(b)}');
  print('union: ${show(a.union(b))}');
  print('intersection: ${show(a.intersection(b))}');
  print('a minus b: ${show(a.difference(b))}');
  print('3 in a: ${a.contains(3) ? 'yes' : 'no'}');

  // A literal like {3, 1, 3} gets a warning for the repeated item, so convert a list.
  print('unique of 3 1 3 2 1, sorted: ${show([3, 1, 3, 2, 1].toSet())}');

  // A record such as (0, 0) is equal to any other record with the same fields, so
  // records work as set members and map keys.
  final path = [(0, 0), (1, 0), (0, 0), (1, 1)];
  final visited = path.toSet();
  print('visited ${path.length} points, ${visited.length} distinct');
  print('(1, 0) visited: ${visited.contains((1, 0)) ? 'yes' : 'no'}');
}
```

**Kotlin**

```kotlin
// setOf makes a LinkedHashSet, which keeps insertion order.
fun show(items: Set<Int>) = items.sorted().joinToString(" ")

fun main() {
    val a = setOf(1, 2, 3, 4)
    val b = setOf(3, 4, 5)
    println("a: ${show(a)}")
    println("b: ${show(b)}")
    println("union: ${show(a union b)}")
    println("intersection: ${show(a intersect b)}")
    println("a minus b: ${show(a subtract b)}")
    println("3 in a: ${if (3 in a) "yes" else "no"}")

    println("unique of 3 1 3 2 1, sorted: ${show(listOf(3, 1, 3, 2, 1).toSet())}")

    // A Pair compares and hashes by its two values, so pairs work as set members.
    val path = listOf(0 to 0, 1 to 0, 0 to 0, 1 to 1)
    val visited = path.toSet()
    println("visited ${path.size} points, ${visited.size} distinct")
    println("(1, 0) visited: ${if ((1 to 0) in visited) "yes" else "no"}")
}
```

Output, identical in all nine:

```text
a: 1 2 3 4
b: 3 4 5
union: 1 2 3 4 5
intersection: 3 4
a minus b: 1 2
3 in a: yes
unique of 3 1 3 2 1, sorted: 1 2 3
visited 4 points, 3 distinct
(1, 0) visited: yes
```

| Language | Union | Intersection | Difference | Add, reporting whether it was new |
| --- | --- | --- | --- | --- |
| C | `\|` on bits | `&` on bits | `& ~` on bits | By hand |
| C++ | `std::ranges::set_union` on sorted sets | `std::ranges::set_intersection` | `std::ranges::set_difference` | `s.insert(x).second` |
| Rust | `&a \| &b` | `&a & &b` | `&a - &b` | `s.insert(x)` returns `bool` |
| Go | A loop over both maps | A loop | A loop | Check, then set |
| Python | `a \| b` | `a & b` | `a - b` | Check, then `add` |
| JavaScript, TypeScript | `a.union(b)` (ES2025) | `a.intersection(b)` | `a.difference(b)` | Check, then `add` |
| Dart | `a.union(b)` | `a.intersection(b)` | `a.difference(b)` | `s.add(x)` returns `bool` |
| Kotlin | `a union b` | `a intersect b` | `a subtract b` | `s.add(x)` returns `Boolean` |

JavaScript's set methods are recent: they reached every major browser in 2024 and the
standard in ES2025. Before that, code wrote `new Set([...a, ...b])` for a union and
`[...a].filter((x) => b.has(x))` for an intersection.

Dart warns about a set literal with a repeated item, such as `{3, 1, 3}`:
`Two elements in a set literal shouldn't be equal.` The example builds that set from a
list instead.

### Tuples

| Language | Make a pair | Read the parts | Compares by value | Usable as a key |
| --- | --- | --- | --- | --- |
| C | `struct Point p = {1, 2};` | `p.x`, `p.y` | No: write a function | Only in code you write |
| C++ | `std::pair{1, 2}`, `std::tuple{1, "a", 2.0}` | `p.first`, `std::get<0>(t)`, `auto [x, y] = p;` | Yes | Yes in `std::map`; `std::unordered_map` needs a hash |
| Rust | `(1, 2)` | `t.0`, `let (x, y) = t;` | Yes | Yes |
| Go | None; use a `struct` | `p.x` | Yes, with `==` | Yes, if every field is comparable |
| Python | `(1, 2)` | `t[0]`, `x, y = t` | Yes | Yes |
| JavaScript | `[1, 2]`, an ordinary array | `t[0]`, `const [x, y] = t;` | **No**: by identity | Only after turning it into a string |
| TypeScript | `[1, 2]` typed as `[number, number]` | The same | **No** | The same |
| Dart | `(1, 2)`, or with names, `(x: 1, y: 2)` | `t.$1`, `t.x`, `var (x, y) = t;` | Yes | Yes |
| Kotlin | `1 to 2` makes a `Pair`; `Triple(1, 2, 3)` | `p.first`, `val (x, y) = p` | Yes | Yes |

A tuple is good for two or three values that belong together for a moment, such as a
function returning a quotient and a remainder (chapter 5). When the values have
meanings worth naming, a struct or a data class says more: `point.x` is clearer than
`point.first` or `point.0`. Chapter 8 covers structs and records.

---

## 7.7 Comparing collections

| Language | `[1, 2] == [1, 2]` | Compare contents |
| --- | --- | --- |
| C | Compares two addresses | A loop, or `memcmp` for plain numbers |
| C++ | `true` for vectors, maps, and sets | `==` |
| Rust | `true` | `==` |
| Go | Compile error for slices: `invalid operation: a == b (slice can only be compared to nil)`. Arrays compare with `==` | `slices.Equal(a, b)`, `maps.Equal(m, n)` |
| Python | `True` | `==` |
| JavaScript, TypeScript | `false`: two different arrays | A loop, or `JSON.stringify` for simple data |
| Dart | `false`: two different lists | `listEquals` in Flutter, or `ListEquality` in the `collection` package |
| Kotlin | `true` for lists, sets, and maps | `==`; but `==` on two arrays compares identity, and Kotlin warns: `'==' on arrays compares only references`. Use `contentEquals` |

The split follows section 7.4. C++ and Rust treat a collection as a value, so `==`
compares the items. In JavaScript and Dart, where assignment shares a reference, `==`
on two lists asks whether both names refer to the same list. Python and Kotlin share
too, but their `==` compares contents, and they keep `is` and `===` for identity. Go
does not compile `==` on slices at all.

---

## 7.8 Changing a collection while you loop over it

Adding or removing items in the middle of a loop over the same collection is a classic
bug. Each language reacts differently:

| Language | What happens |
| --- | --- |
| C++ | Undefined behavior. `push_back` inside a range-`for` over a vector can move the items, and the loop keeps reading the old memory: AddressSanitizer reports `heap-use-after-free`. To remove while looping, use `it = v.erase(it)`, or `std::erase_if` (C++20) |
| Rust | Compile error E0502: `cannot borrow 'v' as mutable because it is also borrowed as immutable`. The loop's borrow lasts until the loop ends |
| Go | Deleting map entries inside `for range` is allowed and safe. Entries added during the loop may or may not be visited |
| Python | Adding to or removing from a dict raises `RuntimeError: dictionary changed size during iteration`, and a set raises `Set changed size during iteration`. A list does not complain, but removing from it skips items: removing every item of `[1, 2, 3, 4]` as you visit it leaves `[2, 4]` |
| JavaScript, TypeScript | No error. A `Map` or `Set` visits entries added during the loop. A `for...of` over an array that you keep pushing to never ends |
| Dart | `Concurrent modification during iteration` |
| Kotlin | `ConcurrentModificationException` |

The safe pattern is the same everywhere: build a new collection, or collect the changes
and apply them after the loop. Many languages have a one-call version for removing:
`retain` in Rust, `slices.DeleteFunc` in Go, a list comprehension in Python, `filter`
in JavaScript, `removeWhere` in Dart, and `removeAll { }` in Kotlin.

---

## 7.9 Why each language looks the way it does

- **C** has the array, and an array name turns into a pointer to its first item almost
  everywhere it is used, which is why a function cannot tell how long an array is.
  C keeps its language and standard library small, so anything more comes from a
  library or from your own code.
- **C++** made containers generic with templates. The Standard Template Library, adopted
  in C++98, separates containers (`vector`, `map`) from algorithms (`sort`, `find`,
  `set_union`) that work on any range of items. Containers are values, so assignment
  copies, which keeps C++'s rule that a variable owns what it holds. Hash tables were
  proposed too late for the 1998 standard, so `std::map` is a sorted tree, and hash
  maps arrived only in C++11, as `unordered_map`.
- **Rust** follows C++'s design, with `Vec` and `HashMap` in the standard library, but
  assignment moves instead of copying, so the cost of a copy is always visible as
  `clone()`. The borrow checker turns C++'s iterator invalidation into a compile error.
  `HashMap` hashes with random keys by default. That is slower than the fastest hash
  functions, but an attacker cannot craft input whose keys all collide.
- **Go** builds slices and maps into the language, because Go had no generics until
  version 1.18 and needed type-safe collections from the start. A slice is a view of an
  array, cheap to pass around and to slice again. Go randomizes map order so that
  programs cannot depend on it. The `slices` and `maps` packages (Go 1.21) added the
  generic helpers that earlier Go code wrote by hand.
- **Python** makes `list` and `dict` part of the syntax, and the language itself is
  built on dicts: every object's attributes and every module's names live in one. A
  new, compact dict design in Python 3.6 happened to keep insertion order, and Python
  3.7 made that a promise.
- **JavaScript** started with objects as maps and arrays that are objects with numeric
  keys, which is why `delete a[1]` leaves a hole and `a.length` stays the same.
  ES2015 added `Map` and `Set`, with real key types and insertion order.
- **TypeScript** adds types to JavaScript's collections, including tuple types and
  `readonly` arrays, but cannot change what they do at run time.
- **Dart** made the insertion-ordered `LinkedHashMap` and `LinkedHashSet` the defaults
  for literals, so that what you write is what you get back, and records (Dart 3) gave
  it tuples that compare by value.
- **Kotlin** uses the JVM's collections (`ArrayList`, `LinkedHashMap`, `HashSet`) and
  adds read-only interfaces in front of them, so that a function can promise not to
  change a list it was given. It also makes `==` call `equals`, so two lists compare
  by contents; in Java, `==` on two lists compares references.

---

## 7.10 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | `sizeof a / sizeof a[0]` on an array parameter | Gives the size of a pointer, not of the array; pass the length separately |
| C | Writing past the end of an array | Undefined behavior, with no error; the classic buffer overflow (chapter 39) |
| C++ | `m[key]` just to check a value | Adds the key with a default value |
| C++ | `push_back` or `erase` while looping over the same vector | Undefined behavior: iterators point at moved or freed memory |
| C++ | Passing a `std::vector` by value | Copies every element; take `const std::vector<T>&` |
| C++ | `std::vector<bool>` | Packs bits, so `&v[0]` is not a `bool*`, and it does not behave like other vectors |
| Rust | `let b = a;` and then using `a` | E0382: `a` was moved |
| Rust | `m[&key]` for a key that may be missing | Panics: `no entry found for key`; use `get` |
| Go | `append(s, x)` without `s =` | The new item is lost if the slice had to grow |
| Go | Two `append`s from the same slice | They can write into the same spare slot |
| Go | Writing to a `var m map[K]V` | Panic: `assignment to entry in nil map` |
| Go | Relying on `for range` order over a map | Changes between runs |
| Python | `[[0] * 3] * 3` | One row repeated three times |
| Python | `x = a.sort()` | `x` is `None`: `sort` changes `a` in place; `sorted(a)` returns a new list |
| Python | `def f(items=[])` | One list shared by every call (chapter 5) |
| Python | Removing from a list while looping over it | Skips items |
| JavaScript | `[10, 9, 1].sort()` | `[1, 10, 9]`: sorts as strings |
| JavaScript | `delete a[i]` | Leaves a hole; the length does not change. Use `splice` |
| JavaScript | Arrays as `Set` members or `Map` keys | Compared by identity, so `[0, 0]` twice is two members |
| JavaScript, Dart | `===` or `==` on two lists | Compares identity, not contents |
| JavaScript, Dart | `Array(3).fill([])`, `List.filled(3, [])` | One list repeated three times |
| TypeScript | `xs[10]` typed as `number` | `undefined` at run time, unless `--noUncheckedIndexedAccess` is on |
| Dart | `add` on `List.filled(5, 0)` | `Unsupported operation: Cannot add to a fixed-length list` |
| Dart | `add` on a `const` list | `Unsupported operation: Cannot add to an unmodifiable list` |
| Kotlin | Treating a `List` as frozen | Another variable holding the `MutableList` can still change it |
| Kotlin | Casting `listOf(...)` to `MutableList` and calling `add` | `UnsupportedOperationException` |
| Kotlin | `==` on two arrays | Compares identity; the compiler warns. Use `contentEquals` |
| Kotlin | `println(array)` | Prints something like `[I@1b6d3586`; use `contentToString()` |
| All | Searching a list in a loop that runs for every item | O(n²); use a set or a map (exercises 1 and 3) |

Kotlin fixed one Java pitfall worth knowing about if you read Java code. In Java,
`list.remove(1)` on a `List<Integer>` removes the item at **index** 1. In Kotlin,
`remove(1)` removes the **value** 1, and `removeAt(1)` removes by index, so
`mutableListOf(5, 1, 7).remove(1)` leaves `[5, 7]`.

---

## 7.11 Idiom check

**C: a struct that knows its length.** Keep the pointer, the length, and the capacity
together, as the example's `IntList` does, and pass the struct around instead of a bare
pointer. Grow by doubling with `realloc`, and check the result for `NULL`. For a real
hash map, use a library or write one as in chapter 24.

**C++: `std::vector` and `std::unordered_map` by default.** Reach for `std::map` only
when you need sorted keys. Take containers by `const&` when reading and `&` when
changing. Use `find` or `contains` to look a key up, and keep `[]` for when adding a
missing key is what you want, as in `counts[word]++`. Call `reserve` when you know the
size.

**Rust: `Vec`, `HashMap`, and the `entry` API.** `*counts.entry(word).or_insert(0) += 1`
looks a key up once, inserts it if it is missing, and updates it. Use `get` for keys
that may be missing, `iter()` to read, `iter_mut()` to change in place, and `retain` to
remove items while keeping the rest.

**Go: slices, maps, and `slices` and `maps`.** Write `s = append(s, x)` every time. Use
`v, ok := m[k]` when a zero value could be a real value. Sort keys with
`slices.Sorted(maps.Keys(m))` when the order matters. Use `map[T]struct{}` for a set
when memory matters, or `map[T]bool` for readability.

**Python: comprehensions and the right container.** `[x * 2 for x in xs]`,
`{k: v for k, v in pairs}`, and `{x for x in xs}` build lists, dicts, and sets in one
line. `collections.Counter` counts, `collections.defaultdict(list)` groups, and
`collections.deque` adds and removes at both ends in O(1). Use tuples for keys.

**JavaScript and TypeScript: `Map` and `Set` for data.** Use them instead of plain
objects when keys come from data. Spread to copy (`[...a]`, `new Map(m)`), and pass a
comparison function to every numeric `sort`. In TypeScript, mark parameters
`readonly T[]` when the function only reads.

**Dart: collection literals with `if` and `for`.** Dart lets you build a list with logic
inside the literal: `[for (final x in xs) if (x > 0) x * 2]`. Use `putIfAbsent` and
`update` on maps, and records for small tuples and composite keys.

**Kotlin: read-only by default.** Use `listOf`, `mapOf`, and `setOf` unless something
must change, and expose `List` rather than `MutableList` from a class. The standard
library covers most loops: `groupBy`, `associate`, `distinct`, `partition`, `zip`, and
`getOrPut` for building a map.

---

## 7.12 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Two Sum](exercises/01-two-sum/) | A map from value to index, one pass instead of two loops, 64-bit arithmetic |
| 2 | [Anagram groups](exercises/02-anagram-groups/) | Building map keys, grouping, maps that keep or lose insertion order |
| 3 | [Remove repeats, keep the order](exercises/03-dedupe/) | Sets for membership, ordered sets, why a list search is slow |

```sh
python3 tools/check.py 07                     # your answers, every language
python3 tools/check.py 07 --lang go,python    # some languages
python3 tools/check.py 07 --solutions         # the reference answers
```

**Stretch:** write exercise 2 in Rust or C++ with a single `BTreeMap` or `std::map`
from each key to its list of words, and print the groups by walking the map. The test
`07-group-order` fails, because a sorted map walks its keys alphabetically, not in the
order they arrived. Then fix it while keeping the sorted map: store the position of each
group's first word next to its words, and sort the groups by that position before
printing.

---

Previous: [Chapter 6: Strings and text](../06-strings-and-text/) ·
Next: [Chapter 8: Structs, enums, and null](../08-structs-enums-and-null/)
