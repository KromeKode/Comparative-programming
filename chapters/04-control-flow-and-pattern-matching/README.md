# Chapter 4: Control flow and pattern matching

Every program chooses what to do next and repeats work. The nine languages agree on
the basics (`if`, loops, `break`) and disagree on almost everything around them: whether
a `switch` falls through to the next case, whether the compiler checks that you
handled every case, how you leave two nested loops at once, and whether you can match
on the *shape* of a value instead of comparing it to constants. This chapter compares
all of that.

**Languages:** all 9 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| `if` as a statement and as an expression | Writing compact, readable code in every later chapter |
| `switch` vs `match`, fallthrough, and exhaustiveness | Enums and null (chapter 8), error handling (chapter 11) |
| The loop forms and ranges | Every algorithm in Part 4 |
| `break` and `continue` with labels | Searches and graph traversals (chapters 27 and 29) |
| Pattern matching and destructuring | Parsers (chapter 21), protocol handling (chapters 35 to 38) |

---

## 4.1 The concept: choosing and repeating

Any program can be built from three pieces: **sequence** (do this, then that),
**selection** (do this or that), and **iteration** (do this again). Early languages
also had `goto`, which jumps anywhere. In 1968 Edsger Dijkstra's letter "Go To
Statement Considered Harmful" argued that free jumps made programs impossible to
follow, and **structured programming** replaced most of them with `if`, `while`, and
`for`.

All nine languages here are structured. They differ in how much jumping they still
allow (`goto`, labeled `break`, `switch` fallthrough) and in how much checking they do
for you: whether a `match` must cover every case, and whether a condition must be a
`bool`.

---

## 4.2 `if` and `else`

| Language | Shape | Parentheses around the condition | Braces | Else-if |
| --- | --- | --- | --- | --- |
| C | `if (x > 0) { ... } else { ... }` | Required | Optional for one statement | `else if` |
| C++ | `if (x > 0) { ... } else { ... }` | Required | Optional for one statement | `else if` |
| Rust | `if x > 0 { ... } else { ... }` | Not used: a warning (`unused_parens`) | Required | `else if` |
| Go | `if x > 0 { ... } else { ... }` | Not used: `gofmt` removes them | Required, with `{` on the same line | `else if` |
| Python | `if x > 0:` and an indented block | Not used | None: indentation marks the block | `elif` |
| JavaScript | `if (x > 0) { ... } else { ... }` | Required | Optional for one statement | `else if` |
| TypeScript | `if (x > 0) { ... } else { ... }` | Required | Optional for one statement | `else if` |
| Dart | `if (x > 0) { ... } else { ... }` | Required | Optional for one statement | `else if` |
| Kotlin | `if (x > 0) { ... } else { ... }` | Required | Optional for one statement | `else if` |

**Why Rust and Go require braces.** In 2014, Apple's TLS code (the code that checks
HTTPS certificates) contained this:

```c
if ((err = SSLHashSHA1.update(&hashCtx, &signedParams)) != 0)
    goto fail;
    goto fail;  /* indented like part of the if, but it always runs */
if ((err = SSLHashSHA1.final(&hashCtx, &hashOut)) != 0)
    goto fail;
```

The second `goto fail;` is not inside the `if`. It always runs, so the signature check
after it never happened, and iPhones and Macs accepted forged certificates. With
mandatory braces the bug cannot be written. In C and C++, GCC's `-Wall` now warns
(`this 'if' clause does not guard...`), and safety rules such as MISRA C and CERT C
require braces on every `if`.
Go's braces must also start on the same line as the `if`, because Go inserts
semicolons at line ends: `if x > 0` followed by `{` on the next line is a syntax error.

**`if` as a value.** Chapter 3 showed that Rust's and Kotlin's `if` is an expression
and the other languages use `c ? a : b` or `a if c else b`. Rust and Kotlin code lean
on this:

```rust
let parity = if n % 2 == 0 { "even" } else { "odd" };
```

```kotlin
val parity = if (n % 2 == 0) "even" else "odd"
```

**Declaring a variable inside the condition.** Several languages let a condition
introduce a variable that only exists inside the `if`. This keeps temporary values out
of the surrounding scope:

| Language | Example |
| --- | --- |
| C | none |
| C++ (C++17) | `if (auto it = ages.find(name); it != ages.end()) { ... }` |
| Rust | `if let Some(age) = ages.get(name) { ... }`, and with a let chain (1.88): `if let Some(&age) = ages.get(name) && age >= 18 { ... }` |
| Go | `if n, err := strconv.Atoi(s); err == nil { ... }` |
| Python | `if (match := pattern.search(line)) is not None: ...` |
| JavaScript, TypeScript | none |
| Dart (Dart 3) | `if (int.tryParse(s) case final n?) { ... }` |
| Kotlin | none; `ages[name]?.let { age -> ... }` runs the block only when the value is not null |

Rust's **let chains** join pattern tests and boolean tests with `&&` in one condition.
They need the 2024 edition, which this book uses. The Roman numerals exercise uses one
to look one letter ahead:

```rust
if let Some(&next) = values.get(i + 1)
    && value < next
{
    total -= value;
}
```

---

## 4.3 `switch` and `match`

A `switch` picks one branch by comparing a value against constants. A `match` (Rust),
`match` statement (Python), Dart 3 `switch`, or Kotlin `when` does more: it compares
against **patterns**, which section 4.6 covers. The program below names HTTP status
codes, which needs exact values, groups of values, and ranges. The code is in
[`examples/01-switch/`](examples/01-switch/).

**C**

```c
#include <stdio.h>

static const char *describe(int code) {
    const char *text = "unknown";
    switch (code) {
    case 200:
        text = "ok";
        break;  // without break, execution falls into the next case
    case 301:
    case 302:
    case 307:
    case 308:  // several labels can share one body
        text = "redirect";
        break;
    case 404:
        text = "not found";
        break;
    default:
        // Case labels must be single constants, so ranges need an if.
        if (code >= 400 && code <= 499) {
            text = "client error";
        } else if (code >= 500 && code <= 599) {
            text = "server error";
        }
        break;
    }
    return text;
}

int main(void) {
    const int codes[] = {200, 301, 308, 404, 418, 500, 503, 99};
    for (size_t i = 0; i < sizeof codes / sizeof codes[0]; i++) {
        printf("%d: %s\n", codes[i], describe(codes[i]));
    }
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <string_view>

std::string_view describe(int code) {
    switch (code) {
        case 200:
            return "ok";  // return leaves the switch, so no break is needed
        case 301:
        case 302:
        case 307:
        case 308:  // several labels can share one body
            return "redirect";
        case 404:
            return "not found";
        default:
            break;
    }
    // Case labels must be single constants, so ranges need an if.
    if (code >= 400 && code <= 499) {
        return "client error";
    }
    if (code >= 500 && code <= 599) {
        return "server error";
    }
    return "unknown";
}

int main() {
    for (int code : {200, 301, 308, 404, 418, 500, 503, 99}) {
        std::cout << code << ": " << describe(code) << '\n';
    }
}
```

**Rust**

```rust
fn describe(code: u16) -> &'static str {
    match code {
        200 => "ok",
        301 | 302 | 307 | 308 => "redirect",
        // Arms are tried from top to bottom, so 404 must come before 400..=499.
        404 => "not found",
        400..=499 => "client error",
        500..=599 => "server error",
        _ => "unknown", // a match must cover every possible value
    }
}

fn main() {
    for code in [200, 301, 308, 404, 418, 500, 503, 99] {
        println!("{code}: {}", describe(code));
    }
}
```

**Go**

```go
package main

import "fmt"

func describe(code int) string {
	// Go never falls through to the next case, so there is no break to forget.
	switch code {
	case 200:
		return "ok"
	case 301, 302, 307, 308:
		return "redirect"
	case 404:
		return "not found"
	}
	// A switch with no value tests each case as a condition, top to bottom.
	switch {
	case code >= 400 && code <= 499:
		return "client error"
	case code >= 500 && code <= 599:
		return "server error"
	}
	return "unknown"
}

func main() {
	for _, code := range []int{200, 301, 308, 404, 418, 500, 503, 99} {
		fmt.Printf("%d: %s\n", code, describe(code))
	}
}
```

**Python**

```python
def describe(code: int) -> str:
    match code:  # Python 3.10 and later
        case 200:
            return "ok"
        case 301 | 302 | 307 | 308:
            return "redirect"
        # Cases are tried from top to bottom, so 404 must come before the range.
        case 404:
            return "not found"
        case _ if 400 <= code <= 499:
            return "client error"
        case _ if 500 <= code <= 599:
            return "server error"
        case _:
            return "unknown"


def main() -> None:
    for code in [200, 301, 308, 404, 418, 500, 503, 99]:
        print(f"{code}: {describe(code)}")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
function describe(code) {
  // switch compares with ===, and falls through like C without a break or return.
  switch (code) {
    case 200:
      return "ok";
    case 301:
    case 302:
    case 307:
    case 308:
      return "redirect";
    case 404:
      return "not found";
  }
  if (code >= 400 && code <= 499) {
    return "client error";
  }
  if (code >= 500 && code <= 599) {
    return "server error";
  }
  return "unknown";
}

for (const code of [200, 301, 308, 404, 418, 500, 503, 99]) {
  console.log(`${code}: ${describe(code)}`);
}
```

**TypeScript**

```ts
function describe(code: number): string {
  // switch compares with ===, and falls through like C without a break or return.
  switch (code) {
    case 200:
      return "ok";
    case 301:
    case 302:
    case 307:
    case 308:
      return "redirect";
    case 404:
      return "not found";
  }
  if (code >= 400 && code <= 499) {
    return "client error";
  }
  if (code >= 500 && code <= 599) {
    return "server error";
  }
  return "unknown";
}

for (const code of [200, 301, 308, 404, 418, 500, 503, 99]) {
  console.log(`${code}: ${describe(code)}`);
}
```

**Dart**

```dart
// A switch expression (Dart 3) gives a value. Cases never fall through.
String describe(int code) => switch (code) {
  200 => 'ok',
  301 || 302 || 307 || 308 => 'redirect',
  // Cases are tried from top to bottom, so 404 must come before the range.
  404 => 'not found',
  >= 400 && <= 499 => 'client error',
  >= 500 && <= 599 => 'server error',
  _ => 'unknown',
};

void main() {
  for (final code in [200, 301, 308, 404, 418, 500, 503, 99]) {
    print('$code: ${describe(code)}');
  }
}
```

**Kotlin**

```kotlin
// when is an expression: it gives a value. Branches never fall through.
fun describe(code: Int): String =
    when (code) {
        200 -> "ok"
        301, 302, 307, 308 -> "redirect"
        // Branches are tried from top to bottom, so 404 must come before the range.
        404 -> "not found"
        in 400..499 -> "client error" // .. includes both ends
        in 500..599 -> "server error"
        else -> "unknown"
    }

fun main() {
    for (code in listOf(200, 301, 308, 404, 418, 500, 503, 99)) {
        println("$code: ${describe(code)}")
    }
}
```

Output, identical in all nine:

```text
200: ok
301: redirect
308: redirect
404: not found
418: client error
500: server error
503: server error
99: unknown
```

| Language | Falls through to the next case | Several values in one case | Ranges | Checks that every value is handled | Gives a value |
| --- | --- | --- | --- | --- | --- |
| C | **Yes**, until `break` | Stacked `case` labels | No (GCC and Clang allow `case 400 ... 499:`) | No; `-Wall` warns about a missing `enum` value | No |
| C++ | **Yes**, until `break` | Stacked `case` labels | No (same extension) | No; `-Wall` warns about a missing `enum` value | No |
| Rust | Never | `301 \| 302` | `400..=499` | **Yes**: a compile error | Yes |
| Go | Never, unless the case ends with `fallthrough` | `case 301, 302:` | Conditions: `case code >= 400 && code <= 499:` | No | No |
| Python | Never | `case 301 \| 302:` | Guards: `case _ if 400 <= code <= 499:` | No | No |
| JavaScript | **Yes**, until `break` | Stacked `case` labels | No | No | No |
| TypeScript | **Yes**, until `break` | Stacked `case` labels | No | Only with the `never` trick in section 4.6 | No |
| Dart | Never (Dart 3); an empty case shares the next body | `301 \|\| 302` | `>= 400 && <= 499` | **Yes** for enums, `bool`, and sealed classes, and always for switch expressions | Yes, as a switch expression |
| Kotlin | Never | `301, 302 ->` | `in 400..499 ->` | **Yes** for enums, `Boolean`, and sealed classes; a `when` that gives a value needs `else` otherwise | Yes |

**Fallthrough** means that without a `break`, a C-style `switch` keeps running into the
next case's code. Occasionally that is what you want. Usually it is a forgotten
`break`. `-Wextra` warns about it in C and C++ (`this statement may fall through`),
and you mark the intentional ones with `[[fallthrough]];` (C++17 and C23). Go, Rust,
Python, Dart, and Kotlin never fall through, and Go makes you write `fallthrough` to ask
for it.

**What you can switch on** also differs. C and C++ can switch only on integers,
characters, and enums, so `switch (name)` on a string is a compile error. Go,
JavaScript, TypeScript, and Dart compare strings too, and Rust and Python can match
strings, tuples, and nested data. JavaScript compares with `===`, so a case never
matches an object or array unless it is the same object. Kotlin's `when` compares any
value with `==`, checks types with `is`, and can leave out the value entirely, which
turns it into a tidy `else if` chain.

**Order matters when cases overlap.** Rust, Python, Dart, Kotlin, and Go's
condition-only `switch` try cases from top to bottom and take the first match. If
`400..=499` came before `404`, the `404` arm would never run. Rust warns about that
(`unreachable pattern`).

---

## 4.4 Loops

The program below runs the common loop shapes in each language. The code is in
[`examples/02-loops/`](examples/02-loops/).

**C**

```c
#include <stdio.h>

int main(void) {
    printf("up:");
    for (int i = 0; i < 5; i++) {
        printf(" %d", i);
    }
    printf("\n");

    printf("down:");
    for (int i = 5; i >= 1; i--) {
        printf(" %d", i);
    }
    printf("\n");

    printf("by threes:");
    for (int i = 0; i < 10; i += 3) {
        printf(" %d", i);
    }
    printf("\n");

    printf("halving:");
    int n = 100;
    while (n > 0) {
        printf(" %d", n);
        n /= 2;
    }
    printf("\n");

    const char *colors[] = {"red", "green", "blue"};
    const size_t count = sizeof colors / sizeof colors[0];
    printf("colors:");
    for (size_t i = 0; i < count; i++) {
        printf(" %s", colors[i]);
    }
    printf("\n");

    printf("numbered:");
    for (size_t i = 0; i < count; i++) {
        printf(" %zu=%s", i + 1, colors[i]);
    }
    printf("\n");

    // do-while tests the condition after the body, so the body always runs once.
    int m = 100;
    do {
        m *= 2;
    } while (m < 50);
    printf("do-while: %d\n", m);
    return 0;
}
```

**C++**

```cpp
#include <array>
#include <cstddef>
#include <iostream>
#include <string_view>

int main() {
    std::cout << "up:";
    for (int i = 0; i < 5; ++i) {
        std::cout << ' ' << i;
    }
    std::cout << '\n';

    std::cout << "down:";
    for (int i = 5; i >= 1; --i) {
        std::cout << ' ' << i;
    }
    std::cout << '\n';

    std::cout << "by threes:";
    for (int i = 0; i < 10; i += 3) {
        std::cout << ' ' << i;
    }
    std::cout << '\n';

    std::cout << "halving:";
    int n = 100;
    while (n > 0) {
        std::cout << ' ' << n;
        n /= 2;
    }
    std::cout << '\n';

    constexpr std::array<std::string_view, 3> colors{"red", "green", "blue"};
    std::cout << "colors:";
    for (std::string_view color : colors) {  // range-based for (C++11)
        std::cout << ' ' << color;
    }
    std::cout << '\n';

    std::cout << "numbered:";
    for (std::size_t i = 0; i < colors.size(); ++i) {
        std::cout << ' ' << i + 1 << '=' << colors[i];
    }
    std::cout << '\n';

    // do-while tests the condition after the body, so the body always runs once.
    int m = 100;
    do {
        m *= 2;
    } while (m < 50);
    std::cout << "do-while: " << m << '\n';
}
```

**Rust**

```rust
fn main() {
    print!("up:");
    for i in 0..5 {
        print!(" {i}"); // 0..5 stops before 5
    }
    println!();

    print!("down:");
    for i in (1..=5).rev() {
        print!(" {i}"); // 1..=5 includes 5
    }
    println!();

    print!("by threes:");
    for i in (0..10).step_by(3) {
        print!(" {i}");
    }
    println!();

    print!("halving:");
    let mut n = 100;
    while n > 0 {
        print!(" {n}");
        n /= 2;
    }
    println!();

    let colors = ["red", "green", "blue"];
    print!("colors:");
    for color in colors {
        print!(" {color}");
    }
    println!();

    print!("numbered:");
    for (i, color) in colors.iter().enumerate() {
        print!(" {}={color}", i + 1);
    }
    println!();

    // Rust has no do-while. A loop with the test at the end does the same job.
    let mut m = 100;
    loop {
        m *= 2;
        if m >= 50 {
            break;
        }
    }
    println!("do-while: {m}");
}
```

**Go**

```go
package main

import "fmt"

func main() {
	fmt.Print("up:")
	for i := range 5 { // Go 1.22 and later: range over an integer
		fmt.Print(" ", i)
	}
	fmt.Println()

	fmt.Print("down:")
	for i := 5; i >= 1; i-- {
		fmt.Print(" ", i)
	}
	fmt.Println()

	fmt.Print("by threes:")
	for i := 0; i < 10; i += 3 {
		fmt.Print(" ", i)
	}
	fmt.Println()

	fmt.Print("halving:")
	n := 100
	for n > 0 { // Go has no while: a for with only a condition does that job
		fmt.Print(" ", n)
		n /= 2
	}
	fmt.Println()

	colors := []string{"red", "green", "blue"}
	fmt.Print("colors:")
	for _, color := range colors {
		fmt.Print(" ", color)
	}
	fmt.Println()

	fmt.Print("numbered:")
	for i, color := range colors {
		fmt.Printf(" %d=%s", i+1, color)
	}
	fmt.Println()

	// No do-while either: loop forever and break at the end.
	m := 100
	for {
		m *= 2
		if m >= 50 {
			break
		}
	}
	fmt.Println("do-while:", m)
}
```

**Python**

```python
def main() -> None:
    line = "up:"
    for i in range(5):  # stops before 5
        line += f" {i}"
    print(line)

    line = "down:"
    for i in range(5, 0, -1):  # stops before 0
        line += f" {i}"
    print(line)

    line = "by threes:"
    for i in range(0, 10, 3):
        line += f" {i}"
    print(line)

    line = "halving:"
    n = 100
    while n > 0:
        line += f" {n}"
        n //= 2
    print(line)

    colors = ["red", "green", "blue"]
    line = "colors:"
    for color in colors:
        line += f" {color}"
    print(line)

    line = "numbered:"
    for i, color in enumerate(colors, start=1):
        line += f" {i}={color}"
    print(line)

    # Python has no do-while: loop forever and break at the end.
    m = 100
    while True:
        m *= 2
        if m >= 50:
            break
    print(f"do-while: {m}")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
let line = "up:";
for (let i = 0; i < 5; i++) {
  line += ` ${i}`;
}
console.log(line);

line = "down:";
for (let i = 5; i >= 1; i--) {
  line += ` ${i}`;
}
console.log(line);

line = "by threes:";
for (let i = 0; i < 10; i += 3) {
  line += ` ${i}`;
}
console.log(line);

line = "halving:";
let n = 100;
while (n > 0) {
  line += ` ${n}`;
  n = Math.trunc(n / 2);
}
console.log(line);

const colors = ["red", "green", "blue"];
line = "colors:";
for (const color of colors) {
  // for...of gives the values. for...in would give the keys "0", "1", "2".
  line += ` ${color}`;
}
console.log(line);

line = "numbered:";
for (const [i, color] of colors.entries()) {
  line += ` ${i + 1}=${color}`;
}
console.log(line);

// do-while tests the condition after the body, so the body always runs once.
let m = 100;
do {
  m *= 2;
} while (m < 50);
console.log(`do-while: ${m}`);
```

**TypeScript**

```ts
let line: string = "up:";
for (let i = 0; i < 5; i++) {
  line += ` ${i}`;
}
console.log(line);

line = "down:";
for (let i = 5; i >= 1; i--) {
  line += ` ${i}`;
}
console.log(line);

line = "by threes:";
for (let i = 0; i < 10; i += 3) {
  line += ` ${i}`;
}
console.log(line);

line = "halving:";
let n = 100;
while (n > 0) {
  line += ` ${n}`;
  n = Math.trunc(n / 2);
}
console.log(line);

const colors: string[] = ["red", "green", "blue"];
line = "colors:";
for (const color of colors) {
  // for...of gives the values. for...in would give the keys "0", "1", "2".
  line += ` ${color}`;
}
console.log(line);

line = "numbered:";
for (const [i, color] of colors.entries()) {
  line += ` ${i + 1}=${color}`;
}
console.log(line);

// do-while tests the condition after the body, so the body always runs once.
let m = 100;
do {
  m *= 2;
} while (m < 50);
console.log(`do-while: ${m}`);
export {};
```

**Dart**

```dart
void main() {
  var line = 'up:';
  for (var i = 0; i < 5; i++) {
    line += ' $i';
  }
  print(line);

  line = 'down:';
  for (var i = 5; i >= 1; i--) {
    line += ' $i';
  }
  print(line);

  line = 'by threes:';
  for (var i = 0; i < 10; i += 3) {
    line += ' $i';
  }
  print(line);

  line = 'halving:';
  var n = 100;
  while (n > 0) {
    line += ' $n';
    n ~/= 2;
  }
  print(line);

  const colors = ['red', 'green', 'blue'];
  line = 'colors:';
  for (final color in colors) {
    line += ' $color';
  }
  print(line);

  line = 'numbered:';
  for (final (i, color) in colors.indexed) {
    line += ' ${i + 1}=$color'; // indexed gives (index, value) records
  }
  print(line);

  // do-while tests the condition after the body, so the body always runs once.
  var m = 100;
  do {
    m *= 2;
  } while (m < 50);
  print('do-while: $m');
}
```

**Kotlin**

```kotlin
fun main() {
    var line = "up:"
    for (i in 0..<5) { // 0..<5 is 0 to 4; 0..5 would include the 5
        line += " $i"
    }
    println(line)

    line = "down:"
    for (i in 5 downTo 1) {
        line += " $i"
    }
    println(line)

    line = "by threes:"
    for (i in 0..<10 step 3) {
        line += " $i"
    }
    println(line)

    line = "halving:"
    var n = 100
    while (n > 0) {
        line += " $n"
        n /= 2
    }
    println(line)

    val colors = listOf("red", "green", "blue")
    line = "colors:"
    for (color in colors) {
        line += " $color"
    }
    println(line)

    line = "numbered:"
    for ((i, color) in colors.withIndex()) {
        line += " ${i + 1}=$color" // withIndex gives (index, value) pairs
    }
    println(line)

    // do-while tests the condition after the body, so the body always runs once.
    var m = 100
    do {
        m *= 2
    } while (m < 50)
    println("do-while: $m")
}
```

Output, identical in all nine:

```text
up: 0 1 2 3 4
down: 5 4 3 2 1
by threes: 0 3 6 9
halving: 100 50 25 12 6 3 1
colors: red green blue
numbered: 1=red 2=green 3=blue
do-while: 200
```

| Language | Count from 0 to n − 1 | Repeat while a condition holds | Body runs at least once | Each item | Each item with its index |
| --- | --- | --- | --- | --- | --- |
| C | `for (int i = 0; i < n; i++)` | `while (c)` | `do { } while (c);` | an index loop | an index loop |
| C++ | `for (int i = 0; i < n; ++i)` | `while (c)` | `do { } while (c);` | `for (const auto& x : v)` | an index loop, or `std::views::enumerate` (C++23) |
| Rust | `for i in 0..n` | `while c` | `loop { ...; if !c { break; } }` | `for x in &v` | `for (i, x) in v.iter().enumerate()` |
| Go | `for i := range n` (Go 1.22) | `for c` | `for { ...; if !c { break } }` | `for _, x := range v` | `for i, x := range v` |
| Python | `for i in range(n)` | `while c:` | `while True: ... if not c: break` | `for x in v:` | `for i, x in enumerate(v):` |
| JavaScript | `for (let i = 0; i < n; i++)` | `while (c)` | `do { } while (c);` | `for (const x of v)` | `for (const [i, x] of v.entries())` |
| TypeScript | `for (let i = 0; i < n; i++)` | `while (c)` | `do { } while (c);` | `for (const x of v)` | `for (const [i, x] of v.entries())` |
| Dart | `for (var i = 0; i < n; i++)` | `while (c)` | `do { } while (c);` | `for (final x in v)` | `for (final (i, x) in v.indexed)` |
| Kotlin | `for (i in 0..<n)` | `while (c)` | `do { } while (c)` | `for (x in v)` | `for ((i, x) in v.withIndex())` |

Things to notice:

- **Go has one loop keyword.** `for` with three parts is a counting loop, `for` with a
  condition is a `while`, and `for` with nothing loops forever.
- **Rust and Python count with ranges, and ranges leave out the end.** `0..5` and
  `range(5)` both mean 0 to 4. Rust's `1..=5` includes the 5. Python's
  `range(5, 0, -1)` counts down and stops *before* 0.
- **Kotlin's `..` is the opposite of Rust's.** In Kotlin `0..5` *includes* the 5, and
  `0..<5` (or the older `0 until 5`) leaves it out. Kotlin has no C-style three-part
  `for`; it counts down with `5 downTo 1`, and `5..1` is simply empty.
- **Only C, C++, JavaScript, TypeScript, Dart, and Kotlin have `do-while`.** The others
  write a loop that tests at the end and breaks.
- **JavaScript has two "for each" loops, and one is a trap.** `for...of` gives the
  values. `for...in` gives the property *names* as strings, so on an array it yields
  `"0"`, `"1"`, `"2"`, plus any inherited properties. Use `for...of` for arrays.
- **Go 1.22 changed loop variables.** Before 1.22, one variable `i` was reused for
  every pass of a `for` loop, so closures and goroutines started in the loop all saw
  its last value. Since 1.22 each pass gets a fresh variable. Chapter 5 shows why that
  matters.

---

## 4.5 `break`, `continue`, and labels

`break` leaves the innermost loop and `continue` skips to its next pass. Leaving
*two* loops at once is where the languages split. The program below searches a grid
for each of two numbers, and stops searching as soon as it finds one. The code is in
[`examples/03-labeled-break/`](examples/03-labeled-break/).

**C**

```c
#include <stdio.h>

static const int grid[3][3] = {{3, 8, 1}, {9, 4, 7}, {2, 6, 5}};

int main(void) {
    const int targets[] = {7, 10};
    for (int t = 0; t < 2; t++) {
        int target = targets[t];
        int checked = 0;
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                checked++;
                if (grid[r][c] == target) {
                    printf("%d: row %d, column %d (checked %d cells)\n", target, r, c,
                           checked);
                    goto next_target;  // C has no labeled break; goto is the usual way out
                }
            }
        }
        printf("%d: not found (checked %d cells)\n", target, checked);
    next_target:;
    }
    return 0;
}
```

**C++**

```cpp
#include <array>
#include <iostream>
#include <optional>
#include <utility>

constexpr std::array<std::array<int, 3>, 3> grid{{{3, 8, 1}, {9, 4, 7}, {2, 6, 5}}};

// C++ has no labeled break. Putting the loops in a function lets return leave both.
std::optional<std::pair<int, int>> find(int target, int& checked) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            ++checked;
            if (grid[r][c] == target) {
                return std::pair{r, c};
            }
        }
    }
    return std::nullopt;
}

int main() {
    for (int target : {7, 10}) {
        int checked = 0;
        if (auto found = find(target, checked)) {
            auto [r, c] = *found;
            std::cout << target << ": row " << r << ", column " << c << " (checked " << checked
                      << " cells)\n";
        } else {
            std::cout << target << ": not found (checked " << checked << " cells)\n";
        }
    }
}
```

**Rust**

```rust
const GRID: [[i32; 3]; 3] = [[3, 8, 1], [9, 4, 7], [2, 6, 5]];

fn main() {
    'targets: for target in [7, 10] {
        let mut checked = 0;
        for (r, row) in GRID.iter().enumerate() {
            for (c, &value) in row.iter().enumerate() {
                checked += 1;
                if value == target {
                    println!("{target}: row {r}, column {c} (checked {checked} cells)");
                    continue 'targets; // leaves both inner loops at once
                }
            }
        }
        println!("{target}: not found (checked {checked} cells)");
    }
}
```

**Go**

```go
package main

import "fmt"

var grid = [3][3]int{{3, 8, 1}, {9, 4, 7}, {2, 6, 5}}

func main() {
targets:
	for _, target := range []int{7, 10} {
		checked := 0
		for r, row := range grid {
			for c, value := range row {
				checked++
				if value == target {
					fmt.Printf("%d: row %d, column %d (checked %d cells)\n", target, r, c, checked)
					continue targets // leaves both inner loops at once
				}
			}
		}
		fmt.Printf("%d: not found (checked %d cells)\n", target, checked)
	}
}
```

**Python**

```python
from itertools import product

GRID = [[3, 8, 1], [9, 4, 7], [2, 6, 5]]


def main() -> None:
    for target in [7, 10]:
        checked = 0
        # Python has no labeled break, so turn the two loops into one.
        for r, c in product(range(len(GRID)), range(len(GRID[0]))):
            checked += 1
            if GRID[r][c] == target:
                print(f"{target}: row {r}, column {c} (checked {checked} cells)")
                break
        else:  # runs only when the loop ended without a break
            print(f"{target}: not found (checked {checked} cells)")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const grid = [
  [3, 8, 1],
  [9, 4, 7],
  [2, 6, 5],
];

targets: for (const target of [7, 10]) {
  let checked = 0;
  for (const [r, row] of grid.entries()) {
    for (const [c, value] of row.entries()) {
      checked++;
      if (value === target) {
        console.log(`${target}: row ${r}, column ${c} (checked ${checked} cells)`);
        continue targets; // leaves both inner loops at once
      }
    }
  }
  console.log(`${target}: not found (checked ${checked} cells)`);
}
```

**TypeScript**

```ts
const grid: number[][] = [
  [3, 8, 1],
  [9, 4, 7],
  [2, 6, 5],
];

targets: for (const target of [7, 10]) {
  let checked = 0;
  for (const [r, row] of grid.entries()) {
    for (const [c, value] of row.entries()) {
      checked++;
      if (value === target) {
        console.log(`${target}: row ${r}, column ${c} (checked ${checked} cells)`);
        continue targets; // leaves both inner loops at once
      }
    }
  }
  console.log(`${target}: not found (checked ${checked} cells)`);
}
export {};
```

**Dart**

```dart
const grid = [
  [3, 8, 1],
  [9, 4, 7],
  [2, 6, 5],
];

void main() {
  targets:
  for (final target in [7, 10]) {
    var checked = 0;
    for (final (r, row) in grid.indexed) {
      for (final (c, value) in row.indexed) {
        checked++;
        if (value == target) {
          print('$target: row $r, column $c (checked $checked cells)');
          continue targets; // leaves both inner loops at once
        }
      }
    }
    print('$target: not found (checked $checked cells)');
  }
}
```

**Kotlin**

```kotlin
val grid = listOf(listOf(3, 8, 1), listOf(9, 4, 7), listOf(2, 6, 5))

fun main() {
    targets@ for (target in listOf(7, 10)) {
        var checked = 0
        for ((r, row) in grid.withIndex()) {
            for ((c, value) in row.withIndex()) {
                checked++
                if (value == target) {
                    println("$target: row $r, column $c (checked $checked cells)")
                    continue@targets // leaves both inner loops at once
                }
            }
        }
        println("$target: not found (checked $checked cells)")
    }
}
```

Output, identical in all nine:

```text
7: row 1, column 2 (checked 6 cells)
10: not found (checked 9 cells)
```

| Language | Leave nested loops | A loop that gives a value | `goto` |
| --- | --- | --- | --- |
| C | `goto` a label after the loops | No | Yes |
| C++ | Move the loops into a function and `return`, or `goto` | No | Yes |
| Rust | `break 'label;` and `continue 'label;` | Yes: `break value` in `loop` and in labeled blocks | No |
| Go | `break label` and `continue label` | No | Yes, but it cannot jump into a block or over a variable declaration |
| Python | None. Use a function and `return`, or turn the loops into one | No, but loops have an `else` block | No |
| JavaScript | `break label;` and `continue label;` | No | No |
| TypeScript | `break label;` and `continue label;` | No | No |
| Dart | `break label;` and `continue label;` | No | No |
| Kotlin | `break@label` and `continue@label` | No, but `return@run value` inside `run { }` comes close | No |

**Python rejected labeled `break`.** PEP 3136 proposed it in 2007, and Guido van Rossum
turned it down, arguing that code complicated enough to need it is rare and better
restructured. The Python example instead flattens the two loops into one with
`itertools.product` and uses **`for ... else`**: the `else` block runs only when the
loop finishes without a `break`. It reads as "no break". The guessing-game exercise
uses it for the "gave up" case.

**Rust loops can give a value.** `break` inside a `loop` can carry a result, and since
Rust 1.65 so can a labeled block, which replaces many "found" flags:

```rust
let found = 'search: {
    for (r, row) in GRID.iter().enumerate() {
        for (c, &value) in row.iter().enumerate() {
            if value == target {
                break 'search Some((r, c));
            }
        }
    }
    None
};
```

**`break` inside a `switch` leaves the `switch`.** In C, C++, JavaScript, TypeScript,
and Go, a `break` inside a `switch` that is inside a loop stops the `switch`, not the
loop. Go users hit this most, because Go's `switch` needs no `break` and a `break`
written to stop the loop silently does nothing useful. Use a labeled `break`, or
`return`.

---

## 4.6 Pattern matching

A **pattern** describes the shape of a value and can pull pieces out of it. `(0, y)`
matches any pair whose first part is 0 and names the second part `y`. Rust, Python, and
Dart have full pattern matching. Kotlin's `when` sits in between: it matches values,
ranges, and types, but cannot take a pair apart. The program below sorts points into the
origin, the two axes, the diagonal, and everything else. The code is in
[`examples/04-patterns/`](examples/04-patterns/).

**C**

```c
#include <stdio.h>

// C has no pattern matching: test each shape with if, most specific first.
static void classify(int x, int y) {
    printf("(%d, %d): ", x, y);
    if (x == 0 && y == 0) {
        printf("origin\n");
    } else if (y == 0) {
        printf("on the x axis at x = %d\n", x);
    } else if (x == 0) {
        printf("on the y axis at y = %d\n", y);
    } else if (x == y) {
        printf("on the diagonal at %d\n", x);
    } else {
        printf("x = %d, y = %d\n", x, y);
    }
}

int main(void) {
    const int points[][2] = {{0, 0}, {3, 0}, {0, -2}, {4, 4}, {-1, 5}};
    for (size_t i = 0; i < sizeof points / sizeof points[0]; i++) {
        classify(points[i][0], points[i][1]);
    }
    return 0;
}
```

**C++**

```cpp
#include <format>
#include <iostream>
#include <string>
#include <utility>

// C++ has no pattern matching yet. Structured bindings unpack the pair,
// and an if chain does the rest.
std::string classify(std::pair<int, int> point) {
    auto [x, y] = point;
    if (x == 0 && y == 0) {
        return "origin";
    }
    if (y == 0) {
        return std::format("on the x axis at x = {}", x);
    }
    if (x == 0) {
        return std::format("on the y axis at y = {}", y);
    }
    if (x == y) {
        return std::format("on the diagonal at {}", x);
    }
    return std::format("x = {}, y = {}", x, y);
}

int main() {
    for (auto point : {std::pair{0, 0}, {3, 0}, {0, -2}, {4, 4}, {-1, 5}}) {
        std::cout << std::format("({}, {}): {}\n", point.first, point.second, classify(point));
    }
}
```

**Rust**

```rust
fn classify(point: (i32, i32)) -> String {
    match point {
        (0, 0) => "origin".to_string(),
        (x, 0) => format!("on the x axis at x = {x}"), // x binds the first value
        (0, y) => format!("on the y axis at y = {y}"),
        (x, y) if x == y => format!("on the diagonal at {x}"), // a guard
        (x, y) => format!("x = {x}, y = {y}"),
    }
}

fn main() {
    for (x, y) in [(0, 0), (3, 0), (0, -2), (4, 4), (-1, 5)] {
        println!("({x}, {y}): {}", classify((x, y)));
    }
}
```

**Go**

```go
package main

import "fmt"

// Go has no pattern matching. A switch with no value tests conditions in order.
func classify(x, y int) string {
	switch {
	case x == 0 && y == 0:
		return "origin"
	case y == 0:
		return fmt.Sprintf("on the x axis at x = %d", x)
	case x == 0:
		return fmt.Sprintf("on the y axis at y = %d", y)
	case x == y:
		return fmt.Sprintf("on the diagonal at %d", x)
	default:
		return fmt.Sprintf("x = %d, y = %d", x, y)
	}
}

func main() {
	points := [][2]int{{0, 0}, {3, 0}, {0, -2}, {4, 4}, {-1, 5}}
	for _, p := range points {
		fmt.Printf("(%d, %d): %s\n", p[0], p[1], classify(p[0], p[1]))
	}
}
```

**Python**

```python
def classify(point: tuple[int, int]) -> str:
    match point:
        case (0, 0):
            return "origin"
        case (x, 0):  # x binds the first value
            return f"on the x axis at x = {x}"
        case (0, y):
            return f"on the y axis at y = {y}"
        case (x, y) if x == y:  # a guard
            return f"on the diagonal at {x}"
        case (x, y):
            return f"x = {x}, y = {y}"
    raise AssertionError("unreachable: the last case matches every pair")


def main() -> None:
    for x, y in [(0, 0), (3, 0), (0, -2), (4, 4), (-1, 5)]:
        print(f"({x}, {y}): {classify((x, y))}")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// JavaScript has no pattern matching. Destructuring unpacks the pair,
// and an if chain does the rest.
function classify([x, y]) {
  if (x === 0 && y === 0) {
    return "origin";
  }
  if (y === 0) {
    return `on the x axis at x = ${x}`;
  }
  if (x === 0) {
    return `on the y axis at y = ${y}`;
  }
  if (x === y) {
    return `on the diagonal at ${x}`;
  }
  return `x = ${x}, y = ${y}`;
}

const points = [
  [0, 0],
  [3, 0],
  [0, -2],
  [4, 4],
  [-1, 5],
];
for (const [x, y] of points) {
  console.log(`(${x}, ${y}): ${classify([x, y])}`);
}
```

**TypeScript**

```ts
type Point = [number, number];

// TypeScript has no pattern matching. Destructuring unpacks the pair,
// and an if chain does the rest.
function classify([x, y]: Point): string {
  if (x === 0 && y === 0) {
    return "origin";
  }
  if (y === 0) {
    return `on the x axis at x = ${x}`;
  }
  if (x === 0) {
    return `on the y axis at y = ${y}`;
  }
  if (x === y) {
    return `on the diagonal at ${x}`;
  }
  return `x = ${x}, y = ${y}`;
}

const points: Point[] = [
  [0, 0],
  [3, 0],
  [0, -2],
  [4, 4],
  [-1, 5],
];
for (const [x, y] of points) {
  console.log(`(${x}, ${y}): ${classify([x, y])}`);
}
export {};
```

**Dart**

```dart
// (int, int) is a record type (Dart 3). Switch patterns can take it apart.
String classify((int, int) point) => switch (point) {
  (0, 0) => 'origin',
  (var x, 0) => 'on the x axis at x = $x', // x binds the first value
  (0, var y) => 'on the y axis at y = $y',
  (var x, var y) when x == y => 'on the diagonal at $x', // a guard
  (var x, var y) => 'x = $x, y = $y',
};

void main() {
  for (final (x, y) in [(0, 0), (3, 0), (0, -2), (4, 4), (-1, 5)]) {
    print('($x, $y): ${classify((x, y))}');
  }
}
```

**Kotlin**

```kotlin
fun classify(point: Pair<Int, Int>): String {
    // when cannot take a pair apart, so destructure it first.
    val (x, y) = point
    // A when with no subject tests each condition in order, like an else-if chain.
    return when {
        x == 0 && y == 0 -> "origin"
        y == 0 -> "on the x axis at x = $x"
        x == 0 -> "on the y axis at y = $y"
        x == y -> "on the diagonal at $x"
        else -> "x = $x, y = $y"
    }
}

fun main() {
    for ((x, y) in listOf(0 to 0, 3 to 0, 0 to -2, 4 to 4, -1 to 5)) {
        println("($x, $y): ${classify(x to y)}")
    }
}
```

Output, identical in all nine:

```text
(0, 0): origin
(3, 0): on the x axis at x = 3
(0, -2): on the y axis at y = -2
(4, 4): on the diagonal at 4
(-1, 5): x = -1, y = 5
```

In Rust, Python, and Dart, each arm states the shape it wants and binds the parts it
needs. In C, C++, Go, JavaScript, and TypeScript the same logic is an `if` chain over
`x` and `y`, which works but describes the shape less directly. Kotlin unpacks the pair
first with `val (x, y) = point` and then uses a `when` with no subject, which is the
same `if` chain with less punctuation.

| Language | Pattern matching | Unpack values | Guards | Checks that every case is handled |
| --- | --- | --- | --- | --- |
| C | No | No | — | — |
| C++ | No; proposals exist but are not yet standard | Structured bindings: `auto [x, y] = point;` | — | — |
| Rust | `match`, `if let`, `while let`, `let ... else` | In any pattern: tuples, structs, enums, slices | `if`, and `if let` (1.95) | **Yes**: a compile error |
| Go | No; a type switch matches on types only | No | — | No |
| Python | `match` (3.10) | Sequences, dictionaries, and classes | `if` | No; type checkers can, with `assert_never` |
| JavaScript | No (a TC39 proposal exists) | Destructuring: `const [x, y] = point;` | — | No |
| TypeScript | No | Destructuring | — | Only with the `never` trick below |
| Dart | `switch` and if-case (Dart 3) | Records, lists, maps, and objects | `when` | **Yes** for enums, `bool`, and sealed classes |
| Kotlin | `when` on values, ranges (`in`), and types (`is`); no structural patterns | Destructuring: `val (x, y) = point`, for pairs and data classes | `if` after a branch condition (2.2) | **Yes** for enums, `Boolean`, and sealed classes |

**Exhaustiveness** means the compiler proves that every possible value reaches some
arm. In Rust, delete the last arm of `classify` and the program no longer compiles:
`error[E0004]: non-exhaustive patterns`, followed by examples of pairs that no arm
covers, such as `(i32::MIN..=-1_i32, 1_i32..=i32::MAX)`. That check pays off most with enums (chapter 8): add a new variant and the compiler lists
every `match` that needs updating.

TypeScript gets the same check with a trick. Inside the last branch, after every case
has been handled, the value's type has narrowed to `never`, the type with no values.
Assigning it to a `never` variable compiles only while every case is handled:

```ts
type Shape =
  | { kind: "circle"; radius: number }
  | { kind: "square"; side: number }
  | { kind: "triangle"; base: number; height: number };

function area(shape: Shape): number {
  switch (shape.kind) {
    case "circle":
      return Math.PI * shape.radius ** 2;
    case "square":
      return shape.side ** 2;
    default: {
      const unhandled: never = shape; // error: the triangle case is missing
      return unhandled;
    }
  }
}
```

Kotlin gets the check from **sealed** types: a `sealed interface` can only be
implemented in the same package and module, so the compiler knows every case. Inside
each branch, a **smart cast** treats `shape` as the matched type without a cast:

```kotlin
sealed interface Shape
data class Circle(val radius: Double) : Shape
data class Square(val side: Double) : Shape

fun area(shape: Shape): Double =
    when (shape) {
        is Circle -> Math.PI * shape.radius * shape.radius // shape is a Circle here
        is Square -> shape.side * shape.side
    }
```

Add a `Triangle` class and `area` stops compiling: `'when' expression must be
exhaustive. Add the 'is Triangle' branch or an 'else' branch.`

**Rust's `if let` guards** (Rust 1.95) let a match arm run only when a second pattern
also matches. The guessing-game solution uses one to treat a line as a guess only if it
parses as a number:

```rust
match text {
    "" => continue,
    _ if let Ok(guess) = text.parse::<i64>() => { /* use guess */ }
    _ => println!("{text}: not a number"),
}
```

**Rust's `let ... else`** is the opposite: bind the pattern, or leave the block.

```rust
let Ok(n) = text.parse::<i32>() else {
    continue; // the else block must leave: continue, break, return, or panic
};
```

**Python's capture trap.** In a `match` statement, a bare name is a *capture pattern*:
it matches anything and assigns it to that name. It is never a comparison:

```python
QUIT = "quit"
match command:
    case "go":
        ...
    case QUIT:  # matches every command, and sets QUIT to it
        ...
```

If a capture case comes before other cases, Python raises a `SyntaxError` (`name
capture 'QUIT' makes remaining patterns unreachable`). As the last case, it silently
matches everything. Compare against a constant with a dotted name such as
`Command.QUIT`, or a literal such as `"quit"`.

---

## 4.7 Why each language looks the way it does

- **C** treats `switch` as a jump to a label: the compiler often builds a table of
  addresses and jumps straight to the right `case`. Labels do not end anything, so
  execution falls through to the next one. `goto` stays for jumping to shared cleanup
  code at the end of a function, a common style in the Linux kernel.
- **C++** inherited C's statements and added safer forms on top: range-based `for`,
  `if` and `switch` with an initializer, `[[fallthrough]]`, and structured bindings.
  Pattern matching has been proposed several times but is not in the standard yet.
- **Rust** takes `match` and exhaustiveness checking from the ML family of languages
  (OCaml, Haskell). Making `if`, `match`, and blocks into expressions removes the need
  for `?:` and many temporary variables. Braces are required, parentheses are not.
- **Go** keeps one loop keyword and makes `switch` stop after each case, so the common
  case needs no `break`. Its rule about braces on the same line comes from its
  automatic semicolons. It keeps `goto`, but restricted.
- **Python** uses indentation as syntax. It went without any `switch` for 30 years;
  `match` arrived in Python 3.10 (PEP 634) as full structural pattern matching rather
  than a C-style `switch`.
- **JavaScript** copied C and Java's statements, fallthrough included. It added
  `for...of` in 2015 because `for...in` walks property names, not values.
- **TypeScript** adds no new runtime control flow, but it tracks control flow to
  *narrow* types: inside `case "circle":`, it knows `shape` has a `radius`.
- **Dart** started with a Java-style `switch`, but made falling out of a non-empty case
  a compile error instead of silent fallthrough. Dart 3 (2023) added patterns,
  records, switch expressions, and sealed classes, which turned `switch` into
  exhaustive pattern matching.
- **Kotlin** replaced Java's `switch` with `when`, which never falls through, takes any
  value or none, and gives a value. Sealed classes bring exhaustiveness checking, and
  smart casts remove the casts Java needed after an `instanceof`. It has labeled
  `break` and `continue` but no `goto`, and no structural patterns: unpacking happens in
  `val (x, y) = ...` declarations instead.

---

## 4.8 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C, C++, JavaScript, TypeScript | Forgetting `break` at the end of a case | Runs into the next case; `-Wextra` warns in C and C++ |
| C, C++ | An `if` without braces, then a second indented line | Only the first line is guarded; `-Wall` warns |
| C, C++ | `for (unsigned i = n - 1; i >= 0; i--)` | Never ends: an unsigned value is always ≥ 0; `-Wextra` warns |
| C, C++ | `switch` on a string | Compile error: cases must be integer constants |
| Rust | Expecting `0..5` to include 5 | It stops at 4; write `0..=5` |
| Rust | Parentheses around an `if` condition | `unused_parens` warning, which `-D warnings` turns into an error |
| Go | `break` in a `switch` meant to stop the loop | Only leaves the `switch`; use a label or `return` |
| Go | `{` on the line after `if` or `for` | Syntax error: Go inserted a semicolon at the line end |
| Python | `case QUIT:` meaning "equals QUIT" | Captures any value into `QUIT` |
| Python | Reading loop `else` as "if the loop was empty" | It runs whenever the loop ended without `break` |
| JavaScript | `for (const i in array)` | `i` is a string key: `"0"`, `"1"`, ... |
| TypeScript | Adding a member to a union type | Old `switch` statements silently ignore it, unless they use the `never` check |
| Dart | A switch expression on an `int` without `_` | Compile error: the cases are not exhaustive |
| Kotlin | `for (i in 0..n)` meaning 0 to n − 1 | `..` includes `n`, so one pass too many; use `0..<n` or `list.indices` |
| Kotlin | `for (i in 5..1)` to count down | Runs zero times; write `5 downTo 1` |
| Kotlin | `return` inside `forEach { }` meaning "skip this one" | Returns from the whole function; use `return@forEach`, or a plain `for` loop with `continue` |
| All | `<=` where `<` was meant in a loop condition | One pass too many: an off-by-one error |

---

## 4.9 Idiom check

**C: one exit path with `goto`.** When a function acquires several resources, C code
often jumps forward to shared cleanup instead of repeating it in every error branch:

```c
int copy_file(const char *src, const char *dst) {
    int result = -1;
    FILE *in = fopen(src, "rb");
    if (in == NULL) {
        goto done;
    }
    FILE *out = fopen(dst, "wb");
    if (out == NULL) {
        goto close_in;  // in is open and must be closed
    }
    int ch;
    while ((ch = fgetc(in)) != EOF) {
        fputc(ch, out);
    }
    result = 0;
    fclose(out);
close_in:
    fclose(in);
done:
    return result;
}
```

Chapter 10 shows how C++ and Rust do this automatically with destructors.

**Rust: loop over items, not indexes.** Write `for x in &v` rather than
`for i in 0..v.len()`, and reach for `match` when you are asking about the shape of a
value. Use `let ... else` to bail out early and keep the main path unindented.

**Go: return early, keep the happy path on the left.** Go code checks for the bad
case, returns, and continues unindented:

```go
n, err := strconv.Atoi(text)
if err != nil {
	return err
}
// use n here, at the outer level
```

**Python: `enumerate`, not `range(len(...))`.** `for i, x in enumerate(items):` is
clearer and avoids indexing mistakes. Use `match` for data with structure and a `dict`
for plain lookup tables.

**JavaScript and TypeScript: `for...of` for arrays**, and in TypeScript a union type
with a `kind` field plus a `never` check, so the compiler catches a missing case.

**Dart: switch expressions** for choosing a value, and `for (final (i, x) in
list.indexed)` when you need the index.

**Kotlin: `when` for anything with more than two branches**, `withIndex()` rather than
counting indexes, and `?: return` or `?: continue` to leave early on `null`, the same
"happy path on the left" shape as Go:

```kotlin
val n = text.toIntOrNull() ?: continue
// use n here, at the outer level
```

---

## 4.10 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Collatz length](exercises/01-collatz/) | `while` loops, 64-bit arithmetic, `if` as an expression |
| 2 | [Number guessing game](exercises/02-guessing-game/) | Reading until a condition, `break`, `continue`, comparing three ways, parsing |
| 3 | [Roman numerals](exercises/03-roman-numerals/) | Nested loops, `switch` and `match` on characters, looking ahead |

```sh
python3 tools/check.py 04                     # your answers, every language
python3 tools/check.py 04 --lang rust,dart    # some languages
python3 tools/check.py 04 --solutions         # the reference answers
```

**Stretch:** rewrite chapter 1's FizzBuzz by matching on the pair
`(n % 3, n % 5)`: in Rust with `match`, in Python with `match`, and in Dart with a
switch expression on a record. Then write it in Go with a `switch` that has no value,
and in Kotlin, whose `when` can compare whole pairs (`0 to 0 ->`) but has no `_` for
one half. Compare which version states the rules most directly.

---

Previous: [Chapter 3: Operators and expressions](../03-operators-and-expressions/) ·
Next: [Chapter 5: Functions and closures](../05-functions-and-closures/)
