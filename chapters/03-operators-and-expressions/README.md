# Chapter 3: Operators and expressions

Operators look like the most portable part of any language. `+` is `+` everywhere.
But `-7 / 2` is -3 in C and -4 in Python, `"" == 0` is true in JavaScript and an error
in Rust, and `x & 1 == 0` means two different things depending on the language. This
chapter compares arithmetic, equality, logic, and bit operations across the eight
languages, and shows where the same symbols give different answers.

**Languages:** all 8 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Integer division and remainder, and why `-7 / 2` has two answers | Circular buffers, hash tables (chapter 24), and number theory (chapter 40) |
| Equality: the same value vs the same object | Every collection chapter, and defining your own `==` in chapter 12 |
| Short-circuit logic and truthiness | Guard clauses and error handling (chapter 11) |
| Bitwise operators and shifts | Flags, hashing, and the competitive programming toolkit (chapter 41) |
| Precedence rules that differ between languages | Reading code written in a language you know less well |

---

## 3.1 The concept: expressions and statements

An **expression** produces a value: `2 + 3`, `x > 0`, `max(a, b)`. A **statement**
does something: declares a variable, runs a loop, returns from a function. Operators
build expressions. The languages disagree about which constructs are expressions, and
that decides what you can write on the right side of `=`.

| Language | `if` gives a value | Assignment gives a value | A block gives a value |
| --- | --- | --- | --- |
| C | No; use `c ? a : b` | Yes: `a = b = 0` works | No |
| C++ | No; use `c ? a : b` | Yes | No |
| Rust | Yes: `if c { a } else { b }` | No: it gives `()` | Yes: its last line, if that line has no `;` |
| Go | No, and there is no `?:` | No: assignment is a statement | No |
| Python | No; use `a if c else b` | Only with the walrus operator `:=` | No |
| JavaScript | No; use `c ? a : b` | Yes | No |
| TypeScript | No; use `c ? a : b` | Yes | No |
| Dart | No; use `c ? a : b`, or a `switch` expression | Yes | No |

Rust is the outlier. Almost everything in Rust is an expression, so a block can
compute a value:

```rust
let sign = if x < 0 { -1 } else { 1 };
let area = {
    let width = 3;
    let height = 4;
    width * height // no semicolon: this is the value of the block
};
```

---

## 3.2 Arithmetic and integer division

All eight languages have `+`, `-`, and `*`. Division is where they split.

| Language | `/` on two integers | Integer division | Remainder | Power |
| --- | --- | --- | --- | --- |
| C | Integer, truncated | `/` | `%` (integers only; `fmod` for floats) | `pow` in `<math.h>`, for doubles |
| C++ | Integer, truncated | `/` | `%` (integers only; `std::fmod`) | `std::pow` in `<cmath>` |
| Rust | Integer, truncated | `/` | `%` (floats too) | `2_i32.pow(10)`, `x.powi(2)`, `x.powf(0.5)` |
| Go | Integer, truncated | `/` | `%` (integers only; `math.Mod`) | `math.Pow`, for `float64` only |
| Python | Float | `//`, floored | `%`, floored | `**`, and `pow(base, exp, mod)` |
| JavaScript | Float (all numbers are) | none: `Math.trunc(a / b)` | `%`, truncated (floats too) | `**`, `Math.pow` |
| TypeScript | Float | none: `Math.trunc(a / b)` | `%`, truncated | `**`, `Math.pow` |
| Dart | `double` | `~/`, truncated | `%`, Euclidean; `remainder()`, truncated | `pow` in `dart:math` |

The program below divides 7 and −7 in each language. The code is in
[`examples/01-division/`](examples/01-division/).

**C**

```c
#include <stdio.h>

int main(void) {
    // Integer division truncates: it drops the fraction, rounding toward zero.
    printf("7 / 2 = %d\n", 7 / 2);
    printf("-7 / 2 = %d\n", -7 / 2);
    // The remainder takes the sign of the left operand, so (a / b) * b + a % b == a.
    printf("7 %% 3 = %d\n", 7 % 3);  // %% prints a single % sign
    printf("-7 %% 3 = %d\n", -7 % 3);
    printf("7 %% -3 = %d\n", 7 % -3);
    // If either side is a double, the other is converted and the result is a double.
    printf("7.0 / 2 = %g\n", 7.0 / 2);
    return 0;
}
```

**C++**

```cpp
#include <iostream>

int main() {
    // Integer division truncates toward zero, exactly as in C.
    std::cout << "7 / 2 = " << 7 / 2 << '\n';
    std::cout << "-7 / 2 = " << -7 / 2 << '\n';
    // The remainder takes the sign of the left operand.
    std::cout << "7 % 3 = " << 7 % 3 << '\n';
    std::cout << "-7 % 3 = " << -7 % 3 << '\n';
    std::cout << "7 % -3 = " << 7 % -3 << '\n';
    // If either side is a double, the other is converted and the result is a double.
    std::cout << "7.0 / 2 = " << 7.0 / 2 << '\n';
}
```

**Rust**

```rust
fn main() {
    // Integer division truncates toward zero, as in C.
    println!("7 / 2 = {}", 7 / 2);
    println!("-7 / 2 = {}", -7 / 2);
    // The remainder takes the sign of the left operand.
    println!("7 % 3 = {}", 7 % 3);
    println!("-7 % 3 = {}", -7 % 3);
    println!("7 % -3 = {}", 7 % -3);
    // Rust never converts between number types for you: 7.0 / 2 does not compile.
    println!("7.0 / 2.0 = {}", 7.0 / 2.0);
    // Euclidean division: the remainder is never negative.
    println!("(-7).div_euclid(2) = {}", (-7_i32).div_euclid(2));
    println!("(-7).rem_euclid(3) = {}", (-7_i32).rem_euclid(3));
}
```

**Go**

```go
package main

import "fmt"

func main() {
	// Integer division truncates toward zero, as in C.
	fmt.Println("7 / 2 =", 7/2)
	fmt.Println("-7 / 2 =", -7/2)
	// The remainder takes the sign of the left operand.
	fmt.Println("7 % 3 =", 7%3)
	fmt.Println("-7 % 3 =", -7%3)
	fmt.Println("7 % -3 =", 7%-3)
	// 7.0 is an untyped float constant, so the whole expression is a float.
	fmt.Println("7.0 / 2 =", 7.0/2)
}
```

**Python**

```python
def main() -> None:
    # // is floor division: it rounds down, toward negative infinity.
    print("7 // 2 =", 7 // 2)
    print("-7 // 2 =", -7 // 2)
    # % matches //, so the remainder takes the sign of the right operand.
    print("7 % 3 =", 7 % 3)
    print("-7 % 3 =", -7 % 3)
    print("7 % -3 =", 7 % -3)
    # / is always true division and always returns a float.
    print("7 / 2 =", 7 / 2)
    # int() truncates a float toward zero, which gives C's answer.
    print("int(-7 / 2) =", int(-7 / 2))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// There is no integer type, so / always gives the exact quotient.
// Math.trunc drops the fraction, which matches C's integer division.
console.log("Math.trunc(7 / 2) =", Math.trunc(7 / 2));
console.log("Math.trunc(-7 / 2) =", Math.trunc(-7 / 2));
// % truncates like C: the remainder takes the sign of the left operand.
console.log("7 % 3 =", 7 % 3);
console.log("-7 % 3 =", -7 % 3);
console.log("7 % -3 =", 7 % -3);
console.log("7 / 2 =", 7 / 2);
// Math.floor rounds down instead, which matches Python's //.
console.log("Math.floor(-7 / 2) =", Math.floor(-7 / 2));
```

**TypeScript**

```ts
// There is no integer type, so / always gives the exact quotient.
// Math.trunc drops the fraction, which matches C's integer division.
console.log("Math.trunc(7 / 2) =", Math.trunc(7 / 2));
console.log("Math.trunc(-7 / 2) =", Math.trunc(-7 / 2));
// % truncates like C: the remainder takes the sign of the left operand.
console.log("7 % 3 =", 7 % 3);
console.log("-7 % 3 =", -7 % 3);
console.log("7 % -3 =", 7 % -3);
console.log("7 / 2 =", 7 / 2);
// Math.floor rounds down instead, which matches Python's //.
console.log("Math.floor(-7 / 2) =", Math.floor(-7 / 2));
export {};
```

**Dart**

```dart
void main() {
  // ~/ is integer division: it truncates toward zero, like C.
  print('7 ~/ 2 = ${7 ~/ 2}');
  print('-7 ~/ 2 = ${-7 ~/ 2}');
  // % is Euclidean modulo: for a nonzero divisor the result is never negative.
  print('7 % 3 = ${7 % 3}');
  print('-7 % 3 = ${-7 % 3}');
  print('7 % -3 = ${7 % -3}');
  // / always returns a double, even for two ints.
  print('7 / 2 = ${7 / 2}');
  // remainder() truncates, which gives C's answer.
  print('(-7).remainder(3) = ${(-7).remainder(3)}');
}
```

Output:

*C, C++ and Go:*

```text
7 / 2 = 3
-7 / 2 = -3
7 % 3 = 1
-7 % 3 = -1
7 % -3 = 1
7.0 / 2 = 3.5
```

*Rust:*

```text
7 / 2 = 3
-7 / 2 = -3
7 % 3 = 1
-7 % 3 = -1
7 % -3 = 1
7.0 / 2.0 = 3.5
(-7).div_euclid(2) = -4
(-7).rem_euclid(3) = 2
```

*Python:*

```text
7 // 2 = 3
-7 // 2 = -4
7 % 3 = 1
-7 % 3 = 2
7 % -3 = -2
7 / 2 = 3.5
int(-7 / 2) = -3
```

*JavaScript and TypeScript:*

```text
Math.trunc(7 / 2) = 3
Math.trunc(-7 / 2) = -3
7 % 3 = 1
-7 % 3 = -1
7 % -3 = 1
7 / 2 = 3.5
Math.floor(-7 / 2) = -4
```

*Dart:*

```text
7 ~/ 2 = 3
-7 ~/ 2 = -3
7 % 3 = 1
-7 % 3 = 2
7 % -3 = 1
7 / 2 = 3.5
(-7).remainder(3) = -1
```

### Three ways to round a quotient

When a division does not come out even, the language has to round the quotient, and
the remainder follows from that choice, because `quotient * b + remainder == a` must
hold. There are three common choices.

| Kind | Quotient rounds | Remainder sign | `-7` by `2` | `-7` by `3` | `7` by `-3` | Who uses it |
| --- | --- | --- | --- | --- | --- | --- |
| Truncated | toward zero | same as `a` | −3 r −1 | −2 r −1 | −2 r 1 | C, C++, Rust, Go, JavaScript, Dart's `~/` and `remainder()` |
| Floored | down | same as `b` | −4 r 1 | −3 r 2 | −3 r −2 | Python's `//` and `%` |
| Euclidean | so the remainder is never negative | always ≥ 0 | −4 r 1 | −3 r 2 | −2 r 1 | Rust's `div_euclid` and `rem_euclid`, Dart's `%` |

For positive numbers all three agree, which is why this difference hides until a
negative number shows up. The classic bug is stepping backward through a circular
buffer:

```c
int prev = (i - 1) % n;   // when i is 0: -1 in C, C++, Rust, Go, JavaScript
                          // n - 1 in Python, and in Dart, whose % is Euclidean
```

In the truncating languages, write `(i + n - 1) % n` instead. Rust also has
`rem_euclid` for signed integers: `(-1_i32).rem_euclid(5)` is 4. Exercise 3 has you
compute both kinds by hand.

### Dividing by zero

| Language | Integer `1 / 0` | Float `1.0 / 0.0` |
| --- | --- | --- |
| C, C++ | **Undefined behavior**; on x86 the program usually crashes | `inf` |
| Rust | Panic: `attempt to divide by zero`. A compile error when the divisor is known to be 0 | `inf` |
| Go | Panic: `integer divide by zero`. A compile error for constants | `+Inf` for variables; a compile error for constants |
| Python | `ZeroDivisionError` | `ZeroDivisionError` as well |
| JavaScript, TypeScript | `Infinity`, because both are floats. `0 / 0` is `NaN`; `1n / 0n` throws `RangeError` | `Infinity` |
| Dart | `1 ~/ 0` throws `IntegerDivisionByZeroException`; `1 / 0` is `Infinity` | `Infinity` |

One integer division also overflows: the most negative `int` divided by −1. In C and
C++, `INT_MIN / -1` is undefined behavior. Rust panics, even in release builds. Go
defines the answer as `INT_MIN`.

### Powers, and the `^` trap

`^` means XOR in all eight languages, never a power. `2 ^ 3` is 1, not 8. Python,
JavaScript, and TypeScript have a real power operator, `**`, with two rules of its own:

```python
-2 ** 2       # -4: ** binds tighter than the minus sign
2 ** 3 ** 2   # 512: ** groups from the right, so this is 2 ** 9
```

JavaScript refuses to guess on the first one: `-2 ** 2` is a `SyntaxError`, and you
must write `(-2) ** 2` or `-(2 ** 2)`.

---

## 3.3 Assignment and increment

| Language | `+=` and friends | `++` and `--` | `if (x = 5)` |
| --- | --- | --- | --- |
| C | Yes | Prefix and postfix, both expressions | Compiles; `-Wall` warns |
| C++ | Yes | Prefix and postfix, both expressions | Compiles; `-Wall` warns |
| Rust | Yes | None: write `x += 1` | Compile error: assignment gives `()`, not `bool` |
| Go | Yes, plus `&^=` | `x++` as a statement only | Compile error |
| Python | Yes, no `++` | None: `++x` is `+(+x)`, and `x++` is a syntax error | Syntax error; use `:=` when you mean it |
| JavaScript | Yes, plus `**=`, `&&=`, `\|\|=`, `??=` | Prefix and postfix | Compiles silently |
| TypeScript | Yes, same as JavaScript | Prefix and postfix | Compiles silently |
| Dart | Yes, plus `~/=` and `??=` | Prefix and postfix | Compile error, unless `x` is a `bool` |

The last column is a famous bug. `if (x = 5)` assigns 5 to `x` and then tests 5,
which is true, so the branch always runs. C and C++ compile it with a warning.
JavaScript and TypeScript compile it silently. Rust, Go, and Python reject it outright.
Dart rejects it because a condition must be a `bool`, so `if (done = true)` still slips
through.

When you do want to assign inside an expression, Python 3.8 added the walrus
operator, which looks different on purpose:

```python
if (n := len(items)) > 10:
    print(f"{n} items is too many")
```

`++` causes a second kind of trouble in C and C++: `i++ + i++` changes `i` twice
without saying which happens first, which is undefined behavior. `-Wall` warns
(`operation on 'i' may be undefined`). Go keeps `x++` but makes it a statement, so it
can never be part of a larger expression. Rust and Python leave it out entirely.

---

## 3.4 Comparison and equality

Equality has two meanings. Two values can be **equal** (the same contents) or
**identical** (the same object in memory). Two strings built separately, both holding
`hello`, are equal but not identical. Each language picks which question `==` asks.
The code is in [`examples/02-equality/`](examples/02-equality/).

**C**

```c
#include <math.h>
#include <stdio.h>
#include <string.h>

static const char *yes_no(int condition) {
    return condition ? "yes" : "no";
}

int main(void) {
    char first[] = "hello";
    char second[] = "hello";  // a second array holding the same characters
    const char *a = first;
    const char *b = second;
    // == on two pointers compares the addresses, not the characters.
    printf("strings with ==: %s\n", yes_no(a == b));
    printf("strings with strcmp: %s\n", yes_no(strcmp(a, b) == 0));

    int xs[] = {1, 2, 3};
    int ys[] = {1, 2, 3};
    // memcmp compares raw bytes. Fine for ints; not for structs with padding.
    printf("arrays with memcmp: %s\n", yes_no(memcmp(xs, ys, sizeof xs) == 0));

    double nan = NAN;
    printf("NaN == NaN: %s\n", yes_no(nan == nan));
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <limits>
#include <string>
#include <vector>

const char* yes_no(bool condition) {
    return condition ? "yes" : "no";
}

int main() {
    std::string a = "hello";
    std::string b = "hel";
    b += "lo";  // built separately, same characters
    // std::string defines == to compare the characters.
    std::cout << "strings with ==: " << yes_no(a == b) << '\n';

    std::vector<int> xs{1, 2, 3};
    std::vector<int> ys{1, 2, 3};
    std::cout << "vectors with ==: " << yes_no(xs == ys) << '\n';

    double nan = std::numeric_limits<double>::quiet_NaN();
    std::cout << "NaN == NaN: " << yes_no(nan == nan) << '\n';
}
```

**Rust**

```rust
fn yes_no(condition: bool) -> &'static str {
    if condition { "yes" } else { "no" }
}

fn main() {
    let a = String::from("hello");
    let mut b = String::from("hel");
    b.push_str("lo"); // built separately, same characters
    // == compares contents for every type that implements PartialEq.
    println!("strings with ==: {}", yes_no(a == b));
    // std::ptr::eq asks a different question: are these the same place in memory?
    let same_place = std::ptr::eq(&a, &b);
    println!("strings with std::ptr::eq: {}", yes_no(same_place));

    let xs = vec![1, 2, 3];
    let ys = vec![1, 2, 3];
    println!("vectors with ==: {}", yes_no(xs == ys));

    let nan = f64::NAN;
    println!("NaN == NaN: {}", yes_no(nan == nan));
}
```

**Go**

```go
package main

import (
	"fmt"
	"math"
	"slices"
	"strings"
)

func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func main() {
	a := "hello"
	b := strings.Join([]string{"hel", "lo"}, "") // built separately, same characters
	fmt.Println("strings with ==:", yesNo(a == b))

	// Arrays have a fixed size, and == compares them element by element.
	xs := [3]int{1, 2, 3}
	ys := [3]int{1, 2, 3}
	fmt.Println("arrays with ==:", yesNo(xs == ys))

	// Slices cannot be compared with == at all: it is a compile error.
	s1 := []int{1, 2, 3}
	s2 := []int{1, 2, 3}
	fmt.Println("slices with slices.Equal:", yesNo(slices.Equal(s1, s2)))

	nan := math.NaN()
	fmt.Println("NaN == NaN:", yesNo(nan == nan))
}
```

**Python**

```python
import math


def yes_no(condition: bool) -> str:
    return "yes" if condition else "no"


def main() -> None:
    a = "hello"
    b = "".join(["hel", "lo"])  # built separately, same characters
    print("strings with ==:", yes_no(a == b))
    # `is` asks a different question: are these the same object?
    print("strings with is:", yes_no(a is b))

    xs = [1, 2, 3]
    ys = [1, 2, 3]
    print("lists with ==:", yes_no(xs == ys))

    nan = math.nan
    print("NaN == NaN:", yes_no(nan == nan))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const yesNo = (condition) => (condition ? "yes" : "no");

const a = "hello";
const b = ["hel", "lo"].join(""); // built separately, same characters
console.log("strings with ===:", yesNo(a === b));

// Arrays are objects, and === on objects asks whether they are the same object.
const xs = [1, 2, 3];
const ys = [1, 2, 3];
console.log("arrays with ===:", yesNo(xs === ys));
const sameItems = xs.length === ys.length && xs.every((x, i) => x === ys[i]);
console.log("arrays element by element:", yesNo(sameItems));

const nan = 0 / 0;
console.log("NaN === NaN:", yesNo(nan === nan));
console.log("Number.isNaN:", yesNo(Number.isNaN(nan)));
```

**TypeScript**

```ts
const yesNo = (condition: boolean): string => (condition ? "yes" : "no");

const a: string = "hello";
const b: string = ["hel", "lo"].join(""); // built separately, same characters
console.log("strings with ===:", yesNo(a === b));

// Arrays are objects, and === on objects asks whether they are the same object.
const xs: number[] = [1, 2, 3];
const ys: number[] = [1, 2, 3];
console.log("arrays with ===:", yesNo(xs === ys));
const sameItems = xs.length === ys.length && xs.every((x, i) => x === ys[i]);
console.log("arrays element by element:", yesNo(sameItems));

const nan: number = 0 / 0;
console.log("NaN === NaN:", yesNo(nan === nan));
console.log("Number.isNaN:", yesNo(Number.isNaN(nan)));
export {};
```

**Dart**

```dart
String yesNo(bool condition) => condition ? 'yes' : 'no';

void main() {
  final a = 'hello';
  final b = ['hel', 'lo'].join(); // built separately, same characters
  print('strings with ==: ${yesNo(a == b)}');

  // A List's == asks whether the two lists are the same object.
  final xs = [1, 2, 3];
  final ys = [1, 2, 3];
  print('lists with ==: ${yesNo(xs == ys)}');
  // Equal const values are created once, at compile time, so they are one object.
  print('const lists with ==: ${yesNo(const [1, 2, 3] == const [1, 2, 3])}');
  var sameItems = xs.length == ys.length;
  for (var i = 0; sameItems && i < xs.length; i++) {
    sameItems = xs[i] == ys[i];
  }
  print('lists element by element: ${yesNo(sameItems)}');

  final nan = double.nan;
  print('NaN == NaN: ${yesNo(nan == nan)}');
}
```

Output:

*C:*

```text
strings with ==: no
strings with strcmp: yes
arrays with memcmp: yes
NaN == NaN: no
```

*C++:*

```text
strings with ==: yes
vectors with ==: yes
NaN == NaN: no
```

*Rust:*

```text
strings with ==: yes
strings with std::ptr::eq: no
vectors with ==: yes
NaN == NaN: no
```

*Go:*

```text
strings with ==: yes
arrays with ==: yes
slices with slices.Equal: yes
NaN == NaN: no
```

*Python:*

```text
strings with ==: yes
strings with is: no
lists with ==: yes
NaN == NaN: no
```

*JavaScript and TypeScript:*

```text
strings with ===: yes
arrays with ===: no
arrays element by element: yes
NaN === NaN: no
Number.isNaN: yes
```

*Dart:*

```text
strings with ==: yes
lists with ==: no
const lists with ==: yes
lists element by element: yes
NaN == NaN: no
```

| Language | Same contents | Same object | `==` on two strings |
| --- | --- | --- | --- |
| C | `strcmp`, `memcmp`, or field by field | `==` on pointers | Compares addresses: almost always a bug |
| C++ | `==`, when the type defines it | `&a == &b` | Compares text for `std::string`, addresses for `const char*` |
| Rust | `==`, from the `PartialEq` trait | `std::ptr::eq(&a, &b)` | Compares text |
| Go | `==` for strings, arrays, and structs; `slices.Equal`, `maps.Equal` | `==` on pointers | Compares text |
| Python | `==` | `is` | Compares text |
| JavaScript | `===` for primitives; nothing built in for arrays and objects | `===` on objects | Compares text |
| TypeScript | Same as JavaScript | Same as JavaScript | Compares text |
| Dart | `==`, which a class can override | `identical(a, b)` | Compares text |

Python's `is` is for `None` and little else. `x is 5` happens to work for small
integers, because CPython reuses one object for each of them, and Python prints a
`SyntaxWarning` because it is still the wrong question.

**NaN is not equal to itself, in every language.** The IEEE 754 standard says a NaN
("not a number") compares unequal to everything, including another NaN. Test for it
with `isnan(x)` in C, `std::isnan` in C++, `x.is_nan()` in Rust, `math.IsNaN` in Go,
`math.isnan` in Python, `Number.isNaN` in JavaScript, and `x.isNaN` in Dart. Python has
one twist: `[x] == [x]` is `True` even when `x` is NaN, because list comparison checks
identity before it checks equality.

### JavaScript's `==` and `===`

JavaScript has two equality operators. `===` (strict equality) compares without
converting anything. `==` (loose equality) converts both sides toward a common type
first, using rules few people remember. Cover the second column and test yourself:

| Expression | Result | Why |
| --- | --- | --- |
| `"1" == 1` | `true` | The string is converted to a number |
| `"1" === 1` | `false` | Different types are never strictly equal |
| `"" == 0` | `true` | `""` converts to `0` |
| `"0" == 0` | `true` | `"0"` converts to `0` |
| `"" == "0"` | `false` | Both are strings, so nothing converts, and the text differs |
| `"0" == false` | `true` | `false` converts to `0`, and so does `"0"` |
| `[] == false` | `true` | `[]` converts to `""`, then to `0` |
| `[] == ![]` | `true` | `![]` is `false`, because every object is truthy |
| `null == undefined` | `true` | A special rule |
| `null == 0` | `false` | `null` is only loosely equal to `undefined` |
| `null >= 0` | `true` | `>=` converts `null` to `0`, but `==` does not |
| `NaN == NaN` | `false` | NaN never equals anything |

Rows 3 to 5 show that `==` is not even transitive: `""` equals `0` and `"0"` equals
`0`, but `""` does not equal `"0"`. Use `===` and `!==` always. The one common
exception is `x == null`, which checks for `null` and `undefined` together.

TypeScript catches many of these mistakes before the code runs. Comparing a `string`
with a `number`, comparing two array literals, and comparing with `NaN` are all
compile errors.

### Chained comparisons

| Language | `1 < x < 10` with `x = 50` |
| --- | --- |
| C, C++ | Compiles and is `true`: `1 < x` gives 1, and `1 < 10`. `-Wall` warns |
| Rust | Compile error: comparison operators cannot be chained |
| Go | Compile error: mismatched types |
| Python | `False`. It means `1 < x and x < 10`, as in math |
| JavaScript | `true`, silently: `true < 10` converts `true` to 1 |
| TypeScript | Compile error: `<` cannot compare a `boolean` with a `number` |
| Dart | Compile error |

---

## 3.5 Logical operators and truthiness

`&&` and `||` (`and` and `or` in Python) **short-circuit** in all eight languages:
they stop as soon as the answer is known. `false && anything` is false without
looking at `anything`, so the right side never runs. That makes guards like
`x != 0 && 10 / x > 1` safe. The program below counts how often the right side runs,
and also picks a default name. The code is in [`examples/03-logic/`](examples/03-logic/).

**C**

```c
#include <stdbool.h>
#include <stdio.h>

static int calls = 0;  // counts how many times the right side ran

static bool big_ratio(int x) {
    calls++;
    return 10 / x > 1;  // dividing by zero here would be undefined behavior
}

static const char *display_name(const char *name) {
    // C's || always gives 0 or 1, never one of its operands, so use ?: for defaults.
    return name[0] != '\0' ? name : "stranger";
}

int main(void) {
    int values[] = {0, 5};
    for (int i = 0; i < 2; i++) {
        int x = values[i];
        // && stops as soon as the left side is false, so big_ratio never sees 0.
        bool ok = x != 0 && big_ratio(x);
        printf("x = %d: %s\n", x, ok ? "yes" : "no");
    }
    printf("right side ran %d of 2 times\n", calls);
    printf("name \"\": %s\n", display_name(""));
    printf("name \"Ada\": %s\n", display_name("Ada"));
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <string>

int calls = 0;  // counts how many times the right side ran

bool big_ratio(int x) {
    ++calls;
    return 10 / x > 1;  // dividing by zero here would be undefined behavior
}

std::string display_name(const std::string& name) {
    return name.empty() ? "stranger" : name;
}

int main() {
    for (int x : {0, 5}) {
        // && stops as soon as the left side is false, so big_ratio never sees 0.
        bool ok = x != 0 && big_ratio(x);
        std::cout << "x = " << x << ": " << (ok ? "yes" : "no") << '\n';
    }
    std::cout << "right side ran " << calls << " of 2 times\n";
    std::cout << "name \"\": " << display_name("") << '\n';
    std::cout << "name \"Ada\": " << display_name("Ada") << '\n';
}
```

**Rust**

```rust
fn big_ratio(x: i32, calls: &mut u32) -> bool {
    *calls += 1;
    10 / x > 1 // dividing by zero here would panic
}

fn display_name(name: &str) -> &str {
    // if is an expression, so it does the job of C's ?: operator.
    if name.is_empty() { "stranger" } else { name }
}

fn main() {
    let mut calls = 0; // counts how many times the right side ran
    for x in [0, 5] {
        // && stops as soon as the left side is false, so big_ratio never sees 0.
        let ok = x != 0 && big_ratio(x, &mut calls);
        println!("x = {x}: {}", if ok { "yes" } else { "no" });
    }
    println!("right side ran {calls} of 2 times");
    println!("name \"\": {}", display_name(""));
    println!("name \"Ada\": {}", display_name("Ada"));
}
```

**Go**

```go
package main

import (
	"cmp"
	"fmt"
)

var calls = 0 // counts how many times the right side ran

func bigRatio(x int) bool {
	calls++         // ++ is a statement in Go, never part of an expression
	return 10/x > 1 // dividing by zero here would panic
}

func displayName(name string) string {
	// Go has no ?: operator. cmp.Or returns its first argument that is not the zero value.
	return cmp.Or(name, "stranger")
}

func main() {
	for _, x := range []int{0, 5} {
		// && stops as soon as the left side is false, so bigRatio never sees 0.
		ok := x != 0 && bigRatio(x)
		answer := "no"
		if ok {
			answer = "yes"
		}
		fmt.Printf("x = %d: %s\n", x, answer)
	}
	fmt.Printf("right side ran %d of 2 times\n", calls)
	fmt.Printf("name %q: %s\n", "", displayName(""))
	fmt.Printf("name %q: %s\n", "Ada", displayName("Ada"))
}
```

**Python**

```python
calls = 0  # counts how many times the right side ran


def big_ratio(x: int) -> bool:
    global calls
    calls += 1
    return 10 // x > 1  # dividing by zero here would raise ZeroDivisionError


def display_name(name: str) -> str:
    # `or` returns its left operand if that is truthy, and its right operand otherwise.
    return name or "stranger"


def main() -> None:
    for x in [0, 5]:
        # `and` stops as soon as the left side is false, so big_ratio never sees 0.
        ok = x != 0 and big_ratio(x)
        print(f"x = {x}: {'yes' if ok else 'no'}")
    print(f"right side ran {calls} of 2 times")
    print(f'name "": {display_name("")}')
    print(f'name "Ada": {display_name("Ada")}')


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
let calls = 0; // counts how many times the right side ran

function bigRatio(x) {
  calls++;
  return 10 / x > 1; // 10 / 0 does not crash: it is Infinity, a silently wrong answer
}

function displayName(name) {
  // || returns its left operand if that is truthy, and its right operand otherwise.
  return name || "stranger";
}

for (const x of [0, 5]) {
  // && stops as soon as the left side is falsy, so bigRatio never sees 0.
  const ok = x !== 0 && bigRatio(x);
  console.log(`x = ${x}: ${ok ? "yes" : "no"}`);
}
console.log(`right side ran ${calls} of 2 times`);
console.log(`name "": ${displayName("")}`);
console.log(`name "Ada": ${displayName("Ada")}`);
```

**TypeScript**

```ts
let calls = 0; // counts how many times the right side ran

function bigRatio(x: number): boolean {
  calls++;
  return 10 / x > 1; // 10 / 0 does not crash: it is Infinity, a silently wrong answer
}

function displayName(name: string): string {
  // || returns its left operand if that is truthy, and its right operand otherwise.
  return name || "stranger";
}

for (const x of [0, 5]) {
  // && stops as soon as the left side is falsy, so bigRatio never sees 0.
  const ok: boolean = x !== 0 && bigRatio(x);
  console.log(`x = ${x}: ${ok ? "yes" : "no"}`);
}
console.log(`right side ran ${calls} of 2 times`);
console.log(`name "": ${displayName("")}`);
console.log(`name "Ada": ${displayName("Ada")}`);
export {};
```

**Dart**

```dart
var calls = 0; // counts how many times the right side ran

bool bigRatio(int x) {
  calls++;
  return 10 ~/ x > 1; // dividing by zero here would throw
}

String displayName(String name) {
  // Conditions must be bool, so test isEmpty explicitly.
  return name.isEmpty ? 'stranger' : name;
}

void main() {
  for (final x in [0, 5]) {
    // && stops as soon as the left side is false, so bigRatio never sees 0.
    final ok = x != 0 && bigRatio(x);
    print('x = $x: ${ok ? 'yes' : 'no'}');
  }
  print('right side ran $calls of 2 times');
  print('name "": ${displayName('')}');
  print('name "Ada": ${displayName('Ada')}');
}
```

Output, identical in all eight:

```text
x = 0: no
x = 5: yes
right side ran 1 of 2 times
name "": stranger
name "Ada": Ada
```

Without the guard, `10 / 0` would be undefined behavior in C and C++, a panic in
Rust and Go, and an exception in Python and Dart. In JavaScript and TypeScript it
would be `Infinity`, and `Infinity > 1` is `true`: no crash, just a wrong answer.

| Language | And, or, not | What `a \|\| b` returns | A default for an empty string |
| --- | --- | --- | --- |
| C | `&&`, `\|\|`, `!` | `0` or `1` | `s[0] != '\0' ? s : "stranger"` |
| C++ | `&&`, `\|\|`, `!` | `bool` | `s.empty() ? "stranger" : s` |
| Rust | `&&`, `\|\|`, `!` | `bool`, and both sides must be `bool` | `if s.is_empty() { "stranger" } else { s }` |
| Go | `&&`, `\|\|`, `!` | `bool`, and both sides must be `bool` | `cmp.Or(s, "stranger")` |
| Python | `and`, `or`, `not` | One of its operands | `s or "stranger"` |
| JavaScript | `&&`, `\|\|`, `!` | One of its operands | `s \|\| "stranger"` |
| TypeScript | `&&`, `\|\|`, `!` | One of its operands | `s \|\| "stranger"` |
| Dart | `&&`, `\|\|`, `!` | `bool`, and both sides must be `bool` | `s.isEmpty ? 'stranger' : s` |

### Truthiness

Which values count as false in a condition?

| Language | A condition accepts | Values that count as false |
| --- | --- | --- |
| C | Any number or pointer | `0`, `0.0`, `NULL` |
| C++ | Numbers, pointers, and classes with `explicit operator bool` (streams, smart pointers, `std::optional`) | `0`, `0.0`, `nullptr`, and whatever those classes decide |
| Rust | `bool` only | `false` |
| Go | `bool` only | `false` |
| Python | Anything | `False`, `None`, `0`, `0.0`, `""`, and empty collections such as `[]`, `{}`, `set()` |
| JavaScript | Anything | `false`, `0`, `-0`, `0n`, `""`, `null`, `undefined`, `NaN` |
| TypeScript | Anything | Same as JavaScript |
| Dart | `bool` only | `false` |

Python and JavaScript disagree about containers: an empty list is false in Python and
true in JavaScript, where every object is truthy. `"0"` is true in both.

**`||` vs `??`.** In JavaScript, `||` replaces every falsy value, including real values
like `0` and `""`. The `??` operator (2020) replaces only `null` and `undefined`. Dart
has `??` too. Dart has no truthiness, so `??` is its only default operator; for an
empty string you test `isEmpty`.

```js
const volume = settings.volume || 50; // a muted volume of 0 becomes 50: a bug
const volume = settings.volume ?? 50; // only a missing volume becomes 50
```

**The conditional operator** is `c ? a : b` in C, C++, JavaScript, TypeScript, and Dart.
Rust uses `if c { a } else { b }`, Python uses `a if c else b`, and Go has none: you
write an `if` statement, or reach for `cmp.Or` when the test is "is it empty?".

---

## 3.6 Bitwise operators

Bitwise operators work on the binary digits of integers. They store several yes/no
flags in one number, mask off parts of a value, and make some arithmetic faster.

| Operation | C, C++, Python, JavaScript, TypeScript, Dart | Rust | Go |
| --- | --- | --- | --- |
| AND | `a & b` | `a & b` | `a & b` |
| OR | `a \| b` | `a \| b` | `a \| b` |
| XOR | `a ^ b` | `a ^ b` | `a ^ b` |
| NOT (flip every bit) | `~a` | `!a` | `^a` |
| AND NOT (clear bits) | `a & ~b` | `a & !b` | `a &^ b` |
| Shift left | `a << n` | `a << n` | `a << n` |
| Shift right | `a >> n` | `a >> n` | `a >> n` |
| Shift right, filling with zeros | `a >>> n` in JavaScript, TypeScript, and Dart; an unsigned type in C and C++ | use an unsigned type | use an unsigned type |

The program below stores read, write, and execute permissions as three bits, much
like Unix file permissions. The code is in [`examples/04-bits/`](examples/04-bits/).

**C**

```c
#include <stdio.h>

enum { READ = 1, WRITE = 2, EXEC = 4 };  // one bit each: 001, 010, 100

// C17's printf has no binary format (C23 adds %b), so print the bits by hand.
static void print_bits(const char *label, unsigned value) {
    char text[9];
    for (int i = 0; i < 8; i++) {
        text[i] = ((value >> (7 - i)) & 1u) ? '1' : '0';
    }
    text[8] = '\0';
    printf("%s: %s\n", label, text);
}

int main(void) {
    unsigned perms = READ | WRITE;  // turn bits on with |
    print_bits("read | write", perms);
    // Test a bit with &. The parentheses matter: == binds tighter than & in C.
    printf("has write? %s\n", (perms & WRITE) != 0 ? "yes" : "no");
    printf("has exec? %s\n", (perms & EXEC) != 0 ? "yes" : "no");
    perms &= ~WRITE;  // turn a bit off: AND with every bit except WRITE
    print_bits("clear write", perms);
    perms ^= EXEC;  // flip a bit with ^
    print_bits("toggle exec", perms);
    printf("5 << 2 = %d\n", 5 << 2);      // shifting left by n multiplies by 2^n
    printf("-16 >> 2 = %d\n", -16 >> 2);  // implementation-defined for negative numbers
    // 0x16 is 10110 in binary. Binary literals such as 0b10110 arrived in C23.
    printf("set bits in 10110: %d\n", __builtin_popcount(0x16u));  // GCC and Clang
    return 0;
}
```

**C++**

```cpp
#include <bit>
#include <bitset>
#include <iostream>

constexpr unsigned READ = 0b001;
constexpr unsigned WRITE = 0b010;
constexpr unsigned EXEC = 0b100;

int main() {
    unsigned perms = READ | WRITE;  // turn bits on with |
    std::cout << "read | write: " << std::bitset<8>(perms) << '\n';
    // Test a bit with &. The parentheses matter: == binds tighter than & in C++.
    std::cout << "has write? " << ((perms & WRITE) != 0 ? "yes" : "no") << '\n';
    std::cout << "has exec? " << ((perms & EXEC) != 0 ? "yes" : "no") << '\n';
    perms &= ~WRITE;  // turn a bit off: AND with every bit except WRITE
    std::cout << "clear write: " << std::bitset<8>(perms) << '\n';
    perms ^= EXEC;  // flip a bit with ^
    std::cout << "toggle exec: " << std::bitset<8>(perms) << '\n';
    std::cout << "5 << 2 = " << (5 << 2) << '\n';  // << also means "print", so wrap it
    std::cout << "-16 >> 2 = " << (-16 >> 2) << '\n';  // defined as arithmetic since C++20
    std::cout << "set bits in 10110: " << std::popcount(0b10110u) << '\n';
}
```

**Rust**

```rust
const READ: u8 = 0b001;
const WRITE: u8 = 0b010;
const EXEC: u8 = 0b100;

fn yes_no(condition: bool) -> &'static str {
    if condition { "yes" } else { "no" }
}

fn main() {
    let mut perms = READ | WRITE; // turn bits on with |
    println!("read | write: {perms:08b}");
    // & binds tighter than == in Rust, so no parentheses are needed.
    println!("has write? {}", yes_no(perms & WRITE != 0));
    println!("has exec? {}", yes_no(perms & EXEC != 0));
    perms &= !WRITE; // Rust writes bitwise NOT as !, not ~
    println!("clear write: {perms:08b}");
    perms ^= EXEC; // flip a bit with ^
    println!("toggle exec: {perms:08b}");
    println!("5 << 2 = {}", 5 << 2);
    println!("-16 >> 2 = {}", -16 >> 2); // signed types copy the sign bit in
    println!("set bits in 10110: {}", 0b10110_u32.count_ones());
}
```

**Go**

```go
package main

import (
	"fmt"
	"math/bits"
)

const (
	Read  = 1 << iota // 001: iota counts 0, 1, 2 down the block
	Write             // 010
	Exec              // 100
)

func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func main() {
	perms := Read | Write // turn bits on with |
	fmt.Printf("read | write: %08b\n", perms)
	// & binds tighter than == in Go, so no parentheses are needed.
	fmt.Println("has write?", yesNo(perms&Write != 0))
	fmt.Println("has exec?", yesNo(perms&Exec != 0))
	perms &^= Write // Go's AND NOT operator clears bits
	fmt.Printf("clear write: %08b\n", perms)
	perms ^= Exec // flip a bit with ^
	fmt.Printf("toggle exec: %08b\n", perms)
	fmt.Println("5 << 2 =", 5<<2)
	fmt.Println("-16 >> 2 =", -16>>2)
	fmt.Println("set bits in 10110:", bits.OnesCount(0b10110))
}
```

**Python**

```python
READ = 0b001
WRITE = 0b010
EXEC = 0b100


def main() -> None:
    perms = READ | WRITE  # turn bits on with |
    print(f"read | write: {perms:08b}")
    # & binds tighter than == in Python, so no parentheses are needed.
    print("has write?", "yes" if perms & WRITE != 0 else "no")
    print("has exec?", "yes" if perms & EXEC != 0 else "no")
    perms &= ~WRITE  # turn a bit off: AND with every bit except WRITE
    print(f"clear write: {perms:08b}")
    perms ^= EXEC  # flip a bit with ^
    print(f"toggle exec: {perms:08b}")
    print("5 << 2 =", 5 << 2)
    print("-16 >> 2 =", -16 >> 2)
    print("set bits in 10110:", (0b10110).bit_count())  # Python 3.10 and later


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const READ = 0b001;
const WRITE = 0b010;
const EXEC = 0b100;

const bits8 = (n) => n.toString(2).padStart(8, "0");

function countOnes(n) {
  let count = 0;
  for (let rest = n; rest !== 0; rest >>>= 1) {
    count += rest & 1;
  }
  return count;
}

let perms = READ | WRITE; // turn bits on with |
console.log(`read | write: ${bits8(perms)}`);
// Test a bit with &. The parentheses matter: === binds tighter than & in JavaScript.
console.log(`has write? ${(perms & WRITE) !== 0 ? "yes" : "no"}`);
console.log(`has exec? ${(perms & EXEC) !== 0 ? "yes" : "no"}`);
perms &= ~WRITE; // turn a bit off: AND with every bit except WRITE
console.log(`clear write: ${bits8(perms)}`);
perms ^= EXEC; // flip a bit with ^
console.log(`toggle exec: ${bits8(perms)}`);
console.log(`5 << 2 = ${5 << 2}`);
console.log(`-16 >> 2 = ${-16 >> 2}`);
console.log(`set bits in 10110: ${countOnes(0b10110)}`);
```

**TypeScript**

```ts
const READ = 0b001;
const WRITE = 0b010;
const EXEC = 0b100;

const bits8 = (n: number): string => n.toString(2).padStart(8, "0");

function countOnes(n: number): number {
  let count = 0;
  for (let rest = n; rest !== 0; rest >>>= 1) {
    count += rest & 1;
  }
  return count;
}

let perms: number = READ | WRITE; // turn bits on with |
console.log(`read | write: ${bits8(perms)}`);
// Test a bit with &. The parentheses matter: === binds tighter than & in TypeScript.
console.log(`has write? ${(perms & WRITE) !== 0 ? "yes" : "no"}`);
console.log(`has exec? ${(perms & EXEC) !== 0 ? "yes" : "no"}`);
perms &= ~WRITE; // turn a bit off: AND with every bit except WRITE
console.log(`clear write: ${bits8(perms)}`);
perms ^= EXEC; // flip a bit with ^
console.log(`toggle exec: ${bits8(perms)}`);
console.log(`5 << 2 = ${5 << 2}`);
console.log(`-16 >> 2 = ${-16 >> 2}`);
console.log(`set bits in 10110: ${countOnes(0b10110)}`);
export {};
```

**Dart**

```dart
// Dart has no binary literals, so build each flag with a shift.
const read = 1 << 0; // 001
const write = 1 << 1; // 010
const exec = 1 << 2; // 100

String bits8(int n) => n.toRadixString(2).padLeft(8, '0');

int countOnes(int n) {
  var count = 0;
  for (var rest = n; rest != 0; rest >>>= 1) {
    count += rest & 1;
  }
  return count;
}

void main() {
  var perms = read | write; // turn bits on with |
  print('read | write: ${bits8(perms)}');
  // & binds tighter than == in Dart, so no parentheses are needed.
  print('has write? ${perms & write != 0 ? 'yes' : 'no'}');
  print('has exec? ${perms & exec != 0 ? 'yes' : 'no'}');
  perms &= ~write; // turn a bit off: AND with every bit except write
  print('clear write: ${bits8(perms)}');
  perms ^= exec; // flip a bit with ^
  print('toggle exec: ${bits8(perms)}');
  print('5 << 2 = ${5 << 2}');
  print('-16 >> 2 = ${-16 >> 2}');
  print('set bits in 10110: ${countOnes(int.parse('10110', radix: 2))}');
}
```

Output, identical in all eight:

```text
read | write: 00000011
has write? yes
has exec? no
clear write: 00000001
toggle exec: 00000101
5 << 2 = 20
-16 >> 2 = -4
set bits in 10110: 3
```

### Shifts that go too far

| Language | `>>` on a negative number | Shifting by the full width or more |
| --- | --- | --- |
| C | Implementation-defined; every mainstream compiler copies the sign bit | **Undefined behavior** |
| C++ | Copies the sign bit, as defined since C++20 | **Undefined behavior** |
| Rust | Copies the sign bit for signed types, fills with zeros for unsigned | Panic in debug builds; in release, the shift amount wraps (`1u32 << 33` is 2) |
| Go | Copies the sign bit for signed types, fills with zeros for unsigned | Defined: the result is 0, or −1 for a negative number shifted right |
| Python | Copies the sign bit (it rounds down) | No limit: `1 << 64` is 18446744073709551616 |
| JavaScript, TypeScript | Works on 32 bits and copies the sign bit; `>>>` fills with zeros | The amount wraps at 32: `1 << 32` is 1 |
| Dart | Works on 64 bits and copies the sign bit; `>>>` fills with zeros | The result is 0: `1 << 64` is 0 |

**JavaScript's bitwise operators work on 32-bit integers.** Every operand is converted
to a signed 32-bit integer first, so `2 ** 31 | 0` is −2147483648 and `2 ** 32 | 0` is 0.
Dart's are 64-bit natively, so `-16 >>> 2` is 1073741820 in JavaScript but
4611686018427387900 in Dart.

### Writing and printing binary

| Language | Binary literal | Print in binary | Count the 1 bits |
| --- | --- | --- | --- |
| C | `0b1010` from C23 | `%b` from C23; a loop before that | `__builtin_popcount` (GCC, Clang); `stdc_count_ones` in C23 |
| C++ | `0b1010` | `std::bitset<8>(x)`, `std::format("{:08b}", x)` | `std::popcount` (C++20) |
| Rust | `0b1010` | `format!("{x:08b}")` | `x.count_ones()` |
| Go | `0b1010` | `fmt.Sprintf("%08b", x)` | `bits.OnesCount(x)` |
| Python | `0b1010` | `f"{x:08b}"`, `bin(x)` | `x.bit_count()` (3.10) |
| JavaScript, TypeScript | `0b1010` | `x.toString(2).padStart(8, "0")` | a loop |
| Dart | none: `int.parse('1010', radix: 2)` | `x.toRadixString(2).padLeft(8, '0')` | a loop |

---

## 3.7 Precedence

Precedence decides which operator applies first. Every language puts `*` before `+`,
as in math, and `&&` before `||`. The differences are in the bitwise and shift
operators.

| Expression | C, C++, JavaScript, TypeScript | Rust, Python, Dart | Go |
| --- | --- | --- | --- |
| `x & 1 == 0` | `x & (1 == 0)`, which is always 0 | `(x & 1) == 0` | `(x & 1) == 0` |
| `a << 1 + 1` | `a << (1 + 1)` | `a << (1 + 1)` | `(a << 1) + 1` |

The first row is a historical accident. Early C had no `&&` or `||`: `&` and `|` did
both jobs, so they sat just below `==`, where logical operators belong. When `&&` and
`||` were added, Dennis Ritchie kept the old precedence so existing code would not
break. C++, Java, and JavaScript copied it. Newer languages fixed it, and `-Wall` warns
about it in C and C++ (`suggest parentheses around comparison in operand of '&'`).

Go has only five precedence levels and puts `<<` and `&` with `*`, so a shift binds
like a multiplication. `gofmt` shows this with spacing: it formats the expression as
`a<<1 + 1`.

Two more traps:

- **A minus sign is not part of a number literal** in Rust or Dart. `-7.abs()` means
  `-(7.abs())`, which is −7. Write `(-7).abs()`, or `(-7_i32).abs()` in Rust.
- **In C++, `<<` is also the print operator.** `std::cout << 5 << 2` prints `52`. Wrap
  shifts in parentheses: `std::cout << (5 << 2)`.

When an expression mixes bitwise operators with comparisons, or shifts with
arithmetic, add parentheses. The compiler does not need them, but the next reader
does.

---

## 3.8 Operator overloading

Some languages let your own types define what `+` or `==` means. This is how C++'s
`std::string` supports `a + b` and how a game's 2D vector type can write
`position + velocity`.

| Language | Your type can define `+` | Your type can define `==` |
| --- | --- | --- |
| C | No | No |
| C++ | Yes: `Vec2 operator+(Vec2, Vec2)` | Yes: `operator==`, and C++20 can generate all six comparisons with `operator<=> = default` |
| Rust | Yes, by implementing the `Add` trait | Yes: `#[derive(PartialEq)]` or implement `PartialEq` |
| Go | No | No, but `==` already compares structs field by field |
| Python | Yes: `__add__` | Yes: `__eq__`, which `@dataclass` writes for you |
| JavaScript | No | No: `===` on objects always means "same object" |
| TypeScript | No | No |
| Dart | Yes: `Vec2 operator +(Vec2 other)` | Yes: `operator ==`, together with `hashCode` |

Here is the same small vector type in Python and Rust:

```python
from dataclasses import dataclass


@dataclass
class Vec2:
    x: float
    y: float

    def __add__(self, other: "Vec2") -> "Vec2":
        return Vec2(self.x + other.x, self.y + other.y)


print(Vec2(1, 2) + Vec2(3, 4))  # Vec2(x=4, y=6)
```

```rust
use std::ops::Add;

#[derive(Debug, Clone, Copy, PartialEq)]
struct Vec2 {
    x: f64,
    y: f64,
}

impl Add for Vec2 {
    type Output = Vec2;

    fn add(self, other: Vec2) -> Vec2 {
        Vec2 {
            x: self.x + other.x,
            y: self.y + other.y,
        }
    }
}
```

In Go, JavaScript, and TypeScript you write a method instead: `a.Add(b)` or
`a.add(b)`. Chapter 12 covers defining equality and ordering for your own types, and
chapter 45 uses vector operators for game physics.

---

## 3.9 Why each language looks the way it does

- **C** keeps its operators close to machine instructions. Where machines disagreed,
  such as shifting a negative number or overflowing, C left the result up to the
  compiler. Its odd `&` precedence is a fossil from before `&&` existed.
- **C++** inherited C's operators and added overloading, so library types such as
  `std::string`, `std::vector`, and `std::complex` work with the same symbols as
  built-in types. That is also how `<<` came to mean "print".
- **Rust** keeps C's symbols but removes its traps: no `++`, no assignment inside a
  condition, `bool`-only conditions, corrected `&` precedence, and no chained
  comparisons. Overloading goes through traits such as `Add` and `PartialEq`, so it is
  visible in the type's definition.
- **Go** aims for code that reads the same no matter who wrote it. `++` is only a
  statement, there is no `?:`, and there is no overloading, so `a + b` always means
  built-in addition. The Go FAQ says the designers left out `?:` because they had
  seen it used to build impenetrably complex expressions.
- **Python** reads like English (`and`, `or`, `not`, `is`, `in`) and chains comparisons
  as in math. Python 2.2 added `//` so that `/` would stop meaning two different
  things. Floored division keeps `%` non-negative for a positive divisor, which suits
  clock and calendar arithmetic.
- **JavaScript** was built in ten days in 1995 for small scripts in web pages, and
  `==` converted types freely so that form input like `"5"` would compare equal to
  `5`. `===` was
  added a few years later to undo that, and `??` arrived in 2020.
- **TypeScript** cannot change what JavaScript's operators do at run time, so it
  rejects code where they cannot be meaningful: `"1" === 1`, `[1] === [1]`, and
  `1 < x < 10` are compile errors.
- **Dart** borrows C's symbols with Java-style discipline: conditions must be `bool`,
  and `~/` exists so that `/` always means real division. Its Euclidean `%` means
  `i % n` is always a valid index into a list of length `n`.

---

## 3.10 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| All | `2 ^ 3` for a power | XOR: the answer is 1 |
| C, C++ | `if (x = 5)` | Assigns 5, then is always true |
| C, C++ | `x & 1 == 0` | Means `x & (1 == 0)`, which is always 0 |
| C, C++ | `i++ + i++` | Undefined behavior |
| C, C++ | `(i - 1) % n` when `i` is 0 | −1, which is not a valid index |
| C, C++ | `INT_MIN / -1`, or `1 << 32` on a 32-bit `int` | Undefined behavior |
| C | Comparing strings with `==` | Compares addresses, not text |
| C++ | `std::cout << 5 << 2` | Prints `52`; wrap shifts in parentheses |
| Rust | Expecting `rem_euclid` to match Python's `%` | They differ when the divisor is negative |
| Rust | `!x` on an integer meaning "is zero" | It flips every bit: `!0u8` is 255 |
| Rust, Dart | `-7.abs()` | −7: the method runs before the minus sign |
| Go | `a << 1 + 1` | Means `(a << 1) + 1`, unlike C |
| Python | `x is 5`, `s is "text"` | Tests identity, not equality; only works by accident |
| Python | `++x` | Does nothing: it is `+(+x)` |
| Python | `-7 // 2` | −4, not −3 |
| JavaScript | `==` | Converts types: `"" == 0` is `true` |
| JavaScript | `value \|\| fallback` when `value` can be `0` or `""` | Replaces real values; use `??` |
| JavaScript | Bitwise operators on numbers ≥ 2^31 | They come out negative: `2 ** 31 \| 0` is −2147483648 |
| JavaScript | `[1, 2] === [1, 2]` | `false`: compares identity |
| TypeScript | Trusting types to make `==` safe | Types are erased; data from `JSON.parse` can be anything |
| Dart | `/` on two `int`s | Returns a `double`; use `~/` |

---

## 3.11 Idiom check

**C and C++: parenthesize every bitwise test**, and keep `-Wall` on so the compiler
catches the ones you miss:

```c
if ((flags & WRITE) != 0) { ... }   // not: if (flags & WRITE != 0)
```

**Rust: use the named methods when the default does not fit.** `rem_euclid` for
indexes that can go negative, `checked_div` when the divisor might be 0, and
`wrapping_shl` when you want the shift amount to wrap on purpose.

**Go: no ternary, so set a default and override it.** This is the shape the
standard library uses:

```go
label := "odd"
if n%2 == 0 {
	label = "even"
}
```

**Python: let truthiness and `None` checks read naturally.** Write `if not items:`
rather than `if len(items) == 0:`, write `if x is None:` rather than `if x == None:`,
and use `divmod(a, b)` when you need both the quotient and the remainder.

**JavaScript and TypeScript: `===` always, `??` for defaults, `Number.isNaN` for NaN.**
Never compare with `NaN` directly, and remember that `isNaN("abc")` (without
`Number.`) converts its argument first and returns `true`.

**Dart: `??` and `??=` for defaults, `~/` for integer division**, and
`identical(a, b)` only when you truly mean "the same object".

---

## 3.12 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Leap year](exercises/01-leap-year/) | `%`, `&&`, `\|\|`, and building one boolean expression |
| 2 | [Bit count](exercises/02-bit-count/) | Binary output, counting bits, unsigned 32-bit numbers, JavaScript's `>>>` |
| 3 | [Division kinds](exercises/03-division-kinds/) | Truncated vs floored division, negative remainders |

```sh
python3 tools/check.py 03                    # your answers, every language
python3 tools/check.py 03 --lang python,go   # some languages
python3 tools/check.py 03 --solutions        # the reference answers
```

**Stretch:** a positive integer `n` is a power of two exactly when `n & (n - 1)` is 0.
Write `is_power_of_two(n)` in C, Go, and Python, first without any parentheses around
`n & (n - 1)`. Which compile, which give the right answer, and why?

---

Previous: [Chapter 2: Values, types, and variables](../02-values-types-and-variables/) ·
Next: [Chapter 4: Control flow and pattern matching](../04-control-flow-and-pattern-matching/)
