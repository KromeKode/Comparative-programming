# Chapter 8: Structs, enums, and null

Chapter 7 kept many values of one kind together. This chapter builds new types out of
values of different kinds. A **struct** holds several named fields at once: a point has
an x and a y. An **enum** holds exactly one of a fixed set of choices: a traffic light
is red, green, or yellow. A **sum type** is an enum whose choices carry their own data:
a payment is cash, or a card with its last four digits, or a voucher with a code and an
amount. And every language needs a way to say that a value is not there at all, which
is where **null** comes in, along with the optional types that many languages now use
instead. The nine languages agree on structs, disagree about what an enum is, and
split sharply on whether the compiler checks for a missing value.

**Languages:** all 9 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| How to define structs, records, and data classes, and what each language writes for you: printing, equality, and copies | Abstraction and OOP (chapter 12) |
| Which structs copy on assignment and which share | The memory model and ownership (chapters 9 and 10) |
| Plain enums, and how far each language lets a bad value in | State machines, parsers (chapter 21) |
| Sum types, and compilers that check that every case is handled | Error handling (chapter 11), trees (chapter 26) |
| How each language says "no value", and which ones catch a missing check before the program runs | Every chapter, starting with error handling (chapter 11) |

---

## 8.1 The concept: and, or, and nothing

A **struct** groups a fixed set of named fields, each with its own type. Every value of
the struct has all of its fields: an x *and* a y. Because a struct can hold any
combination of its fields' values, type theory calls it a **product type**. A struct of
a `bool` and a three-value enum has 2 × 3 = 6 possible values.

An **enum**, in its plain form, is a type with a short list of named values, and a
variable of that type holds exactly one of them: red *or* green *or* yellow. A **sum
type** lets each of those choices, called **variants**, carry its own fields. A cash
payment needs no data, a card payment needs the last four digits, and a voucher needs a
code and an amount. The possible values of a sum type add up across its variants, which
is where the name comes from. A language with both kinds is said to have **algebraic
data types**.

A **null** is a special value that means "nothing here". Tony Hoare added the null
reference to ALGOL W in 1965 and in 2009 called it his "billion-dollar mistake": once
any reference may be null, a variable that should hold a user can hold nothing instead,
and the mistake shows up only when the program runs and crashes. The fix that newer
languages use is to put "maybe nothing" into the type. An **optional type**, such as
Rust's `Option<String>`, is a sum type with two variants, `Some(value)` and `None`. A
plain `String` then always holds a string, and an `Option<String>` must be checked
before its string can be used.

---

## 8.2 What each language calls them

| Language | Struct | Plain enum | Sum type | "No value" | Optional type |
| --- | --- | --- | --- | --- | --- |
| C | `struct` | `enum`, which is an `int` | A `struct` with an `enum` tag and a `union`, by hand | `NULL`, for pointers | None |
| C++ | `struct` or `class` | `enum class` (C++11) | `std::variant` (C++17) | `nullptr`, for pointers | `std::optional<T>` (C++17) |
| Rust | `struct` | `enum` | `enum` with data | `None`; references are never null | `Option<T>` |
| Go | `struct` | Constants with `iota` | An `interface` and a type switch | `nil`, for pointers, slices, maps, interfaces, channels, and functions | None; zero values and `v, ok` |
| Python | A class, usually a `@dataclass` | `enum.Enum` | A union of classes, with `match` (3.10) | `None` | `str \| None` as a type hint |
| JavaScript | An object, or a `class` | None; strings by convention | Objects with a `kind` field | `null` and `undefined` | None |
| TypeScript | `type` or `interface`, describing an object | A union of string literals; `enum` exists but is not erasable | A discriminated union | `null` and `undefined` | `string \| null` with `--strict` |
| Dart | A `class`, or a record | `enum` | A `sealed class` (Dart 3) | `null` | `String?` |
| Kotlin | A `data class` | `enum class` | A `sealed interface` or `sealed class` | `null` | `String?` |

C has the raw pieces: a `struct` for "and", a `union` that stores one of several types
in the same memory, and an `enum` that names integers. C++ keeps all three and adds
safer versions in the library: `enum class`, `std::variant`, and `std::optional`. Rust
takes its design from the ML family of languages, where the `enum` with data is the
main tool and `Option` is an ordinary enum in the standard library. Go has structs,
uses typed constants for enums, uses interfaces where other languages use sum types,
and has `nil`. Python, JavaScript, TypeScript, Dart, and Kotlin build all of these from
classes or objects, and the last three make "can this be null?" part of every type.

---

## 8.3 Structs

The program below makes a point, reads its fields, makes a copy with one field changed,
assigns the point to a second variable and changes it through that one, compares two
points with the same fields, and calls a method. The code is in
[`examples/01-structs/`](examples/01-structs/).

**C**

```c
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// A struct groups named fields. Assigning one copies every field.
typedef struct {
    int x;
    int y;
} Point;

// C has no methods: a function that takes the struct does the same job.
static int steps_from_origin(Point p) {
    return abs(p.x) + abs(p.y);
}

// C cannot compare structs with ==, so equality is a function you write.
static bool point_equal(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

int main(void) {
    Point p = {1, 2};
    printf("p = (%d, %d)\n", p.x, p.y);
    printf("p.x + p.y = %d\n", p.x + p.y);

    // Designated initializers name the fields they set.
    Point q = {.x = 5, .y = p.y};
    printf("q = p with x = 5: q = (%d, %d), p = (%d, %d)\n", q.x, q.y, p.x, p.y);

    Point r = p; // a copy
    r.y = 9;
    printf("r = p, then r.y = 9: r = (%d, %d), p = (%d, %d)\n", r.x, r.y, p.x, p.y);

    Point a = {3, -4};
    Point b = {3, -4};
    printf("a = (%d, %d), b = (%d, %d), a == b: %s\n", a.x, a.y, b.x, b.y,
           point_equal(a, b) ? "yes" : "no");
    printf("steps from (0, 0) to a: %d\n", steps_from_origin(a));
    return 0;
}
```

**C++**

```cpp
#include <cstdlib>
#include <iostream>

struct Point {
    int x;
    int y;

    // A member function, called as p.steps_from_origin().
    int steps_from_origin() const { return std::abs(x) + std::abs(y); }

    // Since C++20, asking for the default == compares every field in order.
    bool operator==(const Point &) const = default;
};

// Teaches << how to print a Point.
std::ostream &operator<<(std::ostream &out, const Point &p) {
    return out << '(' << p.x << ", " << p.y << ')';
}

int main() {
    Point p{1, 2};
    std::cout << "p = " << p << '\n';
    std::cout << "p.x + p.y = " << p.x + p.y << '\n';

    // Designated initializers (C++20) name the fields they set.
    Point q{.x = 5, .y = p.y};
    std::cout << "q = p with x = 5: q = " << q << ", p = " << p << '\n';

    Point r = p; // a copy
    r.y = 9;
    std::cout << "r = p, then r.y = 9: r = " << r << ", p = " << p << '\n';

    Point a{3, -4};
    Point b{3, -4};
    std::cout << "a = " << a << ", b = " << b << ", a == b: " << (a == b ? "yes" : "no")
              << '\n';
    std::cout << "steps from (0, 0) to a: " << a.steps_from_origin() << '\n';
}
```

**Rust**

```rust
use std::fmt;

// derive asks the compiler to write clone, copy, and == for Point.
#[derive(Clone, Copy, PartialEq)]
struct Point {
    x: i32,
    y: i32,
}

// Methods live in an impl block, apart from the fields.
impl Point {
    fn steps_from_origin(&self) -> i32 {
        self.x.abs() + self.y.abs()
    }
}

// Display is how {} prints a Point.
impl fmt::Display for Point {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}

fn main() {
    let p = Point { x: 1, y: 2 };
    println!("p = {p}");
    println!("p.x + p.y = {}", p.x + p.y);

    // Struct update syntax: x is 5, and the other fields come from p.
    let q = Point { x: 5, ..p };
    println!("q = p with x = 5: q = {q}, p = {p}");

    // Point is Copy, so this copies. Without Copy, it would move p into r.
    let mut r = p;
    r.y = 9;
    println!("r = p, then r.y = 9: r = {r}, p = {p}");

    let a = Point { x: 3, y: -4 };
    let b = Point { x: 3, y: -4 };
    let same = if a == b { "yes" } else { "no" };
    println!("a = {a}, b = {b}, a == b: {same}");
    println!("steps from (0, 0) to a: {}", a.steps_from_origin());
}
```

**Go**

```go
package main

import "fmt"

// A struct type. Assigning a struct copies every field.
type point struct {
	x, y int
}

// A method: the receiver (p point) comes before the name.
func (p point) stepsFromOrigin() int {
	return abs(p.x) + abs(p.y)
}

// String makes fmt print a point as (x, y).
func (p point) String() string {
	return fmt.Sprintf("(%d, %d)", p.x, p.y)
}

func abs(n int) int {
	if n < 0 {
		return -n
	}
	return n
}

func yesNo(b bool) string {
	if b {
		return "yes"
	}
	return "no"
}

func main() {
	p := point{x: 1, y: 2}
	fmt.Println("p =", p)
	fmt.Println("p.x + p.y =", p.x+p.y)

	q := p // a copy, so changing q leaves p alone
	q.x = 5
	fmt.Printf("q = p with x = 5: q = %v, p = %v\n", q, p)

	r := p
	r.y = 9
	fmt.Printf("r = p, then r.y = 9: r = %v, p = %v\n", r, p)

	// Structs compare with == when all their fields can.
	a := point{3, -4}
	b := point{3, -4}
	fmt.Printf("a = %v, b = %v, a == b: %s\n", a, b, yesNo(a == b))
	fmt.Println("steps from (0, 0) to a:", a.stepsFromOrigin())
}
```

**Python**

```python
import dataclasses
from dataclasses import dataclass


# @dataclass writes __init__, __repr__, and __eq__ from the list of fields.
@dataclass
class Point:
    x: int
    y: int

    def steps_from_origin(self) -> int:
        return abs(self.x) + abs(self.y)

    def __str__(self) -> str:
        return f"({self.x}, {self.y})"


def main() -> None:
    p = Point(1, 2)
    print(f"p = {p}")
    print(f"p.x + p.y = {p.x + p.y}")

    # replace() makes a new Point with some fields changed.
    q = dataclasses.replace(p, x=5)
    print(f"q = p with x = 5: q = {q}, p = {p}")

    # Not a copy: r and p name the same object.
    r = p
    r.y = 9
    print(f"r = p, then r.y = 9: r = {r}, p = {p}")

    a = Point(3, -4)
    b = Point(3, -4)
    print(f"a = {a}, b = {b}, a == b: {'yes' if a == b else 'no'}")
    print(f"steps from (0, 0) to a: {a.steps_from_origin()}")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// A plain object is JavaScript's struct. A variable holds a reference to it.
const show = (p) => `(${p.x}, ${p.y})`;
const stepsFromOrigin = (p) => Math.abs(p.x) + Math.abs(p.y);

const p = { x: 1, y: 2 };
console.log(`p = ${show(p)}`);
console.log(`p.x + p.y = ${p.x + p.y}`);

// Spread copies p's fields into a new object, and x: 5 replaces one of them.
const q = { ...p, x: 5 };
console.log(`q = p with x = 5: q = ${show(q)}, p = ${show(p)}`);

// Not a copy: r and p refer to the same object.
const r = p;
r.y = 9;
console.log(`r = p, then r.y = 9: r = ${show(r)}, p = ${show(p)}`);

// === on objects asks whether they are the same object, not whether fields match.
const a = { x: 3, y: -4 };
const b = { x: 3, y: -4 };
const same = a === b ? "yes" : "no";
console.log(`a = ${show(a)}, b = ${show(b)}, a == b: ${same}`);
console.log(`steps from (0, 0) to a: ${stepsFromOrigin(a)}`);
```

**TypeScript**

```ts
// A type describes the shape of an object. Any object with these fields fits.
type Point = { x: number; y: number };

const show = (p: Point): string => `(${p.x}, ${p.y})`;
const stepsFromOrigin = (p: Point): number => Math.abs(p.x) + Math.abs(p.y);

const p: Point = { x: 1, y: 2 };
console.log(`p = ${show(p)}`);
console.log(`p.x + p.y = ${p.x + p.y}`);

// Spread copies p's fields into a new object, and x: 5 replaces one of them.
const q: Point = { ...p, x: 5 };
console.log(`q = p with x = 5: q = ${show(q)}, p = ${show(p)}`);

// Not a copy: r and p refer to the same object.
const r = p;
r.y = 9;
console.log(`r = p, then r.y = 9: r = ${show(r)}, p = ${show(p)}`);

// === on objects asks whether they are the same object, not whether fields match.
const a: Point = { x: 3, y: -4 };
const b: Point = { x: 3, y: -4 };
const same = a === b ? "yes" : "no";
console.log(`a = ${show(a)}, b = ${show(b)}, a == b: ${same}`);
console.log(`steps from (0, 0) to a: ${stepsFromOrigin(a)}`);
```

**Dart**

```dart
// A primary constructor (Dart 3.13) declares the fields and the constructor at once.
class Point(var int x, var int y) {
  int stepsFromOrigin() => x.abs() + y.abs();

  // Dart has no built-in "copy with changes", so classes write their own.
  Point copyWith({int? x, int? y}) => Point(x ?? this.x, y ?? this.y);

  // Without these two, == would ask whether both are the same object.
  @override
  bool operator ==(Object other) =>
      other is Point && other.x == x && other.y == y;

  @override
  int get hashCode => Object.hash(x, y);

  @override
  String toString() => '($x, $y)';
}

void main() {
  final p = Point(1, 2);
  print('p = $p');
  print('p.x + p.y = ${p.x + p.y}');

  final q = p.copyWith(x: 5);
  print('q = p with x = 5: q = $q, p = $p');

  // Not a copy: r and p refer to the same object.
  final r = p;
  r.y = 9;
  print('r = p, then r.y = 9: r = $r, p = $p');

  final a = Point(3, -4);
  final b = Point(3, -4);
  print('a = $a, b = $b, a == b: ${a == b ? 'yes' : 'no'}');
  print('steps from (0, 0) to a: ${a.stepsFromOrigin()}');
}
```

**Kotlin**

```kotlin
import kotlin.math.abs

// A data class gets equals, hashCode, toString, and copy written for it.
data class Point(var x: Int, var y: Int) {
    fun stepsFromOrigin() = abs(x) + abs(y)

    override fun toString() = "($x, $y)"
}

fun main() {
    val p = Point(1, 2)
    println("p = $p")
    println("p.x + p.y = ${p.x + p.y}")

    // copy() makes a new Point, with the named fields changed.
    val q = p.copy(x = 5)
    println("q = p with x = 5: q = $q, p = $p")

    // Not a copy: r and p refer to the same object.
    val r = p
    r.y = 9
    println("r = p, then r.y = 9: r = $r, p = $p")

    // == calls equals, which a data class writes to compare every field.
    val a = Point(3, -4)
    val b = Point(3, -4)
    println("a = $a, b = $b, a == b: ${if (a == b) "yes" else "no"}")
    println("steps from (0, 0) to a: ${a.stepsFromOrigin()}")
}
```

Output:

*C, C++, Rust and Go:*

```text
p = (1, 2)
p.x + p.y = 3
q = p with x = 5: q = (5, 2), p = (1, 2)
r = p, then r.y = 9: r = (1, 9), p = (1, 2)
a = (3, -4), b = (3, -4), a == b: yes
steps from (0, 0) to a: 7
```

*Python, Dart and Kotlin:*

```text
p = (1, 2)
p.x + p.y = 3
q = p with x = 5: q = (5, 2), p = (1, 2)
r = p, then r.y = 9: r = (1, 9), p = (1, 9)
a = (3, -4), b = (3, -4), a == b: yes
steps from (0, 0) to a: 7
```

*JavaScript and TypeScript:*

```text
p = (1, 2)
p.x + p.y = 3
q = p with x = 5: q = (5, 2), p = (1, 2)
r = p, then r.y = 9: r = (1, 9), p = (1, 9)
a = (3, -4), b = (3, -4), a == b: no
steps from (0, 0) to a: 7
```

Two lines differ. In C, C++, Rust, and Go, `r = p` copies the struct, so changing `r`
leaves `p` alone. In Python, JavaScript, TypeScript, Dart, and Kotlin, a variable holds
a reference to an object, so `r` and `p` are the same point. That is the split from
section 7.4, with one change: Go's slices share, but its structs copy. And in
JavaScript and TypeScript, `===` asks whether two objects are the same object, so two
points with equal fields are not equal.

### What the language writes for you

A struct needs a few things beyond its fields: a way to print it, a way to compare two
of them, and a way to copy one with a change. Some languages write these for you.

| Language | Define | Print for debugging | `==` | Copy with one field changed |
| --- | --- | --- | --- | --- |
| C | `typedef struct { int x; int y; } Point;` | By hand | Not allowed: `invalid operands to binary == (have 'Point' and 'Point')` | Copy, then assign; or `{.x = 5, .y = p.y}` |
| C++ | `struct Point { int x; int y; };` | By hand: an `operator<<` or a `std::formatter` | `bool operator==(const Point &) const = default;` (C++20); without it, `no match for 'operator=='` | Copy, then assign; or `Point{.x = 5, .y = p.y}` |
| Rust | `struct Point { x: i32, y: i32 }` | `#[derive(Debug)]`, then `{:?}` prints `Point { x: 1, y: 2 }` | `#[derive(PartialEq)]`; without it, E0369: `binary operation '==' cannot be applied to type 'Point'` | `Point { x: 5, ..p }` |
| Go | `type point struct{ x, y int }` | Built in: `%v` prints `{1 2}`, `%+v` prints `{x:1 y:2}`, `%#v` prints `main.point{x:1, y:2}` | Built in, if every field can be compared | Copy, then assign |
| Python | `@dataclass` on a class that lists `x: int` and `y: int` | Generated: `Point(x=1, y=2)` | Generated; a plain class compares identity | `dataclasses.replace(p, x=5)`, or `copy.replace` (3.13) |
| JavaScript | `{ x: 1, y: 2 }`, or a `class` | `console.log` prints `{ x: 1, y: 2 }`; a template string shows `[object Object]` | None: `===` compares identity | `{ ...p, x: 5 }` |
| TypeScript | `type Point = { x: number; y: number };` | As JavaScript | As JavaScript | As JavaScript |
| Dart | `class Point(final int x, final int y);` (Dart 3.13), or a record `(x: 1, y: 2)` | `Instance of 'Point'`, unless you override `toString`; a record prints `(x: 1, y: 2)` | Identity, unless you override `==` and `hashCode`; records compare by value | By hand, usually a `copyWith` method |
| Kotlin | `data class Point(val x: Int, val y: Int)` | Generated: `Point(x=1, y=2)` | Generated; a plain class compares identity | `p.copy(x = 5)` |

Go is the only language here that compares and prints any struct with no help, and its
`==` has a limit: comparing two structs that hold a slice does not compile, with
`invalid operation: a == b (struct containing []int cannot be compared)`. Kotlin's
`data class`, Python's `@dataclass`, and Rust's `derive` are three answers to the same
problem: they generate code that a Dart programmer, or a C++ programmer for anything
beyond `==`, writes by hand. Dart's records compare by value with no extra code, but
they cannot have methods of their own or change after they are made.

**Rust copies only what is `Copy`.** The example derives `Copy`, so `let mut r = p;`
copies the point. A struct can be `Copy` only if every field is, so a struct holding a
`String` cannot: `E0204: the trait 'Copy' cannot be implemented for this type`. Without
`Copy`, `let r = p;` moves the struct, and `p` can no longer be used (section 7.4).
Chapter 10 explains why.

**TypeScript checks shape, not names.** Any object with a numeric `x` and `y` is a
`Point`, whichever code made it. That is called **structural typing**. An object
literal with an extra field is still rejected, because it is probably a typo:

```text
Object literal may only specify known properties, and 'z' does not exist in type 'Point'.
```

C++, Rust, Go, Dart, and Kotlin use **nominal typing**: two struct types with the same
fields are still different types.

### Leaving a field out

| Language | `Point` with only `x` given |
| --- | --- |
| C | `Point p = {1};` sets `y` to 0; `-Wextra` warns: `missing initializer for field 'y' of 'Point'` |
| C++ | `Point p{1};` sets `y` to 0; `-Wextra` warns: `missing initializer for member 'Point::y'` |
| Rust | E0063: `missing field 'y' in initializer of 'Point'` |
| Go | `point{x: 1}` sets `y` to 0, with no warning |
| Python | `TypeError: Point.__init__() missing 1 required positional argument: 'y'` |
| JavaScript | `{ x: 1 }` simply has no `y`; reading it gives `undefined` |
| TypeScript | `Property 'y' is missing in type '{ x: number; }' but required in type 'Point'.` |
| Dart | `2 positional arguments expected by 'Point.new', but 1 found.` |
| Kotlin | `no value passed for parameter 'y'.` |

Go and C lean on **zero values**: a field nobody set is 0, `""`, `false`, or a null
pointer. Go guarantees it everywhere, so `var p point` is `{0 0}`, and Go code is
written so that the zero value is a sensible starting point. C guarantees it only for
the fields you leave out of an initializer and for global variables. A local `Point p;`
with no initializer holds whatever was in that memory, and reading it is undefined
behavior. `Point p = {0};` zeroes every field.

C++, Python, Dart, and Kotlin can also give a field a default value where it is
declared, such as `y: Int = 0` in Kotlin. Rust uses `#[derive(Default)]` and fills in
the other fields with `..Default::default()`. TypeScript can mark a property optional,
`y?: number`, so that it is `undefined` when left out. C and Go have only zero values.
Python's `@dataclass` refuses a mutable default, because every object would share it:

```text
ValueError: mutable default <class 'list'> for field items is not allowed: use default_factory
```

---

## 8.4 Enums

The program below defines a traffic light with three values, lists them, prints the
number behind one, steps through the cycle, says what each light means, adds up how
long one cycle takes, and turns text back into a light. The code is in
[`examples/02-enums/`](examples/02-enums/).

**C**

```c
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// An enum gives names to integers: RED is 0, GREEN is 1, and YELLOW is 2.
typedef enum { RED, GREEN, YELLOW } Light;
enum { LIGHT_COUNT = YELLOW + 1 };

// Anything else a light needs lives in arrays indexed by it.
static const char *const NAMES[LIGHT_COUNT] = {"red", "green", "yellow"};
static const int SECONDS[LIGHT_COUNT] = {30, 25, 5};

static Light next(Light light) {
    return (Light)((light + 1) % LIGHT_COUNT);
}

static const char *meaning(Light light) {
    switch (light) {
    case RED:
        return "stop";
    case GREEN:
        return "go";
    case YELLOW:
        return "slow down";
    }
    return "?"; // a Light variable can hold any int, so C needs a fallback
}

// Sets *out and returns true if text names a light.
static bool parse_light(const char *text, Light *out) {
    for (int i = 0; i < LIGHT_COUNT; i++) {
        if (strcmp(NAMES[i], text) == 0) {
            *out = (Light)i;
            return true;
        }
    }
    return false;
}

static void show_parse(const char *text) {
    Light light;
    if (parse_light(text, &light)) {
        printf("parse \"%s\": %s\n", text, NAMES[light]);
    } else {
        printf("parse \"%s\": not a light\n", text);
    }
}

int main(void) {
    printf("lights:");
    for (int i = 0; i < LIGHT_COUNT; i++) {
        printf(" %s", NAMES[i]);
    }
    printf("\n");
    printf("yellow is number %d\n", YELLOW);

    Light light = RED;
    for (int i = 0; i < LIGHT_COUNT; i++) {
        printf("after %s: %s\n", NAMES[light], NAMES[next(light)]);
        light = next(light);
    }
    printf("red means %s, green means %s, yellow means %s\n", meaning(RED),
           meaning(GREEN), meaning(YELLOW));

    int total = 0;
    for (int i = 0; i < LIGHT_COUNT; i++) {
        total += SECONDS[i];
    }
    printf("one cycle: %d seconds\n", total);

    show_parse("green");
    show_parse("blue");
    return 0;
}
```

**C++**

```cpp
#include <array>
#include <iostream>
#include <optional>
#include <string_view>

// An enum class is scoped (Light::Red) and does not turn into an int by itself.
enum class Light { Red, Green, Yellow };

// C++ cannot list an enum's values or names for you, so the program keeps its own.
constexpr std::array ALL_LIGHTS = {Light::Red, Light::Green, Light::Yellow};

std::string_view name(Light light) {
    switch (light) {
    case Light::Red:
        return "red";
    case Light::Green:
        return "green";
    case Light::Yellow:
        return "yellow";
    }
    return "?"; // an enum class can still hold any value of its underlying int
}

Light next(Light light) {
    auto number = (static_cast<int>(light) + 1) % static_cast<int>(ALL_LIGHTS.size());
    return static_cast<Light>(number);
}

std::string_view meaning(Light light) {
    switch (light) {
    case Light::Red:
        return "stop";
    case Light::Green:
        return "go";
    case Light::Yellow:
        return "slow down";
    }
    return "?";
}

int seconds(Light light) {
    switch (light) {
    case Light::Red:
        return 30;
    case Light::Green:
        return 25;
    case Light::Yellow:
        return 5;
    }
    return 0;
}

std::optional<Light> parse_light(std::string_view text) {
    for (Light light : ALL_LIGHTS) {
        if (name(light) == text) {
            return light;
        }
    }
    return std::nullopt;
}

void show_parse(std::string_view text) {
    auto light = parse_light(text);
    std::cout << "parse \"" << text << "\": " << (light ? name(*light) : "not a light")
              << '\n';
}

int main() {
    std::cout << "lights:";
    for (Light light : ALL_LIGHTS) {
        std::cout << ' ' << name(light);
    }
    std::cout << '\n';
    std::cout << "yellow is number " << static_cast<int>(Light::Yellow) << '\n';

    Light light = Light::Red;
    for (std::size_t i = 0; i < ALL_LIGHTS.size(); i++) {
        std::cout << "after " << name(light) << ": " << name(next(light)) << '\n';
        light = next(light);
    }
    std::cout << "red means " << meaning(Light::Red) << ", green means "
              << meaning(Light::Green) << ", yellow means " << meaning(Light::Yellow)
              << '\n';

    int total = 0;
    for (Light each : ALL_LIGHTS) {
        total += seconds(each);
    }
    std::cout << "one cycle: " << total << " seconds\n";

    show_parse("green");
    show_parse("blue");
}
```

**Rust**

```rust
#[derive(Clone, Copy, PartialEq)]
enum Light {
    Red,
    Green,
    Yellow,
}

impl Light {
    // Rust cannot list an enum's values for you, so the type keeps its own list.
    const ALL: [Light; 3] = [Light::Red, Light::Green, Light::Yellow];

    fn name(self) -> &'static str {
        match self {
            Light::Red => "red",
            Light::Green => "green",
            Light::Yellow => "yellow",
        }
    }

    fn next(self) -> Light {
        match self {
            Light::Red => Light::Green,
            Light::Green => Light::Yellow,
            Light::Yellow => Light::Red,
        }
    }

    fn meaning(self) -> &'static str {
        match self {
            Light::Red => "stop",
            Light::Green => "go",
            Light::Yellow => "slow down",
        }
    }

    fn seconds(self) -> u32 {
        match self {
            Light::Red => 30,
            Light::Green => 25,
            Light::Yellow => 5,
        }
    }

    fn parse(text: &str) -> Option<Light> {
        Light::ALL.into_iter().find(|light| light.name() == text)
    }
}

fn show_parse(text: &str) {
    match Light::parse(text) {
        Some(light) => println!("parse \"{text}\": {}", light.name()),
        None => println!("parse \"{text}\": not a light"),
    }
}

fn main() {
    let names: Vec<&str> = Light::ALL.iter().map(|light| light.name()).collect();
    println!("lights: {}", names.join(" "));
    println!("yellow is number {}", Light::Yellow as i32);

    let mut light = Light::Red;
    for _ in Light::ALL {
        println!("after {}: {}", light.name(), light.next().name());
        light = light.next();
    }
    println!(
        "red means {}, green means {}, yellow means {}",
        Light::Red.meaning(),
        Light::Green.meaning(),
        Light::Yellow.meaning()
    );

    let total: u32 = Light::ALL.iter().map(|light| light.seconds()).sum();
    println!("one cycle: {total} seconds");

    show_parse("green");
    show_parse("blue");
}
```

**Go**

```go
package main

import "fmt"

// Go has no enum keyword. A named integer type and iota, which counts up from 0
// inside a const block, do the same job.
type light int

const (
	red light = iota
	green
	yellow
)

var lights = []light{red, green, yellow}

// String gives each value a name when fmt prints it.
func (l light) String() string {
	return [...]string{"red", "green", "yellow"}[l]
}

func (l light) next() light {
	return (l + 1) % light(len(lights))
}

func (l light) meaning() string {
	switch l {
	case red:
		return "stop"
	case green:
		return "go"
	case yellow:
		return "slow down"
	}
	return "?" // a light can hold any int, so Go needs a fallback
}

func (l light) seconds() int {
	return [...]int{30, 25, 5}[l]
}

func parseLight(text string) (light, bool) {
	for _, l := range lights {
		if l.String() == text {
			return l, true
		}
	}
	return 0, false
}

func showParse(text string) {
	if l, ok := parseLight(text); ok {
		fmt.Printf("parse %q: %v\n", text, l)
	} else {
		fmt.Printf("parse %q: not a light\n", text)
	}
}

func main() {
	fmt.Print("lights:")
	for _, l := range lights {
		fmt.Print(" ", l)
	}
	fmt.Println()
	fmt.Println("yellow is number", int(yellow))

	l := red
	for range lights {
		fmt.Printf("after %v: %v\n", l, l.next())
		l = l.next()
	}
	fmt.Printf("red means %s, green means %s, yellow means %s\n",
		red.meaning(), green.meaning(), yellow.meaning())

	total := 0
	for _, each := range lights {
		total += each.seconds()
	}
	fmt.Printf("one cycle: %d seconds\n", total)

	showParse("green")
	showParse("blue")
}
```

**Python**

```python
from enum import StrEnum, auto


# On a StrEnum, auto() makes each value the member's name in lowercase.
class Light(StrEnum):
    RED = auto()
    GREEN = auto()
    YELLOW = auto()

    def next(self) -> "Light":
        members = list(Light)
        return members[(members.index(self) + 1) % len(members)]

    def meaning(self) -> str:
        match self:
            case Light.RED:
                return "stop"
            case Light.GREEN:
                return "go"
            case Light.YELLOW:
                return "slow down"


SECONDS = {Light.RED: 30, Light.GREEN: 25, Light.YELLOW: 5}


def parse_light(text: str) -> Light | None:
    try:
        return Light(text)
    except ValueError:
        return None


def show_parse(text: str) -> None:
    light = parse_light(text)
    print(f'parse "{text}": {light if light is not None else "not a light"}')


def main() -> None:
    print("lights:", " ".join(Light))
    print("yellow is number", list(Light).index(Light.YELLOW))

    light = Light.RED
    for _ in Light:
        print(f"after {light}: {light.next()}")
        light = light.next()
    print(
        f"red means {Light.RED.meaning()}, green means {Light.GREEN.meaning()}, "
        f"yellow means {Light.YELLOW.meaning()}"
    )

    print(f"one cycle: {sum(SECONDS[light] for light in Light)} seconds")

    show_parse("green")
    show_parse("blue")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// JavaScript has no enums. A frozen array of strings is a common stand-in.
const LIGHTS = Object.freeze(["red", "green", "yellow"]);
const SECONDS = Object.freeze({ red: 30, green: 25, yellow: 5 });

const next = (light) => LIGHTS[(LIGHTS.indexOf(light) + 1) % LIGHTS.length];

function meaning(light) {
  switch (light) {
    case "red":
      return "stop";
    case "green":
      return "go";
    case "yellow":
      return "slow down";
    default:
      throw new Error(`not a light: ${light}`);
  }
}

// find returns undefined when nothing matches.
const parseLight = (text) => LIGHTS.find((light) => light === text);

function showParse(text) {
  console.log(`parse "${text}": ${parseLight(text) ?? "not a light"}`);
}

console.log("lights:", LIGHTS.join(" "));
console.log("yellow is number", LIGHTS.indexOf("yellow"));

let light = "red";
for (const _ of LIGHTS) {
  console.log(`after ${light}: ${next(light)}`);
  light = next(light);
}
console.log(
  `red means ${meaning("red")}, green means ${meaning("green")}, ` +
    `yellow means ${meaning("yellow")}`,
);

const total = LIGHTS.reduce((sum, each) => sum + SECONDS[each], 0);
console.log(`one cycle: ${total} seconds`);

showParse("green");
showParse("blue");
```

**TypeScript**

```ts
// TypeScript's own enum makes code that runs, which --erasableSyntaxOnly rules out.
// A union of string literals, built from one array, is the usual choice instead.
const LIGHTS = ["red", "green", "yellow"] as const;
type Light = (typeof LIGHTS)[number]; // "red" | "green" | "yellow"

const SECONDS: Record<Light, number> = { red: 30, green: 25, yellow: 5 };

const next = (light: Light): Light =>
  LIGHTS[(LIGHTS.indexOf(light) + 1) % LIGHTS.length];

// The compiler knows these three cases cover every Light, so no default is needed.
function meaning(light: Light): string {
  switch (light) {
    case "red":
      return "stop";
    case "green":
      return "go";
    case "yellow":
      return "slow down";
  }
}

const parseLight = (text: string): Light | undefined =>
  LIGHTS.find((light) => light === text);

function showParse(text: string): void {
  console.log(`parse "${text}": ${parseLight(text) ?? "not a light"}`);
}

console.log("lights:", LIGHTS.join(" "));
console.log("yellow is number", LIGHTS.indexOf("yellow"));

let light: Light = "red";
for (const _ of LIGHTS) {
  console.log(`after ${light}: ${next(light)}`);
  light = next(light);
}
console.log(
  `red means ${meaning("red")}, green means ${meaning("green")}, ` +
    `yellow means ${meaning("yellow")}`,
);

const total = LIGHTS.reduce((sum, each) => sum + SECONDS[each], 0);
console.log(`one cycle: ${total} seconds`);

showParse("green");
showParse("blue");
```

**Dart**

```dart
// An enhanced enum can carry fields, set by its constructor.
enum Light {
  red(30),
  green(25),
  yellow(5);

  const Light(this.seconds);
  final int seconds;

  Light get next => values[(index + 1) % values.length];

  // A switch expression must cover every value, or it does not compile.
  String get meaning => switch (this) {
    Light.red => 'stop',
    Light.green => 'go',
    Light.yellow => 'slow down',
  };
}

void showParse(String text) {
  // asNameMap() maps each name to its value; a missing name gives null.
  final light = Light.values.asNameMap()[text];
  print('parse "$text": ${light?.name ?? 'not a light'}');
}

void main() {
  print('lights: ${Light.values.map((light) => light.name).join(' ')}');
  print('yellow is number ${Light.yellow.index}');

  var light = Light.red;
  for (final _ in Light.values) {
    print('after ${light.name}: ${light.next.name}');
    light = light.next;
  }
  print(
    'red means ${Light.red.meaning}, green means ${Light.green.meaning}, '
    'yellow means ${Light.yellow.meaning}',
  );

  final total = Light.values.fold(0, (sum, each) => sum + each.seconds);
  print('one cycle: $total seconds');

  showParse('green');
  showParse('blue');
}
```

**Kotlin**

```kotlin
// An enum class can carry properties, set by its constructor.
enum class Light(val seconds: Int) {
    RED(30),
    GREEN(25),
    YELLOW(5);

    val next: Light
        get() = entries[(ordinal + 1) % entries.size]

    // A when expression must cover every value, or it does not compile.
    val meaning: String
        get() =
            when (this) {
                RED -> "stop"
                GREEN -> "go"
                YELLOW -> "slow down"
            }

    override fun toString() = name.lowercase()
}

// find returns null when nothing matches.
fun parseLight(text: String): Light? = Light.entries.find { it.toString() == text }

fun showParse(text: String) {
    println("parse \"$text\": ${parseLight(text) ?: "not a light"}")
}

fun main() {
    println("lights: ${Light.entries.joinToString(" ")}")
    println("yellow is number ${Light.YELLOW.ordinal}")

    var light = Light.RED
    repeat(Light.entries.size) {
        println("after $light: ${light.next}")
        light = light.next
    }
    println(
        "red means ${Light.RED.meaning}, green means ${Light.GREEN.meaning}, " +
            "yellow means ${Light.YELLOW.meaning}"
    )

    println("one cycle: ${Light.entries.sumOf { it.seconds }} seconds")

    showParse("green")
    showParse("blue")
}
```

Output, identical in all nine:

```text
lights: red green yellow
yellow is number 2
after red: green
after green: yellow
after yellow: red
red means stop, green means go, yellow means slow down
one cycle: 60 seconds
parse "green": green
parse "blue": not a light
```

The output is the same, but the code shows two families. In C, C++, and Go, an enum is a
named integer, and everything else about it (its name, the list of values, and how long
each light lasts) lives in arrays or `switch` statements beside it. In Rust, Python,
Dart, and Kotlin, an enum is a type of its own that cannot hold a fourth value. Dart's
and Kotlin's enums can also carry fields, set by a constructor, as in `RED(30)`.
JavaScript has no enum at all, and TypeScript's usual stand-in is a union of strings.

| Language | An enum value is | A bad value | Name as text | All the values | From text |
| --- | --- | --- | --- | --- | --- |
| C | An `int` | `Light l = 7;` compiles, with no warning | An array, by hand | A loop up to a count | By hand |
| C++ | A scoped integer that does not convert by itself | `Light l = 7;` is an error, `cannot convert 'int' to 'Light'`, but `static_cast<Light>(7)` compiles | By hand | By hand | By hand |
| Rust | A value of its own type | Impossible: `1 as Light` is E0605, `non-primitive cast` | By hand, or `{:?}` with `#[derive(Debug)]` prints `Red` | By hand, as a `const` array | By hand |
| Go | A named integer | `var l light = 7` compiles | A `String()` method | By hand | By hand |
| Python | An object with `.name` and `.value` | `Light(7)` raises `ValueError: 7 is not a valid Light` | `.name`, or the value of a `StrEnum` | `list(Light)` | `Light("red")`; `Light("blue")` raises `ValueError: 'blue' is not a valid Light` |
| JavaScript | Usually a string | Any value at all | The string itself | An array | `includes` or `find` |
| TypeScript | A string, checked against a union | `const l: Light = "blue"` is an error: `Type '"blue"' is not assignable to type '"green" \| "red" \| "yellow"'.` | The string itself | The array the union came from | `find` |
| Dart | An object with `.name` and `.index` | Impossible | `.name` | `Light.values` | `Light.values.asNameMap()[s]`; `byName(s)` throws `Invalid argument (name): No enum value with that name: "blue"` |
| Kotlin | An object with `name` and `ordinal` | Impossible | `name` | `Light.entries` (Kotlin 1.9) | `Light.valueOf("RED")`; a bad name throws `IllegalArgumentException: No enum constant Light.BLUE` |

**C, C++, and Go enums can hold any number.** C's `Light l = 7;` and Go's
`var l light = 7` compile without complaint, and C++ allows it through a cast. That is
why the C, C++, and Go versions of `meaning` end with a fallback `return "?"` after a
`switch` that already covers every light. In Rust, Dart, and Kotlin there is no fourth
light to fall back for.

**Go's zero value is the first constant.** `iota` starts at 0, so a `light` field that
was never set reads as `red`. Go programs often start the list with an `unknown`
constant so that "not set" cannot pass for a real value.

**Python's `StrEnum` equals its string.** `Light.RED == "red"` is `True` for a
`StrEnum`, which is handy for parsing and printing. A plain `Enum` member never equals
its value: with `class Color(Enum): RED = 1`, `Color.RED == 1` is `False`.

**TypeScript's `enum` is not allowed here.** TypeScript has an `enum` keyword, but it
generates an object at run time, so it is not plain JavaScript with types removed. The
book's checker passes `--erasableSyntaxOnly`, which rejects it:
`error TS1294: This syntax is not allowed when 'erasableSyntaxOnly' is enabled.` Node,
which runs TypeScript by deleting the types, stops with this:

```text
SyntaxError [ERR_UNSUPPORTED_TYPESCRIPT_SYNTAX]: TypeScript enum is not supported in strip-only mode
```

A union of string literals, built from an `as const` array as in the example, needs no
code at run time, and its values print as themselves.

### Checking every case

Delete the `yellow` case from `meaning` and see who notices:

| Language | What happens |
| --- | --- |
| C | Warning, with `-Wall`: `enumeration value 'YELLOW' not handled in switch [-Wswitch]` |
| C++ | Warning, with `-Wall`: `enumeration value 'Yellow' not handled in switch [-Wswitch]` |
| Rust | Error E0004: `non-exhaustive patterns: 'Light::Yellow' not covered` |
| Go | Nothing; yellow falls through to `return "?"` |
| Python | Nothing; `meaning` returns `None` for yellow |
| JavaScript | Nothing until run time, when the `default` case throws |
| TypeScript | Error, because the function could end without returning: `error TS2366: Function lacks ending return statement and return type does not include 'undefined'.` |
| Dart | Error: `The type 'Light' isn't exhaustively matched by the switch cases since it doesn't match the pattern 'Light.yellow'.` |
| Kotlin | Error: `'when' expression must be exhaustive. Add the 'YELLOW' branch or an 'else' branch.` |

The book's checker compiles C and C++ with `-Werror`, so their warnings stop the build
too. The check works only while the `switch` lists the values one by one. A `default`
case in C, C++, or TypeScript, a `_` arm in Rust or Dart, or an `else` in Kotlin covers
every value, including ones added next year, so the compiler stops pointing out the
places that need a new case.

---

## 8.5 Sum types

The program below describes four payments for a 12.50 order: cash, a card, a small
voucher, and a voucher that covers the whole order. Each kind of payment carries
different data, and `describe` handles each kind, with an extra condition for a voucher
that covers the total. The code is in
[`examples/03-sum-types/`](examples/03-sum-types/).

**C**

```c
#include <stdio.h>

// A tagged union: the tag says which member of the union holds the data.
typedef enum { CASH, CARD, VOUCHER } PaymentKind;

typedef struct {
    PaymentKind kind;
    union {
        struct {
            const char *last4;
        } card;
        struct {
            const char *code;
            long cents;
        } voucher;
    };
} Payment;

// Writes cents as dollars and cents, such as 12.50, into buf.
static const char *money(long cents, char buf[static 32]) {
    snprintf(buf, 32, "%ld.%02ld", cents / 100, cents % 100);
    return buf;
}

static void describe(const Payment *payment, long total) {
    char a[32], b[32];
    // Nothing stops this code from reading payment->voucher when kind is CARD.
    // Checking the tag first is up to you.
    switch (payment->kind) {
    case CASH:
        printf("cash: pay %s in cash\n", money(total, a));
        break;
    case CARD:
        printf("card ending %s: charge %s to the card\n", payment->card.last4,
               money(total, a));
        break;
    case VOUCHER: {
        long cents = payment->voucher.cents;
        if (cents >= total) {
            printf("voucher %s (%s): covers the order\n", payment->voucher.code,
                   money(cents, a));
        } else {
            printf("voucher %s (%s): %s left to pay\n", payment->voucher.code,
                   money(cents, a), money(total - cents, b));
        }
        break;
    }
    }
}

int main(void) {
    long total = 1250;
    const Payment payments[] = {
        {.kind = CASH},
        {.kind = CARD, .card = {"4242"}},
        {.kind = VOUCHER, .voucher = {"SPRING", 500}},
        {.kind = VOUCHER, .voucher = {"BIG", 2000}},
    };
    char buf[32];
    printf("order total: %s\n", money(total, buf));
    for (size_t i = 0; i < sizeof payments / sizeof payments[0]; i++) {
        describe(&payments[i], total);
    }
    return 0;
}
```

**C++**

```cpp
#include <format>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

struct Cash {};
struct Card {
    std::string last4;
};
struct Voucher {
    std::string code;
    long cents;
};

// A variant holds exactly one of its types, and remembers which one.
using Payment = std::variant<Cash, Card, Voucher>;

// Combines several lambdas into one object that std::visit can call.
template <class... Fs> struct overloaded : Fs... {
    using Fs::operator()...;
};

std::string money(long cents) {
    return std::format("{}.{:02}", cents / 100, cents % 100);
}

std::string describe(const Payment &payment, long total) {
    // std::visit calls the lambda for the type the variant holds.
    return std::visit(
        overloaded{
            [&](const Cash &) { return std::format("cash: pay {} in cash", money(total)); },
            [&](const Card &card) {
                return std::format("card ending {}: charge {} to the card", card.last4,
                                   money(total));
            },
            [&](const Voucher &voucher) {
                if (voucher.cents >= total) {
                    return std::format("voucher {} ({}): covers the order", voucher.code,
                                       money(voucher.cents));
                }
                return std::format("voucher {} ({}): {} left to pay", voucher.code,
                                   money(voucher.cents), money(total - voucher.cents));
            },
        },
        payment);
}

int main() {
    long total = 1250;
    std::vector<Payment> payments = {Cash{}, Card{"4242"}, Voucher{"SPRING", 500},
                                     Voucher{"BIG", 2000}};
    std::cout << "order total: " << money(total) << '\n';
    for (const auto &payment : payments) {
        std::cout << describe(payment, total) << '\n';
    }
}
```

**Rust**

```rust
// Each variant of an enum can carry its own fields.
enum Payment {
    Cash,
    Card { last4: String },
    Voucher { code: String, cents: i64 },
}

fn money(cents: i64) -> String {
    format!("{}.{:02}", cents / 100, cents % 100)
}

fn describe(payment: &Payment, total: i64) -> String {
    // match must cover every variant. The `if` on the third arm is a guard.
    match payment {
        Payment::Cash => format!("cash: pay {} in cash", money(total)),
        Payment::Card { last4 } => {
            format!("card ending {last4}: charge {} to the card", money(total))
        }
        Payment::Voucher { code, cents } if *cents >= total => {
            format!("voucher {code} ({}): covers the order", money(*cents))
        }
        Payment::Voucher { code, cents } => {
            let left = money(total - cents);
            format!("voucher {code} ({}): {left} left to pay", money(*cents))
        }
    }
}

fn main() {
    let total = 1250;
    let payments = [
        Payment::Cash,
        Payment::Card {
            last4: "4242".to_string(),
        },
        Payment::Voucher {
            code: "SPRING".to_string(),
            cents: 500,
        },
        Payment::Voucher {
            code: "BIG".to_string(),
            cents: 2000,
        },
    ];
    println!("order total: {}", money(total));
    for payment in &payments {
        println!("{}", describe(payment, total));
    }
}
```

**Go**

```go
package main

import "fmt"

// Go has no sum types. The usual stand-in is an interface that only the variants
// implement; the unexported method keeps other packages from adding more.
type payment interface{ isPayment() }

type cash struct{}

type card struct{ last4 string }

type voucher struct {
	code  string
	cents int
}

func (cash) isPayment()    {}
func (card) isPayment()    {}
func (voucher) isPayment() {}

func money(cents int) string {
	return fmt.Sprintf("%d.%02d", cents/100, cents%100)
}

func describe(p payment, total int) string {
	// A type switch runs the case for the type p holds. Nothing checks that every
	// variant has a case, so default catches anything missed.
	switch p := p.(type) {
	case cash:
		return fmt.Sprintf("cash: pay %s in cash", money(total))
	case card:
		return fmt.Sprintf("card ending %s: charge %s to the card", p.last4, money(total))
	case voucher:
		if p.cents >= total {
			return fmt.Sprintf("voucher %s (%s): covers the order", p.code, money(p.cents))
		}
		left := money(total - p.cents)
		return fmt.Sprintf("voucher %s (%s): %s left to pay", p.code, money(p.cents), left)
	default:
		return "unknown payment"
	}
}

func main() {
	total := 1250
	payments := []payment{cash{}, card{"4242"}, voucher{"SPRING", 500}, voucher{"BIG", 2000}}
	fmt.Println("order total:", money(total))
	for _, p := range payments {
		fmt.Println(describe(p, total))
	}
}
```

**Python**

```python
from dataclasses import dataclass


@dataclass(frozen=True)
class Cash:
    pass


@dataclass(frozen=True)
class Card:
    last4: str


@dataclass(frozen=True)
class Voucher:
    code: str
    cents: int


# A union of classes: a Payment is exactly one of these three.
type Payment = Cash | Card | Voucher


def money(cents: int) -> str:
    return f"{cents // 100}.{cents % 100:02}"


def describe(payment: Payment, total: int) -> str:
    # Each case is a pattern. Card(last4) checks the type and binds its field.
    match payment:
        case Cash():
            return f"cash: pay {money(total)} in cash"
        case Card(last4):
            return f"card ending {last4}: charge {money(total)} to the card"
        case Voucher(code, cents) if cents >= total:
            return f"voucher {code} ({money(cents)}): covers the order"
        case Voucher(code, cents):
            left = money(total - cents)
            return f"voucher {code} ({money(cents)}): {left} left to pay"


def main() -> None:
    total = 1250
    payments: list[Payment] = [
        Cash(),
        Card("4242"),
        Voucher("SPRING", 500),
        Voucher("BIG", 2000),
    ]
    print(f"order total: {money(total)}")
    for payment in payments:
        print(describe(payment, total))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// A plain object whose kind field says which variant it is.
const payments = [
  { kind: "cash" },
  { kind: "card", last4: "4242" },
  { kind: "voucher", code: "SPRING", cents: 500 },
  { kind: "voucher", code: "BIG", cents: 2000 },
];

const money = (cents) =>
  `${Math.floor(cents / 100)}.${String(cents % 100).padStart(2, "0")}`;

function describe(payment, total) {
  switch (payment.kind) {
    case "cash":
      return `cash: pay ${money(total)} in cash`;
    case "card":
      return `card ending ${payment.last4}: charge ${money(total)} to the card`;
    case "voucher": {
      const { code, cents } = payment;
      if (cents >= total) {
        return `voucher ${code} (${money(cents)}): covers the order`;
      }
      return `voucher ${code} (${money(cents)}): ${money(total - cents)} left to pay`;
    }
    default:
      throw new Error(`unknown payment kind: ${payment.kind}`);
  }
}

const total = 1250;
console.log(`order total: ${money(total)}`);
for (const payment of payments) {
  console.log(describe(payment, total));
}
```

**TypeScript**

```ts
// A discriminated union: the kind field tells the compiler which variant it has.
type Payment =
  | { kind: "cash" }
  | { kind: "card"; last4: string }
  | { kind: "voucher"; code: string; cents: number };

const money = (cents: number): string =>
  `${Math.floor(cents / 100)}.${String(cents % 100).padStart(2, "0")}`;

function describe(payment: Payment, total: number): string {
  switch (payment.kind) {
    case "cash":
      return `cash: pay ${money(total)} in cash`;
    case "card": // in this case, TypeScript knows payment has last4
      return `card ending ${payment.last4}: charge ${money(total)} to the card`;
    case "voucher": {
      const { code, cents } = payment;
      if (cents >= total) {
        return `voucher ${code} (${money(cents)}): covers the order`;
      }
      return `voucher ${code} (${money(cents)}): ${money(total - cents)} left to pay`;
    }
    default: {
      // If a new kind is added and not handled above, this line stops compiling.
      const unhandled: never = payment;
      throw new Error(`unknown payment: ${JSON.stringify(unhandled)}`);
    }
  }
}

const payments: Payment[] = [
  { kind: "cash" },
  { kind: "card", last4: "4242" },
  { kind: "voucher", code: "SPRING", cents: 500 },
  { kind: "voucher", code: "BIG", cents: 2000 },
];
const total = 1250;
console.log(`order total: ${money(total)}`);
for (const payment of payments) {
  console.log(describe(payment, total));
}
```

**Dart**

```dart
// A sealed class can only be extended in this file, so the compiler knows every
// variant.
sealed class Payment {}

class Cash extends Payment {}

class Card(final String last4) extends Payment;

class Voucher(final String code, final int cents) extends Payment;

String money(int cents) =>
    '${cents ~/ 100}.${(cents % 100).toString().padLeft(2, '0')}';

// A switch expression on a sealed type must cover every subclass.
String describe(Payment payment, int total) => switch (payment) {
  Cash() => 'cash: pay ${money(total)} in cash',
  Card(:final last4) =>
    'card ending $last4: charge ${money(total)} to the card',
  Voucher(:final code, :final cents) when cents >= total =>
    'voucher $code (${money(cents)}): covers the order',
  Voucher(:final code, :final cents) =>
    'voucher $code (${money(cents)}): ${money(total - cents)} left to pay',
};

void main() {
  const total = 1250;
  final payments = [
    Cash(),
    Card('4242'),
    Voucher('SPRING', 500),
    Voucher('BIG', 2000),
  ];
  print('order total: ${money(total)}');
  for (final payment in payments) {
    print(describe(payment, total));
  }
}
```

**Kotlin**

```kotlin
// A sealed interface can only be implemented in this package and module, so the
// compiler knows every variant.
sealed interface Payment

data object Cash : Payment

data class Card(val last4: String) : Payment

data class Voucher(val code: String, val cents: Int) : Payment

fun money(cents: Int) = "%d.%02d".format(cents / 100, cents % 100)

// when on a sealed type must cover every variant. The `if` is a guard (Kotlin 2.2).
fun describe(payment: Payment, total: Int): String =
    when (payment) {
        Cash -> "cash: pay ${money(total)} in cash"
        is Card -> "card ending ${payment.last4}: charge ${money(total)} to the card"
        is Voucher if payment.cents >= total ->
            "voucher ${payment.code} (${money(payment.cents)}): covers the order"
        is Voucher -> {
            val left = money(total - payment.cents)
            "voucher ${payment.code} (${money(payment.cents)}): $left left to pay"
        }
    }

fun main() {
    val total = 1250
    val payments = listOf(Cash, Card("4242"), Voucher("SPRING", 500), Voucher("BIG", 2000))
    println("order total: ${money(total)}")
    for (payment in payments) {
        println(describe(payment, total))
    }
}
```

Output, identical in all nine:

```text
order total: 12.50
cash: pay 12.50 in cash
card ending 4242: charge 12.50 to the card
voucher SPRING (5.00): 7.50 left to pay
voucher BIG (20.00): covers the order
```

| Language | Declare the variants | Take one apart | Extra condition |
| --- | --- | --- | --- |
| C | An `enum` tag, plus a `union` of one `struct` per variant | `switch` on the tag, then read the matching union member | `if` inside the case |
| C++ | One `struct` per variant, and `std::variant<Cash, Card, Voucher>` | `std::visit` with one lambda per type, or `std::get_if<Card>(&p)` | `if` inside the lambda |
| Rust | `enum` with fields on each variant | `match`, which binds the fields: `Payment::Card { last4 } =>` | A guard: `if *cents >= total` |
| Go | One `struct` per variant, all implementing an `interface` | A type switch: `switch p := p.(type)` | `if` inside the case |
| Python | One `@dataclass` per variant, and `type Payment = Cash \| Card \| Voucher` | `match` with class patterns: `case Card(last4):` | A guard: `if cents >= total` |
| JavaScript | Objects with a `kind` field | `switch (payment.kind)` | `if` inside the case |
| TypeScript | A union of object types with a common `kind` field | `switch (payment.kind)`, which narrows the type in each case | `if` inside the case |
| Dart | A `sealed class` and one subclass per variant | A `switch` expression with object patterns: `Card(:final last4) =>` | A guard: `when cents >= total` |
| Kotlin | A `sealed interface`, with a `data object` or `data class` per variant | `when` with `is Card ->`, which smart-casts | A guard: `is Voucher if payment.cents >= total` (Kotlin 2.2) |

**The C version trusts you.** A tagged union is a promise that the code checks the tag
before reading the union. Nothing enforces it: reading `payment->voucher.cents` when
the tag says `CARD` compiles and reads whatever bytes the card left there. C++'s
`std::variant` keeps the tag for you and checks it: `std::get<Voucher>(payment)` on a
card throws `std::bad_variant_access`, with the message
`std::get: wrong index for variant`.

**Go's interface is open.** Any type with an `isPayment` method could join the
interface, which is why the method is unexported: no other package can add a variant.
But within the package nothing checks that the type switch covers every variant, so the
`default` case is there to catch a missing one.

**TypeScript narrows on the `kind` field.** Inside `case "card":`, TypeScript knows the
payment is the card variant, so `payment.last4` is allowed, and in the other cases it
is an error. This pattern is called a **discriminated union**. The `never` trick in the
`default` case turns a missing case into a compile error: if every kind was handled,
nothing is left for `payment` to be, so its type there is `never`.

### Checking every case

Add a fourth variant, a gift card, without touching `describe`:

| Language | What happens |
| --- | --- |
| C | Warning with `-Wall`, as for a plain enum: `enumeration value 'GIFT_CARD' not handled in switch` |
| C++ | `std::visit` fails to compile; the first of many lines is `error: no type named 'type' in 'struct std::invoke_result<...>'` |
| Rust | Error E0004: `non-exhaustive patterns: '&Payment::GiftCard { .. }' not covered` |
| Go, Python, JavaScript | Nothing; the new kind reaches the `default` case or falls off the end |
| TypeScript | Error in the `never` line: `Type '{ kind: "giftcard"; balance: number; }' is not assignable to type 'never'.` |
| Dart | Error: `The type 'Payment' isn't exhaustively matched by the switch cases since it doesn't match the pattern 'GiftCard()'.` |
| Kotlin | Error: `'when' expression must be exhaustive. Add the 'is GiftCard' branch or an 'else' branch.` |

A sum type also keeps out values that make no sense. The alternative is one struct with
a field for every kind: a `kind`, a `last4` that only cards use, and a `code` and
`cents` that only vouchers use. Then nothing stops a "cash" payment from having a card
number, every function has to decide what that means, and a reader cannot tell which
fields go together. With a sum type, a card payment has a `last4` and nothing else.
This is often summed up as **make illegal states unrepresentable**.

---

## 8.6 Null and optional values

The program below keeps a list of users, some with a nickname and some without. It
shows each user's nickname or, if there is none, their name; measures each nickname,
counting a missing one as 0; looks up users who may not exist; and chains the two, so
that a missing user and a missing nickname both end in "none". The code is in
[`examples/04-null/`](examples/04-null/).

**C**

```c
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *name;
    const char *nickname; // NULL when the user has no nickname
} User;

// Returns a pointer to the user, or NULL if nobody has that name.
static const User *find(const User *users, size_t count, const char *name) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(users[i].name, name) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

int main(void) {
    const User users[] = {
        {"ada", "Countess"},
        {"alan", NULL},
        {"grace", "Amazing Grace"},
    };
    size_t count = sizeof users / sizeof users[0];

    // Every pointer that might be NULL has to be checked before it is used.
    for (size_t i = 0; i < count; i++) {
        const char *shown = users[i].nickname != NULL ? users[i].nickname : users[i].name;
        printf("%s is shown as %s\n", users[i].name, shown);
    }

    printf("nickname lengths:");
    for (size_t i = 0; i < count; i++) {
        printf(" %zu", users[i].nickname != NULL ? strlen(users[i].nickname) : 0);
    }
    printf("\n");

    const char *lookups[] = {"alan", "linus"};
    for (size_t i = 0; i < 2; i++) {
        const User *user = find(users, count, lookups[i]);
        printf("find \"%s\": %s\n", lookups[i], user != NULL ? user->name : "nobody");
    }

    const char *names[] = {"ada", "alan", "linus"};
    for (size_t i = 0; i < 3; i++) {
        const User *user = find(users, count, names[i]);
        const char *nickname = "none";
        if (user != NULL && user->nickname != NULL) {
            nickname = user->nickname;
        }
        printf("nickname of \"%s\": %s\n", names[i], nickname);
    }
    return 0;
}
```

**C++**

```cpp
#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct User {
    std::string name;
    std::optional<std::string> nickname; // empty when the user has no nickname
};

// Returns a pointer to the user, or nullptr if nobody has that name.
const User *find(const std::vector<User> &users, std::string_view name) {
    auto it = std::ranges::find(users, name, &User::name);
    return it != users.end() ? &*it : nullptr;
}

int main() {
    std::vector<User> users = {
        {"ada", "Countess"},
        {"alan", std::nullopt},
        {"grace", "Amazing Grace"},
    };

    for (const auto &user : users) {
        // value_or gives a default when the optional is empty.
        std::cout << user.name << " is shown as " << user.nickname.value_or(user.name)
                  << '\n';
    }

    std::cout << "nickname lengths:";
    for (const auto &user : users) {
        std::cout << ' ' << (user.nickname ? user.nickname->size() : 0);
    }
    std::cout << '\n';

    for (std::string_view name : {"alan", "linus"}) {
        const User *user = find(users, name);
        std::cout << "find \"" << name << "\": " << (user ? user->name : "nobody") << '\n';
    }

    for (std::string_view name : {"ada", "alan", "linus"}) {
        const User *user = find(users, name);
        std::string nickname = "none";
        if (user != nullptr && user->nickname) {
            nickname = *user->nickname;
        }
        std::cout << "nickname of \"" << name << "\": " << nickname << '\n';
    }
}
```

**Rust**

```rust
struct User {
    name: String,
    nickname: Option<String>, // None when the user has no nickname
}

fn find<'a>(users: &'a [User], name: &str) -> Option<&'a User> {
    users.iter().find(|user| user.name == name)
}

fn main() {
    let users = vec![
        User {
            name: "ada".to_string(),
            nickname: Some("Countess".to_string()),
        },
        User {
            name: "alan".to_string(),
            nickname: None,
        },
        User {
            name: "grace".to_string(),
            nickname: Some("Amazing Grace".to_string()),
        },
    ];

    for user in &users {
        // unwrap_or gives a default when the Option is None.
        let shown = user.nickname.as_deref().unwrap_or(&user.name);
        println!("{} is shown as {shown}", user.name);
    }

    // map_or applies a function to the value inside, or gives a default for None.
    let lengths: Vec<String> = users
        .iter()
        .map(|user| user.nickname.as_deref().map_or(0, str::len).to_string())
        .collect();
    println!("nickname lengths: {}", lengths.join(" "));

    for name in ["alan", "linus"] {
        match find(&users, name) {
            Some(user) => println!("find \"{name}\": {}", user.name),
            None => println!("find \"{name}\": nobody"),
        }
    }

    for name in ["ada", "alan", "linus"] {
        // and_then chains a second step that may also give None.
        let nickname = find(&users, name).and_then(|user| user.nickname.as_deref());
        println!("nickname of \"{name}\": {}", nickname.unwrap_or("none"));
    }
}
```

**Go**

```go
package main

import "fmt"

// Go has no optional type. A zero value such as "" often stands for "none", and a
// second result, ok, says whether a lookup found anything.
type user struct {
	name     string
	nickname string // "" when the user has no nickname
}

func find(users []user, name string) (user, bool) {
	for _, u := range users {
		if u.name == name {
			return u, true
		}
	}
	return user{}, false
}

func main() {
	users := []user{
		{name: "ada", nickname: "Countess"},
		{name: "alan"},
		{name: "grace", nickname: "Amazing Grace"},
	}

	for _, u := range users {
		shown := u.name
		if u.nickname != "" {
			shown = u.nickname
		}
		fmt.Printf("%s is shown as %s\n", u.name, shown)
	}

	fmt.Print("nickname lengths:")
	for _, u := range users {
		fmt.Print(" ", len(u.nickname))
	}
	fmt.Println()

	for _, name := range []string{"alan", "linus"} {
		if u, ok := find(users, name); ok {
			fmt.Printf("find %q: %s\n", name, u.name)
		} else {
			fmt.Printf("find %q: nobody\n", name)
		}
	}

	for _, name := range []string{"ada", "alan", "linus"} {
		nickname := "none"
		if u, ok := find(users, name); ok && u.nickname != "" {
			nickname = u.nickname
		}
		fmt.Printf("nickname of %q: %s\n", name, nickname)
	}
}
```

**Python**

```python
from dataclasses import dataclass


@dataclass
class User:
    name: str
    nickname: str | None = None  # None when the user has no nickname


def find(users: list[User], name: str) -> User | None:
    return next((user for user in users if user.name == name), None)


def main() -> None:
    users = [User("ada", "Countess"), User("alan"), User("grace", "Amazing Grace")]

    # Python has no ?. or ?? operator, so the checks for None are spelled out.
    for user in users:
        shown = user.nickname if user.nickname is not None else user.name
        print(f"{user.name} is shown as {shown}")

    lengths = [len(user.nickname) if user.nickname is not None else 0 for user in users]
    print("nickname lengths:", *lengths)

    for name in ["alan", "linus"]:
        found = find(users, name)
        print(f'find "{name}": {found.name if found is not None else "nobody"}')

    for name in ["ada", "alan", "linus"]:
        found = find(users, name)
        nickname = found.nickname if found is not None else None
        print(f'nickname of "{name}": {nickname if nickname is not None else "none"}')


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// alan has no nickname property at all, so reading it gives undefined.
const users = [
  { name: "ada", nickname: "Countess" },
  { name: "alan" },
  { name: "grace", nickname: "Amazing Grace" },
];

// Array find gives undefined when nothing matches.
const find = (name) => users.find((user) => user.name === name);

// a ?? b is b only when a is null or undefined.
for (const user of users) {
  console.log(`${user.name} is shown as ${user.nickname ?? user.name}`);
}

// a?.b stops and gives undefined when a is null or undefined.
const lengths = users.map((user) => user.nickname?.length ?? 0);
console.log("nickname lengths:", lengths.join(" "));

for (const name of ["alan", "linus"]) {
  console.log(`find "${name}": ${find(name)?.name ?? "nobody"}`);
}

for (const name of ["ada", "alan", "linus"]) {
  console.log(`nickname of "${name}": ${find(name)?.nickname ?? "none"}`);
}
```

**TypeScript**

```ts
// nickname?: string means the property may be missing, so its type is
// string | undefined, and the compiler makes you handle undefined.
type User = { name: string; nickname?: string };

const users: User[] = [
  { name: "ada", nickname: "Countess" },
  { name: "alan" },
  { name: "grace", nickname: "Amazing Grace" },
];

const find = (name: string): User | undefined =>
  users.find((user) => user.name === name);

// a ?? b is b only when a is null or undefined.
for (const user of users) {
  console.log(`${user.name} is shown as ${user.nickname ?? user.name}`);
}

// a?.b stops and gives undefined when a is null or undefined.
const lengths = users.map((user) => user.nickname?.length ?? 0);
console.log("nickname lengths:", lengths.join(" "));

for (const name of ["alan", "linus"]) {
  console.log(`find "${name}": ${find(name)?.name ?? "nobody"}`);
}

for (const name of ["ada", "alan", "linus"]) {
  console.log(`nickname of "${name}": ${find(name)?.nickname ?? "none"}`);
}
```

**Dart**

```dart
// String? may hold null; plain String never does, and the compiler checks it.
class User(final String name, {final String? nickname});

void main() {
  final users = [
    User('ada', nickname: 'Countess'),
    User('alan'),
    User('grace', nickname: 'Amazing Grace'),
  ];

  // firstOrNull gives null when nothing matches.
  User? find(String name) =>
      users.where((user) => user.name == name).firstOrNull;

  // a ?? b is b only when a is null.
  for (final user in users) {
    print('${user.name} is shown as ${user.nickname ?? user.name}');
  }

  // a?.b stops and gives null when a is null.
  final lengths = users.map((user) => user.nickname?.length ?? 0);
  print('nickname lengths: ${lengths.join(' ')}');

  for (final name in ['alan', 'linus']) {
    print('find "$name": ${find(name)?.name ?? 'nobody'}');
  }

  for (final name in ['ada', 'alan', 'linus']) {
    print('nickname of "$name": ${find(name)?.nickname ?? 'none'}');
  }
}
```

**Kotlin**

```kotlin
// String? may hold null; plain String never does, and the compiler checks it.
data class User(val name: String, val nickname: String? = null)

// find gives null when nothing matches.
fun find(users: List<User>, name: String): User? = users.find { it.name == name }

fun main() {
    val users = listOf(User("ada", "Countess"), User("alan"), User("grace", "Amazing Grace"))

    // a ?: b (the Elvis operator) is b only when a is null.
    for (user in users) {
        println("${user.name} is shown as ${user.nickname ?: user.name}")
    }

    // a?.b stops and gives null when a is null.
    val lengths = users.map { it.nickname?.length ?: 0 }
    println("nickname lengths: ${lengths.joinToString(" ")}")

    for (name in listOf("alan", "linus")) {
        println("find \"$name\": ${find(users, name)?.name ?: "nobody"}")
    }

    for (name in listOf("ada", "alan", "linus")) {
        println("nickname of \"$name\": ${find(users, name)?.nickname ?: "none"}")
    }
}
```

Output, identical in all nine:

```text
ada is shown as Countess
alan is shown as alan
grace is shown as Amazing Grace
nickname lengths: 8 0 13
find "alan": alan
find "linus": nobody
nickname of "ada": Countess
nickname of "alan": none
nickname of "linus": none
```

The languages fall into three groups. In **C, Go, Python, and JavaScript**, null is
allowed wherever a pointer or object is, and the code has to remember to check. In
**Rust**, references cannot be null, and a value that may be missing has the type
`Option<T>`. (Raw pointers can be null, but only `unsafe` code may follow them.) In
**TypeScript, Dart, and Kotlin**, a type says whether it allows null (`String` or
`String?` in Dart and Kotlin, `string` or `string | null` in TypeScript), and the
compiler refuses to use a value that may be null until the code has checked. C++ sits
between the first two groups: a pointer can be null, but a value that may be missing can
be a `std::optional`.

### Using a missing value by mistake

| Language | Using the missing nickname as if it were there |
| --- | --- |
| C | `strlen(NULL)` is undefined behavior; on the machine that tested this book, `Segmentation fault` |
| C++ | Dereferencing a null pointer is undefined behavior. `opt.value()` on an empty optional throws `std::bad_optional_access`, whose message is `bad optional access`; `*opt` is undefined behavior |
| Rust | Does not compile: `Option<String>` has no `to_uppercase` (E0599), and is not a `String` (E0308). `unwrap()` on `None` compiles, then panics: `called 'Option::unwrap()' on a 'None' value` |
| Go | `panic: runtime error: invalid memory address or nil pointer dereference` |
| Python | `AttributeError: 'NoneType' object has no attribute 'upper'` |
| JavaScript | `TypeError: Cannot read properties of undefined (reading 'length')` |
| TypeScript | Does not compile with `--strict`: `error TS18048: 'user.nickname' is possibly 'undefined'.` |
| Dart | Does not compile: `The property 'length' can't be unconditionally accessed because the receiver can be 'null'.` |
| Kotlin | Does not compile: `only safe (?.) or non-null asserted (!!.) calls are allowed on a nullable receiver of type 'String?'.` |

Each of the checked languages also has a way to say "trust me, it is there". Rust's
`unwrap()` and `expect("reason")` panic on `None`. Dart's `s!` throws
`Null check operator used on a null value`, and Kotlin's `s!!` throws
`NullPointerException`. TypeScript's `s!` only silences the compiler: it is deleted
along with the other types, so a missing value still fails at run time with the
JavaScript `TypeError`. Use these when the value cannot be missing, and the reason is
something the compiler cannot see.

### Operators for missing values

| Language | Use a default | Use the value only if it is there | Insist that it is there |
| --- | --- | --- | --- |
| C | `p != NULL ? p : d` | `if (p != NULL)` | Use it; undefined behavior if `NULL` |
| C++ | `opt.value_or(d)` | `if (opt)`, then `*opt` or `opt->size()` | `opt.value()` |
| Rust | `opt.unwrap_or(d)` | `if let Some(x) = opt`, `opt.map(f)`, `opt.and_then(f)`, `?` in a function that returns `Option` | `opt.unwrap()`, `opt.expect("reason")` |
| Go | Zero values, and `v, ok` from a lookup | `if ok`, `if p != nil` | Use it; panics if `nil` |
| Python | `x if x is not None else d` | `if x is not None:` | Use it; raises if `None` |
| JavaScript | `x ?? d` | `x?.length`, `x?.method()` | Use it; throws if `null` or `undefined` |
| TypeScript | `x ?? d` | `x?.length` | `x!`, checked by nobody |
| Dart | `x ?? d` | `x?.length` | `x!` |
| Kotlin | `x ?: d` (the Elvis operator) | `x?.length`, `x?.let { ... }` | `x!!` |

**Python has no `?.` or `??`.** It spells out `is not None`, as the example does. The
shortcut `x or d` is common, but it replaces every falsy value, not just `None`:
`"" or "default"` is `'default'` and `0 or 5` is `5`. Use it only when an empty string
or a zero really should get the default. JavaScript's `||` has the same problem, which
is why `??` was added in ES2020: `0 || 5` is `5`, but `0 ?? 5` is `0`.

**Rust's `Option` costs nothing for references.** A reference can never be null, so
Rust stores `None` as the null address: `Option<&u8>` takes 8 bytes on a 64-bit
machine, the same as `&u8`. An `Option<i32>` needs room for a separate tag, so it takes
8 bytes where an `i32` takes 4.

**C++ has no optional reference.** `std::optional<int&>` does not compile before C++26,
so a C++ function that may find nothing, like `find` in the example, returns a pointer
that may be `nullptr`.

### JavaScript's two nothings

JavaScript has both `undefined` and `null`. `undefined` is what you get when something
was never set: a missing property, a missing argument, a variable declared without a
value, a function with no `return`, or an index past the end of an array. `null` is a
value that code sets on purpose to mean "empty". In practice they mean the same thing,
and the language mostly treats them alike, with some exceptions:

```js
const user = { name: "alan" };
console.log(user.nickname); // undefined: there is no such property
console.log(typeof undefined, typeof null); // undefined object
console.log(null == undefined, null === undefined); // true false
console.log(JSON.stringify({ a: undefined, b: null })); // {"b":null}
const greet = (name = "you") => `hi ${name}`;
console.log(greet(undefined), greet(null)); // hi you hi null
```

`typeof null` is `"object"` because of a bug in the first version of JavaScript, kept
ever since so that old code keeps working. JSON has `null` but no `undefined`, so
`JSON.stringify` drops properties whose value is `undefined`. A default parameter
replaces `undefined` but not `null`. The `?.` and `??` operators treat both as missing,
which is usually what you want.

TypeScript distinguishes a property that may be left out from one that must be present
but may hold `undefined`. `nickname?: string`, as in the example, may be left out, and
reading it gives `string | undefined`. With `nickname: string | undefined`, an object
without the property is an error:

```text
Property 'nickname' is missing in type '{ name: string; }' but required in type 'User'.
```

### Go's many nils

Go's `nil` is the zero value of pointers, slices, maps, channels, functions, and
interfaces, and each behaves differently. A `nil` slice works like an empty one:
`len` is 0, and `append` works. A `nil` map can be read but not written (section 7.5).
A `nil` pointer panics when you follow it. And an interface holding a `nil` pointer is
not itself `nil`:

```go
var p *myError = nil
var err error = p
fmt.Println(err == nil) // false
```

An interface value is a pair, a type and a value, and it is `nil` only when both are.
Here the type is `*myError`, so `err != nil`, and code that checks `if err != nil`
reports an error that is not there. Chapter 11 returns to this.

---

## 8.7 Why each language looks the way it does

- **C** gives you the raw pieces and trusts you with them. A `struct` lays its fields
  out in memory in order, a `union` lets several types share the same bytes to save
  memory, and an `enum` names integer constants. `NULL` is a pointer that points
  nowhere, and checking for it is your job.
- **C++** inherited C's pieces and made them safer in the library rather than the
  language. `enum class` (C++11) stopped enums from turning into integers. C++17 added
  `std::optional` and `std::variant`, modeled on the Boost libraries of the same
  names. C++20 let the compiler write `==` and the other comparisons with `= default`.
- **Rust** comes from the ML family, where sum types and `match` are the everyday way
  to model data. Leaving out null was a deliberate choice: `Option` is an ordinary
  enum, and the compiler's exhaustiveness check makes every `None` visible. `derive`
  writes the boilerplate, but only when you ask, so a type never gets `==` or copying
  by accident.
- **Go** keeps the language small. Structs and zero values cover most needs, typed
  constants with `iota` stand in for enums, and interfaces with type switches stand in
  for sum types. The cost is that the compiler cannot check that a switch covers every
  case, and `nil` means slightly different things for different types.
- **Python** builds everything from classes. `@dataclass` (Python 3.7) writes the
  usual methods from a list of fields, `enum` arrived in Python 3.4, and `match`
  (Python 3.10) gave Python pattern matching on classes. Type hints such as
  `str | None` describe the code for tools like mypy, but Python itself does not check
  them when it runs.
- **JavaScript** grew from a small scripting language where objects are bags of
  properties. It has two values for nothing: `null`, which it took from Java along
  with much of its look, and `undefined`, which the language itself uses for anything
  never set. ES2020 added `?.` and `??` to make both easier to handle.
- **TypeScript** describes JavaScript's objects and cannot change what they do at run
  time. Its unions and narrowing are strong enough to give JavaScript sum types without
  new run-time code. `strictNullChecks` (TypeScript 2.0, 2016) made null a separate
  type, and `--erasableSyntaxOnly` (TypeScript 5.8, 2025) exists because Node now runs
  TypeScript by deleting the types, which only works for syntax like unions, not
  `enum`.
- **Dart** moved from "anything can be null" to sound null safety in Dart 2.12 (2021),
  and Dart 3 (2023) made it required, along with records, patterns, and `sealed`
  classes. Primary constructors (Dart 3.13) shortened the class definitions in this
  chapter. Dart still has no data classes, so `==`, `hashCode`, and `copyWith` are
  written by hand or generated by a package.
- **Kotlin** set out to remove one of Java's most common crashes, the
  `NullPointerException`, so nullable types were in Kotlin 1.0 from the start.
  `data class` writes the methods Java programmers wrote by hand, and `sealed` classes
  with `when` give it sum types that the compiler checks.

---

## 8.8 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | Reading a local struct before setting it | Undefined behavior; `Point p = {0};` zeroes it |
| C | Reading the wrong member of a tagged union | Compiles, and reads leftover bytes |
| C | Assuming an `enum` variable holds one of its constants | It can hold any `int` |
| C, C++ | A `default:` in a switch over an enum | Turns off the warning for a missing case |
| C++ | `*opt` on an empty `std::optional` | Undefined behavior; `value()` throws instead |
| C++ | `std::get<T>` on a variant holding another type | Throws `std::bad_variant_access` |
| Rust | `let r = p;` for a struct that is not `Copy` | Moves `p`; using it again is E0382 |
| Rust | `unwrap()` on a `None` you did not expect | Panics: `called 'Option::unwrap()' on a 'None' value` |
| Go | A field whose enum type starts at `iota` = 0 | An unset field looks like the first constant |
| Go | A `nil` pointer stored in an interface | The interface is not `nil` |
| Go | `==` on a struct with a slice, map, or function field | Compile error: `struct containing []int cannot be compared` |
| Python | `x or default` when `x` may be `""` or `0` | The real value is replaced too |
| Python | A plain class used as a record | `==` compares identity; use `@dataclass` |
| Python | A list as a dataclass default | `ValueError`; use `field(default_factory=list)` |
| JavaScript | `===` on two objects with the same fields | `false`: it compares identity |
| JavaScript | `typeof x === "object"` to check for an object | Also true for `null` |
| JavaScript | `x \|\| d` when `x` may be `0` or `""` | The real value is replaced; use `??` |
| JavaScript | A default parameter for `null` | Defaults replace only `undefined` |
| TypeScript | `enum` with `--erasableSyntaxOnly` | `error TS1294`; use a union of strings |
| TypeScript | `x!` | Silences the compiler but checks nothing at run time |
| Dart | `==` on two class instances with the same fields | `false` unless the class overrides `==` and `hashCode` |
| Dart | A `late` variable read before it is set | `LateInitializationError: Local 'name' has not been initialized.` |
| Kotlin | `!!` on a value that is `null` | `NullPointerException` |
| Kotlin | A `lateinit var` read before it is set | `UninitializedPropertyAccessException: lateinit property name has not been initialized` |
| Kotlin | `var` fields in a data class used as a map key or set member | Changing a field changes its hash, so the entry is lost |
| All | A struct with optional fields for every kind of value | Any mix of fields is possible; use a sum type |

---

## 8.9 Idiom check

**C: name the struct with `typedef`, and pair every union with a tag.** Initialize with
designated initializers, `{.x = 1, .y = 2}`, so that the code still reads correctly if
the fields are reordered. Keep a count after the last enum constant, or beside it as
the examples do, to size arrays and loops. Return a pointer that may be `NULL` for "not
found", and say so in a comment.

**C++: aggregates, `= default`, and the library types.** A plain `struct` with public
fields and `bool operator==(const T &) const = default;` covers most records. Use
`enum class`, `std::optional<T>` for a value that may be missing, and `std::variant`
with `std::visit` for a sum type. Avoid `default:` in switches over your own enums.

**Rust: `derive` what you need, and `match` on enums.** A struct usually starts with
`#[derive(Debug, Clone, PartialEq)]`, plus `Copy` for small ones. Model choices as enums
with data, return `Option<T>` for a value that may be missing, and prefer `map`,
`and_then`, `unwrap_or`, and `?` to `unwrap()`.

**Go: useful zero values and `v, ok`.** Design structs so that the zero value is a
valid, empty starting point. Use `iota` constants with a `String()` method, and reserve
0 for "unknown" when that matters. Return `(value, bool)` or `(value, error)` rather
than a `nil` pointer for "not found".

**Python: `@dataclass`, `Enum`, and `match`.** Use `@dataclass(frozen=True)` for small
records that should not change, `Enum` or `StrEnum` for fixed choices, and `match` with
class patterns for sum types. Write `x: str | None = None` for an optional field, and
compare with `is None`.

**JavaScript and TypeScript: plain objects, unions, and `??`.** Use object literals for
records and spread to copy them. In TypeScript, use unions of string literals for
enums and discriminated unions with a `never` check for sum types. Use `?.` and `??`
rather than `||` and `&&` for missing values.

**Dart: primary constructors, enhanced enums, and sealed classes.** Declare simple
classes with a primary constructor and `final` fields, use records when value equality
is enough, and use `sealed` classes with switch expressions for sum types. Keep `!` for
the rare case where the code knows more than the compiler.

**Kotlin: `data class`, `sealed interface`, and `?.` with `?:`.** Make data classes
with `val` fields and change them with `copy`. Model choices with sealed interfaces,
`data object`, and exhaustive `when` expressions without `else`. Chain nullable values
with `?.`, finish with `?:`, and treat every `!!` as a question to answer in a code
review.

---

## 8.10 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Shape areas](exercises/01-shape-areas/) | A sum type for the shapes, one exhaustive match, parsing numbers strictly |
| 2 | [Optional config values](exercises/02-config/) | Keeping "not set" apart from the default, optional types or sentinels, strict whole numbers |
| 3 | [A robot on a grid](exercises/03-robot/) | An enum for directions, a struct for the robot, turning with modular arithmetic |

```sh
python3 tools/check.py 08                     # your answers, every language
python3 tools/check.py 08 --lang rust,kotlin  # some languages
python3 tools/check.py 08 --solutions         # the reference answers
```

**Stretch:** the C and Go answers to exercise 2 use 0 to mean "no timeout set". Change
the rules so that `timeout = 0` is allowed and means "never time out", printed as
`timeout: never`, and update your C or Go answer. What replaces the sentinel: a
pointer, a separate `bool`, or a struct of both? Compare that with what changes in a
language that has an optional type.

---

Previous: [Chapter 7: Built-in collections](../07-built-in-collections/) ·
Next: [Chapter 9: Memory model](../09-memory-model/)
