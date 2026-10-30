# Chapter 2: Values, types, and variables

Every program stores values in variables, and every value has a type. This chapter
compares how the nine languages declare variables, which number types they offer,
what happens when a number gets too big, and how they convert one type into another.
These are the rules that decide whether `2147483647 + 1` is 2147483648, -2147483648,
an error, or a bug that only shows up in production.

**Languages:** all 9 · **Time:** 3 to 4 hours

| You will learn | Why it matters later |
| --- | --- |
| Static vs dynamic and strong vs weak typing | The single biggest difference in how the languages feel |
| Mutable variables, read-only variables, and constants | Chapter 10 (ownership) builds on Rust's `let` vs `let mut` |
| Fixed-size integers and what overflow does | Every DSA chapter, and chapter 39's security bugs |
| Why `0.1 + 0.2` is not `0.3` | Chapters 42 to 45 (numeric code and game physics) |
| Explicit and implicit conversions | Chapter 11 (error handling): what happens when parsing fails |

---

## 2.1 The concept: values, types, and variables

A **value** is a piece of data, such as `42`, `3.14`, `true`, or `"Ada"`. A **type**
says what kind of value it is and therefore which operations make sense: you can
divide numbers, but not booleans. A **variable** is a name that refers to a value.

Two questions separate the nine languages.

1. **When are types checked?** A **statically typed** language checks types when the
   program is compiled, before it runs. A **dynamically typed** language checks them
   while the program runs, at the moment each operation happens.
2. **How much does the language convert for you?** A **strongly typed** language
   refuses to mix types without being told. A **weakly typed** language converts
   silently, so `"3" * 4` might become `12`.

| Language | Types checked | Implicit conversions | Example: `"3" + 4` |
| --- | --- | --- | --- |
| C | Compile time | Many, between all number types | Compiles, and does pointer arithmetic (a bug) |
| C++ | Compile time | Many, inherited from C | Same as C |
| Rust | Compile time | None between number types | Compile error |
| Go | Compile time | None between number types | Compile error |
| Python | Run time | Few (`int` to `float` in arithmetic) | `TypeError` when that line runs |
| JavaScript | Run time | Many | `"34"`, and `"3" * 4` is `12` |
| TypeScript | Compile time, then erased | Checker forbids most of JavaScript's | `"34"` is allowed; `"3" * 4` is a compile error |
| Dart | Compile time, and guaranteed at run time | None between number types | Compile error |
| Kotlin | Compile time, and the JVM checks casts at run time | None between number types | `"34"`: a `String`'s `+` accepts anything; `"3" * 4` is a compile error |

Dart's checking is called **sound**: if the compiler says a variable is an `int`, it
is an `int` at run time, every time. TypeScript's checking is deliberately not sound,
because it has to fit on top of JavaScript. Its types are checked, then erased, so a
value from outside the program (a JSON file, user input) can still be the wrong type
at run time.

Kotlin is nearly sound. The gap is Java: a value returned by Java code has a
**platform type**, shown as `String!` in error messages, and Kotlin lets you treat it as
never null. If the Java code does return `null`, the program throws a
`NullPointerException` at run time. Chapter 8 covers null in all nine languages.

---

## 2.2 Declaring variables

The program below declares a variable that changes, a variable that does not, two
constants, and a boolean. The code is in [`examples/01-variables/`](examples/01-variables/).

**C**

```c
#include <stdbool.h>
#include <stdio.h>

#define MAX_PLAYERS 4  // a macro: the preprocessor pastes 4 wherever it appears

int main(void) {
    int count = 0;                // variables can change unless marked const
    const int birth_year = 1815;  // assigning to it again is a compile error
    const double pi = 3.14159;
    bool ready = true;            // bool needs <stdbool.h> before C23

    count += 3;

    printf("count = %d\n", count);
    printf("birth year = %d\n", birth_year);
    printf("pi = %g\n", pi);
    printf("max players = %d\n", MAX_PLAYERS);
    printf("ready = %s\n", ready ? "true" : "false");  // printf has no format for bool
    return 0;
}
```

**C++**

```cpp
#include <iostream>

constexpr int max_players = 4;  // a compile-time constant with a real type

int main() {
    int count = 0;
    const int birth_year = 1815;
    const double pi = 3.14159;
    bool ready = true;

    count += 3;

    std::cout << "count = " << count << '\n';
    std::cout << "birth year = " << birth_year << '\n';
    std::cout << "pi = " << pi << '\n';
    std::cout << "max players = " << max_players << '\n';
    std::cout << "ready = " << std::boolalpha << ready << '\n';  // without boolalpha: 1
}
```

**Rust**

```rust
const MAX_PLAYERS: u32 = 4; // constants must spell out their type

fn main() {
    let mut count = 0; // `mut` allows it to change
    let birth_year = 1815; // immutable: `birth_year = 1816;` would not compile
    let pi = 3.14159;
    let ready = true;

    count += 3;

    println!("count = {count}");
    println!("birth year = {birth_year}");
    println!("pi = {pi}");
    println!("max players = {MAX_PLAYERS}");
    println!("ready = {ready}");
}
```

**Go**

```go
package main

import "fmt"

const maxPlayers = 4 // a compile-time constant
const pi = 3.14159

func main() {
	count := 0               // := declares a variable and infers its type (int)
	var birthYear int = 1815 // Go has no read-only variables, only constants
	ready := true

	count += 3

	fmt.Println("count =", count)
	fmt.Println("birth year =", birthYear)
	fmt.Println("pi =", pi)
	fmt.Println("max players =", maxPlayers)
	fmt.Println("ready =", ready)
}
```

**Python**

```python
from typing import Final

MAX_PLAYERS: Final = 4  # Final is a hint for type checkers; Python itself does not enforce it

count = 0
birth_year = 1815  # no way to make a variable read-only
pi = 3.14159
ready = True

count += 3

print("count =", count)
print("birth year =", birth_year)
print("pi =", pi)
print("max players =", MAX_PLAYERS)
print("ready =", ready)  # prints True, with a capital T
```

**JavaScript**

```js
const MAX_PLAYERS = 4;

let count = 0; // let: can be reassigned
const birthYear = 1815; // const: cannot be reassigned
const pi = 3.14159;
const ready = true;

count += 3;

console.log(`count = ${count}`);
console.log(`birth year = ${birthYear}`);
console.log(`pi = ${pi}`);
console.log(`max players = ${MAX_PLAYERS}`);
console.log(`ready = ${ready}`);
```

**TypeScript**

```ts
const MAX_PLAYERS = 4; // its type is the literal 4, not just number

let count: number = 0;
const birthYear: number = 1815;
const pi = 3.14159; // inferred as number
const ready: boolean = true;

count += 3;

console.log(`count = ${count}`);
console.log(`birth year = ${birthYear}`);
console.log(`pi = ${pi}`);
console.log(`max players = ${MAX_PLAYERS}`);
console.log(`ready = ${ready}`);
```

**Dart**

```dart
const maxPlayers = 4; // compile-time constant

void main() {
  var count = 0; // var: type inferred (int), can be reassigned
  final birthYear = 1815; // final: assigned exactly once
  const pi = 3.14159;
  final bool ready = true; // types can also be written out

  count += 3;

  print('count = $count');
  print('birth year = $birthYear');
  print('pi = $pi');
  print('max players = $maxPlayers');
  print('ready = $ready');
}
```

**Kotlin**

```kotlin
const val MAX_PLAYERS = 4 // compile-time constant: allowed only at top level or in an object

fun main() {
    var count = 0 // var: can be reassigned; the type (Int) is inferred
    val birthYear = 1815 // val: assigned exactly once
    val pi = 3.14159
    val ready: Boolean = true // types can also be written out

    count += 3

    println("count = $count")
    println("birth year = $birthYear")
    println("pi = $pi")
    println("max players = $MAX_PLAYERS")
    println("ready = $ready")
}
```

Output (Python prints `True` instead of `true`):

```text
count = 3
birth year = 1815
pi = 3.14159
max players = 4
ready = true
```

| Language | Can change | Set once | Compile-time constant | Reading a variable before it has a value |
| --- | --- | --- | --- | --- |
| C | `int x = 0;` | `const int x = 0;` | `#define X 0`, or `constexpr` in C23 | Garbage value: undefined behavior |
| C++ | `int x = 0;` or `auto x = 0;` | `const int x = 0;` | `constexpr int x = 0;` | Garbage value: undefined behavior |
| Rust | `let mut x = 0;` | `let x = 0;` | `const X: i32 = 0;` | Compile error |
| Go | `x := 0` or `var x int` | not possible | `const x = 0` | Never happens: every variable starts at its zero value (`0`, `""`, `false`, `nil`) |
| Python | `x = 0` | not possible (`Final` is a hint) | not possible | `NameError` when that line runs |
| JavaScript | `let x = 0;` | `const x = 0;` | `const` | `undefined`, or `ReferenceError` for a `let` used above its declaration |
| TypeScript | `let x: number = 0;` | `const x = 0;` | `const`, `as const` | Compile error with `--strict` |
| Dart | `var x = 0;` or `int x = 0;` | `final x = 0;` | `const x = 0;` | Compile error |
| Kotlin | `var x = 0` or `var x: Int = 0` | `val x = 0` | `const val X = 0`, only at the top level or in an `object` | Compile error |

Notice the defaults. In C, C++, Go, Python, and JavaScript a plain declaration can
change, and you opt in to read-only. In Rust a plain `let` is read-only, and you opt in
to change with `mut`. That one choice pushes Rust code toward values that never change,
which makes it easier to reason about and is what lets the compiler check ownership
in chapter 10. Kotlin sits in between: `val` and `var` are the same length, so neither
is the default, and IntelliJ flags a `var` that never changes.

**`const` does not always mean the same thing.** In JavaScript, TypeScript, Dart, and
Kotlin, `const`, `final`, or `val` stops you from pointing the name at a different
value, but an object it points to can still change inside:

```js
const scores = [90, 85];
scores.push(70);      // allowed: the array changed, the binding did not
scores = [];          // TypeError: Assignment to constant variable.
```

Dart's `const` goes further: a `const` list is frozen all the way down, and it is built
at compile time. Chapter 10 covers this in detail.

**Rust allows shadowing.** You can declare a new variable with the same name, often
to change its type, without needing `mut`:

```rust
let input = "42";                 // a &str
let input: i32 = input.parse().unwrap(); // a new variable, now an i32
```

---

## 2.3 Number types

| Language | Default integer | Fixed-size integers | Integers of any size | Floating point |
| --- | --- | --- | --- | --- |
| C | `int`, usually 32 bits | `int8_t` to `int64_t`, `uint8_t` to `uint64_t` in `<stdint.h>` | not built in (GMP library) | `float` (32-bit), `double` (64-bit) |
| C++ | `int`, usually 32 bits | `std::int8_t` and friends in `<cstdint>` | not built in (Boost.Multiprecision) | `float`, `double` |
| Rust | `i32` | `i8` to `i128`, `u8` to `u128`, `isize`, `usize` | not in std (`num-bigint` crate) | `f32`, `f64` (default) |
| Go | `int`, 64 bits on 64-bit machines | `int8` to `int64`, `uint8` to `uint64`, `byte`, `rune` | `math/big.Int` | `float32`, `float64` (default) |
| Python | `int`, any size | none (NumPy has them for arrays) | `int` | `float` (64-bit) |
| JavaScript | `number`, a 64-bit float | none (typed arrays such as `Int32Array`) | `BigInt`, written `123n` | `number` |
| TypeScript | `number` | same as JavaScript | `bigint` | `number` |
| Dart | `int`, 64 bits natively | none | `BigInt` | `double` (64-bit) |
| Kotlin | `Int`, always 32 bits | `Byte`, `Short`, `Int`, `Long`, and unsigned `UByte` to `ULong` | `java.math.BigInteger` | `Float`, `Double` (default) |

**C's sizes depend on the platform.** The C standard only promises minimums: `int` is
at least 16 bits and `long` at least 32. On Linux and macOS `long` is 64 bits, and on
Windows it is 32. Whenever the size matters, use the fixed-width types from
`<stdint.h>`, such as `int64_t`, and print them with the matching macros from
`<inttypes.h>`, such as `PRId64`.

**JavaScript has one number type.** Every `number` is a 64-bit float, which can hold
whole numbers exactly only up to 2^53 (9,007,199,254,740,991). That is enough for most
counts and indexes, but not for 64-bit database IDs, which is why APIs often send them
as strings.

**Dart's `int` depends on where it runs.** It is 64 bits in the Dart VM and in compiled
executables. When Dart is compiled to JavaScript for the web, `int` becomes a JavaScript
number with the same 2^53 limit.

**Kotlin's sizes come from the JVM**, so they are the same on every machine: `Int` is 32
bits and `Long` is 64. A whole-number literal too big for an `Int` becomes a `Long` by
itself, so `val big = 3000000000` is a `Long`.

---

## 2.4 Integer overflow

A fixed-size integer has a largest value. A 32-bit signed integer stops at
2,147,483,647. This program asks each language what happens one past that. The code
is in [`examples/02-integer-limits/`](examples/02-integer-limits/).

**C**

```c
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    int32_t i32 = INT32_MAX;
    printf("int32 max: %" PRId32 "\n", i32);

    // i32 + 1 would be undefined behavior: signed overflow is not allowed in C.
    // GCC and Clang provide a checked add that reports overflow instead.
    int32_t sum;
    if (__builtin_add_overflow(i32, 1, &sum)) {
        printf("int32 max + 1: overflow\n");
    }

    uint32_t u32 = UINT32_MAX;  // unsigned types are defined to wrap around
    u32 += 1;
    printf("uint32 max + 1: %" PRIu32 "\n", u32);

    printf("int64 max: %" PRId64 "\n", INT64_MAX);
    return 0;
}
```

**C++**

```cpp
#include <cstdint>
#include <iostream>
#include <limits>

int main() {
    std::int32_t i32 = std::numeric_limits<std::int32_t>::max();
    std::cout << "int32 max: " << i32 << '\n';

    // As in C, i32 + 1 is undefined behavior. Use the compiler's checked add:
    std::int32_t sum;
    if (__builtin_add_overflow(i32, 1, &sum)) {
        std::cout << "int32 max + 1: overflow\n";
    }

    std::uint32_t u32 = std::numeric_limits<std::uint32_t>::max();
    u32 += 1;  // unsigned types wrap around
    std::cout << "uint32 max + 1: " << u32 << '\n';

    std::cout << "int64 max: " << std::numeric_limits<std::int64_t>::max() << '\n';
}
```

**Rust**

```rust
fn main() {
    let i32_max = i32::MAX;
    println!("int32 max: {i32_max}");

    // Plain `i32_max + 1` panics in a debug build and wraps in a release build.
    // Say which behavior you want instead:
    println!("checked_add(1): {:?}", i32_max.checked_add(1));
    println!("wrapping_add(1): {}", i32_max.wrapping_add(1));
    println!("saturating_add(1): {}", i32_max.saturating_add(1));

    println!("int64 max: {}", i64::MAX);
}
```

**Go**

```go
package main

import (
	"fmt"
	"math"
)

func main() {
	var i32 int32 = math.MaxInt32
	fmt.Println("int32 max:", i32)
	i32++ // signed overflow is defined in Go: it wraps around
	fmt.Println("int32 max + 1:", i32)

	var i64 int64 = math.MaxInt64
	fmt.Println("int64 max:", i64)
	i64++
	fmt.Println("int64 max + 1:", i64)
}
```

**Python**

```python
i32_max = 2**31 - 1
print("int32 max:", i32_max)
print("int32 max + 1:", i32_max + 1)  # Python ints have no fixed size, so they never overflow

i64_max = 2**63 - 1
print("int64 max:", i64_max)
print("int64 max + 1:", i64_max + 1)
print("2 ** 100:", 2**100)
```

**JavaScript**

```js
const i32Max = 2 ** 31 - 1;
console.log(`int32 max: ${i32Max}`);
console.log(`int32 max + 1: ${i32Max + 1}`); // numbers are 64-bit floats, so no overflow yet
console.log(`int32 max + 1, as a 32-bit int: ${(i32Max + 1) | 0}`); // | works on 32-bit ints

const safe = Number.MAX_SAFE_INTEGER; // 2^53 - 1
console.log(`largest safe integer: ${safe}`);
console.log(`largest safe integer + 2: ${safe + 2}`); // off by one, and no error

const i64Max = 2n ** 63n - 1n; // BigInt: whole numbers of any size
console.log(`int64 max (BigInt): ${i64Max}`);
console.log(`int64 max + 1 (BigInt): ${i64Max + 1n}`);
```

**TypeScript**

```ts
const i32Max: number = 2 ** 31 - 1;
console.log(`int32 max: ${i32Max}`);
console.log(`int32 max + 1: ${i32Max + 1}`); // numbers are 64-bit floats, so no overflow yet
console.log(`int32 max + 1, as a 32-bit int: ${(i32Max + 1) | 0}`); // | works on 32-bit ints

const safe: number = Number.MAX_SAFE_INTEGER; // 2^53 - 1
console.log(`largest safe integer: ${safe}`);
console.log(`largest safe integer + 2: ${safe + 2}`); // off by one, and no error

const i64Max: bigint = 2n ** 63n - 1n; // BigInt: whole numbers of any size
console.log(`int64 max (BigInt): ${i64Max}`);
console.log(`int64 max + 1 (BigInt): ${i64Max + 1n}`);
```

**Dart**

```dart
void main() {
  var i32Max = 2147483647;
  print('int32 max: $i32Max');
  print('int32 max + 1: ${i32Max + 1}'); // Dart's int is 64 bits, so this still fits
  print('int32 max + 1, as 32 bits: ${(i32Max + 1).toSigned(32)}');

  var i64Max = 9223372036854775807;
  print('int64 max: $i64Max');
  print('int64 max + 1: ${i64Max + 1}'); // native Dart wraps silently
}
```

**Kotlin**

```kotlin
fun main() {
    val i32Max = Int.MAX_VALUE
    println("int32 max: $i32Max")
    println("int32 max + 1: ${i32Max + 1}") // wraps silently, as Java does

    // Math.addExact throws instead of wrapping.
    val checked =
        try {
            Math.addExact(i32Max, 1).toString()
        } catch (e: ArithmeticException) {
            "ArithmeticException: ${e.message}"
        }
    println("Math.addExact(int32 max, 1): $checked")

    val i64Max = Long.MAX_VALUE
    println("int64 max: $i64Max")
    println("int64 max + 1: ${i64Max + 1}")
}
```

The output is different in every language, and that is the point:

*C and C++:*

```text
int32 max: 2147483647
int32 max + 1: overflow
uint32 max + 1: 0
int64 max: 9223372036854775807
```

*Rust:*

```text
int32 max: 2147483647
checked_add(1): None
wrapping_add(1): -2147483648
saturating_add(1): 2147483647
int64 max: 9223372036854775807
```

*Go:*

```text
int32 max: 2147483647
int32 max + 1: -2147483648
int64 max: 9223372036854775807
int64 max + 1: -9223372036854775808
```

*Python:*

```text
int32 max: 2147483647
int32 max + 1: 2147483648
int64 max: 9223372036854775807
int64 max + 1: 9223372036854775808
2 ** 100: 1267650600228229401496703205376
```

*JavaScript and TypeScript:*

```text
int32 max: 2147483647
int32 max + 1: 2147483648
int32 max + 1, as a 32-bit int: -2147483648
largest safe integer: 9007199254740991
largest safe integer + 2: 9007199254740992
int64 max (BigInt): 9223372036854775807
int64 max + 1 (BigInt): 9223372036854775808
```

*Dart:*

```text
int32 max: 2147483647
int32 max + 1: 2147483648
int32 max + 1, as 32 bits: -2147483648
int64 max: 9223372036854775807
int64 max + 1: -9223372036854775808
```

*Kotlin:*

```text
int32 max: 2147483647
int32 max + 1: -2147483648
Math.addExact(int32 max, 1): ArithmeticException: integer overflow
int64 max: 9223372036854775807
int64 max + 1: -9223372036854775808
```

| Language | `int32 max + 1` by default | Tools to choose the behavior |
| --- | --- | --- |
| C | **Undefined behavior** for signed types; unsigned types wrap | `__builtin_add_overflow` (GCC, Clang), `ckd_add` in C23, `-fsanitize=undefined` to catch it |
| C++ | **Undefined behavior** for signed types; unsigned types wrap | Same builtins; `std::add_sat` in C++26 |
| Rust | Panic in a debug build, wrap in a release build | `checked_add`, `wrapping_add`, `saturating_add`, `overflowing_add` |
| Go | Wraps, as defined by the language spec | Check by hand; `math/bits` for unsigned carries |
| Python | No overflow: integers grow as needed | none needed |
| JavaScript, TypeScript | No overflow, but whole numbers past 2^53 silently lose precision | `BigInt`, `Number.isSafeInteger` |
| Dart | Wraps at 64 bits | Check by hand; `BigInt` |
| Kotlin | Wraps, as in Java | `Math.addExact` and friends, which throw `ArithmeticException` |

**Undefined behavior is worse than a wrong answer.** In C and C++ the compiler is
allowed to assume signed overflow never happens, and it optimizes on that assumption.
This function looks like an overflow check:

```c
int will_overflow(int x) {
    return x + 1 < x;   // "if adding 1 made it smaller, we overflowed"
}
```

GCC at `-O2` compiles it to `return 0;`. Since `x + 1` can never overflow in a correct
program, `x + 1 < x` can never be true, so the compiler deletes the check. Chapter 39
shows how bugs like this become security holes. Check *before* you add, as the
checked-add exercise does, or use `__builtin_add_overflow`.

**Rust picks safety in development and speed in release.** Debug builds (`cargo run`)
check every addition and panic with `attempt to add with overflow`. Release builds
(`cargo run --release`) skip the check and wrap. If wrapping is what you want, say so
with `wrapping_add`, and the code will behave the same in both builds.

---

## 2.5 Floating point

Every language here stores `double` (or `f64`, `float64`, `number`) the same way: as a
64-bit IEEE 754 binary float. Most decimal fractions, including 0.1, have no exact
binary form, just as 1/3 has no exact decimal form. So all nine languages agree:
`0.1 + 0.2` is not `0.3`. The code is in [`examples/03-floats/`](examples/03-floats/).

**C**

```c
#include <math.h>
#include <stdio.h>

int main(void) {
    double a = 0.1;
    double b = 0.2;
    double sum = a + b;

    // %.17g prints enough digits to show the stored value; plain %g would round it to 0.3.
    printf("0.1 + 0.2 = %.17g\n", sum);
    printf("equal to 0.3? %s\n", sum == 0.3 ? "yes" : "no");
    printf("close to 0.3? %s\n", fabs(sum - 0.3) < 1e-9 ? "yes" : "no");
    return 0;
}
```

**C++**

```cpp
#include <cmath>
#include <iomanip>
#include <iostream>

int main() {
    double a = 0.1;
    double b = 0.2;
    double sum = a + b;

    // std::cout shows 6 significant digits by default, which would print 0.3.
    std::cout << "0.1 + 0.2 = " << std::setprecision(17) << sum << '\n';
    std::cout << "equal to 0.3? " << (sum == 0.3 ? "yes" : "no") << '\n';
    std::cout << "close to 0.3? " << (std::abs(sum - 0.3) < 1e-9 ? "yes" : "no") << '\n';
}
```

**Rust**

```rust
fn main() {
    let a = 0.1;
    let b = 0.2;
    let sum: f64 = a + b;

    println!("0.1 + 0.2 = {sum}");
    println!("equal to 0.3? {}", if sum == 0.3 { "yes" } else { "no" });
    println!("close to 0.3? {}", if (sum - 0.3).abs() < 1e-9 { "yes" } else { "no" });
}
```

**Go**

```go
package main

import (
	"fmt"
	"math"
)

func main() {
	// Variables, not constants: Go evaluates constant expressions exactly,
	// so the constant expression 0.1 + 0.2 really is 0.3.
	a, b := 0.1, 0.2
	sum := a + b

	fmt.Println("0.1 + 0.2 =", sum)
	fmt.Println("equal to 0.3?", yesNo(sum == 0.3))
	fmt.Println("close to 0.3?", yesNo(math.Abs(sum-0.3) < 1e-9))
}

func yesNo(b bool) string {
	if b {
		return "yes"
	}
	return "no"
}
```

**Python**

```python
import math

total = 0.1 + 0.2

print("0.1 + 0.2 =", total)
print("equal to 0.3?", "yes" if total == 0.3 else "no")
print("close to 0.3?", "yes" if math.isclose(total, 0.3) else "no")
```

**JavaScript**

```js
const sum = 0.1 + 0.2;

console.log(`0.1 + 0.2 = ${sum}`);
console.log(`equal to 0.3? ${sum === 0.3 ? "yes" : "no"}`);
console.log(`close to 0.3? ${Math.abs(sum - 0.3) < 1e-9 ? "yes" : "no"}`);
```

**TypeScript**

```ts
const sum: number = 0.1 + 0.2;

console.log(`0.1 + 0.2 = ${sum}`);
console.log(`equal to 0.3? ${sum === 0.3 ? "yes" : "no"}`);
console.log(`close to 0.3? ${Math.abs(sum - 0.3) < 1e-9 ? "yes" : "no"}`);
```

**Dart**

```dart
void main() {
  final sum = 0.1 + 0.2;

  print('0.1 + 0.2 = $sum');
  print('equal to 0.3? ${sum == 0.3 ? 'yes' : 'no'}');
  print('close to 0.3? ${(sum - 0.3).abs() < 1e-9 ? 'yes' : 'no'}');
}
```

**Kotlin**

```kotlin
import kotlin.math.abs

fun main() {
    val sum = 0.1 + 0.2

    println("0.1 + 0.2 = $sum")
    println("equal to 0.3? ${if (sum == 0.3) "yes" else "no"}")
    println("close to 0.3? ${if (abs(sum - 0.3) < 1e-9) "yes" else "no"}")
}
```

Output, identical in all nine:

```text
0.1 + 0.2 = 0.30000000000000004
equal to 0.3? no
close to 0.3? yes
```

Three things to notice:

- **Never compare floats with `==`.** Check whether they are close: within `1e-9`, or
  with a relative tolerance such as Python's `math.isclose`.
- **C++ and C hide the error by default.** `std::cout` prints 6 significant digits and
  C's `%g` does the same, so both would print `0.3`. The other languages print the
  shortest text that converts back to exactly the same float, which shows the error.
- **Go's constants are exact.** `fmt.Println(0.1 + 0.2)` prints `0.3` in Go, because Go
  computes constant expressions with exact arithmetic before converting to `float64`.
  That is why the Go example stores the numbers in variables first.

Use integers for money. Store cents in an `int64`, or use a decimal type such as
Python's `decimal.Decimal`. A float cannot hold $0.10 exactly.

---

## 2.6 Converting between types

Converting covers three jobs: parsing text into numbers, turning floats into integers,
and mixing integer and float arithmetic. The code is in
[`examples/04-conversions/`](examples/04-conversions/).

**C**

```c
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    long parsed = strtol("42", NULL, 10);  // text to number
    printf("\"42\" + 1 as a number: %ld\n", parsed + 1);

    double x = 3.99;
    printf("3.99 truncated: %d\n", (int)x);    // a cast drops the fraction
    printf("-3.99 truncated: %d\n", (int)-x);
    printf("2.5 rounded: %.0f\n", round(2.5));   // halfway rounds away from zero
    printf("-2.5 rounded: %.0f\n", round(-2.5));

    int seven = 7;
    int two = 2;
    printf("7 / 2 as integers: %d\n", seven / two);
    printf("7 / 2 as floats: %g\n", (double)seven / two);  // convert one side first
    return 0;
}
```

**C++**

```cpp
#include <cmath>
#include <iostream>
#include <string>

int main() {
    int parsed = std::stoi("42");  // throws std::invalid_argument on bad input
    std::cout << "\"42\" + 1 as a number: " << parsed + 1 << '\n';

    double x = 3.99;
    std::cout << "3.99 truncated: " << static_cast<int>(x) << '\n';
    std::cout << "-3.99 truncated: " << static_cast<int>(-x) << '\n';
    std::cout << "2.5 rounded: " << std::lround(2.5) << '\n';  // halfway rounds away from zero
    std::cout << "-2.5 rounded: " << std::lround(-2.5) << '\n';

    int seven = 7;
    int two = 2;
    std::cout << "7 / 2 as integers: " << seven / two << '\n';
    std::cout << "7 / 2 as floats: " << static_cast<double>(seven) / two << '\n';
}
```

**Rust**

```rust
fn main() {
    let parsed: i32 = "42".parse().expect("not a number"); // parse returns a Result
    println!("\"42\" + 1 as a number: {}", parsed + 1);

    let x: f64 = 3.99;
    println!("3.99 truncated: {}", x as i32); // `as` drops the fraction
    println!("-3.99 truncated: {}", -x as i32);
    println!("2.5 rounded: {}", 2.5_f64.round() as i64); // halfway rounds away from zero
    println!("-2.5 rounded: {}", (-2.5_f64).round() as i64);

    let seven: i32 = 7;
    let two: i32 = 2;
    println!("7 / 2 as integers: {}", seven / two);
    println!("7 / 2 as floats: {}", seven as f64 / two as f64); // no automatic conversion
}
```

**Go**

```go
package main

import (
	"fmt"
	"math"
	"strconv"
)

func main() {
	parsed, err := strconv.Atoi("42")
	if err != nil {
		panic(err)
	}
	fmt.Println(`"42" + 1 as a number:`, parsed+1)

	x := 3.99 // int(3.99) would not compile: Go refuses to truncate a constant
	fmt.Println("3.99 truncated:", int(x))
	fmt.Println("-3.99 truncated:", int(-x))
	fmt.Println("2.5 rounded:", math.Round(2.5)) // halfway rounds away from zero
	fmt.Println("-2.5 rounded:", math.Round(-2.5))

	seven, two := 7, 2
	fmt.Println("7 / 2 as integers:", seven/two)
	fmt.Println("7 / 2 as floats:", float64(seven)/float64(two)) // no automatic conversion
}
```

**Python**

```python
parsed = int("42")
print('"42" + 1 as a number:', parsed + 1)

x = 3.99
print("3.99 truncated:", int(x))  # int() drops the fraction
print("-3.99 truncated:", int(-x))
print("2.5 rounded:", round(2.5))  # 2: halfway rounds to the even neighbor
print("-2.5 rounded:", round(-2.5))

print("7 / 2 as integers:", 7 // 2)  # // is integer (floor) division
print("7 / 2 as floats:", 7 / 2)  # / always gives a float
```

**JavaScript**

```js
const parsed = Number("42");
console.log(`"42" + 1 as a number: ${parsed + 1}`); // "42" + 1 without Number() is "421"

const x = 3.99;
console.log(`3.99 truncated: ${Math.trunc(x)}`);
console.log(`-3.99 truncated: ${Math.trunc(-x)}`);
console.log(`2.5 rounded: ${Math.round(2.5)}`); // halfway rounds up, toward +infinity
console.log(`-2.5 rounded: ${Math.round(-2.5)}`); // so this gives -2

console.log(`7 / 2 as integers: ${Math.trunc(7 / 2)}`); // there is no integer division
console.log(`7 / 2 as floats: ${7 / 2}`);
```

**TypeScript**

```ts
const parsed: number = Number("42");
console.log(`"42" + 1 as a number: ${parsed + 1}`); // "42" + 1 without Number() is "421"

const x: number = 3.99;
console.log(`3.99 truncated: ${Math.trunc(x)}`);
console.log(`-3.99 truncated: ${Math.trunc(-x)}`);
console.log(`2.5 rounded: ${Math.round(2.5)}`); // halfway rounds up, toward +infinity
console.log(`-2.5 rounded: ${Math.round(-2.5)}`); // so this gives -2

console.log(`7 / 2 as integers: ${Math.trunc(7 / 2)}`); // there is no integer division
console.log(`7 / 2 as floats: ${7 / 2}`);
```

**Dart**

```dart
void main() {
  final parsed = int.parse('42'); // throws FormatException on bad input
  print('"42" + 1 as a number: ${parsed + 1}');

  const x = 3.99;
  print('3.99 truncated: ${x.toInt()}');
  print('-3.99 truncated: ${(-x).toInt()}');
  print('2.5 rounded: ${2.5.round()}'); // halfway rounds away from zero
  print('-2.5 rounded: ${(-2.5).round()}');

  print('7 / 2 as integers: ${7 ~/ 2}'); // ~/ is integer division
  print('7 / 2 as floats: ${7 / 2}'); // / always gives a double
}
```

**Kotlin**

```kotlin
import kotlin.math.roundToInt

fun main() {
    val parsed = "42".toInt() // throws NumberFormatException on bad input
    println("\"42\" + 1 as a number: ${parsed + 1}")

    val x = 3.99
    println("3.99 truncated: ${x.toInt()}")
    println("-3.99 truncated: ${(-x).toInt()}")
    println("2.5 rounded: ${2.5.roundToInt()}") // halfway rounds toward positive infinity
    println("-2.5 rounded: ${(-2.5).roundToInt()}")

    val seven = 7
    val two = 2
    println("7 / 2 as integers: ${seven / two}")
    println("7 / 2 as floats: ${seven / two.toDouble()}") // Int / Double gives a Double
}
```

Output:

*C, C++, Rust, Go and Dart:*

```text
"42" + 1 as a number: 43
3.99 truncated: 3
-3.99 truncated: -3
2.5 rounded: 3
-2.5 rounded: -3
7 / 2 as integers: 3
7 / 2 as floats: 3.5
```

*Python:*

```text
"42" + 1 as a number: 43
3.99 truncated: 3
-3.99 truncated: -3
2.5 rounded: 2
-2.5 rounded: -2
7 / 2 as integers: 3
7 / 2 as floats: 3.5
```

*JavaScript, TypeScript and Kotlin:*

```text
"42" + 1 as a number: 43
3.99 truncated: 3
-3.99 truncated: -3
2.5 rounded: 3
-2.5 rounded: -2
7 / 2 as integers: 3
7 / 2 as floats: 3.5
```

| Language | Parse text | Float to int | Mixing `int` and `float` |
| --- | --- | --- | --- |
| C | `strtol`, `strtod` (`atoi` cannot report errors) | `(int)x` truncates; out of range is undefined behavior | Automatic: the `int` becomes a `double` |
| C++ | `std::stoi`, `std::from_chars` | `static_cast<int>(x)` truncates | Automatic, as in C |
| Rust | `"42".parse::<i32>()` returns a `Result` | `x as i32` truncates and saturates (`300.0 as u8` is 255, NaN becomes 0) | Compile error: convert with `as` |
| Go | `strconv.Atoi`, `strconv.ParseFloat`, each returning an error | `int(x)` truncates | Compile error: convert with `float64(n)` |
| Python | `int("42")`, `float("3.5")` raise `ValueError` | `int(x)` truncates | Automatic: the result is a `float` |
| JavaScript | `Number("42")`, `parseInt("42", 10)`; bad input gives `NaN` | `Math.trunc(x)` | Same type already |
| TypeScript | same as JavaScript | same as JavaScript | same as JavaScript |
| Dart | `int.parse`, `int.tryParse` (returns `null` on bad input) | `x.toInt()` truncates | Automatic in arithmetic: the result is a `double` |
| Kotlin | `"42".toInt()` throws, `"42".toIntOrNull()` returns `null` | `x.toInt()` truncates and saturates (`1e20.toInt()` is `Int.MAX_VALUE`, NaN becomes 0) | Operators accept mixed types, so `7 / 2.0` is `3.5`, but `val d: Double = 7` is a compile error |

**Rounding halfway cases is not universal.** `round(2.5)` is 3 in C, C++, Rust, Go, and
Dart, which round halfway cases away from zero. Python gives 2: it rounds to the even
neighbor ("banker's rounding"), which avoids a bias when you add up many rounded
numbers. JavaScript's `Math.round` rounds halfway cases toward positive infinity, so
`Math.round(2.5)` is 3 but `Math.round(-2.5)` is -2. Kotlin has both: `roundToInt()`
works like JavaScript's `Math.round`, and `kotlin.math.round(2.5)` rounds to even like
Python and gives `2.0`.

**Narrowing integer conversions keep the low bits.** Converting a 64-bit integer to 8
bits keeps the lowest 8 bits and throws the rest away, so 300 becomes 44. Rust's `as`,
Go's `uint8(x)`, C's `(uint8_t)x`, and Kotlin's `x.toUByte()` all do this. When you want an error instead, Rust
has `u8::try_from(300)`, which returns `Err`. Exercise 2 explores this.

**Signed and unsigned comparisons in C are a trap.** In `-1 < 1u`, C converts `-1` to
unsigned, where it becomes 4,294,967,295, so the result is false. `-Wall -Wextra`
warns about this (`-Wsign-compare`), which is one more reason to keep those flags on.

---

## 2.7 Type inference

Most of the languages can work out a variable's type from its initial value, so you
do not have to write it.

| Language | Inferred declaration | Type the compiler picks |
| --- | --- | --- |
| C | `auto x = 5;` (C23 only) | `int` |
| C++ | `auto x = 5;` | `int` |
| Rust | `let x = 5;` | `i32`, unless later code needs something else |
| Go | `x := 5` | `int` |
| Python | `x = 5` | none: variables have no type, values do |
| JavaScript | `let x = 5` | none: variables have no type, values do |
| TypeScript | `let x = 5` | `number`; `const x = 5` gets the literal type `5` |
| Dart | `var x = 5;` | `int` |
| Kotlin | `val x = 5` | `Int`, or `Long` if the number is too big for an `Int` |

Rust's inference looks ahead. In the code below, the compiler sees `push(1u8)` and
works out that `bytes` must be a `Vec<u8>`:

```rust
let mut bytes = Vec::new(); // type not known yet
bytes.push(1u8);            // now it is: Vec<u8>
```

Python and JavaScript variables have no type at all. A name can hold an `int` and then
a `str`. Python's optional type hints (`count: int = 0`) change nothing when the program
runs. They are checked by separate tools such as mypy or pyright.

---

## 2.8 Why each language looks the way it does

- **C** maps types straight onto the hardware of the 1970s. `int` was "whatever size
  the machine handles fastest", which is why its size varies. Signed overflow was left
  undefined because different machines handled it differently, and that freedom now
  lets optimizers make faster code.
- **C++** kept C's types for compatibility, then added safer tools on top: `constexpr`
  for real typed constants, brace initialization (`int x{3.7};` is a compile error),
  and `static_cast` to make conversions easy to search for.
- **Rust** removes every silent conversion and makes immutability the default. Overflow
  is a bug unless you say otherwise. The cost is more typing (`as f64`); the payoff is
  that the compiler catches mistakes C leaves to testing.
- **Go** also bans implicit conversions, but chose simple, defined wrapping over
  Rust's panics. Zero values mean there is no such thing as an uninitialized variable.
  Go has no read-only variables, only compile-time constants, to keep the language small.
- **Python** puts types on values instead of variables, and makes `int` unbounded, so
  overflow simply does not exist. The trade-off is speed: each Python `int` is an object
  on the heap.
- **JavaScript** was built for small scripts in web pages, where one number type
  seemed simpler. Its weak typing came from trying to keep scripts running instead of
  failing. `BigInt` was added in 2020 when that one type proved too small.
- **TypeScript** describes JavaScript's types rather than changing them. It keeps
  `number` and JavaScript's runtime rules, and blocks the conversions that are almost
  always bugs, such as `"3" * 4`.
- **Dart** started as an optionally typed language and became fully sound in Dart 2
  and null-safe in Dart 3. It wants Java-like guarantees with JavaScript-like
  convenience (`var`, string interpolation).
- **Kotlin** keeps the JVM's number types, so sizes are fixed everywhere and overflow
  wraps exactly as in Java. It removed Java's silent widening (Java accepts
  `long l = someInt;`, Kotlin does not) and made `val` as short as `var`, so the
  read-only choice costs nothing.

---

## 2.9 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | Assuming `long` is 64 bits | It is 32 bits on Windows; use `int64_t` |
| C | Printing an `int64_t` with `%ld` | Wrong on some platforms; use `PRId64` from `<inttypes.h>` |
| C | Writing an overflow check after the addition | The compiler may delete it (section 2.4) |
| C, C++ | Comparing a signed and an unsigned number | `-1 < 1u` is false |
| C++ | Printing a `uint8_t` or `int8_t` with `std::cout` | It prints as a character; cast to `int` first |
| C++ | Printing a `bool` | Prints `1` unless you use `std::boolalpha` |
| Rust | Testing overflow only in debug builds | Release builds wrap instead of panicking |
| Rust | Using `as` to shrink a number you meant to check | `as` silently keeps the low bits; use `try_from` |
| Go | `int(3.99)` on a constant | Compile error; it works on a variable |
| Go | Expecting `0.1 + 0.2` to show float error | Constants are exact, so it prints `0.3` |
| Python | `round(2.5)` | Gives 2, not 3 |
| Python | `True` and `False` in output | Capitalized, unlike every other language here |
| JavaScript | `"42" + 1` | `"421"`: `+` with a string concatenates |
| JavaScript | 64-bit IDs from an API as numbers | Silently rounded past 2^53; keep them as strings or `BigInt` |
| JavaScript | Bitwise operators on large numbers | They convert to 32 bits first: `2 ** 32 \| 0` is `0` |
| TypeScript | Trusting types on data from `JSON.parse` | It returns `any`; nothing checks it at run time |
| Dart | Expecting `7 / 2` to be `3` | `/` always returns a `double`; use `~/` |
| Dart | Running native code on the web | `int` becomes a JavaScript number with the 2^53 limit |
| Kotlin | `someInt == someLong` | Compile error; convert one side with `toLong()` |
| Kotlin | `Int.MAX_VALUE + 1` | Wraps silently to -2147483648; use `Math.addExact` to get an exception |
| Kotlin | `"%.1f".format(x)` on a machine set to German or French | Prints `2,5`; pass `Locale.ROOT` |
| Kotlin | Expecting `"%.1f".format(0.15)` to match C | Prints `0.2`; C and Python print `0.1`, because Java rounds the shortest decimal form, not the exact binary value |

---

## 2.10 Idiom check

**C: say the size.** Native C code uses fixed-width types whenever the size matters:

```c
#include <inttypes.h>
#include <stdint.h>

int64_t total = 0;                    // not `long total`
printf("%" PRId64 "\n", total);       // not "%ld"
```

**Rust: convert with `try_from` when a value might not fit.** `as` is for when you
know the value fits or truly want truncation:

```rust
let big: i64 = 300;
let small = u8::try_from(big);   // Err(TryFromIntError(PosOverflow))
let wrapped = big as u8;         // 44, no questions asked
```

**JavaScript and TypeScript: parse with `Number`, not `parseInt`, unless you want the
prefix.** `parseInt("42px", 10)` is `42` and `Number("42px")` is `NaN`. For validating
input, `NaN` is usually the answer you want. Always give `parseInt` a radix.

**Kotlin: parse with `toIntOrNull()` and give a fallback with `?:`.** A
`try`/`catch` around `toInt()` works, but reads like Java:

```kotlin
val port = text.toIntOrNull() ?: 8080   // not try { text.toInt() } catch (...) { 8080 }
```

**Python: write `//` when you mean integer division**, and use `math.isclose` for float
comparisons. A comparison like `if total == 0.3:` reads fine and is almost always a bug.

---

## 2.11 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Temperature converter](exercises/01-temperature/) | Parsing floats, arithmetic, printing exactly 1 decimal place |
| 2 | [Byte casts](exercises/02-byte-casts/) | Narrowing conversions, signed vs unsigned, two's complement |
| 3 | [Checked add](exercises/03-checked-add/) | 64-bit integers, detecting overflow, JavaScript's 2^53 limit |

```sh
python3 tools/check.py 02                 # your answers, every language
python3 tools/check.py 02 --lang c,rust   # some languages
python3 tools/check.py 02 --solutions     # the reference answers
```

**Stretch:** print the size in bytes of `int`, `long`, `long long`, `float`, and
`double` in C (`sizeof`), Rust (`std::mem::size_of::<T>()`), and Go
(`unsafe.Sizeof`). Then compile the C version for Windows, or look up the sizes for
64-bit Windows, and compare `long`.

---

Previous: [Chapter 1: Toolchains and Hello World](../01-toolchains-and-hello-world/) ·
Next: [Chapter 3: Operators and expressions](../03-operators-and-expressions/)
