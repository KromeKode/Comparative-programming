# Chapter 5: Functions and closures

A function gives a piece of code a name and some inputs, so you can run it again with
different values. All nine languages have functions, and they agree on the basic call
`f(x, y)`. They disagree on nearly everything around it: whether a parameter can have a
default, whether you can pass arguments by name, how to return two values, whether a
function is a value you can store in a list, and what happens to the variables a
function uses from its surroundings once the code that made it has returned. That last
idea, the **closure**, is the main subject of this chapter.

**Languages:** all 9 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Declaring functions, parameters, and return types | Every chapter |
| Default and named arguments, and any number of arguments | API design (chapter 12), request handlers (chapter 37) |
| Returning several values | Go's `value, err` and error handling (chapter 11) |
| Functions as values: passing, returning, and storing them | Iterators (chapter 14), sorting with a comparator (chapter 28) |
| Closures and what they capture | Threads and tasks (chapter 16), web servers (chapters 35 to 38), GUI callbacks (chapter 48) |
| Recursion and memoization | Backtracking and dynamic programming (chapter 29) |

---

## 5.1 The concept: a named computation with its own variables

A **parameter** is the name in the definition. An **argument** is the value at the
call. In `def area(width, height)`, `width` is a parameter, and in `area(3, 4)`, `3` is
an argument.

Each call gets a fresh set of local variables, kept in a **stack frame**. When the
function returns, its frame is thrown away, and so are its locals. That is why a
function can call itself (each call has its own `n`), and why closures are
interesting: a closure is a function that keeps using some of those variables after
the frame that held them is gone.

A language has **first-class functions** if a function is an ordinary value: you can
pass it as an argument, return it, and store it in a variable or a list. A function
that takes or returns another function is a **higher-order function**. Eight of the
nine languages have first-class functions. C has function pointers, which can be
passed and stored, but it cannot make a new function while the program runs.

---

## 5.2 Declaring a function

| Language | `add` | Return type | Returns the last expression |
| --- | --- | --- | --- |
| C | `int add(int a, int b) { return a + b; }` | Before the name | No |
| C++ | `int add(int a, int b) { return a + b; }` | Before the name, or after: `auto add(int a, int b) -> int` | No |
| Rust | `fn add(a: i32, b: i32) -> i32 { a + b }` | After `->` | **Yes**, if it has no `;` |
| Go | `func add(a, b int) int { return a + b }` | After the parameters | No |
| Python | `def add(a: int, b: int) -> int: return a + b` | After `->`, as an optional hint | Only `lambda` |
| JavaScript | `function add(a, b) { return a + b; }` or `const add = (a, b) => a + b;` | None | Only arrows without braces |
| TypeScript | `function add(a: number, b: number): number { return a + b; }` | After `:` | Only arrows without braces |
| Dart | `int add(int a, int b) => a + b;` | Before the name | Only `=>` functions |
| Kotlin | `fun add(a: Int, b: Int): Int = a + b` | After `:`; may be left out after `=` | Functions written with `=`, and lambdas |

**Rust's last expression.** A Rust block's value is its last expression, so a function
body that ends in `a + b` returns it. Adding a semicolon turns the expression into a
statement, and the function returns `()` instead:

```text
error[E0308]: mismatched types
 --> main.rs:1:27
  |
1 | fn add(a: i32, b: i32) -> i32 {
  |    ---                    ^^^ expected `i32`, found `()`
  |    |
  |    implicitly returns `()` as its body has no tail or `return` expression
2 |     a + b;
```

**Forgetting to return.** If some path through a function ends without a `return`:

| Language | What happens |
| --- | --- |
| C, C++ | Undefined behavior if the caller uses the result. `-Wall` warns: `control reaches end of non-void function` |
| Rust, Go, Kotlin | Compile error (`missing return` in Go, `missing return statement` in Kotlin) |
| TypeScript | Compile error TS2366 when the return type does not include `undefined` |
| Dart | Compile error when the return type is not nullable |
| Python, JavaScript | Returns `None` or `undefined` |

**Declaring before use.** C and C++ read a file from top to bottom, so a function must
be declared before the first call. Either define it higher up, or write a **prototype**
such as `int add(int a, int b);` near the top. Rust, Go, Dart, and Kotlin do not care
about order. JavaScript **hoists** `function` declarations, so you can call them above
the line they are written on, but a `const f = () => ...` does not exist until its line
runs. Python's `def` is a statement that runs, so the function exists from that line on.
A function can still call one defined below it, as long as that `def` has run by the
time of the call.

**`int f()` in C.** In C17 and earlier, empty parentheses mean "takes unknown
arguments", not "takes none". With `int answer();` declared, `answer(1, 2, 3)`
compiles with no warning even under `-Wall -Wextra`. Write `int answer(void)`. C23
changed `()` to mean no parameters, as it always has in C++.

**`static` functions in C.** The C examples mark helper functions `static`, which
makes them visible only inside their own file. Chapter 17 covers what that means for
building programs out of several files.

**How arguments are passed.** C passes everything by value, so a function gets a copy,
and to change the caller's variable you pass a pointer to it. C++ adds references
(`int& x`) and Rust adds borrows (`&mut x`), which let a function change the caller's
value without pointer syntax. Go passes by value too, but a slice, map, or pointer is
a small value that refers to shared data. Python, JavaScript, Dart, and Kotlin pass a
reference to the same object: changing the object is visible to the caller, but
assigning a new value to the parameter is not. Kotlin goes one step further and makes
every parameter read-only, so assigning to one is a compile error (`'val' cannot be
reassigned`). Chapters 9 and 10 look at this in detail.

---

## 5.3 Parameters: defaults, names, and any number of arguments

The program below greets with an optional greeting and punctuation, and adds up any
number of integers. The code is in
[`examples/01-parameters/`](examples/01-parameters/).

**C**

```c
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

// C has no default or named arguments, so every caller passes every argument.
static void greet(const char *name, const char *greeting, const char *punctuation) {
    printf("%s, %s%s\n", greeting, name, punctuation);
}

// A variadic function cannot tell how many arguments it got, so the caller says.
// Nothing checks that they really are ints: passing a double here is undefined behavior.
static int total(int count, ...) {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += va_arg(args, int);
    }
    va_end(args);
    return sum;
}

// For an existing array, C code passes a pointer and a length instead.
static int total_of(const int *values, size_t count) {
    int sum = 0;
    for (size_t i = 0; i < count; i++) {
        sum += values[i];
    }
    return sum;
}

int main(void) {
    greet("Ada", "Hello", "!");
    greet("Grace", "Hi", "!");
    greet("Linus", "Hello", "?");

    printf("total() = %d\n", total(0));
    printf("total(1, 2, 3) = %d\n", total(3, 1, 2, 3));
    const int numbers[] = {4, 3, 2, 1};
    printf("total of [4, 3, 2, 1] = %d\n", total_of(numbers, 4));
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <string>
#include <vector>

// Default arguments fill in from the right. There are no named arguments,
// so to change punctuation you must also pass greeting.
std::string greet(const std::string& name, const std::string& greeting = "Hello",
                  const std::string& punctuation = "!") {
    return greeting + ", " + name + punctuation;
}

// A variadic template takes any number of arguments, each with its real type
// checked. The fold expression (0 + ... + values) adds them all.
template <typename... Ints>
int total(Ints... values) {
    return (0 + ... + values);
}

// A second function with the same name but a different parameter list: an overload.
int total(const std::vector<int>& values) {
    int sum = 0;
    for (int value : values) {
        sum += value;
    }
    return sum;
}

int main() {
    std::cout << greet("Ada") << '\n';
    std::cout << greet("Grace", "Hi") << '\n';
    std::cout << greet("Linus", "Hello", "?") << '\n';

    std::cout << "total() = " << total() << '\n';
    std::cout << "total(1, 2, 3) = " << total(1, 2, 3) << '\n';
    const std::vector<int> numbers{4, 3, 2, 1};
    std::cout << "total of [4, 3, 2, 1] = " << total(numbers) << '\n';
}
```

**Rust**

```rust
// Rust has no default arguments. Option<T> marks a parameter the caller may skip.
fn greet(name: &str, greeting: Option<&str>, punctuation: Option<&str>) -> String {
    let greeting = greeting.unwrap_or("Hello");
    let punctuation = punctuation.unwrap_or("!");
    format!("{greeting}, {name}{punctuation}")
}

// Rust has no variadic functions either (println! is a macro, not a function).
// A slice parameter accepts an array literal, a Vec, or part of either.
fn total(values: &[i32]) -> i32 {
    let mut sum = 0;
    for value in values {
        sum += value;
    }
    sum
}

fn main() {
    println!("{}", greet("Ada", None, None));
    println!("{}", greet("Grace", Some("Hi"), None));
    println!("{}", greet("Linus", None, Some("?")));

    println!("total() = {}", total(&[]));
    println!("total(1, 2, 3) = {}", total(&[1, 2, 3]));
    let numbers = vec![4, 3, 2, 1];
    println!("total of [4, 3, 2, 1] = {}", total(&numbers));
}
```

**Go**

```go
package main

import "fmt"

// Go has no default or named arguments, so every caller passes every argument.
// Parameters of the same type can share one type name.
func greet(name, greeting, punctuation string) string {
	return greeting + ", " + name + punctuation
}

// ...int collects any number of int arguments into a slice named values.
func total(values ...int) int {
	sum := 0
	for _, value := range values {
		sum += value
	}
	return sum
}

func main() {
	fmt.Println(greet("Ada", "Hello", "!"))
	fmt.Println(greet("Grace", "Hi", "!"))
	fmt.Println(greet("Linus", "Hello", "?"))

	fmt.Println("total() =", total())
	fmt.Println("total(1, 2, 3) =", total(1, 2, 3))
	numbers := []int{4, 3, 2, 1}
	fmt.Println("total of [4, 3, 2, 1] =", total(numbers...)) // ... spreads a slice
}
```

**Python**

```python
def greet(name: str, greeting: str = "Hello", punctuation: str = "!") -> str:
    return f"{greeting}, {name}{punctuation}"


# *values collects any number of positional arguments into a tuple.
def total(*values: int) -> int:
    return sum(values)


def main() -> None:
    print(greet("Ada"))
    print(greet("Grace", "Hi"))
    print(greet("Linus", punctuation="?"))  # a keyword argument skips greeting

    print("total() =", total())
    print("total(1, 2, 3) =", total(1, 2, 3))
    numbers = [4, 3, 2, 1]
    print("total of [4, 3, 2, 1] =", total(*numbers))  # * spreads a list


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
function greet(name, greeting = "Hello", punctuation = "!") {
  return `${greeting}, ${name}${punctuation}`;
}

// ...values (a rest parameter) collects any number of arguments into an array.
function total(...values) {
  let sum = 0;
  for (const value of values) {
    sum += value;
  }
  return sum;
}

console.log(greet("Ada"));
console.log(greet("Grace", "Hi"));
console.log(greet("Linus", undefined, "?")); // undefined means "use the default"

console.log(`total() = ${total()}`);
console.log(`total(1, 2, 3) = ${total(1, 2, 3)}`);
const numbers = [4, 3, 2, 1];
console.log(`total of [4, 3, 2, 1] = ${total(...numbers)}`); // ... spreads an array
```

**TypeScript**

```ts
function greet(name: string, greeting = "Hello", punctuation = "!"): string {
  return `${greeting}, ${name}${punctuation}`;
}

// ...values (a rest parameter) collects any number of arguments into an array.
function total(...values: number[]): number {
  let sum = 0;
  for (const value of values) {
    sum += value;
  }
  return sum;
}

console.log(greet("Ada"));
console.log(greet("Grace", "Hi"));
console.log(greet("Linus", undefined, "?")); // undefined means "use the default"

console.log(`total() = ${total()}`);
console.log(`total(1, 2, 3) = ${total(1, 2, 3)}`);
const numbers: number[] = [4, 3, 2, 1];
console.log(`total of [4, 3, 2, 1] = ${total(...numbers)}`); // ... spreads an array
export {};
```

**Dart**

```dart
// Parameters in {} are named, and optional unless marked required.
String greet(
  String name, {
  String greeting = 'Hello',
  String punctuation = '!',
}) => '$greeting, $name$punctuation';

// Dart has no variadic functions: take a List.
int total(List<int> values) {
  var sum = 0;
  for (final value in values) {
    sum += value;
  }
  return sum;
}

void main() {
  print(greet('Ada'));
  print(greet('Grace', greeting: 'Hi'));
  print(greet('Linus', punctuation: '?'));

  print('total() = ${total([])}');
  print('total(1, 2, 3) = ${total([1, 2, 3])}');
  final numbers = [4, 3, 2, 1];
  print('total of [4, 3, 2, 1] = ${total(numbers)}');
}
```

**Kotlin**

```kotlin
// Any parameter can have a default, and any argument can be passed by name.
fun greet(name: String, greeting: String = "Hello", punctuation: String = "!") =
    "$greeting, $name$punctuation"

// vararg collects any number of arguments into an array (an IntArray here).
fun total(vararg values: Int): Int {
    var sum = 0
    for (value in values) {
        sum += value
    }
    return sum
}

fun main() {
    println(greet("Ada"))
    println(greet("Grace", "Hi"))
    println(greet("Linus", punctuation = "?")) // a named argument skips greeting

    println("total() = ${total()}")
    println("total(1, 2, 3) = ${total(1, 2, 3)}")
    val numbers = intArrayOf(4, 3, 2, 1)
    println("total of [4, 3, 2, 1] = ${total(*numbers)}") // * spreads an array
}
```

Output, identical in all nine:

```text
Hello, Ada!
Hi, Grace!
Hello, Linus?
total() = 0
total(1, 2, 3) = 6
total of [4, 3, 2, 1] = 10
```

| Language | Default values | Named arguments | Any number of arguments | Spread a list into the arguments |
| --- | --- | --- | --- | --- |
| C | No | No | `...` with `<stdarg.h>`, with no type checks | No |
| C++ | Yes, for the last parameters only | No | Variadic templates, checked; or C's `...` | No (`std::apply` for a tuple) |
| Rust | No | No | No (macros like `println!` can) | No |
| Go | No | No | `values ...int`, as the last parameter, arrives as a slice | `total(numbers...)` |
| Python | Yes | **Yes**, unless marked positional-only | `*args` (a tuple) and `**kwargs` (a dict) | `f(*items)`, `f(**options)` |
| JavaScript | Yes, for any parameter | No | `...values`, a rest parameter | `f(...items)` |
| TypeScript | Yes, plus `x?: T` for optional | No | `...values: number[]` | `f(...items)` |
| Dart | Yes, if the default is a constant | **Yes**, parameters in `{}` | No | No |
| Kotlin | Yes, for any parameter | **Yes**, for any parameter | `vararg values: Int`, which arrives as an array | `total(*numbers)`, for arrays only |

**Skipping a default in the middle.** C++ fills defaults from the right, so to change
the punctuation you must also pass the greeting. JavaScript and TypeScript treat
`undefined` as "use the default", so `greet("Linus", undefined, "?")` works, but
`null` does not: `greet("Ada", null)` prints `null, Ada`. Python, Dart, and Kotlin name
the argument instead: `greet("Linus", punctuation="?")`.

**C's `...` trusts the caller.** A C variadic function cannot tell how many arguments
it got or what types they are. `printf` reads its format string to find out, and
`total` above takes the count as its first argument. Pass a `double` where it reads an
`int`, and the behavior is undefined. GCC checks `printf` format strings against the
arguments (`-Wformat`, part of `-Wall`), but it cannot check your own variadic
functions. C++'s variadic templates know every argument's type at compile time.

### When a default is evaluated

Python evaluates a default **once**, when the `def` runs, and every call shares that
value. With a list as the default, that is a famous bug:

```python
def add_item(item, items=[]):
    items.append(item)
    return items


print(add_item("a"))  # ['a']
print(add_item("b"))  # ['a', 'b']: the same list as the first call
```

The fix is `items=None`, then `if items is None: items = []` inside. JavaScript, C++,
and Kotlin evaluate the default on each call, so `function addItem(item, items = [])`
gets a new array every time. Dart avoids the question by requiring a constant default. A
non-constant default such as `[]` is a compile error, and a `const []` default is
unmodifiable, so adding to it throws `Unsupported operation: Cannot add to an
unmodifiable list`.

### Named arguments, and what to do without them

Python lets any argument be passed by name, and two markers control it. Parameters
before `/` can only be passed by position, and parameters after `*` only by name:

```python
def connect(host, /, port=5432, *, timeout=10.0, **options):
    print(host, port, timeout, options)


connect("db", 5433, timeout=2.0, ssl=True)  # db 5433 2.0 {'ssl': True}
connect("db", timeout=2.0)  # db 5432 2.0 {}
connect("db", 5433, 2.0)  # TypeError: takes from 1 to 2 positional arguments but 3 were given
```

Dart's parameters in braces are named, and optional unless marked `required`:
`void connect(String host, {int port = 5432, required double timeout})`. A Dart
function can have optional positional parameters in square brackets, or named
parameters in braces, but not both.

Kotlin has the simplest rules. Every parameter can have a default, any expression can
be a default, and every argument can be passed by name, with no markers. Kotlin's
standard library leans on this, as in
`items.joinToString(separator = ", ", prefix = "[", postfix = "]")`. The one exception
is calling Java code, whose parameter names Kotlin cannot rely on.

The other six languages pass a struct or an object with named fields instead. In
JavaScript and TypeScript, destructuring with defaults makes it read like named
arguments:

```js
function connect(host, { port = 5432, timeout = 10 } = {}) {
  console.log(`${host}:${port}, timeout ${timeout}`);
}

connect("db", { timeout: 2 }); // db:5432, timeout 2
connect("db"); // db:5432, timeout 10
```

Rust uses a struct that implements `Default`, and **struct update syntax** fills in
the fields you leave out:

```rust
struct Options {
    port: u16,
    timeout_secs: f64,
}

impl Default for Options {
    fn default() -> Self {
        Options {
            port: 5432,
            timeout_secs: 10.0,
        }
    }
}

fn connect(host: &str, opts: Options) {
    println!("{host}:{}, timeout {:.1}", opts.port, opts.timeout_secs);
}

fn main() {
    // ..Default::default() fills in every field not named here.
    connect(
        "db",
        Options {
            timeout_secs: 2.0,
            ..Default::default()
        },
    );
}
```

Go and C pass a struct and treat a zero field as "use the default", since that is what
a field you leave out becomes. In Go, `Connect("db", Options{Timeout: 2 * time.Second})`.
In C, a compound literal with designated initializers does the same:

```c
#include <stdio.h>

struct options {
    int port;        // 0 means "use the default"
    double timeout;  // 0 means "use the default"
};

static void connect_to(const char *host, struct options opts) {
    int port = opts.port != 0 ? opts.port : 5432;
    double timeout = opts.timeout != 0 ? opts.timeout : 10.0;
    printf("%s:%d, timeout %.1f\n", host, port, timeout);
}

int main(void) {
    connect_to("db", (struct options){.timeout = 2.0});  // fields not named are 0
    return 0;
}
```

The catch with zero-as-default is that you cannot ask for a real zero. When zero is a
valid choice, Rust uses `Option<u16>`, and Go uses a pointer or a separate flag.

---

## 5.4 Returning several values

The program below returns a minimum and maximum together, a quotient and remainder
together, and swaps two variables. The code is in
[`examples/02-multiple-returns/`](examples/02-multiple-returns/).

**C**

```c
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// C returns one value. To give back more, the caller passes pointers to fill in.
static void min_max(const int *values, size_t count, int *lowest, int *highest) {
    *lowest = values[0];
    *highest = values[0];
    for (size_t i = 1; i < count; i++) {
        if (values[i] < *lowest) {
            *lowest = values[i];
        }
        if (values[i] > *highest) {
            *highest = values[i];
        }
    }
}

static void swap(const char **a, const char **b) {
    const char *temp = *a;
    *a = *b;
    *b = temp;
}

int main(void) {
    const int values[] = {3, -1, 4, 1, 5};
    int lowest, highest;
    min_max(values, 5, &lowest, &highest);  // & passes the address of each variable
    printf("min and max of [3, -1, 4, 1, 5]: %d %d\n", lowest, highest);

    // The other way: return a struct. The standard library's div() does this.
    div_t result = div(17, 5);
    printf("17 divided by 5: %d remainder %d\n", result.quot, result.rem);

    const char *first = "hello";
    const char *second = "world";
    swap(&first, &second);
    printf("swapped: %s %s\n", first, second);
    return 0;
}
```

**C++**

```cpp
#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// Return several values as a std::pair (or std::tuple for more than two).
std::pair<int, int> min_max(const std::vector<int>& values) {
    int lowest = values[0];
    int highest = values[0];
    for (int value : values) {
        lowest = std::min(lowest, value);
        highest = std::max(highest, value);
    }
    return {lowest, highest};
}

std::pair<int, int> div_mod(int a, int b) {
    return {a / b, a % b};
}

int main() {
    // Structured bindings (C++17) unpack the pair into two named variables.
    auto [lowest, highest] = min_max({3, -1, 4, 1, 5});
    std::cout << "min and max of [3, -1, 4, 1, 5]: " << lowest << ' ' << highest << '\n';

    auto [quotient, remainder] = div_mod(17, 5);
    std::cout << "17 divided by 5: " << quotient << " remainder " << remainder << '\n';

    std::string first = "hello";
    std::string second = "world";
    std::swap(first, second);  // takes both by reference and swaps them in place
    std::cout << "swapped: " << first << ' ' << second << '\n';
}
```

**Rust**

```rust
// Return several values as a tuple.
fn min_max(values: &[i32]) -> (i32, i32) {
    let mut lowest = values[0];
    let mut highest = values[0];
    for &value in values {
        lowest = lowest.min(value);
        highest = highest.max(value);
    }
    (lowest, highest)
}

fn div_mod(a: i32, b: i32) -> (i32, i32) {
    (a / b, a % b)
}

fn main() {
    let (lowest, highest) = min_max(&[3, -1, 4, 1, 5]); // a pattern unpacks the tuple
    println!("min and max of [3, -1, 4, 1, 5]: {lowest} {highest}");

    let (quotient, remainder) = div_mod(17, 5);
    println!("17 divided by 5: {quotient} remainder {remainder}");

    let mut first = "hello";
    let mut second = "world";
    (first, second) = (second, first); // destructuring assignment swaps them
    println!("swapped: {first} {second}");
}
```

**Go**

```go
package main

import "fmt"

// Go functions can return several values. Naming them documents what they are.
func minMax(values []int) (lowest, highest int) {
	lowest, highest = values[0], values[0]
	for _, value := range values {
		lowest = min(lowest, value)
		highest = max(highest, value)
	}
	return lowest, highest
}

func divMod(a, b int) (int, int) {
	return a / b, a % b
}

func main() {
	lowest, highest := minMax([]int{3, -1, 4, 1, 5})
	fmt.Println("min and max of [3, -1, 4, 1, 5]:", lowest, highest)

	quotient, remainder := divMod(17, 5)
	fmt.Printf("17 divided by 5: %d remainder %d\n", quotient, remainder)

	first, second := "hello", "world"
	first, second = second, first // both right-hand values are read before assigning
	fmt.Println("swapped:", first, second)
}
```

**Python**

```python
def min_max(values: list[int]) -> tuple[int, int]:
    return min(values), max(values)  # the comma makes a tuple


def main() -> None:
    lowest, highest = min_max([3, -1, 4, 1, 5])  # unpacking
    print(f"min and max of [3, -1, 4, 1, 5]: {lowest} {highest}")

    quotient, remainder = divmod(17, 5)  # a built-in that returns a pair
    print(f"17 divided by 5: {quotient} remainder {remainder}")

    first, second = "hello", "world"
    first, second = second, first
    print(f"swapped: {first} {second}")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// JavaScript returns one value, so return an array (or an object) and destructure it.
function minMax(values) {
  let lowest = values[0];
  let highest = values[0];
  for (const value of values) {
    lowest = Math.min(lowest, value);
    highest = Math.max(highest, value);
  }
  return [lowest, highest];
}

function divMod(a, b) {
  return { quotient: Math.trunc(a / b), remainder: a % b };
}

const [lowest, highest] = minMax([3, -1, 4, 1, 5]);
console.log(`min and max of [3, -1, 4, 1, 5]: ${lowest} ${highest}`);

const { quotient, remainder } = divMod(17, 5); // object destructuring, by name
console.log(`17 divided by 5: ${quotient} remainder ${remainder}`);

let first = "hello";
let second = "world";
[first, second] = [second, first];
console.log(`swapped: ${first} ${second}`);
```

**TypeScript**

```ts
// A tuple type says exactly how many values come back, and their types.
function minMax(values: number[]): [number, number] {
  let lowest = values[0];
  let highest = values[0];
  for (const value of values) {
    lowest = Math.min(lowest, value);
    highest = Math.max(highest, value);
  }
  return [lowest, highest];
}

function divMod(a: number, b: number): { quotient: number; remainder: number } {
  return { quotient: Math.trunc(a / b), remainder: a % b };
}

const [lowest, highest] = minMax([3, -1, 4, 1, 5]);
console.log(`min and max of [3, -1, 4, 1, 5]: ${lowest} ${highest}`);

const { quotient, remainder } = divMod(17, 5); // object destructuring, by name
console.log(`17 divided by 5: ${quotient} remainder ${remainder}`);

let first = "hello";
let second = "world";
[first, second] = [second, first];
console.log(`swapped: ${first} ${second}`);
export {};
```

**Dart**

```dart
// A record (Dart 3) bundles several values without declaring a class.
(int, int) minMax(List<int> values) {
  var lowest = values[0];
  var highest = values[0];
  for (final value in values) {
    if (value < lowest) lowest = value;
    if (value > highest) highest = value;
  }
  return (lowest, highest);
}

// Record fields can have names too.
({int quotient, int remainder}) divMod(int a, int b) =>
    (quotient: a ~/ b, remainder: a.remainder(b));

void main() {
  final (lowest, highest) = minMax([3, -1, 4, 1, 5]);
  print('min and max of [3, -1, 4, 1, 5]: $lowest $highest');

  final (:quotient, :remainder) = divMod(17, 5); // :name unpacks a named field
  print('17 divided by 5: $quotient remainder $remainder');

  var first = 'hello';
  var second = 'world';
  (first, second) = (second, first);
  print('swapped: $first $second');
}
```

**Kotlin**

```kotlin
// Pair holds two values without declaring a class.
fun minMax(values: List<Int>): Pair<Int, Int> {
    var lowest = values[0]
    var highest = values[0]
    for (value in values) {
        if (value < lowest) lowest = value
        if (value > highest) highest = value
    }
    return Pair(lowest, highest)
}

// A data class names the values, and still unpacks like a Pair.
data class DivMod(val quotient: Int, val remainder: Int)

fun divMod(a: Int, b: Int) = DivMod(a / b, a % b)

fun main() {
    val (lowest, highest) = minMax(listOf(3, -1, 4, 1, 5))
    println("min and max of [3, -1, 4, 1, 5]: $lowest $highest")

    val (quotient, remainder) = divMod(17, 5) // unpacks by position, not by name
    println("17 divided by 5: $quotient remainder $remainder")

    var first = "hello"
    var second = "world"
    first = second.also { second = first } // also returns second's old value
    println("swapped: $first $second")
}
```

Output, identical in all nine:

```text
min and max of [3, -1, 4, 1, 5]: -1 5
17 divided by 5: 3 remainder 2
swapped: world hello
```

| Language | Return several values as | Unpack with | Swap two variables |
| --- | --- | --- | --- |
| C | Pointer parameters to fill in, or a struct | Field access | A function taking two pointers |
| C++ | `std::pair`, `std::tuple`, or a struct | `auto [a, b] = f();` (C++17) | `std::swap(a, b)` |
| Rust | A tuple `(i32, i32)` or a struct | `let (a, b) = f();` | `(a, b) = (b, a);` or `std::mem::swap` |
| Go | Several results: `(int, int)` | `a, b := f()` | `a, b = b, a` |
| Python | A tuple (the comma makes it) | `a, b = f()` | `a, b = b, a` |
| JavaScript | An array or an object | `const [a, b] = f();` or `const { a, b } = f();` | `[a, b] = [b, a];` |
| TypeScript | A tuple type `[number, number]` or an object type | Same as JavaScript | Same as JavaScript |
| Dart | A record `(int, int)` or `({int quotient, int remainder})` (Dart 3) | `final (a, b) = f();` or `final (:quotient, :remainder) = f();` | `(a, b) = (b, a);` |
| Kotlin | `Pair`, `Triple`, or a `data class` | `val (a, b) = f()`, by position | `a = b.also { b = a }` |

**Go's results are not a tuple.** A Go function can return several values, but there
is no tuple type to hold them. `pair := minMax(values)` is a compile error
(`assignment mismatch: 1 variable but minMax returns 2 values`). The feature exists
for one pattern above all: `value, err := strconv.Atoi(text)`. Chapter 11 is built
around it. Go can also name its results, as `minMax` does, which documents them and
lets a bare `return` return their current values. Go's code review guidelines suggest a bare
`return` only in short functions, because in a long one the reader must hunt for the
values.

**Names or positions.** A pair says nothing about which value is which. When the
values have distinct meanings, name them: an object in JavaScript, named record fields
in Dart, or a small struct in C, C++, Rust, and Go. The C standard library does this
with `div()`, which returns a `div_t` struct with fields `quot` and `rem`.

Kotlin's `data class` names its fields, but unpacking still goes by position. Write
`val (remainder, quotient) = divMod(17, 5)` and you get the quotient in `remainder`,
with no warning. Read the fields by name, `result.quotient`, when the order is easy to
get wrong.

**Swapping without a temporary.** Python, Go, JavaScript, Dart, and Rust (since 1.59)
can assign to several variables at once, and they evaluate the whole right side
before assigning anything, so `a, b = b, a` swaps. C++ has `std::swap`. C needs a
helper that takes pointers. Kotlin cannot assign to two variables at once. Its idiom,
`a = b.also { b = a }`, works because `also` runs the block and then hands back `b`'s
old value.

---

## 5.5 Functions as values

The program below passes functions to `apply_twice`, and builds a new function with
`compose`. The code is in [`examples/03-first-class/`](examples/03-first-class/).

**C**

```c
#include <stdio.h>

// A function pointer type: "pointer to a function taking an int and returning an int".
typedef int (*IntFn)(int);

static int add_one(int x) {
    return x + 1;
}

static int triple(int x) {
    return x * 3;
}

static int square(int x) {  // C has no anonymous functions, so this needs a name too
    return x * x;
}

static int apply_twice(IntFn f, int x) {
    return f(f(x));
}

// C cannot build a new function at run time. To compose, store both pointers
// in a struct and call through it.
struct Composed {
    IntFn outer;
    IntFn inner;
};

static int call_composed(struct Composed c, int x) {
    return c.outer(c.inner(x));
}

int main(void) {
    printf("apply_twice(add_one, 5) = %d\n", apply_twice(add_one, 5));
    printf("apply_twice(triple, 5) = %d\n", apply_twice(triple, 5));
    printf("apply_twice(square, 3) = %d\n", apply_twice(square, 3));
    struct Composed add_then_triple = {triple, add_one};
    printf("compose(triple, add_one)(5) = %d\n", call_composed(add_then_triple, 5));
    return 0;
}
```

**C++**

```cpp
#include <functional>
#include <iostream>

int add_one(int x) {
    return x + 1;
}

int triple(int x) {
    return x * 3;
}

// std::function holds anything callable: a function, a lambda, or an object.
int apply_twice(const std::function<int(int)>& f, int x) {
    return f(f(x));
}

// auto parameters (C++20) make this a template, so any two callables work.
auto compose(auto outer, auto inner) {
    return [outer, inner](int x) { return outer(inner(x)); };
}

int main() {
    std::cout << "apply_twice(add_one, 5) = " << apply_twice(add_one, 5) << '\n';
    std::cout << "apply_twice(triple, 5) = " << apply_twice(triple, 5) << '\n';
    auto square = [](int x) { return x * x; };  // a lambda: a function with no name
    std::cout << "apply_twice(square, 3) = " << apply_twice(square, 3) << '\n';
    auto add_then_triple = compose(triple, add_one);
    std::cout << "compose(triple, add_one)(5) = " << add_then_triple(5) << '\n';
}
```

**Rust**

```rust
fn add_one(x: i32) -> i32 {
    x + 1
}

fn triple(x: i32) -> i32 {
    x * 3
}

// impl Fn(i32) -> i32 accepts a named function or a closure.
fn apply_twice(f: impl Fn(i32) -> i32, x: i32) -> i32 {
    f(f(x))
}

// Returning a closure: `move` copies outer and inner into it, so it can outlive this call.
fn compose(outer: impl Fn(i32) -> i32, inner: impl Fn(i32) -> i32) -> impl Fn(i32) -> i32 {
    move |x| outer(inner(x))
}

fn main() {
    println!("apply_twice(add_one, 5) = {}", apply_twice(add_one, 5));
    println!("apply_twice(triple, 5) = {}", apply_twice(triple, 5));
    let square = |x: i32| x * x; // a closure: a function with no name
    println!("apply_twice(square, 3) = {}", apply_twice(square, 3));
    let add_then_triple = compose(triple, add_one);
    println!("compose(triple, add_one)(5) = {}", add_then_triple(5));
}
```

**Go**

```go
package main

import "fmt"

func addOne(x int) int {
	return x + 1
}

func triple(x int) int {
	return x * 3
}

// func(int) int is the type of any function from int to int.
func applyTwice(f func(int) int, x int) int {
	return f(f(x))
}

func compose(outer, inner func(int) int) func(int) int {
	return func(x int) int {
		return outer(inner(x))
	}
}

func main() {
	fmt.Println("apply_twice(add_one, 5) =", applyTwice(addOne, 5))
	fmt.Println("apply_twice(triple, 5) =", applyTwice(triple, 5))
	square := func(x int) int { return x * x } // a function literal, with no name
	fmt.Println("apply_twice(square, 3) =", applyTwice(square, 3))
	addThenTriple := compose(triple, addOne)
	fmt.Println("compose(triple, add_one)(5) =", addThenTriple(5))
}
```

**Python**

```python
from collections.abc import Callable


def add_one(x: int) -> int:
    return x + 1


def triple(x: int) -> int:
    return x * 3


def apply_twice(f: Callable[[int], int], x: int) -> int:
    return f(f(x))


def compose(outer: Callable[[int], int], inner: Callable[[int], int]) -> Callable[[int], int]:
    def composed(x: int) -> int:
        return outer(inner(x))

    return composed


def main() -> None:
    print("apply_twice(add_one, 5) =", apply_twice(add_one, 5))
    print("apply_twice(triple, 5) =", apply_twice(triple, 5))
    square = lambda x: x * x  # noqa: E731 (a lambda: one expression, no name)
    print("apply_twice(square, 3) =", apply_twice(square, 3))
    add_then_triple = compose(triple, add_one)
    print("compose(triple, add_one)(5) =", add_then_triple(5))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const addOne = (x) => x + 1; // an arrow function stored in a variable
function triple(x) {
  return x * 3;
}

function applyTwice(f, x) {
  return f(f(x));
}

const compose = (outer, inner) => (x) => outer(inner(x));

console.log(`apply_twice(add_one, 5) = ${applyTwice(addOne, 5)}`);
console.log(`apply_twice(triple, 5) = ${applyTwice(triple, 5)}`);
console.log(`apply_twice(square, 3) = ${applyTwice((x) => x * x, 3)}`);
const addThenTriple = compose(triple, addOne);
console.log(`compose(triple, add_one)(5) = ${addThenTriple(5)}`);
```

**TypeScript**

```ts
type IntFn = (x: number) => number; // a function type

const addOne: IntFn = (x) => x + 1;
function triple(x: number): number {
  return x * 3;
}

function applyTwice(f: IntFn, x: number): number {
  return f(f(x));
}

const compose =
  (outer: IntFn, inner: IntFn): IntFn =>
  (x) =>
    outer(inner(x));

console.log(`apply_twice(add_one, 5) = ${applyTwice(addOne, 5)}`);
console.log(`apply_twice(triple, 5) = ${applyTwice(triple, 5)}`);
console.log(`apply_twice(square, 3) = ${applyTwice((x) => x * x, 3)}`);
const addThenTriple = compose(triple, addOne);
console.log(`compose(triple, add_one)(5) = ${addThenTriple(5)}`);
export {};
```

**Dart**

```dart
int addOne(int x) => x + 1;
int triple(int x) => x * 3;

// int Function(int) is the type of any function from int to int.
int applyTwice(int Function(int) f, int x) => f(f(x));

int Function(int) compose(int Function(int) outer, int Function(int) inner) =>
    (x) => outer(inner(x));

void main() {
  print('apply_twice(add_one, 5) = ${applyTwice(addOne, 5)}');
  print('apply_twice(triple, 5) = ${applyTwice(triple, 5)}');
  print('apply_twice(square, 3) = ${applyTwice((x) => x * x, 3)}');
  final addThenTriple = compose(triple, addOne);
  print('compose(triple, add_one)(5) = ${addThenTriple(5)}');
}
```

**Kotlin**

```kotlin
fun addOne(x: Int) = x + 1

fun triple(x: Int) = x * 3

// (Int) -> Int is the type of any function from Int to Int.
fun applyTwice(f: (Int) -> Int, x: Int) = f(f(x))

fun compose(outer: (Int) -> Int, inner: (Int) -> Int): (Int) -> Int = { x -> outer(inner(x)) }

fun main() {
    // ::addOne refers to the function without calling it.
    println("apply_twice(add_one, 5) = ${applyTwice(::addOne, 5)}")
    println("apply_twice(triple, 5) = ${applyTwice(::triple, 5)}")
    // A lambda with one parameter can call it `it`.
    println("apply_twice(square, 3) = ${applyTwice({ it * it }, 3)}")
    val addThenTriple = compose(::triple, ::addOne)
    println("compose(triple, add_one)(5) = ${addThenTriple(5)}")
}
```

Output, identical in all nine:

```text
apply_twice(add_one, 5) = 7
apply_twice(triple, 5) = 45
apply_twice(square, 3) = 81
compose(triple, add_one)(5) = 18
```

| Language | Type of "function from int to int" | A function with no name |
| --- | --- | --- |
| C | `int (*)(int)`, a function pointer | None |
| C++ | `std::function<int(int)>`, or a template parameter | `[](int x) { return x * x; }` |
| Rust | `fn(i32) -> i32`, `impl Fn(i32) -> i32`, or `Box<dyn Fn(i32) -> i32>` | <code>&#124;x&#124; x * x</code> |
| Go | `func(int) int` | `func(x int) int { return x * x }` |
| Python | `Callable[[int], int]` (a hint only) | `lambda x: x * x` |
| JavaScript | No types | `(x) => x * x` |
| TypeScript | `(x: number) => number` | `(x) => x * x` |
| Dart | `int Function(int)` | `(x) => x * x` |
| Kotlin | `(Int) -> Int` | `{ x -> x * x }`, or `{ it * it }` for one parameter |

A function with no name is called a **lambda** or an **anonymous function**. In most of
these languages it can do anything a named function can. Python's `lambda` is limited
to one expression, because Python statements are built from indented lines and cannot
fit inside an expression. Assigning a lambda to a name, as the Python example does,
is legal but discouraged: linters flag it as E731 (`Do not assign a lambda
expression, use a def`), and the example silences that with `# noqa`. A `def` gives
the function a real name in error messages.

Kotlin has two habits worth knowing. `::addOne` refers to a named function without
calling it. And when a lambda is the last argument, it can go after the parentheses, so
`items.map({ it * 2 })` is written `items.map { it * 2 }`. That is why Kotlin code that
takes a block, such as `repeat(3) { ... }` or `buildString { ... }`, reads like
built-in syntax.

**C cannot make functions at run time.** A C function pointer can point only at a
function written in the source code. `compose` would need a new function that
remembers `outer` and `inner`, so the C example stores the two pointers in a struct
and calls through it. GCC's nested functions and Clang's blocks (`^(int x) { ... }`)
are extensions that go further, but they are not standard C.

**Three ways to take a function in Rust and C++.** Rust's `fn(i32) -> i32` is a plain
function pointer, which also accepts a closure that captures nothing. `impl Fn(i32) ->
i32` accepts any function or closure, and the compiler makes a separate copy of
`apply_twice` for each one, so the call can be inlined. `Box<dyn Fn(i32) -> i32>`
accepts any of them through one shared copy, at the cost of an indirect call. C++
makes the same trade between a template parameter (or `auto`, as `compose` uses) and
`std::function`. Chapter 13 explains this static and dynamic dispatch in full.

**Comparing functions.** Go allows comparing a function only to `nil`; `f == g` is a
compile error (`func can only be compared to nil`). Rust closures and C++ lambdas
cannot be compared at all. Python and JavaScript compare function objects by
identity, so two separately written `(x) => x` are never equal. Kotlin does the same
for lambdas, but two references to the same function are equal: `::addOne == ::addOne`
is `true`.

---

## 5.6 Closures

A **closure** is a function together with the variables it uses from the place where
it was made. `make_adder(5)` returns a function that adds 5: the function's code is
`x + n`, and the closure remembers that `n` is 5, even though `make_adder` has
returned. The program below makes adders, an accumulator that changes what it
captured, and three closures in a loop. The code is in
[`examples/04-closures/`](examples/04-closures/).

**C**

```c
#include <stdio.h>

// C has no closures. A closure is a function plus the variables it captured,
// so in C you keep those variables in a struct and pass it in yourself.
struct Adder {
    int n;
};

static int add(struct Adder adder, int x) {
    return x + adder.n;
}

struct Accumulator {
    int total;
};

static int accumulate(struct Accumulator *acc, int x) {
    acc->total += x;  // a pointer, so the change is kept between calls
    return acc->total;
}

int main(void) {
    struct Adder add5 = {5};
    struct Adder add10 = {10};
    printf("add5(10) = %d\n", add(add5, 10));
    printf("add10(10) = %d\n", add(add10, 10));

    struct Accumulator running = {0};
    // One call per statement: C does not fix the order it evaluates arguments in,
    // so printf("%d %d", accumulate(...), accumulate(...)) could run either first.
    printf("running total:");
    printf(" %d", accumulate(&running, 3));
    printf(" %d", accumulate(&running, 5));
    printf(" %d\n", accumulate(&running, 2));

    struct Adder made[3];
    for (int i = 0; i < 3; i++) {
        made[i].n = i;  // each struct stores its own copy of i
    }
    printf("made in a loop:");
    for (int i = 0; i < 3; i++) {
        printf(" %d", add(made[i], 0));
    }
    printf("\n");
    return 0;
}
```

**C++**

```cpp
#include <functional>
#include <iostream>
#include <vector>

// [n] captures n by value: the lambda keeps its own copy.
auto make_adder(int n) {
    return [n](int x) { return x + n; };
}

// An init-capture creates a variable inside the lambda. mutable lets calls change it.
auto make_accumulator() {
    return [total = 0](int x) mutable {
        total += x;
        return total;
    };
}

int main() {
    auto add5 = make_adder(5);
    auto add10 = make_adder(10);
    std::cout << "add5(10) = " << add5(10) << '\n';
    std::cout << "add10(10) = " << add10(10) << '\n';

    auto running = make_accumulator();
    const int a = running(3);
    const int b = running(5);
    const int c = running(2);
    std::cout << "running total: " << a << ' ' << b << ' ' << c << '\n';

    std::vector<std::function<int()>> made;
    for (int i = 0; i < 3; ++i) {
        made.push_back([i] { return i; });  // [&i] would dangle once the loop ends
    }
    std::cout << "made in a loop:";
    for (const auto& f : made) {
        std::cout << ' ' << f();
    }
    std::cout << '\n';
}
```

**Rust**

```rust
// `move` makes the closure own its captured variables, so it can be returned.
fn make_adder(n: i32) -> impl Fn(i32) -> i32 {
    move |x| x + n
}

// A closure that changes what it captured is FnMut, and must be called through `mut`.
fn make_accumulator() -> impl FnMut(i32) -> i32 {
    let mut total = 0;
    move |x| {
        total += x;
        total
    }
}

fn main() {
    let add5 = make_adder(5);
    let add10 = make_adder(10);
    println!("add5(10) = {}", add5(10));
    println!("add10(10) = {}", add10(10));

    let mut running = make_accumulator();
    let (a, b, c) = (running(3), running(5), running(2));
    println!("running total: {a} {b} {c}");

    // Each closure has its own unnamed type, so a Vec of them stores Box<dyn Fn>.
    let mut made: Vec<Box<dyn Fn() -> i32>> = Vec::new();
    for i in 0..3 {
        made.push(Box::new(move || i));
    }
    let values: Vec<String> = made.iter().map(|f| f().to_string()).collect();
    println!("made in a loop: {}", values.join(" "));
}
```

**Go**

```go
package main

import "fmt"

func makeAdder(n int) func(int) int {
	return func(x int) int { return x + n }
}

// Go closures capture variables, not values: total lives on after makeAccumulator returns.
func makeAccumulator() func(int) int {
	total := 0
	return func(x int) int {
		total += x
		return total
	}
}

func main() {
	add5 := makeAdder(5)
	add10 := makeAdder(10)
	fmt.Println("add5(10) =", add5(10))
	fmt.Println("add10(10) =", add10(10))

	running := makeAccumulator()
	fmt.Println("running total:", running(3), running(5), running(2))

	var made []func() int
	for i := 0; i < 3; i++ {
		// Since Go 1.22 each pass gets a fresh i. Before, all three shared one i
		// and this printed 3 3 3.
		made = append(made, func() int { return i })
	}
	fmt.Print("made in a loop:")
	for _, f := range made {
		fmt.Print(" ", f())
	}
	fmt.Println()
}
```

**Python**

```python
from collections.abc import Callable


def make_adder(n: int) -> Callable[[int], int]:
    def add(x: int) -> int:
        return x + n

    return add


def make_accumulator() -> Callable[[int], int]:
    total = 0

    def accumulate(x: int) -> int:
        nonlocal total  # without this, total += x would create a new local variable
        total += x
        return total

    return accumulate


def main() -> None:
    add5 = make_adder(5)
    add10 = make_adder(10)
    print("add5(10) =", add5(10))
    print("add10(10) =", add10(10))

    running = make_accumulator()
    print("running total:", running(3), running(5), running(2))

    made = []
    for i in range(3):
        made.append(lambda: i)  # captures the variable i, not its value now
    print("made in a loop:", *(f() for f in made))

    fixed = []
    for i in range(3):
        fixed.append(lambda i=i: i)  # a default value is evaluated now, once
    print("made in a loop, with i=i:", *(f() for f in fixed))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const makeAdder = (n) => (x) => x + n;

function makeAccumulator() {
  let total = 0;
  return (x) => {
    total += x;
    return total;
  };
}

const add5 = makeAdder(5);
const add10 = makeAdder(10);
console.log(`add5(10) = ${add5(10)}`);
console.log(`add10(10) = ${add10(10)}`);

const running = makeAccumulator();
console.log(`running total: ${running(3)} ${running(5)} ${running(2)}`);

const madeWithLet = [];
for (let i = 0; i < 3; i++) {
  madeWithLet.push(() => i); // let: each pass gets a fresh i
}
console.log(`made in a loop with let: ${madeWithLet.map((f) => f()).join(" ")}`);

const madeWithVar = [];
for (var j = 0; j < 3; j++) {
  madeWithVar.push(() => j); // var: one j shared by every pass, 3 by the time we call
}
console.log(`made in a loop with var: ${madeWithVar.map((f) => f()).join(" ")}`);
```

**TypeScript**

```ts
const makeAdder =
  (n: number) =>
  (x: number): number =>
    x + n;

function makeAccumulator(): (x: number) => number {
  let total = 0;
  return (x) => {
    total += x;
    return total;
  };
}

const add5 = makeAdder(5);
const add10 = makeAdder(10);
console.log(`add5(10) = ${add5(10)}`);
console.log(`add10(10) = ${add10(10)}`);

const running = makeAccumulator();
console.log(`running total: ${running(3)} ${running(5)} ${running(2)}`);

const madeWithLet: (() => number)[] = [];
for (let i = 0; i < 3; i++) {
  madeWithLet.push(() => i); // let: each pass gets a fresh i
}
console.log(`made in a loop with let: ${madeWithLet.map((f) => f()).join(" ")}`);

const madeWithVar: (() => number)[] = [];
for (var j = 0; j < 3; j++) {
  madeWithVar.push(() => j); // var: one j shared by every pass, 3 by the time we call
}
console.log(`made in a loop with var: ${madeWithVar.map((f) => f()).join(" ")}`);
export {};
```

**Dart**

```dart
int Function(int) makeAdder(int n) =>
    (x) => x + n;

int Function(int) makeAccumulator() {
  var total = 0;
  return (x) {
    total += x;
    return total;
  };
}

void main() {
  final add5 = makeAdder(5);
  final add10 = makeAdder(10);
  print('add5(10) = ${add5(10)}');
  print('add10(10) = ${add10(10)}');

  final running = makeAccumulator();
  print('running total: ${running(3)} ${running(5)} ${running(2)}');

  final made = <int Function()>[];
  for (var i = 0; i < 3; i++) {
    made.add(() => i); // a for loop gives each pass a fresh i
  }
  print('made in a loop: ${made.map((f) => f()).join(' ')}');
}
```

**Kotlin**

```kotlin
fun makeAdder(n: Int): (Int) -> Int = { x -> x + n }

fun makeAccumulator(): (Int) -> Int {
    var total = 0
    return { x ->
        total += x // a lambda can change a var it captured
        total // the last line of a lambda is its value
    }
}

fun main() {
    val add5 = makeAdder(5)
    val add10 = makeAdder(10)
    println("add5(10) = ${add5(10)}")
    println("add10(10) = ${add10(10)}")

    val running = makeAccumulator()
    println("running total: ${running(3)} ${running(5)} ${running(2)}")

    val made = mutableListOf<() -> Int>()
    for (i in 0..<3) {
        made.add { i } // each pass of a for loop has its own i
    }
    println("made in a loop: ${made.map { it() }.joinToString(" ")}")
}
```

Output:

*C, C++, Rust, Go, Dart and Kotlin:*

```text
add5(10) = 15
add10(10) = 20
running total: 3 8 10
made in a loop: 0 1 2
```

*Python:*

```text
add5(10) = 15
add10(10) = 20
running total: 3 8 10
made in a loop: 2 2 2
made in a loop, with i=i: 0 1 2
```

*JavaScript and TypeScript:*

```text
add5(10) = 15
add10(10) = 20
running total: 3 8 10
made in a loop with let: 0 1 2
made in a loop with var: 3 3 3
```

### How a closure captures

| Language | Captures | Can change what it captured | Keeps captured variables alive |
| --- | --- | --- | --- |
| C | No closures; pass a struct or a `void *` | Through a pointer | You do |
| C++ | What you list: `[n]` copies, `[&n]` refers, `[=]` copies everything used, `[&]` refers to everything used, `[total = 0]` makes a new variable | Copies only if the lambda is `mutable`; references always | Copies yes, references **no** |
| Rust | Inferred: by shared borrow, mutable borrow, or by value, whichever the body needs. `move` takes everything by value | Yes; the closure is then `FnMut` | Yes, and the borrow checker rejects any capture that would dangle |
| Go | The variable itself | Yes | Yes (the garbage collector) |
| Python | The variable itself, read when the closure runs | Only after `nonlocal name` | Yes |
| JavaScript, TypeScript | The variable itself | Yes, unless it is `const` | Yes |
| Dart | The variable itself | Yes, unless it is `final` | Yes |
| Kotlin | The variable itself | Yes, if it is a `var` | Yes |

**A closure is an object in disguise.** A C++ lambda is shorthand for a class the
compiler writes for you: the captures become fields, and the body becomes the
`operator()` that runs when you call it. That is why C++ asks how to capture each
variable, since you are choosing the types of the fields. A Rust closure is the same
kind of struct. The C example writes that struct by hand: `struct Adder` holds the
captured `n`, and `struct Accumulator` holds `total`. Go, Python, JavaScript,
TypeScript, Dart, and Kotlin capture the variable itself and let the garbage collector
keep it alive as long as any closure refers to it. Kotlin shows how that can work. A
lambda on the JVM receives copies of the values it captures, so when a Kotlin lambda
captures a `var`, the compiler moves that variable into a small object on the heap (a
`Ref.IntRef` for an `Int`), and the function and the lambda share that object.

### Changing captured state

The accumulator changes `total` on every call, and each language makes you say so in
a different place.

**Python** decides that a name is local to a function if the function assigns to it
anywhere. Without `nonlocal total`, the line `total += x` makes `total` a new local,
and reading it fails: `UnboundLocalError: cannot access local variable 'total' where it
is not associated with a value`.

**C++** lambdas cannot change their copies unless marked `mutable`. Leave it out, and
`total += x` fails with `assignment of read-only variable 'total'`.

**Rust** sorts closures into three traits by what they do with their captures:

| Trait | The closure | Can be called |
| --- | --- | --- |
| `Fn` | Only reads its captures | Any number of times, even from several places at once |
| `FnMut` | Changes its captures | Any number of times, through a `mut` binding |
| `FnOnce` | Moves a captured value out, for example returning it | Once |

The accumulator is `FnMut`, so `let running = make_accumulator();` without `mut` fails
with `cannot borrow running as mutable, as it is not declared as mutable`.

**Go, JavaScript, TypeScript, Dart, and Kotlin** let any closure assign to the
variables it captured, with no marker at all. Kotlin's neighbor Java does not: a Java
lambda can only capture variables that are never reassigned.

### Closures made in a loop

The last part of the example makes three closures in a loop, each returning the loop
variable, and calls them after the loop ends. Whether you get `0 1 2` depends on
whether each pass of the loop has its own variable.

- **Python** has one `i` for the whole loop, and a closure reads it when it is
  called, by which time `i` is 2. The usual fix is `lambda i=i: i`, because a default
  is evaluated once, when the `lambda` is made.
- **JavaScript and TypeScript** give `let` in a `for` loop a fresh binding on each
  pass, so the closures see 0, 1, and 2. With `var`, there is one variable, and it is 3
  by the time the closures run.
- **Go** changed this in Go 1.22: each pass now gets its own variable. The change
  depends on the `go` line in a module's `go.mod`. With `go 1.21` or older, the same
  program prints `3 3 3`. This book builds single files outside any module, which uses
  the new rule.
- **Dart** gives each pass of a `for` loop a fresh variable, and always has.
- **Kotlin**'s `for (i in 0..<3)` declares a new read-only `i` for each pass, so the
  closures see 0, 1, and 2. A `var i` declared before a `while` loop is one variable,
  and closures made in that loop all print 3.
- **C++** captures by copy with `[i]`, so each closure has its own value. Writing
  `[&i]` would refer to the loop variable after it no longer exists.
- **Rust** copies `i` into each closure with `move`. Without `move`, the closures would
  borrow `i`, which ends with each pass, and the compiler rejects storing them in a
  list that outlives the loop (`` `i` does not live long enough ``).

### When the captured variables are gone

A closure that outlives the function that made it must not refer to that function's
locals. Garbage-collected languages handle this for you. Rust refuses to compile it.
Without `move`, `make_adder` fails:

```text
error[E0373]: closure may outlive the current function, but it borrows `n`, which is owned by the current function
```

C++ compiles it without a word. This `make_adder` captures `n` by reference, and `n`
is gone once the function returns:

```cpp
std::function<int(int)> make_adder(int n) {
    return [&](int x) { return x + n; };  // n is destroyed when make_adder returns
}
```

GCC 13 accepts that under `-Wall -Wextra -Werror`. Calling the result is undefined
behavior: it may print the right answer, print garbage, or crash. Capture by value
(`[n]` or `[=]`) whenever a lambda will be stored or returned.

---

## 5.7 Recursion and memoization

A **recursive** function calls itself. Every call adds a frame to the stack, so a
deep enough recursion runs out of stack:

| Language | When the stack runs out |
| --- | --- |
| C, C++ | A crash, usually `Segmentation fault`, at a depth that depends on the frame size and the stack limit (often 8 MB on Linux) |
| Rust | A crash with a message that the thread `has overflowed its stack` |
| Go | Stacks grow as needed, up to 1 GB on 64-bit systems, then a fatal `stack overflow` error |
| Python | `RecursionError` once the depth passes 1000 (change it with `sys.setrecursionlimit`) |
| JavaScript, TypeScript | `RangeError: Maximum call stack size exceeded` |
| Dart | A `StackOverflowError` |
| Kotlin | `java.lang.StackOverflowError`, at a depth set by the JVM's thread stack size (`-Xss`) |

**Tail calls are guaranteed in only one language.** A call is a **tail call** if it is
the last thing a function does, as in `return count(n - 1)`. A compiler can reuse the
frame for it, so the recursion takes no extra stack. Kotlin promises this for a
function marked `tailrec` that calls itself, by compiling the recursion into a loop:

```kotlin
tailrec fun count(n: Long): Long = if (n == 0L) 0 else count(n - 1)
```

`count(1_000_000)` returns 0, and the same function without `tailrec` throws
`StackOverflowError`. If the call is not in tail position, as in `1 + count(n - 1)`,
the compiler warns: `a function is marked as tail-recursive but no tail calls are
found`. `tailrec` does not help two functions that call each other.

None of the other eight languages promises tail calls. C and C++ compilers often do
it with optimization on, and Clang and GCC 15 can be required to with a `musttail`
attribute. Rust has an explicit `become` keyword in development, but it is rejected on
stable Rust (`` `become` expression is experimental ``). JavaScript's 2015 standard
requires tail calls in strict mode, but only Safari implements them, so Node and Chrome
do not. Python's creator rejected the idea, because it would make tracebacks lose
frames. When a recursion can go deep, write it as a loop, or in Kotlin, a `tailrec`
function.

**Memoization** means storing the result of each call and returning the stored result
when the same arguments come again. It turns the naive Fibonacci function, which
makes over 300 million calls for `fib(40)`, into one that does 39 additions.
Exercise 1 asks you to write it. The memo has to outlive each call, and the languages
keep it in different places:

| Language | Where the memo can live |
| --- | --- |
| C | A `static` array at file level |
| C++ | A `static` local variable, created on the first call |
| Rust | A `HashMap` passed in as `&mut` (a mutable global needs a lock, `thread_local!`, or `unsafe`) |
| Go | A map captured by a closure |
| Python | Nowhere you can see: `@functools.cache` keeps it |
| JavaScript, TypeScript, Dart | A `Map` at module level |
| Kotlin | A top-level `val memo = mutableMapOf<Int, Long>()`, filled with `getOrPut` |

**Recursive closures.** A closure has no name of its own to call. Go declares the
variable first, then assigns a function literal that refers to it:
`var fib func(int) uint64`, then `fib = func(n int) uint64 { ... fib(n-1) ... }`. A
Rust closure cannot call itself directly, which is one reason Rust code uses a named
`fn` instead. C++23 lets a lambda take itself as its first parameter
(`[](this auto self, int n) { ... }`). A Kotlin lambda cannot refer to the variable it
is being assigned to (`unresolved reference 'fib'`). Local functions in Python,
JavaScript, Dart, and Kotlin can simply call themselves by name.

---

## 5.8 Overloading

**Overloading** means several functions with the same name and different parameter
lists. Only C++ and Kotlin have it in full:

```cpp
double area(double radius) { return 3.14159 * radius * radius; }
double area(double width, double height) { return width * height; }
```

| Language | Overloading | What to do instead |
| --- | --- | --- |
| C | No; C11's `_Generic` picks a function by an argument's type, inside a macro | Different names: `abs`, `labs`, `fabs` |
| C++ | **Yes**, by number and types of parameters | |
| Rust | No | Different names, or a trait implemented for several types (chapter 13) |
| Go | No | Different names: `strconv.Itoa`, `strconv.FormatInt` |
| Python | No; a second `def` with the same name replaces the first | Defaults, or `functools.singledispatch` to choose by type |
| JavaScript | No; the last definition wins | Check the arguments inside the function |
| TypeScript | Several signatures for **one** implementation | Union types |
| Dart | No | Named and optional parameters |
| Kotlin | **Yes**, by number and types of parameters, as in Java | Defaults often replace a family of overloads |

TypeScript's overloads describe one JavaScript function in several ways, because only
one function exists once the types are erased:

```ts
function area(radius: number): number;
function area(width: number, height: number): number;
function area(a: number, b?: number): number {
  return b === undefined ? Math.PI * a * a : a * b;
}
```

C's `_Generic` is a compile-time choice by type, used inside a macro so that one name
works for several types:

```c
#define abs_value(x) _Generic((x), int: abs, long: labs, double: fabs)(x)
```

---

## 5.9 Why each language looks the way it does

- **C** treats a function as an address in the program's code. A function pointer is
  that address, which is why C cannot make new functions at run time: a closure needs
  somewhere to keep the variables it captured, and C leaves all memory decisions to
  you. Variable arguments exist so that `printf` can take any values; ANSI C (1989)
  wrote that down as `...` and `<stdarg.h>`, with no type information at all.
- **C++** added default arguments while it was still called "C with Classes", and
  overloading soon after it became C++. Lambdas arrived in C++11 as short syntax for
  function objects, which is why you choose a capture mode for each variable.
  Variadic templates (C++11) and fold expressions (C++17) give type-checked `...`.
- **Rust** leaves out defaults, overloading, and variadic functions on purpose, so that
  a call means one thing and type inference stays simple. Its closures are structs, as
  in C++, but the borrow checker decides whether a reference capture is safe, and the
  `Fn` traits record what each closure does with its captures.
- **Go** leaves out defaults and overloading too, but adds multiple results, built for
  `value, err`. Closures capture variables, and the garbage collector keeps them alive.
  A variadic `...int` is just a slice.
- **Python** makes a function object each time a `def` runs, which is also when its
  defaults are evaluated. Its APIs lean on keyword arguments, as in
  `sorted(words, key=len, reverse=True)`, so parameters can be passed by name, collected
  with `*args` and `**kwargs`, and restricted with `/` and `*`.
- **JavaScript** had first-class functions from its first version, because its creator
  drew on Scheme. Browser code is mostly callbacks for events, so closures are
  everywhere. ES2015 added arrow functions, defaults, rest and spread, and `let`, which
  fixed the loop-closure problem.
- **TypeScript** describes JavaScript's functions with types. Its overloads are only
  signatures, because the types are erased and one JavaScript function is left.
- **Dart** has named parameters, used everywhere in Flutter, where a widget call such
  as `Text('hi', textAlign: TextAlign.center)` names most of its arguments. Defaults
  must be constants. Records arrived in Dart 3 to return several values without a
  class.
- **Kotlin** added defaults and named arguments so one function could replace the
  stacks of overloads Java APIs need, and kept overloading so it can call and be called
  by Java. Its lambdas may change captured `var`s, which Java's may not, and a trailing
  lambda goes outside the parentheses so that functions taking a block read like
  language syntax. `tailrec` exists because the JVM does not optimize tail calls.

---

## 5.10 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | Declaring `int f();` in C17 | Calls with any arguments compile without a warning; write `int f(void)` |
| C | Passing the wrong type to a `...` function | Undefined behavior; only `printf`-style calls are checked |
| C, C++ | A path through a non-void function with no `return` | Undefined behavior; `-Wall` warns |
| C, C++ | `printf("%d %d", next(), next())` | The order the two calls run in is unspecified; the other seven languages run them left to right |
| C++ | Capturing with `[&]` in a lambda that is returned or stored | A dangling reference, with no warning |
| C++ | Writing the default argument in both the declaration and the definition | Compile error: `default argument given for parameter 2` |
| Rust | A `;` after the last expression | The function returns `()`: `mismatched types` |
| Rust | Returning a closure without `move` | E0373: the closure may outlive the variables it borrows |
| Rust | Calling an `FnMut` closure stored without `mut` | E0596: cannot borrow as mutable |
| Go | `x := f()` when `f` returns two values | Compile error: `assignment mismatch` |
| Go | Closures in a loop with `go 1.21` or older in `go.mod` | All closures share one variable |
| Python | A list or dict as a default value | Shared by every call |
| Python | `total += x` in a closure without `nonlocal total` | `UnboundLocalError` |
| Python | Lambdas made in a loop | All see the last value; use `lambda i=i: i` |
| JavaScript, TypeScript | Passing `null` to get the default | Only `undefined` uses the default |
| JavaScript | `var` in a loop that makes closures | All see the final value; use `let` |
| Dart | A default value that is not a constant | Compile error |
| Kotlin | `val (remainder, quotient) = divMod(a, b)` | Unpacking goes by position, so the values are silently swapped |
| Kotlin | `total(*listOf(1, 2))` | Compile error: `*` spreads arrays only; write `*list.toIntArray()` |
| Kotlin | A recursive memo with Java's `computeIfAbsent` | `ConcurrentModificationException`; use Kotlin's `getOrPut` |
| Kotlin | Assigning to a parameter | Compile error: parameters are read-only; copy into a `var` |
| All | Recursion with no base case, or too deep | A stack overflow, or `RecursionError` in Python |

---

## 5.11 Idiom check

**C: a function pointer plus a `void *`.** Since C has no closures, a function that
takes a callback usually also takes a pointer to the callback's data, and passes it
back on every call:

```c
#include <stddef.h>
#include <stdio.h>

// The C stand-in for a closure: a function pointer plus a void* to its data.
static void for_each(const int *values, size_t count, void (*f)(int, void *), void *data) {
    for (size_t i = 0; i < count; i++) {
        f(values[i], data);
    }
}

static void add_to(int value, void *data) {
    int *total = data;
    *total += value;
}

int main(void) {
    const int numbers[] = {4, 3, 2, 1};
    int total = 0;
    for_each(numbers, 4, add_to, &total);
    printf("total = %d\n", total);  // total = 10
    return 0;
}
```

The `void *` throws away the type, so nothing checks that `add_to` gets an `int *`.

**C++: capture explicitly.** Write `[n]` or `[&total]` instead of `[=]` or `[&]` so
the reader can see what the lambda uses, and capture by value whenever the lambda
outlives the current scope. Take callables as `auto` or a template parameter, and use
`std::function` when you need to store different callables in one place.

**Rust: `impl Fn` in, `impl Fn` out.** Accept closures as `impl Fn(...)`, or as
`impl FnMut(...)` or `impl FnOnce(...)` if you call them through `mut` or only once,
since those accept more closures. Return them as `impl Fn(...)`, and box them as
`Box<dyn Fn(...)>` only when one collection or variable must hold different closures.
For many optional settings, take a struct that implements `Default`.

**Go: small functions, `value, err`, and an options struct.** Return an error as the
last result, and check it right away. Pass a struct when a function needs more than a
few settings.

**Python: keyword arguments for clarity.** `connect("db", timeout=2.0)` is clearer
than a bare `2.0`. Use `None` as the default for a list or dict, `def` rather than an
assigned `lambda`, `@functools.cache` for memoization, and `functools.partial` to fix
some arguments of a function.

**JavaScript and TypeScript: arrows for callbacks, an options object for settings.**
`items.map((x) => x * 2)` is the everyday form. Destructure an options object with
defaults instead of taking many positional parameters.

**Dart: named parameters and tear-offs.** Mark the ones that must be passed as
`required`. Write `=>` for one-expression functions, and pass an existing function by
name instead of wrapping it: `names.forEach(print)`, not
`names.forEach((name) => print(name))`.

**Kotlin: expression bodies, trailing lambdas, and function references.** Write
`fun square(x: Int) = x * x` when the body is one expression. Put a function parameter
last so callers can pass a trailing lambda. Pass an existing function with `::`:
`names.forEach(::println)`, not `names.forEach { println(it) }`. Name the arguments
whenever a bare value would be unclear: `connect("db", timeout = 2.0)`.

---

## 5.12 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Memoized Fibonacci](exercises/01-fibonacci/) | Recursion, keeping a memo between calls, 64-bit and `BigInt` arithmetic |
| 2 | [Counter factory](exercises/02-counters/) | Returning closures, changing captured state, a map of functions |
| 3 | [Function pipeline](exercises/03-pipeline/) | Building functions from data, a list of closures, function pointers in C |

```sh
python3 tools/check.py 05                     # your answers, every language
python3 tools/check.py 05 --lang go,python    # some languages
python3 tools/check.py 05 --solutions         # the reference answers
```

**Stretch:** break exercise 2 on purpose and read the errors. In Rust, change the map
to hold `Box<dyn Fn() -> i64>` (`cannot assign to next, as it is a captured variable
in a Fn closure`). In C++, remove `mutable` from the lambda. In Python, remove the
`nonlocal` line. In Kotlin, declare `next` with `val`. Each error says the same thing in
its own words: this closure was not allowed to change what it captured.

---

Previous: [Chapter 4: Control flow and pattern matching](../04-control-flow-and-pattern-matching/) ·
Next: [Chapter 6: Strings and text](../06-strings-and-text/)
