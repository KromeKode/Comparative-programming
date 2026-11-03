# Chapter 13: Generics and polymorphism

Chapter 12 wrote code that works with many types by giving them a shared interface and
looking up each method as the program runs. This chapter covers the other way:
**generics**, code written once with a type left blank, for the compiler to fill in at
each use. A `Stack<T>` works for numbers, words, and anything else, and the type checker
still knows that a `Stack<int>` holds ints. Behind the same idea, the languages make
very different choices. C++ and Rust compile a separate copy of the code for each type.
Kotlin and TypeScript check the types and then throw them away. Go does something in
between. Those choices decide what a constraint can ask for, what the running program
knows about T, and when a list of cats counts as a list of animals. C++, Rust, Go,
TypeScript, and Kotlin get most of the chapter. C, Python, JavaScript, and Dart share
section 13.8.

**Languages:** C++, Rust, Go, TypeScript, and Kotlin in full; C, Python, JavaScript, and
Dart together in section 13.8 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Generic functions and types, and how each language fills in the type | Iterators and functional style (chapter 14), linked lists, stacks, and queues (chapter 25) |
| Constraints: trait bounds, concepts, type sets, and upper bounds | Iterators and functional style (chapter 14), sorting and searching (chapter 28) |
| Giving one name separate code for each type: traits, overloads, and functions passed in | Macros and metaprogramming (chapter 15), files, JSON, and the CLI (chapter 20) |
| Static and dynamic dispatch, and what each one costs | Debugging and profiling (chapter 19), complexity and benchmarking (chapter 23) |
| Variance: when a list of cats counts as a list of animals | Every function that takes a collection of a base type |

---

## 13.1 The concept: one piece of code, many types

**Polymorphism**, Greek for "many shapes", means one piece of code that works with
values of many types. There are three kinds, and this chapter uses all of them:

- **Parametric polymorphism**, usually called **generics**. The code has a **type
  parameter**, T, and does the same thing whatever T is. A stack pushes and pops the
  same way whether it holds numbers or words, so `Stack<T>` is written once, and
  `Stack<int>` and `Stack<string>` are made from it.
- **Ad hoc polymorphism.** One name has separate code for each type. `show(42)` and
  `show("hi")` both work, but showing a number and showing a string are different code.
  Overloads and Rust's traits are two ways to do this.
- **Subtype polymorphism.** Code written for a base type, such as `Animal`, works on
  every subtype, such as `Cat` and `Dog`. That is chapter 12's interfaces and
  inheritance.

Generic code can't do much with a T it knows nothing about. It can store one, pass it
on, and give it back. To compare two T's or add them, it needs a **constraint**, also
called a **bound**: a promise that T has what the code uses. In Rust,
`largest<T: PartialOrd>` means "for any T that can be ordered", and the compiler refuses
to call it with a type that can't be. Constraints are where ad hoc polymorphism comes
back in, because "can be ordered" means that each type brings its own code for `<`.

The other big question is what the compiled program looks like. There are three answers,
and a variation:

```text
 Source: largest<T>, called with ints, doubles, and strings

 One copy per type            One copy per shape             One copy for every type
 (C++, Rust)                  (Go)                           (Kotlin, TypeScript)

 largest<int>                 Largest[int-shaped]            largest(items)
 largest<double>              Largest[float-shaped]            T is checked, then erased
 largest<string>              Largest[string-shaped]
   each calls its own >         each is passed a dictionary    each > is found at run time
                                for the real type
```

- **One copy per type.** C++ and Rust compile a separate copy of `largest` for each T it
  is used with. This is called **monomorphization**. Each copy calls `int`'s `>` or
  `string`'s `>` directly, and can copy its code inline, so generic code runs as fast as
  code written for one type. The price is compile time and program size.
- **One copy for every type.** Kotlin and TypeScript, like Java, check the types, then
  compile one copy that works on any object, and T is gone. This is called **erasure**.
  A Kotlin `List<Int>` and `List<String>` are the same class when the program runs.
- **In between.** Go compiles one copy for each **shape**, so that types laid out the
  same way in memory share code, and passes each call a hidden **dictionary** with what
  the shared code needs to know about the real type.
- **One copy, with the types kept.** Dart compiles one copy too, but keeps the type
  arguments, so a `List<int>` knows that it is a `List<int>`. Generics like Dart's are
  called **reified**.

Python checks its type parameters only in a separate tool, and JavaScript has no types
to fill in.

When the compiler knows which function a call runs, the call is **statically
dispatched**. One copy per type gives you that. When the program looks the function up
as it runs, as chapter 12's interfaces do, the call is **dynamically dispatched**.
Chapter 5 met the same choice in Rust's `impl Fn` and `dyn Fn`, and in C++ between a
template parameter and `std::function`. Section 13.6 puts the two side by side.

---

## 13.2 What each language gives you

| Language | Generic code | Constraints | How it is compiled | Does the running program know T? | Variance |
| --- | --- | --- | --- | --- | --- |
| C | Macros that write code for each type, or `void *` with a size and function pointers | None | A macro makes a copy per type; `void *` code is one copy | No | None |
| C++ | Templates | Concepts (C++20), or none, and then errors come from inside the template | One copy per type | Yes, each copy is compiled for one T | None; `vector<Cat*>` and `vector<Animal*>` are unrelated |
| Rust | Generic types, functions, and `impl` blocks | Trait bounds, checked against the body too | One copy per type | Yes, each copy is compiled for one T | Only for lifetimes, worked out by the compiler |
| Go | Type parameters in square brackets (Go 1.18) | Interfaces, which can also list types | One copy per shape, plus a dictionary | Yes; a type switch on a T value works | None |
| Python | Type parameters for a type checker (3.12 syntax) | Bounds, often a `Protocol` | Ignored when the program runs | No; `Stack[int]` and `Stack[str]` are both `Stack` | Worked out by the type checker |
| JavaScript | Not needed: every function takes any type | None | One copy | There are no types | None |
| TypeScript | Type parameters | `extends` | Erased: one JavaScript function | No | Worked out from how T is used, with holes; `in` and `out` annotations |
| Dart | Type parameters | `extends` | One copy, with the types kept | Yes; `is List<int>` works | Every generic type is covariant, checked when the program runs |
| Kotlin | Type parameters | Upper bounds with `:`, and `where` | Erased on the JVM: one method | No, except in `inline` functions with `reified` | `out` and `in`, where a type is declared or where it is used |

Read the "How it is compiled" column first, because the other columns follow from it.
Where each type gets its own copy, the copy knows its T, so a constraint can ask for
operators and the calls are direct. Where one copy serves every type, T is gone by the
time the program runs, and anything that depends on it has to be passed in or checked by
hand.

---

## 13.3 Generic functions and types

Each version below has a `Stack` with `push`, `pop`, `peek`, and a size; a `largest`
function, which needs items that can be ordered; and a `Pair` with two type parameters,
whose `swap` returns a pair with the types the other way round. `pop` and `peek` on an
empty stack, and `largest` of an empty list, have no item to return, and each language
has its own way to say so.

**C++**

```cpp
#include <cstddef>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// A class template. Stack<int> and Stack<std::string> are two separate
// classes, each made by the compiler from this one definition.
template <typename T>
class Stack {
public:
    void push(T item) { items_.push_back(std::move(item)); }

    std::optional<T> pop() {
        if (items_.empty()) return std::nullopt;
        T top = std::move(items_.back());
        items_.pop_back();
        return top;
    }

    std::optional<T> peek() const {
        if (items_.empty()) return std::nullopt;
        return items_.back();
    }

    std::size_t size() const { return items_.size(); }

private:
    std::vector<T> items_;
};

// A function template. Nothing says T must support >; the compiler finds out
// when it makes a copy of this function for a type (section 13.4).
template <typename T>
std::optional<T> largest(const std::vector<T>& items) {
    if (items.empty()) return std::nullopt;
    T best = items.front();
    for (const T& item : items) {
        if (item > best) best = item;
    }
    return best;
}

// Two type parameters. swap returns a Pair with them the other way round.
template <typename A, typename B>
struct Pair {
    A first;
    B second;

    Pair<B, A> swap() const { return {second, first}; }
};

template <typename A, typename B>
std::ostream& operator<<(std::ostream& out, const Pair<A, B>& pair) {
    return out << "(" << pair.first << ", " << pair.second << ")";
}

template <typename T>
std::string show(const std::optional<T>& value) {
    if (!value) return "none";
    std::ostringstream text;
    text << *value;
    return text.str();
}

template <typename T>
void report(const std::string& name, Stack<T>& stack) {
    std::string popped = show(stack.pop());
    std::string top = show(stack.peek());
    std::cout << name << ": popped " << popped << ", top is " << top << ", "
              << stack.size() << " left\n";
}

int main() {
    Stack<int> numbers;
    numbers.push(3);
    numbers.push(1);
    numbers.push(4);
    report("numbers", numbers);

    Stack<std::string> words;
    words.push("pear");
    words.push("fig");
    report("words", words);

    Stack<double> empty;
    report("empty", empty);

    std::cout << "largest number: " << show(largest(std::vector{3, 1, 4, 1, 5})) << "\n";
    std::vector<std::string> fruit{"pear", "fig", "apple"};
    std::cout << "largest word: " << show(largest(fruit)) << "\n";
    std::cout << "largest of nothing: " << show(largest(std::vector<int>{})) << "\n";

    Pair pair{7, std::string("seven")};  // Pair<int, std::string>, deduced
    std::cout << pair << " swapped is " << pair.swap() << "\n";
}
```

**Rust**

```rust
use std::fmt::Display;

// A generic type. T is a type parameter: each use of Stack fills it in.
struct Stack<T> {
    items: Vec<T>,
}

impl<T> Stack<T> {
    fn new() -> Self {
        Stack { items: Vec::new() }
    }

    fn push(&mut self, item: T) {
        self.items.push(item);
    }

    fn pop(&mut self) -> Option<T> {
        self.items.pop()
    }

    fn peek(&self) -> Option<&T> {
        self.items.last()
    }

    fn len(&self) -> usize {
        self.items.len()
    }
}

// A generic function. The bound T: PartialOrd lets the body use >, and only
// types that can be compared may be passed in.
fn largest<T: PartialOrd>(items: &[T]) -> Option<&T> {
    let mut best = items.first()?;
    for item in items {
        if item > best {
            best = item;
        }
    }
    Some(best)
}

// Two type parameters. swap returns a Pair with them the other way round.
struct Pair<A, B>(A, B);

impl<A, B> Pair<A, B> {
    fn swap(self) -> Pair<B, A> {
        Pair(self.1, self.0)
    }
}

// A Pair can be printed only if both of its parts can.
impl<A: Display, B: Display> Display for Pair<A, B> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "({}, {})", self.0, self.1)
    }
}

fn show<T: Display>(value: Option<T>) -> String {
    match value {
        Some(value) => value.to_string(),
        None => "none".to_string(),
    }
}

fn report<T: Display>(name: &str, stack: &mut Stack<T>) {
    let popped = show(stack.pop());
    let top = show(stack.peek());
    let left = stack.len();
    println!("{name}: popped {popped}, top is {top}, {left} left");
}

fn main() {
    let mut numbers = Stack::new(); // Stack<i32>, worked out from the pushes
    numbers.push(3);
    numbers.push(1);
    numbers.push(4);
    report("numbers", &mut numbers);

    let mut words: Stack<String> = Stack::new();
    words.push("pear".to_string());
    words.push("fig".to_string());
    report("words", &mut words);

    let mut empty: Stack<f64> = Stack::new(); // nothing to infer from, so say it
    report("empty", &mut empty);

    println!("largest number: {}", show(largest(&[3, 1, 4, 1, 5])));
    println!("largest word: {}", show(largest(&["pear", "fig", "apple"])));
    println!("largest of nothing: {}", show(largest::<i32>(&[])));

    let pair = Pair(7, "seven");
    let text = pair.to_string();
    println!("{text} swapped is {}", pair.swap());
}
```

**Go**

```go
package main

import (
	"cmp"
	"fmt"
)

// A generic type. T is a type parameter, and the constraint any lets it be
// any type at all.
type Stack[T any] struct {
	items []T
}

func (s *Stack[T]) Push(item T) {
	s.items = append(s.items, item)
}

// Pop returns the zero value of T and false when the stack is empty.
func (s *Stack[T]) Pop() (T, bool) {
	var zero T
	if len(s.items) == 0 {
		return zero, false
	}
	top := s.items[len(s.items)-1]
	s.items = s.items[:len(s.items)-1]
	return top, true
}

func (s *Stack[T]) Peek() (T, bool) {
	var zero T
	if len(s.items) == 0 {
		return zero, false
	}
	return s.items[len(s.items)-1], true
}

func (s *Stack[T]) Len() int {
	return len(s.items)
}

// A generic function. cmp.Ordered allows the types that < and > work on:
// integers, floats, and strings.
func Largest[T cmp.Ordered](items []T) (T, bool) {
	var zero T
	if len(items) == 0 {
		return zero, false
	}
	best := items[0]
	for _, item := range items {
		if item > best {
			best = item
		}
	}
	return best, true
}

// Two type parameters. Swap returns a Pair with them the other way round.
type Pair[A, B any] struct {
	First  A
	Second B
}

func (p Pair[A, B]) Swap() Pair[B, A] {
	return Pair[B, A]{p.Second, p.First}
}

func (p Pair[A, B]) String() string {
	return fmt.Sprintf("(%v, %v)", p.First, p.Second)
}

func show[T any](value T, ok bool) string {
	if !ok {
		return "none"
	}
	return fmt.Sprint(value)
}

func report[T any](name string, stack *Stack[T]) {
	popped := show(stack.Pop())
	top := show(stack.Peek())
	fmt.Printf("%s: popped %s, top is %s, %d left\n", name, popped, top, stack.Len())
}

func main() {
	var numbers Stack[int]
	numbers.Push(3)
	numbers.Push(1)
	numbers.Push(4)
	report("numbers", &numbers)

	var words Stack[string]
	words.Push("pear")
	words.Push("fig")
	report("words", &words)

	var empty Stack[float64]
	report("empty", &empty)

	fmt.Println("largest number:", show(Largest([]int{3, 1, 4, 1, 5})))
	fmt.Println("largest word:", show(Largest([]string{"pear", "fig", "apple"})))
	fmt.Println("largest of nothing:", show(Largest([]int{})))

	pair := Pair[int, string]{7, "seven"}
	fmt.Println(pair, "swapped is", pair.Swap())
}
```

**TypeScript**

```ts
// A generic class. T is a type parameter: each use of Stack fills it in.
class Stack<T> {
  #items: T[] = [];

  push(item: T): void {
    this.#items.push(item);
  }

  pop(): T | undefined {
    return this.#items.pop();
  }

  peek(): T | undefined {
    return this.#items.at(-1);
  }

  get size(): number {
    return this.#items.length;
  }
}

// A generic function. TypeScript has no "can be ordered" constraint, so this
// one allows numbers and strings, the types that > compares sensibly.
function largest<T extends number | string>(
  items: readonly T[],
): T | undefined {
  let best = items[0];
  for (const item of items) {
    if (best === undefined || item > best) best = item;
  }
  return best;
}

// Two type parameters. swap returns a Pair with them the other way round.
class Pair<A, B> {
  readonly first: A;
  readonly second: B;

  constructor(first: A, second: B) {
    this.first = first;
    this.second = second;
  }

  swap(): Pair<B, A> {
    return new Pair(this.second, this.first);
  }

  toString(): string {
    return `(${this.first}, ${this.second})`;
  }
}

function show<T>(value: T | undefined): string {
  return value === undefined ? "none" : String(value);
}

function report<T>(name: string, stack: Stack<T>): void {
  const popped = show(stack.pop());
  const top = show(stack.peek());
  console.log(`${name}: popped ${popped}, top is ${top}, ${stack.size} left`);
}

const numbers = new Stack<number>();
numbers.push(3);
numbers.push(1);
numbers.push(4);
report("numbers", numbers);

const words = new Stack<string>();
words.push("pear");
words.push("fig");
report("words", words);

const empty = new Stack<number>();
report("empty", empty);

console.log(`largest number: ${show(largest([3, 1, 4, 1, 5]))}`);
console.log(`largest word: ${show(largest(["pear", "fig", "apple"]))}`);
console.log(`largest of nothing: ${show(largest<number>([]))}`);

const pair = new Pair(7, "seven"); // Pair<number, string>, inferred
console.log(`${pair} swapped is ${pair.swap()}`);
```

**Kotlin**

```kotlin
// A generic class. T is a type parameter: each use of Stack fills it in.
class Stack<T> {
    private val items = mutableListOf<T>()

    fun push(item: T) {
        items.add(item)
    }

    fun pop(): T? = items.removeLastOrNull()

    fun peek(): T? = items.lastOrNull()

    val size: Int
        get() = items.size
}

// A generic function. The bound T : Comparable<T> lets the body use >, and
// only types that can be compared may be passed in.
fun <T : Comparable<T>> largest(items: List<T>): T? {
    var best = items.firstOrNull() ?: return null
    for (item in items) {
        if (item > best) best = item
    }
    return best
}

// Kotlin already has Pair<A, B>. A generic extension function adds swap to it.
fun <A, B> Pair<A, B>.swap(): Pair<B, A> = Pair(second, first)

fun <T> show(value: T?): String = value?.toString() ?: "none"

fun <T> report(name: String, stack: Stack<T>) {
    val popped = show(stack.pop())
    val top = show(stack.peek())
    println("$name: popped $popped, top is $top, ${stack.size} left")
}

fun main() {
    val numbers = Stack<Int>()
    numbers.push(3)
    numbers.push(1)
    numbers.push(4)
    report("numbers", numbers)

    val words = Stack<String>()
    words.push("pear")
    words.push("fig")
    report("words", words)

    val empty = Stack<Double>()
    report("empty", empty)

    println("largest number: ${show(largest(listOf(3, 1, 4, 1, 5)))}")
    println("largest word: ${show(largest(listOf("pear", "fig", "apple")))}")
    println("largest of nothing: ${show(largest(emptyList<Int>()))}")

    val pair = Pair(7, "seven") // Pair<Int, String>, inferred
    println("$pair swapped is ${pair.swap()}")
}
```

All nine print the same lines. The C, Python, JavaScript, and Dart versions are in
section 13.8.

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
numbers: popped 4, top is 1, 2 left
words: popped fig, top is pear, 1 left
empty: popped none, top is none, 0 left
largest number: 5
largest word: pear
largest of nothing: none
(7, seven) swapped is (seven, 7)
```

**C++: templates.** `template <typename T>` before a class or a function makes it a
**template**: a pattern, not yet code. `Stack<int>` and `Stack<std::string>` are two
separate classes, each made from the pattern the first time the code uses it, which is
called **instantiation**. The compiler works out a function template's T from the
arguments, so `largest(fruit)` needs no `<std::string>`, and it can do the same for a
class from the values an object is made from: `Pair pair{7, std::string("seven")}` is a
`Pair<int, std::string>`. With no values there is nothing to work from, and
`Stack empty;` fails with `class template argument deduction failed`. `largest` says
nothing about T. Its body uses `>`, and if T has no `>`, the compiler finds out only
when it instantiates the body, as section 13.4 shows. `pop` returns a
`std::optional<T>`, and `std::nullopt` means "none".

**Rust: generics with bounds.** `struct Stack<T>` declares a type parameter, and
`impl<T> Stack<T>` gives the methods to every `Stack<T>`. Rust works T out from how a
value is used: `numbers` is never given a type, and the first `push(3)` makes it a
`Stack<i32>`. Nothing is ever pushed onto `empty`, so it needs its type written out, or
the compiler stops with `error[E0282]: type annotations needed for 'Stack<_>'`. For the
same reason, `largest::<i32>(&[])` names T with `::<>`, which Rust programmers call the
**turbofish**. The bound in `largest<T: PartialOrd>` is what allows `item > best`.
Without it, the body doesn't compile, whatever the callers pass:
`error[E0369]: binary operation '>' cannot be applied to type '&T'`, with a hint to add
the bound. An `impl` block can have bounds too.
`impl<A: Display, B: Display> Display for Pair<A, B>` makes a pair printable only when
both of its parts are. `Pair(7, vec![1, 2])` is still a pair, but `to_string` on it is
`error[E0599]: 'Pair<{integer}, Vec<{integer}>>' doesn't implement 'std::fmt::Display'`.
`peek` returns `Option<&T>`, a borrowed look at the top item, so it copies nothing.

**Go: type parameters in square brackets.** Go added generics in version 1.18.
`Stack[T any]` has one type parameter, T, and its constraint, `any`, allows every type.
A generic type must always be given its T, as in `var numbers Stack[int]`, while generic
functions work it out from their arguments. `Stack{}` alone fails with
`cannot use generic type Stack[T any] without instantiation`. `Largest[T cmp.Ordered]`
uses the standard library's constraint for the types that `<` and `>` work on. Go has no
optional type, so `Pop` returns a T and a `bool`, the way a map lookup does, and the T
it returns for an empty stack is T's zero value, from `var zero T`. `show(stack.Pop())`
passes both results straight on. One rule surprises people: a method can use its type's
parameters, but can't add its own. `func (s *Stack[T]) Map[U any](...)` is refused with
`method must have no type parameters`, so a helper like that is written as a function:
`func Map[T, U any](s *Stack[T], ...)`.

**TypeScript: type parameters, then erasure.** `class Stack<T>` and
`function largest<T>` look like the other languages' generics, and the compiler checks
them the same way, but the types are erased: the JavaScript that runs has one `Stack`
class and no T. TypeScript infers T where it can, and where it can't, it picks
`unknown`, which accepts anything. `const empty = new Stack()` compiles, and then
`empty.push(1)` and `empty.push("x")` both compile too, so the example writes
`new Stack<number>()`. TypeScript has no constraint for "can be ordered", and it doesn't
check `>` closely. With no constraint at all, `item > best` compiles for any T, objects
included, and `largest([{ a: 1 }, { a: 2 }])` returns `{ a: 1 }`. So this `largest`
allows only `T extends number | string`, the types for which `>` means what it says.
"None" is `undefined`, which `pop` and `at(-1)` already return.

**Kotlin: type parameters with upper bounds.** `class Stack<T>` and
`fun <T : Comparable<T>> largest` read like Rust's generics, with `:` before the bound,
which Kotlin calls an **upper bound**. `Comparable<T>` gives `compareTo`, and with it
`>`. Kotlin works T out from the arguments, and needs it written out when there are
none: `val empty = Stack()` fails with
`cannot infer type for type parameter 'T'. Specify it explicitly.` "None" is `null`, so
`pop` returns `T?`, and that has a catch. If T itself allows `null`, a `Stack<String?>`
that holds a `null` pops `null`, and the caller can't tell that from an empty stack.
Kotlin already has `Pair<A, B>`, so this version adds `swap` to it with a generic
**extension function**, `fun <A, B> Pair<A, B>.swap()`.

---

## 13.4 Constraints: what T must be able to do

`largest` needed one thing from T. The next example needs more. `total` adds up a list,
so T must have a `+` and a zero to start from, and `describe_range` needs both ordering
and printing. Each one works on whole numbers, on decimals, and on a `Money` type that
keeps whole cents.

**C++**

```cpp
#include <concepts>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// A concept names a constraint. Summable asks for a + that gives back a T,
// and for T{} to make a zero to start from.
template <typename T>
concept Summable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    T{};
};

template <typename T>
concept Printable = requires(std::ostream& out, const T& value) { out << value; };

// Money keeps whole cents, so adding never rounds.
struct Money {
    long cents = 0;

    Money operator+(Money other) const { return {cents + other.cents}; }
    auto operator<=>(const Money&) const = default;
};

std::ostream& operator<<(std::ostream& out, Money money) {
    long rest = money.cents % 100;
    return out << "$" << money.cents / 100 << "." << (rest < 10 ? "0" : "") << rest;
}

template <Summable T>
T total(const std::vector<T>& items) {
    T sum{};
    for (const T& item : items) sum = sum + item;
    return sum;
}

// Several constraints, joined with && in a requires clause.
template <typename T>
    requires std::totally_ordered<T> && Printable<T>
std::string describe_range(const std::vector<T>& items) {
    T low = items.front(), high = items.front();
    for (const T& item : items) {
        if (item < low) low = item;
        if (item > high) high = item;
    }
    std::ostringstream text;
    text << low << " to " << high;
    return text.str();
}

template <Printable T>
std::string list(const std::vector<T>& items) {
    std::ostringstream text;
    for (std::size_t i = 0; i < items.size(); i++) text << (i ? ", " : "") << items[i];
    return text.str();
}

int main() {
    std::vector numbers{1, 2, 3};
    std::vector halves{0.5, 0.25};
    std::vector prices{Money{150}, Money{225}};
    std::cout << "total of " << list(numbers) << ": " << total(numbers) << "\n";
    std::cout << "total of " << list(halves) << ": " << total(halves) << "\n";
    std::cout << "total of " << list(prices) << ": " << total(prices) << "\n";

    std::vector digits{3, 9, 1};
    std::vector<std::string> fruit{"pear", "fig", "apple"};
    std::vector costs{Money{200}, Money{99}};
    std::cout << "range of " << list(digits) << ": " << describe_range(digits) << "\n";
    std::cout << "range of " << list(fruit) << ": " << describe_range(fruit) << "\n";
    std::cout << "range of " << list(costs) << ": " << describe_range(costs) << "\n";
}
```

**Rust**

```rust
use std::fmt::Display;
use std::ops::Add;

// Money keeps whole cents, so adding never rounds.
#[derive(Clone, Copy, Default, PartialEq, PartialOrd)]
struct Money(i64);

// Implementing Add is what lets Money use +, and what lets it meet total's bound.
impl Add for Money {
    type Output = Money;

    fn add(self, other: Money) -> Money {
        Money(self.0 + other.0)
    }
}

impl Display for Money {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "${}.{:02}", self.0 / 100, self.0 % 100)
    }
}

// The bounds say what total needs from T: a + that gives back a T, a
// default value to start from, and copying.
fn total<T: Add<Output = T> + Default + Copy>(items: &[T]) -> T {
    let mut sum = T::default();
    for &item in items {
        sum = sum + item;
    }
    sum
}

// When there are several bounds, a where clause keeps the signature readable.
fn describe_range<T>(items: &[T]) -> String
where
    T: PartialOrd + Display,
{
    let (mut low, mut high) = (&items[0], &items[0]);
    for item in items {
        if item < low {
            low = item;
        }
        if item > high {
            high = item;
        }
    }
    format!("{low} to {high}")
}

fn list<T: Display>(items: &[T]) -> String {
    let texts: Vec<String> = items.iter().map(|item| item.to_string()).collect();
    texts.join(", ")
}

fn main() {
    let numbers = [1, 2, 3];
    let halves = [0.5, 0.25];
    let prices = [Money(150), Money(225)];
    println!("total of {}: {}", list(&numbers), total(&numbers));
    println!("total of {}: {}", list(&halves), total(&halves));
    println!("total of {}: {}", list(&prices), total(&prices));

    let digits = [3, 9, 1];
    let fruit = ["pear", "fig", "apple"];
    let costs = [Money(200), Money(99)];
    println!("range of {}: {}", list(&digits), describe_range(&digits));
    println!("range of {}: {}", list(&fruit), describe_range(&fruit));
    println!("range of {}: {}", list(&costs), describe_range(&costs));
}
```

**Go**

```go
package main

import (
	"cmp"
	"fmt"
	"strings"
)

// A constraint can list types. ~int64 means int64 and every type whose
// underlying type is int64, such as Cents below.
type Number interface {
	~int | ~int64 | ~float64
}

// Cents keeps whole cents, so adding never rounds. Its underlying type is
// int64, so + already works on it.
type Cents int64

func (c Cents) String() string {
	return fmt.Sprintf("$%d.%02d", c/100, c%100)
}

// var sum T starts at the zero value, which for every Number is 0.
func Total[T Number](items []T) T {
	var sum T
	for _, item := range items {
		sum += item
	}
	return sum
}

// cmp.Ordered covers ~int, ~string, ~float64, and the rest, so Cents meets
// it too. fmt.Sprint finds the String method when the program runs.
func DescribeRange[T cmp.Ordered](items []T) string {
	low, high := items[0], items[0]
	for _, item := range items {
		low = min(low, item)
		high = max(high, item)
	}
	return fmt.Sprint(low, " to ", high)
}

func list[T any](items []T) string {
	texts := make([]string, len(items))
	for i, item := range items {
		texts[i] = fmt.Sprint(item)
	}
	return strings.Join(texts, ", ")
}

func main() {
	numbers := []int{1, 2, 3}
	halves := []float64{0.5, 0.25}
	prices := []Cents{150, 225}
	fmt.Printf("total of %s: %v\n", list(numbers), Total(numbers))
	fmt.Printf("total of %s: %v\n", list(halves), Total(halves))
	fmt.Printf("total of %s: %v\n", list(prices), Total(prices))

	digits := []int{3, 9, 1}
	fruit := []string{"pear", "fig", "apple"}
	costs := []Cents{200, 99}
	fmt.Printf("range of %s: %s\n", list(digits), DescribeRange(digits))
	fmt.Printf("range of %s: %s\n", list(fruit), DescribeRange(fruit))
	fmt.Printf("range of %s: %s\n", list(costs), DescribeRange(costs))
}
```

**TypeScript**

```ts
// Money keeps whole cents, so adding never rounds.
class Money {
  readonly cents: number;

  constructor(cents: number) {
    this.cents = cents;
  }

  plus(other: Money): Money {
    return new Money(this.cents + other.cents);
  }

  toString(): string {
    const rest = String(this.cents % 100).padStart(2, "0");
    return `$${Math.trunc(this.cents / 100)}.${rest}`;
  }
}

// A constraint can't ask for +, because only numbers, bigints, and strings
// have it. So total takes an Adder: an object that knows how to add Ts.
interface Adder<T> {
  zero: T;
  add(a: T, b: T): T;
}

const numberAdder: Adder<number> = { zero: 0, add: (a, b) => a + b };
const moneyAdder: Adder<Money> = {
  zero: new Money(0),
  add: (a, b) => a.plus(b),
};

function total<T>(items: readonly T[], adder: Adder<T>): T {
  let sum = adder.zero;
  for (const item of items) sum = adder.add(sum, item);
  return sum;
}

// A compare function plays the same part for ordering, as it does for sort.
type Compare<T> = (a: T, b: T) => number;

const byValue: Compare<number | string> = (a, b) =>
  a < b ? -1 : a > b ? 1 : 0;
const byCents: Compare<Money> = (a, b) => a.cents - b.cents;

function describeRange<T>(items: readonly T[], compare: Compare<T>): string {
  let [low, high] = [items[0], items[0]];
  for (const item of items) {
    if (compare(item, low) < 0) low = item;
    if (compare(item, high) > 0) high = item;
  }
  return `${low} to ${high}`;
}

function list<T>(items: readonly T[]): string {
  return items.map(String).join(", ");
}

const numbers = [1, 2, 3];
const halves = [0.5, 0.25];
const prices = [new Money(150), new Money(225)];
console.log(`total of ${list(numbers)}: ${total(numbers, numberAdder)}`);
console.log(`total of ${list(halves)}: ${total(halves, numberAdder)}`);
console.log(`total of ${list(prices)}: ${total(prices, moneyAdder)}`);

const digits = [3, 9, 1];
const fruit = ["pear", "fig", "apple"];
const costs = [new Money(200), new Money(99)];
console.log(`range of ${list(digits)}: ${describeRange(digits, byValue)}`);
console.log(`range of ${list(fruit)}: ${describeRange(fruit, byValue)}`);
console.log(`range of ${list(costs)}: ${describeRange(costs, byCents)}`);
```

**Kotlin**

```kotlin
// Money keeps whole cents, so adding never rounds. Comparable<Money> gives it
// an order, and with it <, >, and the bound on describeRange below.
data class Money(val cents: Long) : Comparable<Money> {
    operator fun plus(other: Money) = Money(cents + other.cents)

    override fun compareTo(other: Money) = cents.compareTo(other.cents)

    override fun toString() = "$" + cents / 100 + "." + (cents % 100).toString().padStart(2, '0')
}

// No interface says "has +": Int's plus and Money's plus are unrelated
// functions. So total takes an Adder, an object that knows how to add Ts.
interface Adder<T> {
    val zero: T

    fun add(a: T, b: T): T
}

object IntAdder : Adder<Int> {
    override val zero = 0

    override fun add(a: Int, b: Int) = a + b
}

object DoubleAdder : Adder<Double> {
    override val zero = 0.0

    override fun add(a: Double, b: Double) = a + b
}

object MoneyAdder : Adder<Money> {
    override val zero = Money(0)

    override fun add(a: Money, b: Money) = a + b
}

fun <T> total(items: List<T>, adder: Adder<T>): T = items.fold(adder.zero, adder::add)

// Int, Double, and String are already Comparable to themselves.
fun <T : Comparable<T>> describeRange(items: List<T>): String {
    var (low, high) = items[0] to items[0]
    for (item in items) {
        if (item < low) low = item
        if (item > high) high = item
    }
    return "$low to $high"
}

fun <T> list(items: List<T>) = items.joinToString(", ")

fun main() {
    val numbers = listOf(1, 2, 3)
    val halves = listOf(0.5, 0.25)
    val prices = listOf(Money(150), Money(225))
    println("total of ${list(numbers)}: ${total(numbers, IntAdder)}")
    println("total of ${list(halves)}: ${total(halves, DoubleAdder)}")
    println("total of ${list(prices)}: ${total(prices, MoneyAdder)}")

    val digits = listOf(3, 9, 1)
    val fruit = listOf("pear", "fig", "apple")
    val costs = listOf(Money(200), Money(99))
    println("range of ${list(digits)}: ${describeRange(digits)}")
    println("range of ${list(fruit)}: ${describeRange(fruit)}")
    println("range of ${list(costs)}: ${describeRange(costs)}")
}
```

All five print the same lines:

*C++, Rust, Go, TypeScript and Kotlin:*

```text
total of 1, 2, 3: 6
total of 0.5, 0.25: 0.75
total of $1.50, $2.25: $3.75
range of 3, 9, 1: 1 to 9
range of pear, fig, apple: apple to pear
range of $2.00, $0.99: $0.99 to $2.00
```

**Rust: traits are the constraints.** Rust's operators are traits, `Add` for `+` and
`PartialOrd` for `<`, and so is printing with `{}`, which is `Display`. So a bound can
ask for any of them. `T: Add<Output = T> + Default + Copy` asks for a `+` that gives
back a T, a `T::default()` to start the sum from, and permission to copy items out of
the slice. `Money` meets the bound because it implements `Add` and derives `Default` and
`Copy`. A type that doesn't is refused at the call, with the reason:

```text
error[E0277]: cannot add `Point` to `Point`
...
note: required by a bound in `total`
```

The bounds are checked in both directions. A caller must pass a type that meets them,
and the body may use only what they promise, which is checked before any caller exists.
That is why the error is at the call and never inside `total`. When there are several
bounds, a `where` clause after the signature keeps it readable, as `describe_range`
shows.

**C++: concepts, or errors from inside the template.** A C++20 **concept** names a set
of requirements, written as expressions that must compile. `Summable` requires that
`a + b` compiles and gives something that converts to T, and that `T{}` makes a value.
`template <Summable T>` checks each caller's type against it, and the error is about the
call:

```text
error: no matching function for call to 'total(std::vector<Point>&)'
note: constraints not satisfied
note: the required expression '(a + b)' is invalid
```

Requirements can be joined with `&&` in a `requires` clause, as `describe_range` joins
the standard `std::totally_ordered` and the example's own `Printable`. Without a
concept, `template <typename T>` accepts every type, and the error comes from inside the
body, wherever `+` is used:
`no match for 'operator+' (operand types are 'Point' and 'const Point')`, after a
`required from here` line that points at the call. For a template deep in a library,
that can be pages of text. Unlike Rust, C++ doesn't check the body against the concept.
A `template <Summable T>` function whose body uses `a * a` compiles, and runs for `int`,
and fails inside the body for a type that has `+` but not `*`.

**Go: interfaces with type sets.** A Go constraint is an interface, and an interface can
list types. `Number` allows `~int | ~int64 | ~float64`, where `~int64` means "`int64`
and every type whose underlying type is `int64`". So `Cents`, declared as
`type Cents int64`, is a `Number` too. Without the `~`, Go says
`Cents does not satisfy Number (possibly missing ~ for int64 in Number)`. Operators come
from the type set: `+` is allowed on a T because every type in the set has it. Go has no
operator overloading, so that is the only way to get an operator, and a struct can never
be a `Number`:
`Point does not satisfy Number (Point missing in ~int | ~int64 | ~float64)`. Under
`any`, `sum += item` is
`invalid operation: operator + not defined on sum (variable of type T constrained by any)`.
An interface that lists types can only be a constraint, so `var n Number` fails with
`cannot use type Number outside a type constraint: interface contains type constraints`.
`cmp.Ordered` is the standard library's type set for `<`, strings included, and
`DescribeRange` uses the built-in `min` and `max` on it. Printing needs no constraint,
because `fmt.Sprint` finds `Cents`'s `String` method as the program runs.

**TypeScript and Kotlin: pass in an object that knows how.** Neither language can ask
for `+` in a constraint. In TypeScript, only numbers, bigints, and strings have `+`, and
`a + b` on a T is `error TS2365: Operator '+' cannot be applied to types 'T' and 'T'.`
In Kotlin, `Int.plus` and `Money.plus` are unrelated functions that happen to share a
name, and no interface covers both, so `sum + item` is refused with
`none of the following candidates is applicable`. Both do by hand what a Rust bound
does: `total` takes an `Adder<T>`, an object that holds a zero and an `add` function.
Ordering is easier in Kotlin, where `Int`, `Double`, and `String` already implement
`Comparable`, and `Money` does too, so `describeRange` has a bound. TypeScript passes a
compare function, the same shape that `sort` takes.

**The object is always there.** An `Adder` is less a workaround than what the other
languages do behind the scenes. The bound `T: Add` says that T comes with an `add`
function, and the compiler finds it from the type and builds it into the copy of `total`
for that T. Haskell, where Rust's traits come from, usually passes such a table of
functions as a hidden argument, which is called a **dictionary**, and Go passes one too
(section 13.6). TypeScript and Kotlin make you pass it yourself. That has one upside: a
type can have more than one, such as an order by name and another by age.

---

## 13.5 One name, a body per type

Sometimes the code really is different for each type. `show` below writes a number as
digits, a string in quotes, a point as `(x, y)`, and a list as its items in brackets,
each item shown the same way. Lists of lists must work too.

**C++**

```cpp
#include <iostream>
#include <string>
#include <vector>

struct Point {
    int x, y;
};

// Overloads: one name, a separate function for each type. The compiler picks
// one from the type of the argument.
std::string show(int value) { return std::to_string(value); }

std::string show(bool value) { return value ? "true" : "false"; }

std::string show(const std::string& value) { return "\"" + value + "\""; }

std::string show(Point point) {
    return "(" + std::to_string(point.x) + ", " + std::to_string(point.y) + ")";
}

// A template covers every vector whose items have a show, including vectors
// of vectors, which call this same template one level down.
template <typename T>
std::string show(const std::vector<T>& items) {
    std::string text = "[";
    for (std::size_t i = 0; i < items.size(); i++) {
        if (i > 0) text += ", ";
        text += show(items[i]);
    }
    return text + "]";
}

int main() {
    using namespace std::string_literals;  // "hi"s is a std::string, not a char array
    std::cout << show(42) << "\n";
    std::cout << show("hi"s) << "\n";
    std::cout << show(true) << "\n";
    std::cout << show(std::vector{1, 2, 3}) << "\n";
    std::cout << show(std::vector<std::vector<std::string>>{{"a", "b"}, {}, {"c"}}) << "\n";
    std::cout << show(Point{1, 2}) << "\n";
    std::cout << show(std::vector<Point>{{1, 2}, {3, 4}}) << "\n";
}
```

**Rust**

```rust
// One trait, implemented separately for each type. The compiler picks the
// implementation from the type, so each type gets its own code.
trait Show {
    fn show(&self) -> String;
}

impl Show for i32 {
    fn show(&self) -> String {
        self.to_string()
    }
}

impl Show for bool {
    fn show(&self) -> String {
        self.to_string()
    }
}

impl Show for &str {
    fn show(&self) -> String {
        format!("\"{self}\"")
    }
}

struct Point {
    x: i32,
    y: i32,
}

impl Show for Point {
    fn show(&self) -> String {
        format!("({}, {})", self.x, self.y)
    }
}

// One impl for every Vec whose items can be shown, so Vec<i32>,
// Vec<Vec<&str>>, and Vec<Point> all get show from these lines.
impl<T: Show> Show for Vec<T> {
    fn show(&self) -> String {
        let items: Vec<String> = self.iter().map(|item| item.show()).collect();
        format!("[{}]", items.join(", "))
    }
}

fn main() {
    println!("{}", 42.show());
    println!("{}", "hi".show());
    println!("{}", true.show());
    println!("{}", vec![1, 2, 3].show());
    println!("{}", vec![vec!["a", "b"], vec![], vec!["c"]].show());
    println!("{}", Point { x: 1, y: 2 }.show());
    let points = vec![Point { x: 1, y: 2 }, Point { x: 3, y: 4 }];
    println!("{}", points.show());
}
```

**Go**

```go
package main

import (
	"fmt"
	"strconv"
	"strings"
)

type Point struct{ X, Y int }

// Go can't choose a function by a type parameter, and can't add methods to
// int or string. So each way of showing a type is a function value...
func showInt(n int) string       { return strconv.Itoa(n) }
func showBool(b bool) string     { return strconv.FormatBool(b) }
func showString(s string) string { return strconv.Quote(s) }
func showPoint(p Point) string   { return fmt.Sprintf("(%d, %d)", p.X, p.Y) }

// ...and listOf builds the function for a list out of the one for its items.
func listOf[T any](showItem func(T) string) func([]T) string {
	return func(items []T) string {
		texts := make([]string, len(items))
		for i, item := range items {
			texts[i] = showItem(item)
		}
		return "[" + strings.Join(texts, ", ") + "]"
	}
}

func main() {
	fmt.Println(showInt(42))
	fmt.Println(showString("hi"))
	fmt.Println(showBool(true))
	fmt.Println(listOf(showInt)([]int{1, 2, 3}))
	fmt.Println(listOf(listOf(showString))([][]string{{"a", "b"}, {}, {"c"}}))
	fmt.Println(showPoint(Point{1, 2}))
	fmt.Println(listOf(showPoint)([]Point{{1, 2}, {3, 4}}))
}
```

**TypeScript**

```ts
class Point {
  readonly x: number;
  readonly y: number;

  constructor(x: number, y: number) {
    this.x = x;
    this.y = y;
  }
}

// The types are gone when the program runs, so one function checks what it
// was given. The union type lists everything it knows how to show.
type Showable = number | boolean | string | Point | Showable[];

function show(value: Showable): string {
  if (typeof value === "string") return `"${value}"`;
  if (value instanceof Point) return `(${value.x}, ${value.y})`;
  if (Array.isArray(value)) return `[${value.map(show).join(", ")}]`;
  return String(value);
}

console.log(show(42));
console.log(show("hi"));
console.log(show(true));
console.log(show([1, 2, 3]));
console.log(show([["a", "b"], [], ["c"]]));
console.log(show(new Point(1, 2)));
console.log(show([new Point(1, 2), new Point(3, 4)]));
```

**Kotlin**

```kotlin
data class Point(val x: Int, val y: Int)

// Overloads: one name, a separate function for each type. The compiler picks
// one from the type of the argument.
fun show(value: Int) = value.toString()

fun show(value: Boolean) = value.toString()

fun show(value: String) = "\"$value\""

fun show(value: Point) = "(${value.x}, ${value.y})"

// Inside a generic function there is no show(item) for an unknown T, so the
// caller passes in the one to use.
fun <T> show(items: List<T>, showItem: (T) -> String) =
    items.joinToString(", ", "[", "]", transform = showItem)

fun main() {
    println(show(42))
    println(show("hi"))
    println(show(true))
    println(show(listOf(1, 2, 3), ::show))
    println(show(listOf(listOf("a", "b"), listOf(), listOf("c"))) { show(it, ::show) })
    println(show(Point(1, 2)))
    println(show(listOf(Point(1, 2), Point(3, 4)), ::show))
}
```

All five print the same lines:

*C++, Rust, Go, TypeScript and Kotlin:*

```text
42
"hi"
true
[1, 2, 3]
[["a", "b"], [], ["c"]]
(1, 2)
[(1, 2), (3, 4)]
```

**Rust: a trait implemented for each type.** This is the Rust answer to overloading that
chapter 5 promised. `trait Show` declares the method, and each `impl Show for` block
gives one type its code, including types from the standard library such as `i32` and
`&str`. The last one is generic: `impl<T: Show> Show for Vec<T>` gives `show` to a
vector of anything that has one, so `Vec<Vec<&str>>` works with no more code. One rule
limits who may write an impl, the **orphan rule**: the trait or the type must belong to
your crate. `Show` is the program's own, so `impl Show for i32` is fine, but
`impl fmt::Display for Vec<i32>` implements a standard trait for a standard type, and is
refused with
`error[E0117]: only traits defined in the current crate can be implemented for types defined outside of the crate`.
Without the rule, two crates could each implement `Display` for `Vec<i32>`, and a
program that used both would have two answers. The usual way round it is a **newtype**:
a struct of your own that wraps the type.

**C++: overloads, and a template for the rest.** C++ picks among functions with the same
name by the types of the arguments (chapter 5). The template for `std::vector<T>` calls
`show(items[i])`, which picks the overload for the item type when the template is
instantiated, and for a vector of vectors, that is the template itself. There are two
traps. First, a string literal such as `"hi"` is a `const char *`, not a `std::string`,
and C++ prefers turning a pointer into a `bool` over building a `std::string`, so
`show("hi")` prints `true`, with no warning even under `-Wall -Wextra`. The `s` suffix
in `"hi"s` makes a `std::string`. Second, order matters. A call inside a template sees
the functions declared above the template, plus those in the namespace of the argument's
type, which for `std::string` is `std`. Put the template above
`show(const std::string&)`, and showing a vector of strings fails with
`no matching function for call to 'show(const std::__cxx11::basic_string<char>&)'`.

**Go: functions as values.** Go has none of the tools above. It has no overloading, a
generic function can't have a separate body for one T, and methods can't be added to
`int` or `string`, which belong to another package. So each way of showing is an
ordinary function, and `listOf` takes the function for the items and returns the
function for a list. `listOf(listOf(showString))` builds the function for a list of
lists of strings. That is section 13.4's `Adder` again: the caller passes in the code
for the type. When the set of types is small and known, a **type switch** works too,
even in a generic function. `switch v := any(value).(type)` checks the real type as the
program runs, and a test program that used one printed
`an int: 42 | a string: hi | a main.Cat`.

**TypeScript: one function that checks as it runs.** TypeScript allows several
signatures for one function, but only one body (chapter 5), and the types are erased, so
the body can't ask what T was. It can ask what the value is: `typeof` for strings,
numbers, and booleans, `instanceof` for classes, and `Array.isArray` for arrays. The
union type `Showable` refers to itself, so it covers lists of lists, and it tells the
compiler exactly what `show` handles: `show(new Map())` is
`error TS2345: Argument of type 'Map<any, any>' is not assignable to parameter of type 'Showable'.`

**Kotlin: overloads, until erasure gets in the way.** Kotlin overloads like C++, so
`show(42)`, `show("hi")`, and `show(Point(1, 2))` each pick their own function. The list
version can't call `show(item)` for an unknown T, so the caller passes the function, and
`::show` picks the overload whose type fits. Overloading on the list's type doesn't work
either. `show(List<Int>)` and `show(List<String>)` are both `show(List)` once the JVM
erases them, and Kotlin refuses with
`platform declaration clash: The following declarations have the same JVM signature`.
Writing `@JvmName("showStrings")` on one of them gives it another name in the compiled
code, and then both work.

---

## 13.6 Static and dynamic dispatch

A generic function with a bound can call the same methods as chapter 12's interfaces.
The program below does both, so the two can be compared. `chorus` is generic, so all the
animals in one call have the same type. `chorus_all`, or `chorusAll`, takes a list of
the interface type, so the animals can be any mix. `longest_name` shows what only the
generic version can do: give back the caller's own type.

**C++**

```cpp
#include <concepts>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

class Animal {
public:
    virtual ~Animal() = default;
    virtual std::string name() const = 0;
    virtual std::string sound() const = 0;
};

// final: nothing can derive from Cat, so a call through a Cat& can only mean
// Cat's own method, and the compiler can call it directly.
class Cat final : public Animal {
public:
    explicit Cat(std::string name) : name_(std::move(name)) {}
    std::string name() const override { return name_; }
    std::string sound() const override { return "meow"; }
    std::string purr() const { return "prrr"; }

private:
    std::string name_;
};

class Dog final : public Animal {
public:
    explicit Dog(std::string name) : name_(std::move(name)) {}
    std::string name() const override { return name_; }
    std::string sound() const override { return "woof"; }

private:
    std::string name_;
};

// Static dispatch. The compiler makes one copy of chorus for Cat and another
// for Dog. All the animals in one call must be the same type.
template <std::derived_from<Animal> A>
std::string chorus(const std::vector<A>& animals) {
    std::string text;
    for (const A& animal : animals) {
        if (!text.empty()) text += ", ";
        text += animal.name() + " says " + animal.sound();
    }
    return text;
}

// Dynamic dispatch. There is one copy, and each call goes through the object's
// table of virtual functions. The animals can be any mix.
std::string chorus_all(const std::vector<const Animal*>& animals) {
    std::string text;
    for (const Animal* animal : animals) {
        if (!text.empty()) text += ", ";
        text += animal->name() + " says " + animal->sound();
    }
    return text;
}

// A template hands back the caller's own type, so the caller can still use
// what only a Cat has.
template <std::derived_from<Animal> A>
const A& longest_name(const std::vector<A>& animals) {
    const A* best = &animals.front();
    for (const A& animal : animals) {
        if (animal.name().size() > best->name().size()) best = &animal;
    }
    return *best;
}

int main() {
    std::vector<Cat> cats{Cat("Tom"), Cat("Kitty")};
    std::vector<Dog> dogs{Dog("Rex")};
    std::cout << "cats: " << chorus(cats) << "\n";
    std::cout << "dogs: " << chorus(dogs) << "\n";

    std::vector<const Animal*> everyone{&cats[0], &dogs[0], &cats[1]};
    std::cout << "everyone: " << chorus_all(everyone) << "\n";

    const Cat& cat = longest_name(cats);
    std::cout << "longest name: " << cat.name() << ", who purrs " << cat.purr() << "\n";
}
```

**Rust**

```rust
trait Animal {
    fn name(&self) -> &str;
    fn sound(&self) -> &str;
}

struct Cat(String);

impl Cat {
    fn purr(&self) -> &str {
        "prrr"
    }
}

impl Animal for Cat {
    fn name(&self) -> &str {
        &self.0
    }

    fn sound(&self) -> &str {
        "meow"
    }
}

struct Dog(String);

impl Animal for Dog {
    fn name(&self) -> &str {
        &self.0
    }

    fn sound(&self) -> &str {
        "woof"
    }
}

// Static dispatch. The compiler makes one copy of chorus for Cat and another
// for Dog, and each copy calls that type's methods directly. All the animals
// in one call must be the same type.
fn chorus<A: Animal>(animals: &[A]) -> String {
    let lines: Vec<String> = animals
        .iter()
        .map(|animal| format!("{} says {}", animal.name(), animal.sound()))
        .collect();
    lines.join(", ")
}

// Dynamic dispatch. There is one copy, and each call looks up the method in
// the table that every &dyn Animal carries. The animals can be any mix.
fn chorus_dyn(animals: &[&dyn Animal]) -> String {
    let lines: Vec<String> = animals
        .iter()
        .map(|animal| format!("{} says {}", animal.name(), animal.sound()))
        .collect();
    lines.join(", ")
}

// A generic function hands back the caller's own type, so the caller can
// still use what only a Cat has.
fn longest_name<A: Animal>(animals: &[A]) -> &A {
    let mut best = &animals[0];
    for animal in animals {
        if animal.name().len() > best.name().len() {
            best = animal;
        }
    }
    best
}

fn main() {
    let cats = [Cat("Tom".into()), Cat("Kitty".into())];
    let dogs = [Dog("Rex".into())];
    println!("cats: {}", chorus(&cats));
    println!("dogs: {}", chorus(&dogs));

    let everyone: [&dyn Animal; 3] = [&cats[0], &dogs[0], &cats[1]];
    println!("everyone: {}", chorus_dyn(&everyone));

    let cat = longest_name(&cats);
    println!("longest name: {}, who purrs {}", cat.name(), cat.purr());
}
```

**Go**

```go
package main

import (
	"fmt"
	"strings"
)

type Animal interface {
	Name() string
	Sound() string
}

type Cat struct{ name string }

func (c Cat) Name() string  { return c.name }
func (c Cat) Sound() string { return "meow" }
func (c Cat) Purr() string  { return "prrr" }

type Dog struct{ name string }

func (d Dog) Name() string  { return d.name }
func (d Dog) Sound() string { return "woof" }

// A generic function takes a []Cat or a []Dog as it is. All the animals in
// one call must be the same type.
func Chorus[A Animal](animals []A) string {
	lines := make([]string, len(animals))
	for i, animal := range animals {
		lines[i] = animal.Name() + " says " + animal.Sound()
	}
	return strings.Join(lines, ", ")
}

// An interface value holds any Animal, and each call is looked up in the
// value's method table. The animals can be any mix.
func ChorusAll(animals []Animal) string {
	lines := make([]string, len(animals))
	for i, animal := range animals {
		lines[i] = animal.Name() + " says " + animal.Sound()
	}
	return strings.Join(lines, ", ")
}

// A generic function hands back the caller's own type, so the caller can
// still use what only a Cat has.
func LongestName[A Animal](animals []A) A {
	best := animals[0]
	for _, animal := range animals {
		if len(animal.Name()) > len(best.Name()) {
			best = animal
		}
	}
	return best
}

func main() {
	cats := []Cat{{"Tom"}, {"Kitty"}}
	dogs := []Dog{{"Rex"}}
	fmt.Println("cats:", Chorus(cats))
	fmt.Println("dogs:", Chorus(dogs))

	everyone := []Animal{cats[0], dogs[0], cats[1]}
	fmt.Println("everyone:", ChorusAll(everyone))

	cat := LongestName(cats)
	fmt.Printf("longest name: %s, who purrs %s\n", cat.Name(), cat.Purr())
}
```

**TypeScript**

```ts
interface Animal {
  readonly name: string;
  sound(): string;
}

class Cat implements Animal {
  readonly name: string;

  constructor(name: string) {
    this.name = name;
  }

  sound(): string {
    return "meow";
  }

  purr(): string {
    return "prrr";
  }
}

class Dog implements Animal {
  readonly name: string;

  constructor(name: string) {
    this.name = name;
  }

  sound(): string {
    return "woof";
  }
}

// The types are erased, so these two compile to the same JavaScript: one
// function that looks up sound() on each object as it runs.
function chorus<A extends Animal>(animals: readonly A[]): string {
  return animals
    .map((animal) => `${animal.name} says ${animal.sound()}`)
    .join(", ");
}

function chorusAll(animals: readonly Animal[]): string {
  return animals
    .map((animal) => `${animal.name} says ${animal.sound()}`)
    .join(", ");
}

// What the type parameter buys is the return type: the caller gets back a
// Cat, not just an Animal.
function longestName<A extends Animal>(animals: readonly A[]): A {
  let best = animals[0];
  for (const animal of animals) {
    if (animal.name.length > best.name.length) best = animal;
  }
  return best;
}

const cats = [new Cat("Tom"), new Cat("Kitty")];
const dogs = [new Dog("Rex")];
console.log(`cats: ${chorus(cats)}`);
console.log(`dogs: ${chorus(dogs)}`);

const everyone: Animal[] = [cats[0], dogs[0], cats[1]];
console.log(`everyone: ${chorusAll(everyone)}`);

const cat = longestName(cats);
console.log(`longest name: ${cat.name}, who purrs ${cat.purr()}`);
```

**Kotlin**

```kotlin
interface Animal {
    val name: String

    fun sound(): String
}

class Cat(override val name: String) : Animal {
    override fun sound() = "meow"

    fun purr() = "prrr"
}

class Dog(override val name: String) : Animal {
    override fun sound() = "woof"
}

// On the JVM the type parameter is erased: this compiles to one method that
// takes a List, and each sound() call is looked up as it runs.
fun <A : Animal> chorus(animals: List<A>) =
    animals.joinToString(", ") { "${it.name} says ${it.sound()}" }

fun chorusAll(animals: List<Animal>) =
    animals.joinToString(", ") { "${it.name} says ${it.sound()}" }

// What the type parameter buys is the return type: the caller gets back a
// Cat, not just an Animal.
fun <A : Animal> longestName(animals: List<A>): A = animals.maxBy { it.name.length }

fun main() {
    val cats = listOf(Cat("Tom"), Cat("Kitty"))
    val dogs = listOf(Dog("Rex"))
    println("cats: ${chorus(cats)}")
    println("dogs: ${chorus(dogs)}")

    val everyone: List<Animal> = listOf(cats[0], dogs[0], cats[1])
    println("everyone: ${chorusAll(everyone)}")

    val cat = longestName(cats)
    println("longest name: ${cat.name}, who purrs ${cat.purr()}")
}
```

All five print the same lines:

*C++, Rust, Go, TypeScript and Kotlin:*

```text
cats: Tom says meow, Kitty says meow
dogs: Rex says woof
everyone: Tom says meow, Rex says woof, Kitty says meow
longest name: Kitty, who purrs prrr
```

**C++ and Rust make a copy for each type.** Listing the functions in the compiled
programs with `nm -C` shows it. The C++ program has `chorus<Cat>`, `chorus<Dog>`, and
one `chorus_all`. The Rust one has `main::chorus::<main::Cat>`,
`main::chorus::<main::Dog>`, and one `main::chorus_dyn`, and its copies go further down:
the `map` and `collect` code inside `chorus` is generic too, and an unoptimized build
has 32 functions whose names mention `main::Cat`, and 28 that mention `main::Dog`. Each
copy knows which `sound` it calls, so it can call it directly or copy its code inline.
In C++, `final` on `Cat` is what lets the compiler be sure, because otherwise a class
derived from `Cat` could override `sound`. Compiled with `-O2`, `chorus<Cat>` makes no
indirect calls, and with `final` taken off, it has two. The dynamic version is one
function, and each call goes through the object's table of methods. Static dispatch is
usually faster, and it makes the program bigger and slower to compile.

**Go makes a copy for each shape.** With inlining turned off (`-gcflags=-l`),
`go tool nm` lists `main.ChorusAll` and one generic copy,
`main.Chorus[go.shape.struct { main.name string }]`. `Cat` and `Dog` are both a struct
holding one string, so they have the same shape and share the code. All pointer types
share one shape too: `Chorus[*Cat]` and `Chorus[*Dog]` both run
`main.Chorus[go.shape.*uint8]`. The shared copy is passed a dictionary for the real
type, and its calls to `Name` and `Sound` go through it, much as an interface call goes
through a method table. So Go's generics save code, and don't always save the cost of a
call. A type parameter can also be an interface type. `Chorus([]Animal{...})` makes A
`Animal`, and then it works like `ChorusAll`.

**Kotlin and TypeScript make one copy for every type.** `javap -s` on the compiled
Kotlin shows `chorus` and `chorusAll` with the same descriptor,
`(Ljava/util/List;)Ljava/lang/String;`: a method that takes a `List` and returns a
`String`. The type parameter is kept only as metadata for the compiler. TypeScript's
`chorus` and `chorusAll` compile to the same JavaScript. Either way, every `sound()`
call is looked up as the program runs, generic or not, and the JVM and the JavaScript
engine are built to make such calls fast.

**Erasure has a cost.** A Kotlin `List<Int>` and `List<String>` are the same class when
the program runs, so the program can't check what T is. `value is T` is refused with
`cannot check for instance of erased type 'T (of fun <T> isA)'`, and so is
`value is List<String>`. TypeScript says the same in its own words: `value instanceof T`
is `error TS2693: 'T' only refers to a type, but is being used as a value here.` Kotlin
has a way out for small functions. An `inline` function with a `reified` type parameter
is copied into each caller with the real type filled in, so `is T` works inside it. The
standard library's `filterIsInstance` is one, and
`listOf(1, "two", 3, "four", 5.0).filterIsInstance<String>()` is `[two, four]`.

**What the type parameter buys.** In Kotlin and TypeScript, `chorus` and `chorusAll`
compile to the same code, so why write the generic one? For the types. `longestName`
returns an `A`, so `longestName(cats)` is a `Cat`, and the caller can call `purr`, which
an `Animal` doesn't have. A version that took a list of `Animal`s could only return an
`Animal`. The other side of the trade is that static dispatch needs all the items in one
call to have one type. In Rust, `chorus(&[cat, dog])` stops at
`error[E0308]: mismatched types`, `expected 'Cat', found 'Dog'`, and a mixed list needs
`&dyn Animal`, as `everyone` has.

**Choosing.** Use a generic function when each call works with one type, when the caller
wants its own type back, or when the calls must be as fast as possible. Use the
interface type when one collection holds a mix, or when one copy of a big function is
better than many. Chapter 5 met the same choice for functions: Rust's
`impl Fn(i32) -> i32` makes a copy of `apply_twice` for each closure and
`Box<dyn Fn(i32) -> i32>` shares one, as a C++ template parameter and `std::function`
do. Chapter 12 used Rust's traits only through `dyn`, with a table of methods, and
`chorus` is the other way to use one, with no table at all.

---

## 13.7 Variance: is a list of cats a list of animals?

A `Cat` is an `Animal`. Is a list of cats a list of animals? It depends on what the list
lets you do. If you only read from it, yes: every item you read is an animal. If you can
add to it, no. As a list of animals, it would accept a dog, and then the list of cats
would hold a dog. **Variance** is the rule, for each generic type, for how `Box<Cat>`
relates to `Box<Animal>`:

- **Covariant:** a `Box<Cat>` is a `Box<Animal>`. That is safe when the box only hands
  Ts out, like a read-only list.
- **Contravariant:** the other way round, a `Box<Animal>` is a `Box<Cat>`. That is safe
  when the box only takes Ts in, like a function that compares two: one that can compare
  any two animals can compare two cats.
- **Invariant:** neither. A box that takes Ts in and hands them out, like a list you can
  add to, must be invariant.

The example asks all three questions. It passes a list of cats where a list of animals
is wanted, adds a dog to a shelter that starts with the cats, and sorts the cats with a
function written for any two animals.

**C++**

```cpp
#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

class Animal {
public:
    virtual ~Animal() = default;
    virtual std::string name() const = 0;
};

class Cat final : public Animal {
public:
    explicit Cat(std::string name) : name_(std::move(name)) {}
    std::string name() const override { return name_; }

private:
    std::string name_;
};

class Dog final : public Animal {
public:
    explicit Dog(std::string name) : name_(std::move(name)) {}
    std::string name() const override { return name_; }

private:
    std::string name_;
};

std::string names(const std::vector<const Animal*>& animals) {
    std::string text;
    for (const Animal* animal : animals) text += (text.empty() ? "" : ", ") + animal->name();
    return text;
}

// Each Cat* converts to an Animal*, but a vector of one is never a vector of
// the other. Build a new vector, converting each pointer.
std::vector<const Animal*> as_animals(const std::vector<Cat>& cats) {
    std::vector<const Animal*> animals;
    for (const Cat& cat : cats) animals.push_back(&cat);
    return animals;
}

int main() {
    std::vector<Cat> cats{Cat("Tom"), Cat("Kitty")};
    std::vector<const Animal*> animals = as_animals(cats);
    std::cout << "names: " << names(animals) << "\n";

    Dog rex("Rex");
    std::vector<const Animal*> shelter = animals;
    shelter.push_back(&rex);
    std::cout << "shelter: " << names(shelter) << "\n";

    // A function for any two animals works on two cats, because each Cat&
    // converts to an Animal& as it is passed.
    auto by_name = [](const Animal& a, const Animal& b) { return a.name() < b.name(); };
    std::vector<Cat> sorted = cats;
    std::ranges::sort(sorted, by_name);
    std::cout << "sorted by name: " << names(as_animals(sorted)) << "\n";
}
```

**Rust**

```rust
trait Animal {
    fn name(&self) -> &str;
}

#[derive(Clone)]
struct Cat(String);

impl Animal for Cat {
    fn name(&self) -> &str {
        &self.0
    }
}

struct Dog(String);

impl Animal for Dog {
    fn name(&self) -> &str {
        &self.0
    }
}

fn names(animals: &[&dyn Animal]) -> String {
    let texts: Vec<&str> = animals.iter().map(|animal| animal.name()).collect();
    texts.join(", ")
}

// A &Cat converts to a &dyn Animal, but a slice of Cats is not a slice of
// &dyn Animal: the items have different sizes. Make a new Vec.
fn as_animals(cats: &[Cat]) -> Vec<&dyn Animal> {
    cats.iter().map(|cat| cat as &dyn Animal).collect()
}

fn main() {
    let cats = vec![Cat("Tom".into()), Cat("Kitty".into())];
    let animals = as_animals(&cats);
    println!("names: {}", names(&animals));

    let rex = Dog("Rex".into());
    let mut shelter = animals.clone();
    shelter.push(&rex);
    println!("shelter: {}", names(&shelter));

    // A function for any two animals works on two cats, because each &Cat
    // converts as it is passed.
    let by_name = |a: &dyn Animal, b: &dyn Animal| a.name().cmp(b.name());
    let mut sorted = cats.clone();
    sorted.sort_by(|a, b| by_name(a, b));
    println!("sorted by name: {}", names(&as_animals(&sorted)));
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

type Animal interface{ Name() string }

type Cat struct{ name string }

func (c Cat) Name() string { return c.name }

type Dog struct{ name string }

func (d Dog) Name() string { return d.name }

func Names(animals []Animal) string {
	texts := make([]string, len(animals))
	for i, animal := range animals {
		texts[i] = animal.Name()
	}
	return strings.Join(texts, ", ")
}

// A []Cat is not a []Animal: a Cat is stored as a bare struct, an Animal as
// an interface value. Converting means copying, one item at a time.
func toAnimals(cats []Cat) []Animal {
	animals := make([]Animal, len(cats))
	for i, cat := range cats {
		animals[i] = cat
	}
	return animals
}

func main() {
	cats := []Cat{{"Tom"}, {"Kitty"}}
	animals := toAnimals(cats)
	fmt.Println("names:", Names(animals))

	shelter := append(slices.Clone(animals), Dog{"Rex"})
	fmt.Println("shelter:", Names(shelter))

	// A func(Animal, Animal) int is not a func(Cat, Cat) int either, so wrap it.
	byName := func(a, b Animal) int { return strings.Compare(a.Name(), b.Name()) }
	sorted := slices.Clone(cats)
	slices.SortFunc(sorted, func(a, b Cat) int { return byName(a, b) })
	fmt.Println("sorted by name:", Names(toAnimals(sorted)))
}
```

**TypeScript**

```ts
interface Animal {
  readonly name: string;
}

// Without kind, a Dog would have the same shape as a Cat, and TypeScript
// would take either one where the other is expected.
class Cat implements Animal {
  readonly name: string;
  readonly kind = "cat";

  constructor(name: string) {
    this.name = name;
  }
}

class Dog implements Animal {
  readonly name: string;
  readonly kind = "dog";

  constructor(name: string) {
    this.name = name;
  }
}

// A readonly array only hands items out, so a Cat[] can be passed where a
// readonly Animal[] is expected.
function names(animals: readonly Animal[]): string {
  return animals.map((animal) => animal.name).join(", ");
}

const cats = [new Cat("Tom"), new Cat("Kitty")];
console.log(`names: ${names(cats)}`);

// TypeScript would also let a Cat[] be an Animal[] that takes dogs (section
// 13.7), so give a shelter that takes dogs its own array.
const shelter: Animal[] = [...cats, new Dog("Rex")];
console.log(`shelter: ${names(shelter)}`);

// A function that compares any two animals can compare two cats.
const byName = (a: Animal, b: Animal) =>
  a.name < b.name ? -1 : a.name > b.name ? 1 : 0;
console.log(`sorted by name: ${names(cats.toSorted(byName))}`);
```

**Kotlin**

```kotlin
interface Animal {
    val name: String
}

class Cat(override val name: String) : Animal

class Dog(override val name: String) : Animal

// List is declared List<out E>: it only hands items out, so a List<Cat> can
// be used as a List<Animal>. Nothing can put a Dog into it that way.
fun names(animals: List<Animal>) = animals.joinToString(", ") { it.name }

fun main() {
    val cats = listOf(Cat("Tom"), Cat("Kitty"))
    println("names: ${names(cats)}")

    // A MutableList takes items in as well, so MutableList<Cat> is not a
    // MutableList<Animal>. A shelter that takes dogs needs its own list.
    val shelter: MutableList<Animal> = cats.toMutableList()
    shelter.add(Dog("Rex"))
    println("shelter: ${names(shelter)}")

    // A Comparator only takes items in, so one for any Animal can sort Cats:
    // sortedWith asks for a Comparator<in Cat>.
    val byName: Comparator<Animal> = compareBy { it.name }
    println("sorted by name: ${names(cats.sortedWith(byName))}")
}
```

All five print the same lines:

*C++, Rust, Go, TypeScript and Kotlin:*

```text
names: Tom, Kitty
shelter: Tom, Kitty, Rex
sorted by name: Kitty, Tom
```

**Kotlin: variance where the type is declared.** The standard library declares
`List<out E>`. `out` promises that an E only ever comes out of a `List`, so a
`List<Cat>` is a `List<Animal>`, and `names(cats)` works. `MutableList<E>` has no `out`,
because `add` takes an E in, so it is invariant, and passing a `MutableList<Cat>` for a
`MutableList<Animal>` fails with
`argument type mismatch: actual type is 'MutableList<Cat>', but 'MutableList<Animal>' was expected.`
The compiler holds a class to its promise: `class Crate<out T>` with a
`fun put(value: T)` is refused with
`type parameter 'T' is declared as 'out' but occurs in 'in' position`. `in` is the
opposite promise, for a type that only takes Ts in, and `sortedWith` asks for a
`Comparator<in T>`, so a `Comparator<Animal>` sorts cats. Variance can also be written
where a type is used. A parameter of type `MutableList<out Animal>` accepts a
`MutableList<Cat>`, and then forbids the calls that would put an animal in:
`receiver type 'MutableList<out Animal>' contains out projection which prohibits the use of 'fun add(element: E): Boolean'.`

**TypeScript: variance worked out from the shape, with two holes.** TypeScript decides
variance from how a type uses T. A `readonly Animal[]` has no methods that take items
in, so a `Cat[]` can be passed as one. The first hole is that an ordinary array counts
as covariant too, even though it has `push`. With section 13.6's `Cat` and `Dog`, this
compiles under `--strict`:

```ts
const cats: Cat[] = [new Cat("Tom")];
const animals: Animal[] = cats;
animals.push(new Dog("Rex"));
cats[1].purr(); // TypeError: cats[1].purr is not a function
```

The second hole is in methods. Under `--strict`, function types are checked properly, so
an object property declared `handle: (animal: Animal) => void` refuses a
`(cat: Cat) => void`, which might be handed a dog. The error ends
`Property 'purr' is missing in type 'Animal' but required in type 'Cat'.` A method
declared with method syntax, `onAnimal(animal: Animal): void`, accepts the same function
without a word. TypeScript checks method parameters loosely on purpose, so that
`Array<T>` and the like can stay covariant. Since TypeScript 4.7, a type parameter can
be marked `in` or `out`, as in Kotlin, but the check is only as strict as the rest:
`interface Box<out T> { get(): T; set(value: T): void }` compiles, because `set` uses
method syntax, and only the property form, `set: (value: T) => void`, gets
`error TS2636`. Shapes matter here too. Without the `kind` fields, a `Cat` and a `Dog`
have the same shape, so TypeScript takes one for the other:
`const cat: Cat = new Dog("Rex")` compiles, and `cat instanceof Cat` is `false` when it
runs.

**Go: no variance at all.** A `[]Cat` is never a `[]Animal`:
`cannot use cats (variable of type []Cat) as []Animal value in argument to Names`. That
is not only caution. A `Cat` in a `[]Cat` is the bare struct, and an `Animal` in a
`[]Animal` is an interface value, a pointer to the method table next to a pointer to the
value (chapter 12), so the two slices are laid out differently, and converting means
building a new slice, as `toAnimals` does. Function types are invariant too, so
`byName`, written for two animals, is wrapped in a function of two cats to sort a
`[]Cat`.

**Rust: variance for lifetimes only.** `Cat` and `Dog` are not subtypes of anything in
Rust. They are separate types that implement a trait, so `&[Cat]` and `&[&dyn Animal]`
are unrelated, and `as_animals` builds a new `Vec`, turning each `&Cat` into a
`&dyn Animal`. The one place where Rust has subtypes is lifetimes. A `&'static str`
lives as long as the program, so it can be used wherever a `&str` that lives less long
is wanted, and since `Vec` is covariant, a `Vec<&'static str>` can be used as a
`Vec<&str>` too. `&mut` is invariant, and that is what stops this:

```rust
fn push_short<'a>(names: &mut Vec<&'a str>, name: &'a str) {
    names.push(name);
}

fn main() {
    let mut names: Vec<&'static str> = vec!["Tom"];
    {
        let local = String::from("Kitty");
        push_short(&mut names, &local);
    }
    println!("{names:?}");
}
```

If `&mut Vec<&'static str>` could be used as `&mut Vec<&'a str>`, `names` would end up
holding a reference to `local` after `local` was freed. The compiler refuses with
`error[E0597]: 'local' does not live long enough`, and explains that the type of `names`
requires `local` to be borrowed for `'static`.

**C++: conversions, not variance.** A `Cat*` converts to an `Animal*`, and a `Cat&` to
an `Animal&`, but templates have no variance. `std::vector<const Cat*>` and
`std::vector<const Animal*>` are unrelated classes, and passing one for the other is
`invalid initialization of reference of type 'const std::vector<const Animal*>&' from expression of type 'std::vector<const Cat*>'`,
so `as_animals` builds a new vector. The sort works because the lambda is called with
two `Cat`s, and each one converts to a `const Animal&` as it is passed. Some library
templates add conversions of their own. A `std::unique_ptr<Cat>` converts to a
`std::unique_ptr<Animal>`, so `std::unique_ptr<Animal> a = std::make_unique<Cat>()`
works. That makes a new pointer, and is not variance: a vector of one is still not a
vector of the other.

---

## 13.8 C, Python, JavaScript, and Dart

Here is section 13.3's program in the other four languages. Each one prints the same
lines as the other five.

**C**

```c
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no generics. One way round that is a macro that writes the code for
// each type you name, which is what a C++ template does for you.
#define DEFINE_STACK(T, Name, FORMAT)                             \
    typedef struct {                                              \
        T *items;                                                 \
        size_t len, cap;                                          \
    } Name;                                                       \
                                                                  \
    static inline void Name##_push(Name *s, T item) {             \
        if (s->len == s->cap) {                                   \
            s->cap = s->cap ? s->cap * 2 : 4;                     \
            T *grown = realloc(s->items, s->cap * sizeof(T));     \
            if (!grown) abort();                                  \
            s->items = grown;                                     \
        }                                                         \
        s->items[s->len++] = item;                                \
    }                                                             \
                                                                  \
    static inline bool Name##_pop(Name *s, T *out) {              \
        if (s->len == 0) return false;                            \
        *out = s->items[--s->len];                                \
        return true;                                              \
    }                                                             \
                                                                  \
    static inline bool Name##_peek(const Name *s, T *out) {       \
        if (s->len == 0) return false;                            \
        *out = s->items[s->len - 1];                              \
        return true;                                              \
    }                                                             \
                                                                  \
    static inline void Name##_report(Name *s, const char *name) { \
        T popped; /* not "T popped, top;", see section 13.8 */    \
        T top;                                                    \
        printf("%s: popped ", name);                              \
        if (Name##_pop(s, &popped)) printf(FORMAT, popped);       \
        else printf("none");                                      \
        printf(", top is ");                                      \
        if (Name##_peek(s, &top)) printf(FORMAT, top);            \
        else printf("none");                                      \
        printf(", %zu left\n", s->len);                           \
    }                                                             \
                                                                  \
    static inline void Name##_free(Name *s) { free(s->items); }

DEFINE_STACK(int, IntStack, "%d")
DEFINE_STACK(const char *, WordStack, "%s")

// The other way is void *, as qsort does: the caller passes the size of an
// item and a function that compares two. Nothing checks that they match.
static const void *largest(const void *items, size_t count, size_t size,
                           int (*compare)(const void *, const void *)) {
    if (count == 0) return NULL;
    const char *bytes = items;
    const void *best = bytes;
    for (size_t i = 1; i < count; i++) {
        if (compare(bytes + i * size, best) > 0) best = bytes + i * size;
    }
    return best;
}

static int compare_ints(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static int compare_words(const void *a, const void *b) {
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

// Without generics, each combination of types needs its own struct.
struct int_word {
    int first;
    const char *second;
};

struct word_int {
    const char *first;
    int second;
};

static struct word_int swap_int_word(struct int_word pair) {
    return (struct word_int){pair.second, pair.first};
}

int main(void) {
    IntStack numbers = {0};
    IntStack_push(&numbers, 3);
    IntStack_push(&numbers, 1);
    IntStack_push(&numbers, 4);
    IntStack_report(&numbers, "numbers");
    IntStack_free(&numbers);

    WordStack words = {0};
    WordStack_push(&words, "pear");
    WordStack_push(&words, "fig");
    WordStack_report(&words, "words");
    WordStack_free(&words);

    IntStack empty = {0};
    IntStack_report(&empty, "empty");
    IntStack_free(&empty);

    int digits[] = {3, 1, 4, 1, 5};
    const int *most = largest(digits, 5, sizeof digits[0], compare_ints);
    printf("largest number: %d\n", *most);
    const char *fruit[] = {"pear", "fig", "apple"};
    const char *const *last = largest(fruit, 3, sizeof fruit[0], compare_words);
    printf("largest word: %s\n", *last);
    printf("largest of nothing: %s\n",
           largest(digits, 0, sizeof digits[0], compare_ints) ? "?" : "none");

    struct int_word pair = {7, "seven"};
    struct word_int swapped = swap_int_word(pair);
    printf("(%d, %s) swapped is (%s, %d)\n", pair.first, pair.second, swapped.first,
           swapped.second);
    return 0;
}
```

**Python**

```python
from collections.abc import Sequence
from dataclasses import dataclass
from typing import Any, Protocol


# A generic class (Python 3.12 syntax). T is a type parameter, checked by mypy
# and pyright. At run time it is ignored: a Stack holds anything.
class Stack[T]:
    def __init__(self) -> None:
        self._items: list[T] = []

    def push(self, item: T) -> None:
        self._items.append(item)

    def pop(self) -> T | None:
        return self._items.pop() if self._items else None

    def peek(self) -> T | None:
        return self._items[-1] if self._items else None

    def __len__(self) -> int:
        return len(self._items)


# A bound: T must have __lt__, so max() and < work on it.
class Ordered(Protocol):
    def __lt__(self, other: Any, /) -> bool: ...


def largest[T: Ordered](items: Sequence[T]) -> T | None:
    return max(items) if items else None


# Two type parameters. swap returns a Pair with them the other way round.
@dataclass(frozen=True)
class Pair[A, B]:
    first: A
    second: B

    def swap(self) -> "Pair[B, A]":
        return Pair(self.second, self.first)

    def __str__(self) -> str:
        return f"({self.first}, {self.second})"


def show[T](value: T | None) -> str:
    return "none" if value is None else str(value)


def report[T](name: str, stack: Stack[T]) -> None:
    popped = show(stack.pop())
    top = show(stack.peek())
    print(f"{name}: popped {popped}, top is {top}, {len(stack)} left")


def main() -> None:
    numbers = Stack[int]()
    numbers.push(3)
    numbers.push(1)
    numbers.push(4)
    report("numbers", numbers)

    words = Stack[str]()
    words.push("pear")
    words.push("fig")
    report("words", words)

    empty = Stack[float]()
    report("empty", empty)

    print(f"largest number: {show(largest([3, 1, 4, 1, 5]))}")
    print(f"largest word: {show(largest(['pear', 'fig', 'apple']))}")
    nothing: list[int] = []
    print(f"largest of nothing: {show(largest(nothing))}")

    pair = Pair(7, "seven")  # Pair[int, str], inferred
    print(f"{pair} swapped is {pair.swap()}")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// JavaScript has no types to fill in, so every class and function already
// works with any type. This is the same code as the TypeScript version,
// without the type parameters.
class Stack {
  #items = [];

  push(item) {
    this.#items.push(item);
  }

  pop() {
    return this.#items.pop();
  }

  peek() {
    return this.#items.at(-1);
  }

  get size() {
    return this.#items.length;
  }
}

// Works on anything > works on. On objects it gives nonsense, not an error.
function largest(items) {
  let best = items[0];
  for (const item of items) {
    if (best === undefined || item > best) best = item;
  }
  return best;
}

class Pair {
  constructor(first, second) {
    this.first = first;
    this.second = second;
  }

  swap() {
    return new Pair(this.second, this.first);
  }

  toString() {
    return `(${this.first}, ${this.second})`;
  }
}

function show(value) {
  return value === undefined ? "none" : String(value);
}

function report(name, stack) {
  const popped = show(stack.pop());
  const top = show(stack.peek());
  console.log(`${name}: popped ${popped}, top is ${top}, ${stack.size} left`);
}

const numbers = new Stack();
numbers.push(3);
numbers.push(1);
numbers.push(4);
report("numbers", numbers);

const words = new Stack();
words.push("pear");
words.push("fig");
report("words", words);

report("empty", new Stack());

console.log(`largest number: ${show(largest([3, 1, 4, 1, 5]))}`);
console.log(`largest word: ${show(largest(["pear", "fig", "apple"]))}`);
console.log(`largest of nothing: ${show(largest([]))}`);

const pair = new Pair(7, "seven");
console.log(`${pair} swapped is ${pair.swap()}`);
```

**Dart**

```dart
// A generic class. T is a type parameter, and unlike in Java, Kotlin, or
// TypeScript, a Stack<int> still knows its T at run time.
class Stack<T> {
  final _items = <T>[];

  void push(T item) => _items.add(item);

  T? pop() => _items.isEmpty ? null : _items.removeLast();

  T? peek() => _items.isEmpty ? null : _items.last;

  int get length => _items.length;
}

// A generic function. int is Comparable<num>, not Comparable<int>, so the
// bound is Comparable<Object>, which both int and String meet.
T? largest<T extends Comparable<Object>>(List<T> items) {
  if (items.isEmpty) return null;
  var best = items.first;
  for (final item in items) {
    if (item.compareTo(best) > 0) best = item;
  }
  return best;
}

// Two type parameters. swap returns a Pair with them the other way round.
class Pair<A, B> {
  final A first;
  final B second;

  const Pair(this.first, this.second);

  Pair<B, A> swap() => Pair(second, first);

  @override
  String toString() => '($first, $second)';
}

String show<T>(T? value) => value?.toString() ?? 'none';

void report<T>(String name, Stack<T> stack) {
  final popped = show(stack.pop());
  final top = show(stack.peek());
  print('$name: popped $popped, top is $top, ${stack.length} left');
}

void main() {
  final numbers = Stack<int>();
  numbers.push(3);
  numbers.push(1);
  numbers.push(4);
  report('numbers', numbers);

  final words = Stack<String>();
  words.push('pear');
  words.push('fig');
  report('words', words);

  final empty = Stack<double>();
  report('empty', empty);

  print('largest number: ${show(largest([3, 1, 4, 1, 5]))}');
  print('largest word: ${show(largest(['pear', 'fig', 'apple']))}');
  print('largest of nothing: ${show(largest(<int>[]))}');

  final pair = Pair(7, 'seven'); // Pair<int, String>, inferred
  print('$pair swapped is ${pair.swap()}');
}
```

**C: macros and `void *`.** C has no generics, and this example uses the two usual ways
round that. The first is a macro that writes the code:
`DEFINE_STACK(int, IntStack, "%d")` expands to a struct and functions for ints,
`DEFINE_STACK(const char *, WordStack, "%s")` to another set for strings, and `##`
pastes the names together. That is what a C++ template does, with no checks until the
expanded code compiles. A macro only substitutes text, so wherever T appears, the type
is pasted in, and `T popped, top;` looks right but isn't: with T as `const char *`, it
becomes `const char * popped, top;`, which makes `top` a plain `const char`. GCC then
reports errors about other lines, such as
`passing argument 2 of 'WordStack_peek' from incompatible pointer type`. One declaration
per line avoids it, as does a `typedef` for the pointer type.

The second way is `void *`, as `qsort` uses. `largest` takes the items as bytes, the
size of one item, and a compare function, so it works with any type, and nothing checks
that the three agree. Sorting the `double`s `2.5, 1.5, 3.5` with `compare_ints` compiles
without a warning under `-Wall -Wextra -Werror`, and leaves them as `2.5 1.5 3.5`.
Without generics, a pair needs a struct for each combination of types, here
`struct int_word` and `struct word_int`. C11's `_Generic`, from chapter 5, picks an
expression by the type of an argument inside a macro, which is ad hoc polymorphism, but
it can't make new types.

**Python: types for the checker only.** Python 3.12's syntax, `class Stack[T]` and
`def largest[T: Ordered]`, declares type parameters for a type checker such as mypy, and
Python itself doesn't check them. `Stack[int]().push("three")` runs, and the stack holds
a string, while `mypy --strict` reports
`Argument 1 to "push" of "Stack" has incompatible type "str"; expected "int"`.
`Stack[int]()` makes a plain `Stack`, and `isinstance(x, list[int])` raises
`TypeError: isinstance() argument 2 cannot be a parameterized generic`. Older code
writes the same with `T = TypeVar("T")` and `class Stack(Generic[T])`, which still
works. The bound is a `Protocol` (chapter 12) with an `__lt__` method, which is what
`max` and `<` call. For a body per type, Python has `functools.singledispatch` (chapter
5), which picks a function by the type of the first argument. mypy works out variance
under the new syntax. A class whose T only comes out is covariant, so a `Reader[int]` is
accepted as a `Reader[object]`, and one with a method that takes a T is invariant:
`Argument 1 to "use" has incompatible type "Box[int]"; expected "Box[object]"`. The
built-in `list` is invariant for the same reason, and when a `list[int]` is passed for a
`list[object]`, mypy's note suggests `Sequence`, "which is covariant".

**JavaScript: nothing to fill in.** Every JavaScript function already takes values of
any type, so generic code is just code: the same as the TypeScript version, without the
types. Nothing is checked. `largest` works on anything `>` works on, and on objects it
gives nonsense rather than an error, because `>` compares their `toString` text (chapter
12).

**Dart: generics that stay when the program runs.** Dart's generics look like Kotlin's
and are checked the same way, but they are reified: "Dart generic types are reified,
which means that they carry their type information around at runtime," as its
documentation puts it. `<int>[1, 2] is List<int>` is `true`, and `is List<String>` is
`false`, a check that Kotlin and TypeScript can't make. Dart's generic types are also
all covariant, so a `List<int>` can be used as a `List<Object>`, even though a list can
be added to. The analyzer allows `objects.add('three')` on such a list, and Dart checks
it when it runs, since the list knows that it is a `List<int>`:
`type 'String' is not a subtype of type 'int' of 'value'`. Bounds have a surprise. `int`
implements `Comparable<num>`, not `Comparable<int>`, so with the bound
`T extends Comparable<T>`, `largest<int>` is refused:
`'int' doesn't conform to the bound 'Comparable<int>' of the type parameter 'T'`. The
example's bound is `Comparable<Object>`, which `int` and `String` both meet, since a
`Comparable<num>` is a `Comparable<Object>`. The cost is that the analyzer then allows
comparing a word with a number, and the mistake shows up only when the program runs, as
`type 'int' is not a subtype of type 'String' of 'other'`.

---

## 13.9 Why each language looks the way it does

- **C** never added generics. `void *` and macros cover the common cases. C99's
  `<tgmath.h>` gave the math functions one name for every floating-point type, so
  `sqrt(2.0f)` returns a `float` and `sqrt(2.0L)` a `long double`, and C11's `_Generic`
  lets any macro choose a function by type the same way.
- **C++** got templates around 1990, and Alexander Stepanov's Standard Template Library,
  accepted into the draft standard in 1994, showed what they could do: containers and
  algorithms written once that run as fast as code written for each type. Templates also
  turned out to be a programming language of their own, run by the compiler (chapter
  15). The cost was error messages. Concepts were planned for the standard that became
  C++11, taken out in 2009, and finally added in C++20.
- **Rust** took its traits from Haskell's type classes, and compiles generics the C++
  way, one copy per type, so that generic code costs nothing at run time. Unlike C++, it
  checks a generic body against its bounds before any caller exists, so errors appear at
  the call.
- **Go** was released in 2012 without generics. Russ Cox's 2009 post "The Generic
  Dilemma" put the choice as "do you want slow programmers, slow compilers and bloated
  binaries, or slow execution times?": C's way, C++'s, or Java's. Generics arrived in Go
  1.18, in March 2022, after years of proposals, and the implementation splits the
  difference, with one copy per shape and a dictionary.
- **Kotlin** runs on the JVM. Java added generics in Java 5, in 2004, by erasure, so
  that new generic code would work with the libraries and class files already written.
  Kotlin inherits the erasure, adds `reified` for inline functions, and replaces Java's
  wildcards, such as `List<? extends Animal>`, with `out` and `in`, written once where a
  type is declared.
- **TypeScript** describes JavaScript, which has no static types. Its design goals
  include "Impose no runtime overhead on emitted programs", so the types are erased, and
  among its non-goals is "Apply a sound or 'provably correct' type system. Instead,
  strike a balance between correctness and productivity." Covariant arrays and loosely
  checked methods are part of that balance.
- **Dart** keeps type arguments when the program runs. That lets it make every generic
  type covariant, which is simple to use, and still catch the unsafe cases, by checking
  them as the program runs. Dart 2, in 2018, made the type system sound, with those
  checks as part of it.
- **Python** added type hints in Python 3.5 (PEP 484, 2015), with `TypeVar` and
  `Generic`, as a layer that the interpreter doesn't check. PEP 695, in Python 3.12
  (2023), gave type parameters their own syntax and had type checkers work out variance.
- **JavaScript** has never needed generics, because it has never had static types.

---

## 13.10 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | `T a, b;` in a macro, with a pointer type for T | Only `a` is a pointer: `const char * a, b;` makes `b` a `const char` |
| C | A `void *` function given a compare function for the wrong type | Compiles cleanly and gives wrong answers: `double`s compared with `compare_ints` aren't sorted |
| C++ | An unconstrained template given a type it can't handle | The error is inside the template body, after `required from here`; add a concept |
| C++ | `show("hi")` with overloads for `bool` and `std::string` | The pointer converts to `bool`, and it prints `true`; write `"hi"s` |
| C++ | A template that calls an overload declared below it | Not found for standard types such as `std::string`; declare the overloads first |
| Rust | Implementing a standard trait for a standard type | `error[E0117]`, the orphan rule; wrap the type in a struct of your own |
| Rust | A generic `new()` with nothing to work T out from | `error[E0282]: type annotations needed`; write the type, or use `::<>` |
| Go | A method with type parameters of its own | `method must have no type parameters`; write a function |
| Go | `int64` in a constraint where `~int64` was meant | Named types such as `Cents` are refused: `possibly missing ~ for int64` |
| Go | Passing a `[]Cat` where a `[]Animal` is wanted | Refused; copy into a new slice |
| TypeScript | `new Stack()` with no type argument | T is `unknown`, and the stack accepts anything |
| TypeScript | Using a `Cat[]` as an `Animal[]`, and then pushing | Compiles, and a `Dog` ends up among the cats; take `readonly Animal[]` |
| TypeScript | `>` on an unconstrained T | Compiles, and compares objects by their text |
| Kotlin | `is T` or `is List<String>` | `cannot check for instance of erased type`; use `inline` with `reified` |
| Kotlin | Overloads that differ only in a type argument | `platform declaration clash`; add `@JvmName` |
| Kotlin | `pop(): T?` when T itself allows `null` | A stored `null` looks the same as an empty stack |
| Python | Trusting type parameters when the program runs | A `Stack[int]` holds strings without complaint; run mypy |
| Dart | Adding to a list through a covariant type | Compiles, then `type 'String' is not a subtype of type 'int'` when it runs |
| Dart | `T extends Comparable<T>` with `int` | `int` is a `Comparable<num>`, so `largest<int>` is refused |

---

## 13.11 Idiom check

**C: `void *` for algorithms, macros for small typed containers.** Write algorithms in
the `qsort` shape, with a size and a compare function, and document what the `void *`
must point at. Use a macro only for small containers that you want typed, wrap pointer
types in a `typedef` before passing them, and declare one variable per line inside it.

**C++: templates with concepts.** Constrain every template, with a standard concept from
`<concepts>` where one fits, such as `std::totally_ordered`, `std::integral`, or
`std::derived_from`, and your own where none does. Let the compiler deduce template
arguments. Use templates when the type is known at compile time and virtual functions
when one container holds a mix, and mark classes `final` when nothing derives from them.

**Rust: generics by default, `dyn` for mixes.** Write generic functions with trait
bounds, and move the bounds into a `where` clause when there are several. Use
`dyn Trait` for collections of mixed types, or to cut compile time and code size.
Implement the standard traits, such as `Display`, `PartialEq`, `PartialOrd`, and `From`,
for your types, so that generic code accepts them, and wrap a foreign type in a newtype
when the orphan rule is in the way.

**Go: write the code first, and add type parameters when it repeats.** Reach for type
parameters when the same code is written for several types, as the standard library's
`slices`, `maps`, and `cmp` packages do. Constrain with `any`, `comparable`,
`cmp.Ordered`, or a small type set, and pass functions for behavior that differs per
type. Keep using interfaces for values whose type is only known as the program runs.

**TypeScript: let inference work, and pass behavior in.** Let the compiler infer type
arguments, constrain with `extends`, and pass compare functions and similar objects for
behavior. Take `readonly T[]` for arrays you only read, and declare callbacks with
property syntax so they are checked strictly. The types are gone when the program runs,
so check values with `typeof`, `instanceof`, or a `kind` field.

**Kotlin: bounds, `out` and `in`, and read-only types.** Use upper bounds, with `where`
for several. Declare `out` or `in` on your own generic types where they fit, and take
`List` rather than `MutableList` when you only read. Keep `inline` with `reified` for
the few functions that must know T, such as `filterIsInstance`.

**Python: the 3.12 syntax, and a checker.** Write `class Stack[T]` and
`def f[T: Bound]`, use `Protocol`s as bounds, and run mypy or pyright. Take `Sequence`
and `Mapping` for collections you only read, since they are covariant and `list` and
`dict` are not. Use `functools.singledispatch` for code that differs per type.

**JavaScript: pass in what varies.** Write functions that take the behavior they need as
arguments, such as a compare function, and check values as the program runs where it
matters. Shared code can document its expected types with JSDoc comments.

**Dart: bounds, and care with covariance.** Use `extends` bounds, with
`Comparable<Object>` or a `Comparator<T>` for ordering. Generic types are covariant and
checked as the program runs, so don't add to a collection that you were given as a wider
type.

---

## 13.12 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Intervals](exercises/01-intervals/) | One generic merge for whole numbers, words, and version numbers, each with its own order |
| 2 | [Typed settings](exercises/02-typed-settings/) | A generic `Setting<T>` in one list with different T's, and typed handles to the same settings |
| 3 | [Three kinds of number](exercises/03-three-kinds-of-number/) | One generic evaluator for whole numbers, fractions, and arithmetic mod 7 |

```sh
python3 tools/check.py 13                     # your answers, every language
python3 tools/check.py 13 --lang rust,go      # some languages
python3 tools/check.py 13 --solutions         # the reference answers
```

**Stretch:** add a fourth kind of number to exercise 3, such as `mod11` or `money` with
two decimal places, in one language that makes a copy per type, such as C++ or Rust, and
one that erases types, such as Kotlin or TypeScript. In each, did the evaluator change?
Then compare the size of the compiled program before and after in C++ or Rust.

---

Previous: [Chapter 12: Abstraction and OOP](../12-abstraction-and-oop/) ·
Next: Chapter 14: Iterators and functional style (coming soon)
