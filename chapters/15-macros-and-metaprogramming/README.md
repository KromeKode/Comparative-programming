# Chapter 15: Macros and metaprogramming

Almost every program so far was written by hand, line by line. This chapter is about
code that writes code, which is called **metaprogramming**. A C macro pastes text into
the program before the compiler reads it. A Rust macro takes tokens and hands back new
ones, and `#[derive(Debug)]` writes a whole method from a struct's fields. C++ and Rust
can run ordinary functions inside the compiler and build the answers into the program.
Python can do all of this as the program runs, because a class, a function, and the call
stack are objects that code can read and change. The reasons are always the same: to
write a list once and have every use of it follow, to stop writing the same few methods
for every type, and to make code that sees its own names and source text, such as a test
runner. The costs are the same too: code nobody wrote is code nobody reads, and an error
inside it can be hard to trace back. C, C++, Rust, and Python get most of the chapter.
Go, JavaScript, TypeScript, Dart, and Kotlin share section 15.8.

**Languages:** C, C++, Rust, and Python in full; Go, JavaScript, TypeScript, Dart, and
Kotlin together in section 15.8 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| What a text macro does, and the mistakes it lets through | Modules, packages, builds (chapter 18), FFI and interop (chapter 23) |
| Writing a list once and generating every use of it | Files, JSON, and the CLI (chapter 21), SQLite from code (chapter 34) |
| Deriving printing, comparing, and copying from a type's fields | Testing basics (chapter 19), REST CRUD API (chapter 38) |
| Running code while compiling, and checking it with `static_assert` | Complexity and benchmarking (chapter 24), number theory (chapter 41) |
| Reflection and decorators: code that reads its own names and types as it runs | Testing basics (chapter 19), debugging and profiling (chapter 20) |

---

## 15.1 The concept: code that writes code

A metaprogram is a program whose input or output is a program. Two questions sort the
kinds that this chapter covers: **when** the code runs, and **what** it works on.

```text
source text ──► preprocessor ──► compiler ─────────────────► program ──► running program
                │                │                                       │
                C and C++        Rust macro_rules! and derive            Python decorators,
                macros paste     C++ templates and constexpr             metaclasses, inspect
                text             Rust const fn                           Go reflect, kotlin-reflect

                before           while compiling                         while the program runs
                compiling
```

- **Before compiling: text.** C's preprocessor is a separate step that reads the source
  and writes out new source. It knows tokens, such as names, numbers, and commas, but
  not types, scope, or which comma belongs to which expression, so a macro can paste in
  text that means something other than it seems to.
- **While compiling: syntax and types.** A Rust macro receives tokens that it matches
  against patterns, such as "an expression" or "a type", and its output is checked like
  any other code. A derive macro is a function that the compiler runs on a struct's
  definition. C++ templates and `constexpr` functions, and Rust's `const fn`, run
  ordinary code inside the compiler, on values and types, and the results become
  constants in the program.
- **While the program runs: live objects.** Python compiles each module to bytecode as
  it is imported, quickly and out of sight, and few tools hook into that step. Most of
  its metaprogramming happens later, because classes and functions are objects that the
  running program can read and change. A decorator is a function that takes a function
  and returns a new one, `@dataclass` reads a class's annotations and writes methods,
  and `inspect` can read the caller's line number. This is **reflection**: a program
  that reads its own structure, such as a type's fields, as it runs. Go, Kotlin, and
  JavaScript have it too, in different amounts.

A fourth kind sits beside these: a separate program that writes source files for the
compiler, such as `go generate` and Dart's `build_runner`. The generated files are
ordinary code that can be read and checked in, which is its main advantage.

The trade is the same everywhere. A metaprogram saves writing, and it keeps things in
step that would otherwise drift: an enum and its names, a struct and its printer, a test
and the list of tests. But the code that runs is code nobody wrote. Errors in it point
into the expansion, debuggers and editors see it differently from the source, and the
earlier the metaprogram runs, the less it knows about the program it is changing.

---

## 15.2 What each language gives you

| Language | Macros | Code that runs while compiling | Reflection as the program runs | Other code generation |
| --- | --- | --- | --- | --- |
| C | The preprocessor: `#define`, `#`, `##`, `__VA_ARGS__`, `#if` | Constant expressions, `_Static_assert`; C23 adds `constexpr` objects | None | Programs that write `.c` files |
| C++ | C's preprocessor | Templates, `constexpr`, `consteval`, `if constexpr`, `static_assert`; C++26 adds reflection, `^^T` | `typeid` names a type, and nothing lists its members | The same |
| Rust | `macro_rules!`, and procedural macros: derive, attribute, and function-like | `const fn`, `const` items, `const` assertions | `type_name` and `Any`, nothing that lists fields | Build scripts, `build.rs` |
| Go | None | Constants only | `reflect`, with struct tags | `go generate`, with tools such as `stringer` |
| Python | None | Constant folding only | Everything: `inspect`, `dataclasses.fields`, `__annotations__`, decorators, metaclasses | Rarely needed |
| JavaScript | None | None | `Object.keys`, `Reflect`, and `Proxy` | Bundlers and compilers, such as Babel |
| TypeScript | None | The type system: `keyof`, `typeof`, and mapped types compute types from types | Only JavaScript's, since types are erased; decorators | The same |
| Dart | None; the planned macros were cancelled in 2025 | `const` values and constructors | `dart:mirrors`, but only under `dart run`, not in compiled programs | `build_runner`, with packages such as `json_serializable` |
| Kotlin | None | `const val` | `kotlin-reflect`: classes, constructors, properties, annotations | KSP, and compiler plugins such as kotlinx.serialization |

Read the table by column. C and C++ are the only languages here with text macros, and
have almost nothing that runs late, so their metaprogramming happens before the program
exists. Rust replaces text macros with macros on tokens and keeps no type information at
run time, so its metaprogramming is all in the compiler. Python is the mirror image of
C: nothing before the program starts, and everything while it runs. The five languages
of section 15.8 lean on reflection (Go, JavaScript, Kotlin), on the type checker
(TypeScript), or on generated files (Go, Dart, Kotlin), and of those five, only Kotlin
lets plugins extend its compiler.

---

## 15.3 Macros that paste text

Each version defines a constant, a squaring function or macro, and a `SHOW` that prints
an expression's text, its value, and the line it is on. Then it squares a call that
counts how often it runs, and switches some code on or off while building.

**C**

```c
#include <stdio.h>

// An object-like macro. Before the compiler sees the code, the preprocessor
// pastes 3 wherever the name LIMIT appears.
#define LIMIT 3

// Function-like macros paste in the text of their arguments, not the values.
#define SQUARE_BAD(x) x * x
#define SQUARE(x) ((x) * (x))

// #expr turns the argument's text into a string literal, and __LINE__ is the
// line the macro is used on, so SHOW can print the code it was given.
#define SHOW(expr) printf("line %d: %s = %d\n", __LINE__, #expr, (expr))

// ## joins two tokens into one. Each use of DEFINE_COUNTER makes a variable
// and a function, with names built from the one it is given.
#define DEFINE_COUNTER(name)     \
    static int name##_count = 0; \
    static int bump_##name(void) { return ++name##_count; }

DEFINE_COUNTER(clicks)
DEFINE_COUNTER(views)

// __VA_ARGS__ stands for every argument after the format.
#define LOG(format, ...) printf("log: " format "\n", __VA_ARGS__)

// The build can set this one: cc -DVERBOSE=1 main.c
#ifndef VERBOSE
#define VERBOSE 0
#endif

int main(void) {
    SHOW(LIMIT * 2);
    SHOW(SQUARE_BAD(1 + 2));  // becomes 1 + 2 * 1 + 2
    SHOW(SQUARE(1 + 2));      // becomes ((1 + 2) * (1 + 2))

    // The argument is pasted in twice, so the call in it runs twice.
    SHOW(SQUARE(bump_clicks()));
    SHOW(clicks_count);

    bump_views();
    LOG("%d clicks, %d view", clicks_count, views_count);

#if VERBOSE
    LOG("built with VERBOSE=%d", VERBOSE);
#else
    printf("VERBOSE is off, so the line that logs it was never compiled\n");
#endif
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <source_location>
#include <string_view>

// C++ has C's preprocessor, but constants, functions, and templates do most
// of its jobs, and they follow the rules for types, scope, and evaluation.
constexpr int limit = 3;

// auto in a parameter (C++20) makes square a template: one copy per type.
constexpr auto square(auto x) { return x * x; }

int clicks = 0;
int bump_clicks() { return ++clicks; }

// As a default argument, std::source_location::current() is the place the
// call came from. Only the expression's text still needs a macro.
template <typename T>
void show(std::string_view text, const T& value,
          std::source_location where = std::source_location::current()) {
    std::cout << "line " << where.line() << ": " << text << " = " << value << "\n";
}

#define SHOW(expr) show(#expr, (expr))

// A variadic template takes any number of arguments of any types, and the
// fold expression prints each one. No format string can get out of step.
template <typename... Parts>
void log_line(const Parts&... parts) {
    std::cout << "log:";
    ((std::cout << " " << parts), ...);
    std::cout << "\n";
}

#ifndef VERBOSE
#define VERBOSE 0
#endif

constexpr bool verbose = VERBOSE;

int main() {
    SHOW(limit * 2);
    SHOW(square(1 + 2));
    SHOW(square(1.5));

    // square is a function, so its argument runs once.
    SHOW(square(bump_clicks()));
    SHOW(clicks);
    log_line("clicks =", clicks, "| limit =", limit);

    // if constexpr picks a branch while compiling. Unlike #if, the branch it
    // drops must still be valid code, since outside a template it is checked.
    if constexpr (verbose) {
        log_line("built with VERBOSE =", VERBOSE);
    } else {
        std::cout << "verbose is off, and the compiler dropped the other branch\n";
    }
}
```

**Rust**

```rust
use std::cell::Cell;

// macro_rules! defines a macro by example. Each rule matches the tokens the
// macro is given and says what code to put in their place. $x:expr matches
// one whole expression, and it stays one expression when it is pasted in.
macro_rules! square {
    ($x:expr) => {
        $x * $x
    };
}

// This one runs its argument once, in a block of its own.
macro_rules! square_once {
    ($x:expr) => {{
        let x = $x;
        x * x
    }};
}

// stringify! turns tokens into a string, as # does in C, and line! is the
// line the macro was used on.
macro_rules! show {
    ($e:expr) => {
        println!("line {}: {} = {}", line!(), stringify!($e), $e)
    };
}

// Macros are hygienic: the tmp below belongs to the macro, so it can't clash
// with a tmp of the caller's, even one passed in as $a.
macro_rules! swap {
    ($a:ident, $b:ident) => {
        let tmp = $a;
        $a = $b;
        $b = tmp;
    };
}

const LIMIT: i32 = 3;

fn main() {
    show!(LIMIT * 2);
    show!(square!(1 + 2));

    let clicks = Cell::new(0);
    let bump = || {
        clicks.set(clicks.get() + 1);
        clicks.get()
    };
    // The argument is pasted in twice, so the call in it runs twice.
    show!(square!(bump()));
    show!(square_once!(bump()));
    show!(clicks.get());

    let mut tmp = 1;
    let mut other = 2;
    swap!(tmp, other);
    println!("after swap!(tmp, other): tmp = {tmp}, other = {other}");

    // cfg! asks how the program is being compiled. rustc -O turns
    // debug_assertions off, and with it every debug_assert!.
    if cfg!(debug_assertions) {
        println!("debug_assertions is on, so debug_assert! checks run");
    } else {
        println!("debug_assertions is off, so debug_assert! compiles to nothing");
    }
}
```

**Python**

```python
import inspect
from typing import Final

# Python has no macros. A constant is an ordinary name, and Final asks the
# type checker to refuse any later assignment to it.
LIMIT: Final = 3


def square(x: int) -> int:
    return x * x


clicks = 0


def bump_clicks() -> int:
    global clicks
    clicks += 1
    return clicks


def log(*parts: object) -> None:
    # A running program can look at its own call stack. stack()[1] is the
    # caller's frame, which knows the line it is on.
    caller = inspect.stack()[1]
    print(f"line {caller.lineno}: log:", *parts)


def main() -> None:
    # An f-string with = prints the expression's text as well as its value.
    print(f"{LIMIT * 2 = }")
    print(f"{square(1 + 2) = }")

    # square is a function, so its argument runs once.
    print(f"{square(bump_clicks()) = }")
    print(f"{clicks = }")
    log(clicks, "clicks")

    # __debug__ is Python's one compile-time switch. python3 -O sets it to
    # False, and the compiler then drops this if statement and every assert.
    if __debug__:
        print("__debug__ is on, so assert statements run")
    else:
        print("running under -O, so assert statements were dropped")


if __name__ == "__main__":
    main()
```

The four print:

*C:*

```text
line 33: LIMIT * 2 = 6
line 34: SQUARE_BAD(1 + 2) = 5
line 35: SQUARE(1 + 2) = 9
line 38: SQUARE(bump_clicks()) = 2
line 39: clicks_count = 2
log: 2 clicks, 1 view
VERBOSE is off, so the line that logs it was never compiled
```

*C++:*

```text
line 41: limit * 2 = 6
line 42: square(1 + 2) = 9
line 43: square(1.5) = 2.25
line 46: square(bump_clicks()) = 1
line 47: clicks = 1
log: clicks = 1 | limit = 3
verbose is off, and the compiler dropped the other branch
```

*Rust:*

```text
line 41: LIMIT * 2 = 6
line 42: square!(1 + 2) = 9
line 50: square!(bump()) = 2
line 51: square_once!(bump()) = 9
line 52: clicks.get() = 3
after swap!(tmp, other): tmp = 2, other = 1
debug_assertions is off, so debug_assert! compiles to nothing
```

*Python:*

```text
LIMIT * 2 = 6
square(1 + 2) = 9
square(bump_clicks()) = 1
clicks = 1
line 37: log: 1 clicks
__debug__ is on, so assert statements run
```

**C: text in, text out.** The preprocessor runs before the compiler and replaces each
macro name with its body, as text. `cc -E -P main.c` shows the result, and line 34
becomes `printf("line %d: %s = %d\n", 34, "SQUARE_BAD(1 + 2)", (1 + 2 * 1 + 2));`. The
argument `1 + 2` was pasted in as it was written, so `x * x` became `1 + 2 * 1 + 2`,
which is 5, and the usual fix is parentheses around every parameter and around the whole
body, as `SQUARE` has. No parentheses fix the next problem. `SQUARE(bump_clicks())`
pastes the call in twice, so it runs twice and gives `1 * 2`, which is 2. With
`SQUARE(i++)`, GCC's `-Wall` catches it, `operation on 'i' may be undefined`, but with a
function call there is no warning at all. `#expr` turns an argument into a string,
before it is expanded, which is why `SHOW` prints `SQUARE_BAD(1 + 2)` and not its
expansion, and `##` glues two tokens into one, so `DEFINE_COUNTER(clicks)` makes
`clicks_count` and `bump_clicks`. `__LINE__` is the line where the macro is used, since
the expansion happens there. In C17, `__VA_ARGS__` needs at least one argument, and
`LOG("hi")` leaves a comma before the `)`:
`error: expected expression before ')' token`. C23's `__VA_OPT__(,)` adds the comma only
when there are arguments. `#if` keeps or drops lines before the compiler sees them, so a
dropped branch can hold anything at all, even `this is not C at all`, and it still
compiles.

A macro also can't have local names of its own. A `SWAP(a, b)` written with
`int tmp = (a); (a) = (b); (b) = tmp;` works until someone calls `SWAP(tmp, other)`.
Then the macro's `tmp` and the caller's are the same name, nothing is swapped, and GCC
and Clang compile it without a warning under `-Wall -Wextra -Werror`.

**C++: the same preprocessor, used less.** C++ keeps every C macro, but most of their
jobs now have tools that follow the language's rules. `constexpr int limit` is a typed
constant with a scope. `constexpr auto square(auto x)` is a function template, so its
argument runs once, and `square(1.5)` works too. `std::source_location::current()`, as a
default argument, is the line of the call, which replaces `__LINE__`. Only the text of
an expression still needs a macro, which is why `SHOW` remains, and since the
preprocessor doesn't know C++'s grammar, it splits arguments at commas it shouldn't:
`SHOW(std::pair<int, int>{1, 2}.first)` gives
`error: macro "SHOW" passed 3 arguments, but takes just 1`. `log_line` is a variadic
template, and the fold expression `((std::cout << " " << parts), ...)` prints every
argument with its own type, so no format string can disagree with the arguments.
`if constexpr` picks a branch while compiling, like `#if`, but it isn't text
substitution: outside a template, the dropped branch must still be valid code, and
calling an undeclared function in it fails with
`'not_declared_anywhere' was not declared in this scope`.

**Rust: macros on tokens.** `macro_rules!` defines a macro by example. Each rule is a
pattern of tokens and the code to write in their place, and `$x:expr` matches one whole
expression. The match is kept as an expression when it is pasted in, so `square!(1 + 2)`
is 9, not 5: the precedence problem is gone. The double evaluation isn't, since
`$x * $x` still uses the expression twice, and `square!(bump())` runs `bump` twice.
`square_once!` fixes that by binding the value to a name inside a block, as a function
would. Rust macros are **hygienic**: a name that the macro's body declares belongs to
the macro, so `swap!(tmp, other)` works even though the macro has its own `tmp`. It
works the other way round too. A macro whose body refers to `total`, meaning the
caller's variable, fails with `error[E0425]: cannot find value 'total' in this scope`,
and the compiler adds that a variable of that name exists but
`is not accessible due to macro hygiene`. Every macro call ends in `!`, so the reader
always knows where one is. `stringify!` and `line!` do what `#` and `__LINE__` do in C,
and `cfg!(debug_assertions)` asks how the program is being built. The checker compiles
with `rustc -O`, which turns `debug_assertions` off, and with it every `debug_assert!`.

**Python: no macros needed.** Python has no preprocessor and no macros.
`LIMIT: Final = 3` is an ordinary variable that a type checker such as mypy refuses to
let anything assign to again. An f-string with `=`, as in `f"{square(1 + 2) = }"`,
prints the expression's text and its value, which covers what `SHOW` is for, and a
function can find the line it was called from as it runs, with
`inspect.stack()[1].lineno`. Python's one compile-time switch is `__debug__`, which is
`True` unless Python runs with `-O`. The compiler treats it as a constant: disassembling
`if __debug__: ... else: ...` shows only the first branch in a normal run, and only the
second under `-O`, which also drops every `assert` statement.

---

## 15.4 One list, many uses

Five HTTP status codes, each with a name, a code, and a text, written once. From that
one list, each program makes an enum, a count, a table to loop over, and lookups by code
and by name.

**C**

```c
#include <stdio.h>
#include <string.h>

// An X-macro. The list is written once, as uses of a macro X that isn't
// defined yet. Each use of the list below defines X, expands the list, and
// undefines X again, so each one turns the same entries into different code.
#define STATUSES(X)                    \
    X(OK, 200, "OK")                   \
    X(MOVED, 301, "Moved Permanently") \
    X(NOT_FOUND, 404, "Not Found")     \
    X(TEAPOT, 418, "I'm a teapot")     \
    X(SERVER_ERROR, 500, "Internal Server Error")

// 1. An enum, with each name given its code.
enum status {
#define X(name, code, text) STATUS_##name = code,
    STATUSES(X)
#undef X
};

// 2. A count: 0 +1 +1 +1 +1 +1.
enum {
#define X(name, code, text) +1
    STATUS_COUNT = 0 STATUSES(X)
#undef X
};

// 3. A table with every entry's name as a string.
static const struct {
    enum status status;
    const char *name;
} all_statuses[] = {
#define X(name, code, text) {STATUS_##name, #name},
    STATUSES(X)
#undef X
};

// 4. A switch with a case for every entry, so none can be left out.
static const char *status_text(enum status status) {
    switch (status) {
#define X(name, code, text) \
    case STATUS_##name:     \
        return text;
        STATUSES(X)
#undef X
    }
    return NULL;
}

static void lookup_code(int code) {
    const char *text = status_text((enum status)code);
    printf("lookup %d: %s\n", code, text ? text : "unknown");
}

static void lookup_name(const char *name) {
    for (size_t i = 0; i < STATUS_COUNT; i++) {
        if (strcmp(all_statuses[i].name, name) == 0) {
            printf("lookup %s: %d\n", name, (int)all_statuses[i].status);
            return;
        }
    }
    printf("lookup %s: unknown\n", name);
}

int main(void) {
    printf("%d statuses\n", STATUS_COUNT);
    for (size_t i = 0; i < STATUS_COUNT; i++) {
        enum status status = all_statuses[i].status;
        printf("%d %s: %s\n", (int)status, all_statuses[i].name, status_text(status));
    }
    lookup_code(404);
    lookup_code(999);
    lookup_name("TEAPOT");
    lookup_name("BREWING");
    return 0;
}
```

**C++**

```cpp
#include <array>
#include <iostream>
#include <optional>
#include <string_view>

// The same X-macro as in C. Until C++26 adds reflection, an enum can't list
// its own names, so the list is still the usual way to keep them in step.
#define STATUSES(X)                    \
    X(OK, 200, "OK")                   \
    X(MOVED, 301, "Moved Permanently") \
    X(NOT_FOUND, 404, "Not Found")     \
    X(TEAPOT, 418, "I'm a teapot")     \
    X(SERVER_ERROR, 500, "Internal Server Error")

enum class Status {
#define X(name, code, text) name = code,
    STATUSES(X)
#undef X
};

struct StatusInfo {
    Status status;
    std::string_view name;
    std::string_view text;

    int code() const { return static_cast<int>(status); }
};

// One table built from the list. The rest of the program is ordinary C++.
constexpr std::array all_statuses{
#define X(name, code, text) StatusInfo{Status::name, #name, text},
    STATUSES(X)
#undef X
};

constexpr std::optional<StatusInfo> find_code(int code) {
    for (const StatusInfo& info : all_statuses) {
        if (static_cast<int>(info.status) == code) return info;
    }
    return std::nullopt;
}

constexpr std::optional<StatusInfo> find_name(std::string_view name) {
    for (const StatusInfo& info : all_statuses) {
        if (info.name == name) return info;
    }
    return std::nullopt;
}

// The lookups are constexpr, so the compiler can run them and check the
// answer before the program exists (section 15.6).
static_assert(find_code(418)->name == "TEAPOT");

int main() {
    std::cout << all_statuses.size() << " statuses\n";
    for (const StatusInfo& info : all_statuses) {
        std::cout << info.code() << " " << info.name << ": " << info.text << "\n";
    }
    for (int code : {404, 999}) {
        auto info = find_code(code);
        std::cout << "lookup " << code << ": " << (info ? info->text : "unknown") << "\n";
    }
    for (std::string_view name : {"TEAPOT", "BREWING"}) {
        auto info = find_name(name);
        std::cout << "lookup " << name << ": ";
        if (info) {
            std::cout << info->code() << "\n";
        } else {
            std::cout << "unknown\n";
        }
    }
}
```

**Rust**

```rust
// A macro that takes the whole list. $( ... );* matches the entries one by
// one, and each $( ... )* in the output repeats once for every entry.
macro_rules! statuses {
    ($($name:ident = $code:literal, $text:literal;)*) => {
        #[derive(Clone, Copy)]
        enum Status {
            $($name = $code,)*
        }

        impl Status {
            const ALL: &[Status] = &[$(Status::$name,)*];

            fn code(self) -> u16 {
                self as u16
            }

            fn name(self) -> &'static str {
                match self {
                    $(Status::$name => stringify!($name),)*
                }
            }

            fn text(self) -> &'static str {
                match self {
                    $(Status::$name => $text,)*
                }
            }

            fn from_code(code: u16) -> Option<Status> {
                match code {
                    $($code => Some(Status::$name),)*
                    _ => None,
                }
            }

            fn from_name(name: &str) -> Option<Status> {
                match name {
                    $(stringify!($name) => Some(Status::$name),)*
                    _ => None,
                }
            }
        }
    };
}

statuses! {
    Ok = 200, "OK";
    Moved = 301, "Moved Permanently";
    NotFound = 404, "Not Found";
    Teapot = 418, "I'm a teapot";
    ServerError = 500, "Internal Server Error";
}

fn main() {
    println!("{} statuses", Status::ALL.len());
    for status in Status::ALL {
        println!("{} {}: {}", status.code(), status.name(), status.text());
    }
    for code in [404, 999] {
        let text = Status::from_code(code).map_or("unknown", Status::text);
        println!("lookup {code}: {text}");
    }
    for name in ["Teapot", "Brewing"] {
        match Status::from_name(name) {
            Some(status) => println!("lookup {name}: {}", status.code()),
            None => println!("lookup {name}: unknown"),
        }
    }
}
```

**Python**

```python
from enum import Enum


# No macro is needed. A class statement runs its body as code, and Enum's
# metaclass then turns each name in it into a member that knows its name.
class Status(Enum):
    OK = 200, "OK"
    MOVED = 301, "Moved Permanently"
    NOT_FOUND = 404, "Not Found"
    TEAPOT = 418, "I'm a teapot"
    SERVER_ERROR = 500, "Internal Server Error"

    # Each member's value is unpacked into these arguments.
    def __init__(self, code: int, text: str) -> None:
        self.code = code
        self.text = text


def find_code(code: int) -> Status | None:
    return next((status for status in Status if status.code == code), None)


def main() -> None:
    print(f"{len(Status)} statuses")
    for status in Status:
        print(f"{status.code} {status.name}: {status.text}")
    for code in [404, 999]:
        found = find_code(code)
        print(f"lookup {code}: {found.text if found else 'unknown'}")
    for name in ["TEAPOT", "BREWING"]:
        # Status[name] looks a member up by its name, as the program runs.
        if name in Status.__members__:
            print(f"lookup {name}: {Status[name].code}")
        else:
            print(f"lookup {name}: unknown")


if __name__ == "__main__":
    main()
```

The Rust version's names are in the style of Rust enum variants, so its output differs:

*C, C++ and Python:*

```text
5 statuses
200 OK: OK
301 MOVED: Moved Permanently
404 NOT_FOUND: Not Found
418 TEAPOT: I'm a teapot
500 SERVER_ERROR: Internal Server Error
lookup 404: Not Found
lookup 999: unknown
lookup TEAPOT: 418
lookup BREWING: unknown
```

*Rust:*

```text
5 statuses
200 Ok: OK
301 Moved: Moved Permanently
404 NotFound: Not Found
418 Teapot: I'm a teapot
500 ServerError: Internal Server Error
lookup 404: Not Found
lookup 999: unknown
lookup Teapot: 418
lookup Brewing: unknown
```

**C: the X-macro.** `STATUSES(X)` is the list, written as uses of a macro `X` that
doesn't exist yet. Each use of the list defines `X` to say what one entry turns into,
expands the list, and undefines `X`. The same five lines become enum values, a `+1` for
the count, a table entry with `#name` as a string, and a `case` that returns the text.
Adding a status is one new line, and every use follows. The `switch` written by hand
would have a different safety net: GCC's `-Wswitch`, part of `-Wall`, reports a missing
case, `enumeration value 'TEAPOT' not handled in switch`, but only for a switch on the
enum, and only when the build has warnings on. With the X-macro, there is nothing to
forget. The cost is that the list is now macro arguments, so a stray comma in a text, or
a mistake in one `X`, gives errors that point into an expansion.

**C++: the same list, and checks while compiling.** C++ uses the same X-macro, because
until C++26 an enum can't list its own names. The macro makes a `constexpr std::array`
of `StatusInfo`, and the rest of the program is ordinary C++ over it. The lookups are
`constexpr`, so the compiler can run them, and
`static_assert(find_code(418)->name == "TEAPOT")` is checked before the program exists
(section 15.6). C++26 adds **reflection**, which the committee voted in at its June 2025
meeting: the operator `^^` turns a type into a value that code can inspect while
compiling, and functions in the header `<meta>` list an enum's names or a struct's
members. GCC 16 has it, with `-std=c++26 -freflection`. The checker's GCC 13 doesn't,
and X-macros will be in C and older C++ code for a long time.

**Rust: a macro that takes the list.** `statuses!` takes the whole list as its input. In
the pattern, `$( ... )*` matches each entry, and in the output, each `$( ... )*` repeats
once for every entry, so one rule writes the enum, `ALL`, and five methods, each with a
`match` that covers every variant. A mistake in the list gets an error about the list: a
code used twice gives `error[E0081]: discriminant value '404' assigned more than once`,
and a missing comma gives `no rules expected '"Not Found"'`. Seeing what a macro expands
to takes a tool. `rustc -Zunpretty=expanded` prints it, but
`the option 'Z' is only accepted on the nightly compiler`, and most people use the
`cargo expand` tool, which uses a nightly compiler behind the scenes.

**Python: a class that runs.** Python needs no macro, because a class statement runs its
body as code, and the class's **metaclass**, the class of the class, can then do what it
likes with the names it finds. `Enum`'s metaclass, `EnumType`, turns each name into a
member that knows its name, and since each value here is a tuple, `__init__` unpacks it
into `code` and `text`. Looking up by name is built in, `Status["TEAPOT"]`, and so is
looking up by value: `Status((404, "Not Found"))` is `Status.NOT_FOUND`, and
`Status(999)` raises `ValueError: 999 is not a valid Status`. The example looks up by
code with `next` over the members, since a value here is a pair. Two members with the
same value don't cause an error. The second becomes an alias of the first, so it
disappears from the loop, unless the class has the `@unique` decorator, which raises
`ValueError: duplicate values found in <enum 'Status'>: FINE -> OK`.

---

## 15.5 Code derived from a type

A `Book` has a title, an author, a year, and whether it is in print. Each program prints
a book with its field names, copies it, compares the copy, makes a reissue with a
different year, orders the two, and counts the distinct books. None of it is written
field by field.

**C**

```c
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// The fields are listed once. Each entry gives a type, a name, and a kind,
// and the kind picks the helper functions below that print and compare it.
#define BOOK_FIELDS(X)              \
    X(const char *, title, string)  \
    X(const char *, author, string) \
    X(int, year, int)               \
    X(bool, in_print, bool)

struct book {
#define X(type, name, kind) type name;
    BOOK_FIELDS(X)
#undef X
};

static void print_string(const char *s) { printf("\"%s\"", s); }
static void print_int(int n) { printf("%d", n); }
static void print_bool(bool b) { printf("%s", b ? "true" : "false"); }

static int compare_string(const char *a, const char *b) { return strcmp(a, b); }
static int compare_int(int a, int b) { return (a > b) - (a < b); }
static int compare_bool(bool a, bool b) { return (int)a - (int)b; }

static const char *const book_fields[] = {
#define X(type, name, kind) #name,
    BOOK_FIELDS(X)
#undef X
};

// Prints a book the way C would write it, as a compound literal.
static void print_book(const struct book *book) {
    const char *sep = "";
    printf("(struct book){");
#define X(type, name, kind)         \
    printf("%s." #name " = ", sep); \
    print_##kind(book->name);       \
    sep = ", ";
    BOOK_FIELDS(X)
#undef X
    printf("}\n");
}

// Compares field by field, in the order of the list, as a dictionary does.
static int compare_books(const struct book *a, const struct book *b) {
    int order = 0;
#define X(type, name, kind) \
    if ((order = compare_##kind(a->name, b->name)) != 0) return order;
    BOOK_FIELDS(X)
#undef X
    return 0;
}

static bool books_equal(const struct book *a, const struct book *b) {
    return compare_books(a, b) == 0;
}

int main(void) {
    printf("fields:");
    for (size_t i = 0; i < sizeof book_fields / sizeof book_fields[0]; i++) {
        printf(" %s", book_fields[i]);
    }
    printf("\n");

    struct book dune = {"Dune", "Frank Herbert", 1965, true};
    printf("dune: ");
    print_book(&dune);

    struct book copy = dune;  // assignment copies every field
    printf("copy == dune: %s\n", books_equal(&copy, &dune) ? "true" : "false");

    struct book reissue = dune;
    reissue.year = 1966;
    printf("reissue: ");
    print_book(&reissue);
    printf("reissue == dune: %s\n", books_equal(&reissue, &dune) ? "true" : "false");
    printf("dune < reissue: %s\n", compare_books(&dune, &reissue) < 0 ? "true" : "false");

    // C has no sets, so count the books that differ from every earlier one.
    struct book books[] = {dune, copy, reissue};
    int distinct = 0;
    for (size_t i = 0; i < 3; i++) {
        bool seen = false;
        for (size_t j = 0; j < i; j++) seen = seen || books_equal(&books[i], &books[j]);
        distinct += !seen;
    }
    printf("distinct books: %d\n", distinct);
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <set>
#include <string>

struct Book {
    std::string title;
    std::string author;
    int year;
    bool in_print;

    // = default asks the compiler to write these. == compares every field,
    // and <=> compares them in order, which gives <, <=, >, and >= too.
    bool operator==(const Book&) const = default;
    auto operator<=>(const Book&) const = default;
};

// C++20 can't list a struct's fields, so printing is written by hand, and
// nothing checks that it keeps up when a field is added.
std::ostream& operator<<(std::ostream& out, const Book& book) {
    return out << "Book{.title = \"" << book.title << "\", .author = \"" << book.author
               << "\", .year = " << book.year << ", .in_print = " << book.in_print << "}";
}

int main() {
    std::cout << std::boolalpha;
    Book dune{"Dune", "Frank Herbert", 1965, true};
    std::cout << "dune: " << dune << "\n";

    Book copy = dune;
    std::cout << "copy == dune: " << (copy == dune) << "\n";

    Book reissue = dune;
    reissue.year = 1966;
    std::cout << "reissue: " << reissue << "\n";
    std::cout << "reissue == dune: " << (reissue == dune) << "\n";
    std::cout << "dune < reissue: " << (dune < reissue) << "\n";

    // std::set keeps one of each, using the < that <=> gave Book.
    std::set<Book> distinct{dune, copy, reissue};
    std::cout << "distinct books: " << distinct.size() << "\n";
}
```

**Rust**

```rust
use std::collections::HashSet;

// A macro that takes a whole struct definition, writes it back out as it
// was, and adds a list of the field names. $(#[$meta:meta])* passes the
// attributes through, so the derive below still applies.
macro_rules! record {
    ($(#[$meta:meta])* struct $name:ident { $($field:ident: $type:ty),* $(,)? }) => {
        $(#[$meta])*
        struct $name {
            $($field: $type),*
        }

        impl $name {
            const FIELDS: &[&str] = &[$(stringify!($field)),*];
        }
    };
}

record! {
    // derive asks the compiler to write each trait's methods from the fields:
    // Debug prints them, PartialEq and Eq compare every field, PartialOrd and
    // Ord compare them in order, Hash lets a HashSet use the struct, and
    // Clone copies it.
    #[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
    struct Book {
        title: String,
        author: String,
        year: u32,
        in_print: bool,
    }
}

fn main() {
    println!("fields: {}", Book::FIELDS.join(" "));

    let dune = Book {
        title: "Dune".to_string(),
        author: "Frank Herbert".to_string(),
        year: 1965,
        in_print: true,
    };
    println!("dune: {dune:?}");

    let copy = dune.clone();
    println!("copy == dune: {}", copy == dune);

    // ..dune.clone() fills in every field that isn't named.
    let reissue = Book {
        year: 1966,
        ..dune.clone()
    };
    println!("reissue: {reissue:?}");
    println!("reissue == dune: {}", reissue == dune);
    println!("dune < reissue: {}", dune < reissue);

    let distinct: HashSet<Book> = [dune, copy, reissue].into_iter().collect();
    println!("distinct books: {}", distinct.len());
}
```

**Python**

```python
from dataclasses import dataclass, fields, replace


# @dataclass reads the annotated names in the class body when the class is
# created, and writes __init__, __repr__, and __eq__ from them. order=True
# adds < and the rest, comparing the fields in order. frozen=True makes the
# fields read-only, which also gives the class a __hash__ for sets.
@dataclass(frozen=True, order=True)
class Book:
    title: str
    author: str
    year: int
    in_print: bool


def main() -> None:
    print("fields:", " ".join(field.name for field in fields(Book)))

    dune = Book("Dune", "Frank Herbert", 1965, True)
    print(f"dune: {dune}")

    copy = replace(dune)
    print(f"copy == dune: {copy == dune}")

    # replace makes a new Book with the fields it names changed.
    reissue = replace(dune, year=1966)
    print(f"reissue: {reissue}")
    print(f"reissue == dune: {reissue == dune}")
    print(f"dune < reissue: {dune < reissue}")

    print(f"distinct books: {len({dune, copy, reissue})}")


if __name__ == "__main__":
    main()
```

Each prints its own form of the record:

*C:*

```text
fields: title author year in_print
dune: (struct book){.title = "Dune", .author = "Frank Herbert", .year = 1965, .in_print = true}
copy == dune: true
reissue: (struct book){.title = "Dune", .author = "Frank Herbert", .year = 1966, .in_print = true}
reissue == dune: false
dune < reissue: true
distinct books: 2
```

*C++:*

```text
dune: Book{.title = "Dune", .author = "Frank Herbert", .year = 1965, .in_print = true}
copy == dune: true
reissue: Book{.title = "Dune", .author = "Frank Herbert", .year = 1966, .in_print = true}
reissue == dune: false
dune < reissue: true
distinct books: 2
```

*Rust:*

```text
fields: title author year in_print
dune: Book { title: "Dune", author: "Frank Herbert", year: 1965, in_print: true }
copy == dune: true
reissue: Book { title: "Dune", author: "Frank Herbert", year: 1966, in_print: true }
reissue == dune: false
dune < reissue: true
distinct books: 2
```

*Python:*

```text
fields: title author year in_print
dune: Book(title='Dune', author='Frank Herbert', year=1965, in_print=True)
copy == dune: True
reissue: Book(title='Dune', author='Frank Herbert', year=1966, in_print=True)
reissue == dune: False
dune < reissue: True
distinct books: 2
```

**C: one more X-macro.** C has no way to ask a struct for its fields, so the list comes
first, and the struct is made from it. Each entry has a type, a name, and a kind, and
the kind picks the helpers, so `print_##kind` becomes `print_string` or `print_int`.
Assignment copies a struct, but `==` doesn't compare one
(`invalid operands to binary ==`), so `books_equal` is written in terms of
`compare_books`. A reissue is a copy with one field changed. C has no sets, so the
distinct count compares each book with the ones before it.

**C++: `= default`.** `bool operator==(const Book&) const = default;` asks the compiler
to write `==` from the fields, and `auto operator<=>(const Book&) const = default;`
writes the three-way comparison, which compares the fields in order and gives `<`, `<=`,
`>`, and `>=` with it. `std::set` uses `<`, so the set works too. The compiler writes
these only if every field can do the same. A member of a type with no `<=>` makes the
defaulted one deleted, and a use of it fails with `use of deleted function`, a note that
it `is implicitly deleted because the default definition would be ill-formed`, and
`no match for 'operator<=>'` for the member. Printing is still written by hand, since
C++20 can't list a struct's fields, and nothing notices when a new field is left out of
it.

**Rust: derive.** `#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]` asks
the compiler to write each trait's methods from the fields: `Debug` prints them,
`PartialEq` and `Eq` compare every field, `PartialOrd` and `Ord` compare them in order,
and `Hash` lets a `HashSet` hold books. A derive fails when a field can't do the same.
With an `f64` field, deriving `Eq` and `Hash` gives
`error[E0277]: the trait bound 'f64: Eq' is not satisfied`, and the same for `Hash`,
because a float isn't equal to itself when it is NaN. Deriving `PartialOrd` without
`PartialEq` gives `can't compare 'Point' with 'Point'`. The `record!` macro around the
struct shows that a `macro_rules!` macro can take a whole item: it writes the struct
back out as it was, with its attributes passed through by `$(#[$meta:meta])*`, and adds
`FIELDS`, the names as strings. `Book { year: 1966, ..dune.clone() }` fills in every
field it doesn't name from another book.

The derives themselves are **procedural macros**: Rust functions that the compiler runs
on a struct's tokens, and whose output tokens become new code. The standard ones are
built into the compiler, and a library such as serde writes its own,
`#[derive(Serialize)]`, as a separate crate that the compiler loads as a plugin. Cargo
usually does the building, but `rustc --crate-type proc-macro` is enough. A derive of
about 30 lines, `FieldNames`, that walks the tokens of a struct and writes a
`field_names` method, compiles to a shared library, `libfield_names.so`, and
`#[derive(FieldNames)]` on a book then gives `["title", "year"]`. The one surprise is
that, without Cargo, the macro's file needs `extern crate proc_macro;`, or the import
fails with `error[E0432]: unresolved import 'proc_macro'`.

**Python: decorators that write methods.** `@dataclass` runs when the class is created.
It reads the annotations in the class body, `title: str` and the rest, and writes
`__init__`, `__repr__`, and `__eq__`. `order=True` adds `<` and the others, comparing
the fields in order, and `frozen=True` makes the fields read-only, which also gives the
class a `__hash__`, so that books can go in a set. Without `frozen`, `__hash__` is
`None`, and `{book}` raises `TypeError: unhashable type: 'Book'`, because something that
can change shouldn't be a set member. It really writes code. In Python 3.13,
`dataclasses.py` builds each method as source text and runs it with `exec`, so
`Book.__init__.__code__.co_filename` is `'<string>'`. It checks what it can, too: a list
as a default value raises
`ValueError: mutable default <class 'list'> for field tags is not allowed: use default_factory`,
since every book would share the one list. `fields(Book)` lists the fields as the
program runs, and `replace` makes a changed copy.

---

## 15.6 Computing while compiling

CRC-32 is a checksum, used by ZIP files, PNG images, and Ethernet, and the fast way to
compute it uses a table of 256 numbers that depends only on a constant, the polynomial
`0xEDB88320`. Each version builds the table, checks it, and computes the checksums of
four texts.

**C**

```c
#include <stdint.h>
#include <stdio.h>

// CRC-32 runs each byte through a table of 256 numbers. Entry i is i after
// eight rounds of shifting right, each round also XOR-ing in the polynomial
// when the bit shifted out was 1.
#define POLY UINT32_C(0xEDB88320)
#define CRC_ROUND(c) (((c) >> 1) ^ (POLY & (UINT32_C(0) - ((c) & 1))))
#define CRC_ROUND_4(c) CRC_ROUND(CRC_ROUND(CRC_ROUND(CRC_ROUND(c))))
#define CRC_ENTRY(i) CRC_ROUND_4(CRC_ROUND_4((uint32_t)(i)))

// The preprocessor can't loop, so these macros paste out 256 entries,
// 4 at a time. Each entry is a constant expression for the compiler to work
// out, which makes the whole table a constant built into the program.
#define ENTRIES_4(i) CRC_ENTRY(i), CRC_ENTRY(i + 1), CRC_ENTRY(i + 2), CRC_ENTRY(i + 3)
#define ENTRIES_16(i) ENTRIES_4(i), ENTRIES_4(i + 4), ENTRIES_4(i + 8), ENTRIES_4(i + 12)
#define ENTRIES_64(i) ENTRIES_16(i), ENTRIES_16(i + 16), ENTRIES_16(i + 32), ENTRIES_16(i + 48)

static const uint32_t crc_table[256] = {
    ENTRIES_64(0), ENTRIES_64(64), ENTRIES_64(128), ENTRIES_64(192),
};

// _Static_assert checks a constant expression while compiling.
_Static_assert(CRC_ENTRY(1) == UINT32_C(0x77073096), "CRC table entry 1 is wrong");
_Static_assert(sizeof crc_table / sizeof crc_table[0] == 256, "the table needs 256 entries");

// C has no way to run this function while compiling, so it runs here, when
// the program does.
static uint32_t crc32(const char *text) {
    uint32_t crc = UINT32_MAX;
    for (const unsigned char *p = (const unsigned char *)text; *p; p++) {
        crc = crc_table[(crc ^ *p) & 0xFF] ^ (crc >> 8);
    }
    return ~crc;
}

int main(void) {
    printf("table[1] = %08x\n", (unsigned)crc_table[1]);
    printf("table[255] = %08x\n", (unsigned)crc_table[255]);
    const char *texts[] = {
        "123456789",
        "hello",
        "The quick brown fox jumps over the lazy dog",
        "",
    };
    for (size_t i = 0; i < sizeof texts / sizeof texts[0]; i++) {
        printf("crc32(\"%s\") = %08x\n", texts[i], (unsigned)crc32(texts[i]));
    }
    return 0;
}
```

**C++**

```cpp
#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

// A constexpr function can run while compiling, loops and all, when its
// result is needed as a constant.
constexpr std::array<std::uint32_t, 256> make_crc_table() {
    std::array<std::uint32_t, 256> table{};
    for (std::uint32_t i = 0; i < 256; i++) {
        std::uint32_t c = i;
        for (int round = 0; round < 8; round++) {
            c = (c & 1) ? 0xEDB88320 ^ (c >> 1) : c >> 1;
        }
        table[i] = c;
    }
    return table;
}

// constexpr on a variable means it must be worked out while compiling, so
// the table is built into the program, and nothing builds it at run time.
constexpr auto crc_table = make_crc_table();

constexpr std::uint32_t crc32(std::string_view text) {
    std::uint32_t crc = 0xFFFFFFFF;
    for (unsigned char byte : text) crc = crc_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
    return ~crc;
}

// static_assert stops the build if the condition is false. The compiler
// runs crc32 to find out, so a wrong table can never be compiled.
static_assert(crc_table[1] == 0x77073096);
static_assert(crc32("123456789") == 0xCBF43926);  // the standard check value

// consteval goes further: every call must be worked out while compiling.
consteval std::uint32_t crc32_now(std::string_view text) { return crc32(text); }

int main() {
    std::printf("table[1] = %08x\n", crc_table[1]);
    std::printf("table[255] = %08x\n", crc_table[255]);
    constexpr std::uint32_t check = crc32_now("123456789");
    std::printf("crc32(\"123456789\") = %08x\n", check);

    // The same function also runs as ordinary code, on text that only
    // exists when the program runs.
    for (std::string_view text : {"hello", "The quick brown fox jumps over the lazy dog", ""}) {
        std::printf("crc32(\"%.*s\") = %08x\n", static_cast<int>(text.size()), text.data(),
                    crc32(text));
    }
}
```

**Rust**

```rust
// A const fn can run while compiling. Inside one, loops are written with
// while, because a for loop needs the Iterator trait, which isn't const.
const fn make_crc_table() -> [u32; 256] {
    let mut table = [0u32; 256];
    let mut i = 0;
    while i < 256 {
        let mut c = i as u32;
        let mut round = 0;
        while round < 8 {
            c = if c & 1 == 1 {
                0xEDB8_8320 ^ (c >> 1)
            } else {
                c >> 1
            };
            round += 1;
        }
        table[i] = c;
        i += 1;
    }
    table
}

// A const item is always worked out while compiling.
const CRC_TABLE: [u32; 256] = make_crc_table();

const fn crc32(bytes: &[u8]) -> u32 {
    let mut crc = u32::MAX;
    let mut i = 0;
    while i < bytes.len() {
        crc = CRC_TABLE[((crc ^ bytes[i] as u32) & 0xFF) as usize] ^ (crc >> 8);
        i += 1;
    }
    !crc
}

// An assert in a const block is checked while compiling, so a wrong table
// stops the build.
const _: () = assert!(CRC_TABLE[1] == 0x7707_3096);
const CHECK: u32 = crc32(b"123456789");
const _: () = assert!(CHECK == 0xCBF4_3926); // the standard check value

fn main() {
    println!("table[1] = {:08x}", CRC_TABLE[1]);
    println!("table[255] = {:08x}", CRC_TABLE[255]);
    println!("crc32(\"123456789\") = {CHECK:08x}");

    // The same function also runs as ordinary code, on text that only
    // exists when the program runs.
    for text in ["hello", "The quick brown fox jumps over the lazy dog", ""] {
        println!("crc32(\"{text}\") = {:08x}", crc32(text.as_bytes()));
    }
}
```

**Python**

```python
def make_crc_table() -> list[int]:
    table = []
    for i in range(256):
        c = i
        for _ in range(8):
            c = 0xEDB88320 ^ (c >> 1) if c & 1 else c >> 1
        table.append(c)
    return table


# Python has no compile-time evaluation. Code at the top of a module runs
# when the module is imported, so the table is built each time the program
# starts, before main runs.
CRC_TABLE = make_crc_table()


def crc32(data: bytes) -> int:
    crc = 0xFFFFFFFF
    for byte in data:
        crc = CRC_TABLE[(crc ^ byte) & 0xFF] ^ (crc >> 8)
    return crc ^ 0xFFFFFFFF


# A check at import time is the nearest thing to a static_assert.
assert crc32(b"123456789") == 0xCBF43926, "the CRC table is wrong"


def main() -> None:
    print(f"table[1] = {CRC_TABLE[1]:08x}")
    print(f"table[255] = {CRC_TABLE[255]:08x}")
    for text in [
        "123456789",
        "hello",
        "The quick brown fox jumps over the lazy dog",
        "",
    ]:
        print(f'crc32("{text}") = {crc32(text.encode()):08x}')


if __name__ == "__main__":
    main()
```

All four print the same lines:

*C, C++, Rust and Python:*

```text
table[1] = 77073096
table[255] = 2d02ef8d
crc32("123456789") = cbf43926
crc32("hello") = 3610a686
crc32("The quick brown fox jumps over the lazy dog") = 414fa339
crc32("") = 00000000
```

**C: the preprocessor can't loop.** C has no way to run a function while compiling, but
a constant expression, such as `(c >> 1) ^ (POLY & (0 - (c & 1)))`, is worked out by the
compiler, and `CRC_ENTRY(i)` nests eight rounds of it. The preprocessor has no loops, so
`ENTRIES_64` pastes out 64 entries from 16 groups of 4, and four of those make the
table. It works, and the table is built into the program, but it is a lot of text: the
preprocessed file is about 4.5 MB with the table and 42 KB without it, and still
compiles in about 0.3 seconds. `_Static_assert` checks a constant expression while
compiling. C23 adds `constexpr`, but only for objects, so `constexpr int limit = 3;`
compiles with `-std=c2x`, and a function still can't run while compiling. The `crc32`
function itself runs when the program does.

**C++: `constexpr` functions.** `make_crc_table` is an ordinary function with loops and
an array, marked `constexpr`, which means it can run while compiling when its result is
needed as a constant. `constexpr auto crc_table = make_crc_table();` requires it, so the
table is built by the compiler. `static_assert` stops the build if a condition is false,
and the compiler runs `crc32` to find out. With the wrong polynomial, the build fails
with `static assertion failed` and a note that
`the comparison reduces to '(3233442988 == 1996959894)'`. `consteval` goes further:
every call must be worked out while compiling, so `crc32_now(argv[0])` is refused, with
`'argv' is not a constant expression`. The compiler limits the work it will do, and a
loop of 100 million steps fails with
`'constexpr' loop iteration count exceeds limit of 262144`, and the message names the
option that raises the limit, `-fconstexpr-loop-limit=`.

Before `constexpr`, which arrived in C++11, compile-time code was written with
templates. A `template <int N> struct Factorial` whose `value` is
`N * Factorial<N - 1>::value`, with a specialization for 0, makes `Factorial<10>::value`
equal 3628800. Without the specialization, the compiler instantiates `Factorial<-1>`,
`Factorial<-2>`, and so on, until `template instantiation depth exceeds maximum of 900`.
That style is called **template metaprogramming**, and it is still how C++ computes with
types, such as "the type of the larger of two integers", but `constexpr` is the way to
compute with values.

**Rust: `const fn`.** A `const fn` can run while compiling, and a `const` item is always
worked out there. Inside a `const fn`, a loop is written with `while`, because a `for`
loop calls `Iterator::next`, which isn't `const`:
`error[E0015]: cannot use 'for' loop on 'std::ops::Range<u32>' in constant functions`.
`const _: () = assert!(...)` is Rust's `static_assert`, and with the wrong polynomial it
gives
`error[E0080]: evaluation panicked: assertion failed: CRC_TABLE[1] == 0x7707_3096`. A
very long evaluation has no fixed limit, as in C++. Instead, rustc stops with
`error: constant evaluation is taking a long time`, from a lint that is set to deny by
default, and a program that really needs the time can allow it.

In all three, the table is data in the program. The 256 numbers are in the compiled
files, starting `00000000 96300777 2c610eee`, and `objdump -d` finds no `0xedb88320` in
any of the code, so no code to build the table was ever compiled.

**Python: at import.** Python has no compile-time evaluation beyond folding constant
expressions, so `24 * 60 * 60` is compiled as `86400`, and nothing larger. Code at the
top level of a module runs when the module is imported, so `CRC_TABLE` is built every
time the program starts, which takes about 130 microseconds here. A check at import,
with `assert`, is the nearest thing to a `static_assert`, and it vanishes under `-O`.
For a table that is slow to build, the Python answer is to compute it once and save it
to a file, or to write it into the source with a script.

---

## 15.7 Code that finds itself

A small test runner. Each test is a function with a few checks, the runner finds all the
tests and runs them in order, and a failed check prints its own source text. Writing
each test's name, or each check's text, a second time is what metaprogramming saves.

**C**

```c
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A test returns NULL when it passes, or the text of the check that failed.
// A failed CHECK leaves the test at once.
#define CHECK(expr)                \
    do {                           \
        if (!(expr)) return #expr; \
    } while (0)

#define TEST(name) static const char *test_##name(void)

TEST(addition) {
    CHECK(2 + 2 == 4);
    CHECK(-3 + 3 == 0);
    return NULL;
}

TEST(strings) {
    char word[] = "abc";
    for (char *p = word; *p; p++) *p = (char)toupper((unsigned char)*p);
    CHECK(strcmp(word, "ABC") == 0);
    CHECK(strlen(word) == 3);
    return NULL;
}

TEST(rounding) {
    CHECK(0.5 + 0.25 == 0.75);
    CHECK(0.1 + 0.2 == 0.3);
    return NULL;
}

static int compare_ints(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

TEST(sorting) {
    int numbers[] = {3, 1, 2};
    qsort(numbers, 3, sizeof numbers[0], compare_ints);
    CHECK(numbers[0] == 1 && numbers[1] == 2 && numbers[2] == 3);
    return NULL;
}

// Standard C can't run code before main or find a function by its name, so
// the list of tests is written out. ENTRY turns each name into a string and
// the function's name.
#define ENTRY(name) {#name, test_##name}

static const struct {
    const char *name;
    const char *(*run)(void);
} tests[] = {
    ENTRY(addition),
    ENTRY(strings),
    ENTRY(rounding),
    ENTRY(sorting),
};

int main(void) {
    size_t count = sizeof tests / sizeof tests[0];
    int failed = 0;
    printf("running %zu tests\n", count);
    for (size_t i = 0; i < count; i++) {
        const char *failure = tests[i].run();
        if (failure) {
            printf("FAIL %s: %s\n", tests[i].name, failure);
            failed++;
        } else {
            printf("PASS %s\n", tests[i].name);
        }
    }
    printf("%d passed, %d failed\n", (int)count - failed, failed);
    return 0;  // a real runner would return 1 when a test fails
}
```

**C++**

```cpp
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

struct Test {
    const char* name;
    void (*run)();
};

// A function-local static is made the first time the function is called, so
// the list exists before any test is added to it, whatever the order.
std::vector<Test>& all_tests() {
    static std::vector<Test> tests;
    return tests;
}

// Making a Registrar adds a test to the list.
struct Registrar {
    Registrar(const char* name, void (*run)()) { all_tests().push_back({name, run}); }
};

// TEST declares the function, then defines a global Registrar for it. Global
// objects are made before main runs, so every test adds itself to the list,
// and the body written after TEST(name) becomes the function's body.
#define TEST(name)                                         \
    static void test_##name();                             \
    static Registrar registrar_##name(#name, test_##name); \
    static void test_##name()

struct CheckFailed {
    const char* text;
};

#define CHECK(expr)                            \
    do {                                       \
        if (!(expr)) throw CheckFailed{#expr}; \
    } while (0)

TEST(addition) {
    CHECK(2 + 2 == 4);
    CHECK(-3 + 3 == 0);
}

TEST(strings) {
    std::string word = "abc";
    std::ranges::transform(word, word.begin(), [](unsigned char c) { return std::toupper(c); });
    CHECK(word == "ABC");
    CHECK(word.size() == 3);
}

TEST(rounding) {
    CHECK(0.5 + 0.25 == 0.75);
    CHECK(0.1 + 0.2 == 0.3);
}

TEST(sorting) {
    std::vector numbers{3, 1, 2};
    std::ranges::sort(numbers);
    CHECK((numbers == std::vector{1, 2, 3}));
}

int main() {
    int failed = 0;
    std::cout << "running " << all_tests().size() << " tests\n";
    for (const Test& test : all_tests()) {
        try {
            test.run();
            std::cout << "PASS " << test.name << "\n";
        } catch (const CheckFailed& failure) {
            std::cout << "FAIL " << test.name << ": " << failure.text << "\n";
            failed++;
        }
    }
    std::cout << all_tests().size() - failed << " passed, " << failed << " failed\n";
    return 0;  // a real runner would return 1 when a test fails
}
```

**Rust**

```rust
// A failed check returns from the test at once, with the check's text.
macro_rules! check {
    ($cond:expr) => {
        if !$cond {
            return Err(stringify!($cond));
        }
    };
}

type TestFn = fn() -> Result<(), &'static str>;

// tests! takes ordinary-looking functions, gives each one a Result to
// return, and builds the list of tests from their names.
macro_rules! tests {
    ($(fn $name:ident() $body:block)*) => {
        $(
            fn $name() -> Result<(), &'static str> {
                $body
                Ok(())
            }
        )*

        const TESTS: &[(&str, TestFn)] = &[$((stringify!($name), $name)),*];
    };
}

tests! {
    fn addition() {
        check!(2 + 2 == 4);
        check!(-3 + 3 == 0);
    }

    fn strings() {
        let word = "abc".to_uppercase();
        check!(word == "ABC");
        check!(word.len() == 3);
    }

    fn rounding() {
        check!(0.5 + 0.25 == 0.75);
        check!(0.1 + 0.2 == 0.3);
    }

    fn sorting() {
        let mut numbers = vec![3, 1, 2];
        numbers.sort();
        check!(numbers == [1, 2, 3]);
    }
}

fn main() {
    let mut failed = 0;
    println!("running {} tests", TESTS.len());
    for (name, run) in TESTS {
        match run() {
            Ok(()) => println!("PASS {name}"),
            Err(text) => {
                println!("FAIL {name}: {text}");
                failed += 1;
            }
        }
    }
    println!("{} passed, {failed} failed", TESTS.len() - failed);
    // a real runner would exit with 1 when a test fails
}
```

**Python**

```python
import functools
import traceback
from collections.abc import Callable

TESTS: list[Callable[[], str | None]] = []


# A decorator is a function that takes a function and returns what should
# take its place. @test above a def runs test(that function) once, when the
# def runs, and binds the name to the result.
def test(body: Callable[[], None]) -> Callable[[], str | None]:
    # wraps copies the name and docstring of body onto run.
    @functools.wraps(body)
    def run() -> str | None:
        try:
            body()
        except AssertionError as error:
            # The traceback knows the file and line that failed, and can
            # read that line of source code back.
            line = traceback.extract_tb(error.__traceback__)[-1].line
            return (line or "").removeprefix("assert ")
        return None

    TESTS.append(run)
    return run


@test
def addition() -> None:
    assert 2 + 2 == 4
    assert -3 + 3 == 0


@test
def strings() -> None:
    word = "abc".upper()
    assert word == "ABC"
    assert len(word) == 3


@test
def rounding() -> None:
    assert 0.5 + 0.25 == 0.75
    assert 0.1 + 0.2 == 0.3


@test
def sorting() -> None:
    assert sorted([3, 1, 2]) == [1, 2, 3]


def main() -> None:
    failed = 0
    print(f"running {len(TESTS)} tests")
    for run in TESTS:
        failure = run()
        if failure is None:
            print(f"PASS {run.__name__}")
        else:
            print(f"FAIL {run.__name__}: {failure}")
            failed += 1
    print(f"{len(TESTS) - failed} passed, {failed} failed")
    # a real runner would exit with 1 when a test fails


if __name__ == "__main__":
    main()
```

All four print the same lines:

*C, C++, Rust and Python:*

```text
running 4 tests
PASS addition
PASS strings
FAIL rounding: 0.1 + 0.2 == 0.3
PASS sorting
3 passed, 1 failed
```

**C: a macro for the text, a list for the tests.** `CHECK(expr)` returns `#expr`, the
check's text, when it fails. Its body is wrapped in `do { ... } while (0)` so that
`CHECK(x);` is one statement, semicolon and all. With plain braces,
`if (a > 0) CHECK(a < 10); else ...` fails with `'else' without a previous 'if'`,
because the `;` after the braces ends the `if`. Standard C can't run code before `main`,
or find a function by its name, so the list of tests is written out, and `ENTRY(name)`
pastes the name in twice, as a string and as a function. GCC and Clang have an
extension, `__attribute__((constructor))`, that runs a function before `main`, which a
test framework can use to register its tests, but it isn't standard C.

**C++: objects that register themselves.** C++ can run code before `main`: the
constructors of global objects. `TEST(name)` declares the function, defines a global
`Registrar` whose constructor adds the function to the list, and then starts the
function's definition, so the braces written after `TEST(addition)` become its body.
Google Test and Catch2 work this way. The list itself is a `static` inside `all_tests`,
made the first time the function is called. If it were a global, a test in another file
could be registered before the list was made, since C++ doesn't say in which order
files' globals are made. A failed `CHECK` throws, so the test stops at once.
`CHECK((numbers == std::vector{1, 2, 3}))` needs the extra parentheses: braces don't
hide commas from the preprocessor, and without them, the line fails with
`macro "CHECK" passed 3 arguments, but takes just 1`.

**Rust: a macro around the tests.** `tests!` takes ordinary-looking functions, rewrites
each one to return a `Result`, and builds `TESTS` from their names. `check!` returns
early with `stringify!` of its condition. That shows what `macro_rules!` can do, but
real Rust tests use the built-in attribute `#[test]`. `rustc --test`, or `cargo test`,
then generates a `main` that finds and runs every function marked with it, and
`assert_eq!` prints both sides: `assertion 'left == right' failed`, then
`left: 0.30000000000000004` and `right: 0.3`.

**Python: decorators and tracebacks.** `@test` above a `def` calls `test` with the new
function when the `def` runs, and the name is bound to what `test` returns. The
decorator adds the test to `TESTS` as it goes, so the list builds itself as the module
is imported. `functools.wraps` copies the name of `body` onto `run`, so `run.__name__`
is `"rounding"`, and without it, it would be `"run"`. When an `assert` fails, the
traceback records the file and line, and `traceback.extract_tb` reads that line of
source back, which gives the failing check's text without any macro. One flag breaks the
runner. Under `python3 -O`, every `assert` statement is removed, and all four tests
pass, `4 passed, 0 failed`.

pytest, Python's most used test framework, takes this further. It rewrites the `assert`
statements in test files as it imports them, with an import hook, described in its
source as a `PEP302/PEP451 import hook which rewrites asserts`. The rewritten `assert`
records each value, so a failure prints `assert 0.30000000000000004 == 0.3`. Rewritten
asserts are ordinary code, so they still run under `-O`, and pytest warns that
`assertions not in test modules or plugins will be ignored`.

---

## 15.8 Go, JavaScript, TypeScript, Dart, and Kotlin

None of these five has macros. Here is section 15.5's `Book` in each of them, written
with what they have instead: a compiler that writes some methods (Go, Dart, Kotlin),
reflection (Go, JavaScript, Kotlin), or the type checker (TypeScript).

**Go**

```go
package main

import (
	"cmp"
	"encoding/json"
	"fmt"
	"reflect"
)

// The text in backquotes after each field is a struct tag. The compiler
// keeps it with the type, and code that reads it with reflect, such as
// encoding/json, can use it as the program runs.
type Book struct {
	Title   string `json:"title"`
	Author  string `json:"author"`
	Year    int    `json:"year"`
	InPrint bool   `json:"in_print"`
}

func compareBool(a, b bool) int {
	switch {
	case a == b:
		return 0
	case a:
		return 1
	default:
		return -1
	}
}

// Go writes no ordering for structs, so this compares the fields in turn.
// cmp.Or returns the first result that isn't 0.
func compareBooks(a, b Book) int {
	return cmp.Or(
		cmp.Compare(a.Title, b.Title),
		cmp.Compare(a.Author, b.Author),
		cmp.Compare(a.Year, b.Year),
		compareBool(a.InPrint, b.InPrint),
	)
}

func main() {
	// reflect describes a type as the program runs: its fields, their
	// names, their types, and their tags.
	bookType := reflect.TypeFor[Book]()
	fmt.Print("fields:")
	for i := range bookType.NumField() {
		field := bookType.Field(i)
		fmt.Printf(" %s (%s, tag %q)", field.Name, field.Type, field.Tag.Get("json"))
	}
	fmt.Println()

	dune := Book{"Dune", "Frank Herbert", 1965, true}
	// %+v prints a struct with its field names, which fmt finds with reflect.
	fmt.Printf("dune: %+v\n", dune)

	copied := dune // assignment copies a struct
	fmt.Println("copied == dune:", copied == dune)

	reissue := dune
	reissue.Year = 1966
	fmt.Printf("reissue: %+v\n", reissue)
	fmt.Println("reissue == dune:", reissue == dune)
	fmt.Println("dune < reissue:", compareBooks(dune, reissue) < 0)

	// A struct whose fields can all be compared with == can be a map key.
	distinct := map[Book]bool{dune: true, copied: true, reissue: true}
	fmt.Println("distinct books:", len(distinct))

	text, _ := json.Marshal(dune)
	fmt.Println("json:", string(text))
}
```

**JavaScript**

```js
// A plain object can list its own keys as the program runs, so one function
// can compare any two objects field by field.
function equal(a, b) {
  const keys = Object.keys(a);
  return (
    keys.length === Object.keys(b).length &&
    keys.every((key) => a[key] === b[key])
  );
}

// Compares field by field, in the order the keys were added.
function compare(a, b) {
  for (const key of Object.keys(a)) {
    if (a[key] < b[key]) return -1;
    if (a[key] > b[key]) return 1;
  }
  return 0;
}

const dune = {
  title: "Dune",
  author: "Frank Herbert",
  year: 1965,
  inPrint: true,
};
console.log("fields:", Object.keys(dune).join(" "));
console.log("dune:", dune);

// === asks whether two objects are the same object, not whether they match.
const copy = { ...dune };
console.log("copy === dune:", copy === dune);
console.log("equal(copy, dune):", equal(copy, dune));

// Spreading copies every field, and a field written after it replaces one.
const reissue = { ...dune, year: 1966 };
console.log("reissue:", reissue);
console.log("equal(reissue, dune):", equal(reissue, dune));
console.log("dune < reissue:", compare(dune, reissue) < 0);

// A Set compares objects the way === does, so it keeps all three.
const books = [dune, copy, reissue];
console.log("distinct objects:", new Set(books).size);
console.log(
  "distinct books:",
  new Set(books.map((b) => JSON.stringify(b))).size,
);
console.log("json:", JSON.stringify(dune));
```

**TypeScript**

```ts
// The type exists only for the compiler. When the program runs, a Book is a
// plain object, and its keys come from the object, not from this type.
type Book = {
  readonly title: string;
  readonly author: string;
  readonly year: number;
  readonly inPrint: boolean;
};

// keyof T is the compiler's list of T's keys. Object.keys returns string[],
// since an object can have more keys than its type says.
function keysOf<T extends object>(value: T): (keyof T)[] {
  return Object.keys(value) as (keyof T)[];
}

function equal<T extends object>(a: T, b: T): boolean {
  const keys = keysOf(a);
  return (
    keys.length === keysOf(b).length && keys.every((key) => a[key] === b[key])
  );
}

function compare<T extends object>(a: T, b: T): number {
  for (const key of keysOf(a)) {
    if (a[key] < b[key]) return -1;
    if (a[key] > b[key]) return 1;
  }
  return 0;
}

const dune: Book = {
  title: "Dune",
  author: "Frank Herbert",
  year: 1965,
  inPrint: true,
};
console.log("fields:", keysOf(dune).join(" "));
console.log("dune:", dune);

const copy: Book = { ...dune };
console.log("copy === dune:", copy === dune);
console.log("equal(copy, dune):", equal(copy, dune));

const reissue: Book = { ...dune, year: 1966 };
console.log("reissue:", reissue);
console.log("equal(reissue, dune):", equal(reissue, dune));
console.log("dune < reissue:", compare(dune, reissue) < 0);

const books = [dune, copy, reissue];
console.log("distinct objects:", new Set(books).size);
console.log(
  "distinct books:",
  new Set(books.map((b) => JSON.stringify(b))).size,
);
console.log("json:", JSON.stringify(dune));
```

**Dart**

```dart
// A record type. Records are the one place where Dart writes the code for
// you: two records are == when their fields are, they have a matching
// hashCode, and toString lists the fields.
typedef Book = ({String title, String author, int year, bool inPrint});

int compareBool(bool a, bool b) => a == b ? 0 : (a ? 1 : -1);

// Dart writes no ordering, and has no reflection outside the Dart VM, so
// this compares the fields one by one.
int compareBooks(Book a, Book b) {
  for (final order in [
    a.title.compareTo(b.title),
    a.author.compareTo(b.author),
    a.year.compareTo(b.year),
    compareBool(a.inPrint, b.inPrint),
  ]) {
    if (order != 0) return order;
  }
  return 0;
}

void main() {
  const Book dune = (
    title: 'Dune',
    author: 'Frank Herbert',
    year: 1965,
    inPrint: true,
  );
  print('dune: $dune');

  const Book copy = (
    title: 'Dune',
    author: 'Frank Herbert',
    year: 1965,
    inPrint: true,
  );
  print('copy == dune: ${copy == dune}');

  // A record has no copy method, so a changed copy names every field.
  final Book reissue = (
    title: dune.title,
    author: dune.author,
    year: 1966,
    inPrint: dune.inPrint,
  );
  print('reissue: $reissue');
  print('reissue == dune: ${reissue == dune}');
  print('dune < reissue: ${compareBooks(dune, reissue) < 0}');

  print('distinct books: ${[dune, copy, reissue].toSet().length}');
}
```

**Kotlin**

```kotlin
import kotlin.reflect.full.primaryConstructor

// A data class. The compiler writes toString, equals, hashCode, and copy
// from the properties in its primary constructor.
data class Book(val title: String, val author: String, val year: Int, val inPrint: Boolean)

// Book::title is a property reference, which works as a function from a
// Book to its title. compareBy tries each one in turn.
val bookOrder = compareBy(Book::title, Book::author, Book::year, Book::inPrint)

fun main() {
    // Reflection, from the kotlin-reflect library, can list them as the
    // program runs.
    val fields = Book::class.primaryConstructor!!.parameters.map { it.name }
    println("fields: ${fields.joinToString(" ")}")

    val dune = Book("Dune", "Frank Herbert", 1965, true)
    println("dune: $dune")

    val copy = dune.copy()
    println("copy == dune: ${copy == dune}")

    // copy makes a new Book with the properties it names changed.
    val reissue = dune.copy(year = 1966)
    println("reissue: $reissue")
    println("reissue == dune: ${reissue == dune}")
    println("dune < reissue: ${bookOrder.compare(dune, reissue) < 0}")

    println("distinct books: ${setOf(dune, copy, reissue).size}")
}
```

Each prints its own form:

*Go:*

```text
fields: Title (string, tag "title") Author (string, tag "author") Year (int, tag "year") InPrint (bool, tag "in_print")
dune: {Title:Dune Author:Frank Herbert Year:1965 InPrint:true}
copied == dune: true
reissue: {Title:Dune Author:Frank Herbert Year:1966 InPrint:true}
reissue == dune: false
dune < reissue: true
distinct books: 2
json: {"title":"Dune","author":"Frank Herbert","year":1965,"in_print":true}
```

*JavaScript and TypeScript:*

```text
fields: title author year inPrint
dune: { title: 'Dune', author: 'Frank Herbert', year: 1965, inPrint: true }
copy === dune: false
equal(copy, dune): true
reissue: { title: 'Dune', author: 'Frank Herbert', year: 1966, inPrint: true }
equal(reissue, dune): false
dune < reissue: true
distinct objects: 3
distinct books: 2
json: {"title":"Dune","author":"Frank Herbert","year":1965,"inPrint":true}
```

*Dart:*

```text
dune: (author: Frank Herbert, inPrint: true, title: Dune, year: 1965)
copy == dune: true
reissue: (author: Frank Herbert, inPrint: true, title: Dune, year: 1966)
reissue == dune: false
dune < reissue: true
distinct books: 2
```

*Kotlin:*

```text
fields: title author year inPrint
dune: Book(title=Dune, author=Frank Herbert, year=1965, inPrint=true)
copy == dune: true
reissue: Book(title=Dune, author=Frank Herbert, year=1966, inPrint=true)
reissue == dune: false
dune < reissue: true
distinct books: 2
```

**Go: reflection, tags, and generated files.** Go has no macros, by design, and the
compiler writes only `==`: two structs are equal when every field is, and a struct whose
fields can all be compared can be a map key, which counts the distinct books. A field
that can't be compared, such as a slice, takes that away, and `a == b` fails with
`invalid operation: a == b (struct containing []string cannot be compared)`. Ordering is
written by hand, with `cmp.Or` returning the first comparison that isn't 0. The rest
comes from `reflect`, which describes a type as the program runs:
`reflect.TypeFor[Book]()` lists the fields, with their names, types, and **struct
tags**, the strings in backquotes. `fmt` uses `reflect` to print `%+v` with field names,
and `encoding/json` uses it to find the fields and read their `json` tags. Only exported
fields, with capital letters, are visible to it, so a struct with lowercase fields
marshals to `{}`, and `go vet` says nothing. A malformed tag, such as `` `json:title` ``
without quotes, is ignored as the program runs, and only `go vet` catches it: ``struct
field tag `json:title` not compatible with reflect.StructTag.Get: bad syntax for struct
tag value``.

For everything else, Go generates code before compiling. A comment such as
`//go:generate stringer -type=Status` names a command, and `go generate` runs it.
`stringer` reads the constants of type `Status` and writes `status_string.go`, which
starts `// Code generated by "stringer -type=Status"; DO NOT EDIT.` and gives `Status` a
`String` method. The file is ordinary Go that is checked in with the rest. `go generate`
"is never run automatically by go build", as its documentation says, so the file can
fall behind the constants. stringer plans for that: the file includes lines such as
`_ = x[NotFound-1]`, so adding a constant without regenerating fails to compile with
`invalid argument: index 1 out of bounds [0:1]`.

**JavaScript: objects that can be read and wrapped.** A JavaScript object is a map from
keys to values, so code can list its keys as it runs, with `Object.keys`, and one
`equal` or `compare` works on any object. Nothing writes methods for you. `===` asks
whether two objects are the same object, so the copy isn't `===` to the original, and a
`Set` keeps all three books. The example counts distinct books by their JSON text
instead, which works because the keys are always added in the same order. Spreading,
`{ ...dune, year: 1966 }`, makes a changed copy. JavaScript can also intercept what code
does to an object. `new Proxy(dune, { get(target, key, receiver) { ... } })` makes an
object whose every property read calls `get`, and `Reflect.get` does the read that would
have happened. With a `get` that logs, `watched.title` prints `read title` and then
returns `"Dune"`, and a `set` handler sees `watched.year = 1966`. Libraries such as Vue
use proxies to notice when data changes.

**TypeScript: types that compute, and then vanish.** TypeScript's types are erased
before the program runs, so there is nothing to reflect on except JavaScript's objects,
and the TypeScript version works exactly as the JavaScript one does. The compile-time
work is in the types. `keyof T` is the compiler's list of `T`'s keys, so `keysOf`
returns `(keyof T)[]`, and `a[key]` is checked against the fields. Exercise 15.2 goes
the other way: `type Book = typeof DEFAULT_BOOK` makes the type from a value. TypeScript
also has **decorators**, functions written as `@logged` above a class method that
replace it, like Python's. TypeScript 5.0 implemented the standard JavaScript proposal,
and `tsc` accepts it, even with `--erasableSyntaxOnly`, since a decorator isn't
TypeScript-only syntax. But no JavaScript engine has shipped decorators yet, so Node 22
can't run the file, and stops at the `@` with
`SyntaxError: Invalid or unexpected token`. Compiled by `tsc` to ES2022, which turns
decorators into ordinary calls, it runs and prints `calling count`.

**Dart: records, and code generation.** A Dart record is the one place where Dart writes
the code for you: two records are `==` when their fields are, they get a matching
`hashCode`, and `toString` lists the fields, with the named ones in alphabetical order,
so `author` comes first. The analyzer knows that two equal constant records are equal,
and warns about a set literal that holds both:
`Two elements in a set literal shouldn't be equal`. That is why the example builds the
set with `toSet()`. A record has no ordering, and no `copyWith`, so a changed copy names
every field. Dart's reflection library, `dart:mirrors`, works under `dart run`, but a
program compiled with `dart compile exe` can't use it,
`Dart library 'dart:mirrors' is not available on this platform`, and neither can Flutter
apps, because a compiler that has to keep everything reflectable can't remove unused
code. Dart planned macros to fill the gap, and on January 29, 2025, the team announced
that "we've made the difficult decision to stop our work on macros". Dart code generates
instead: packages such as `json_serializable` and `freezed` are run by `build_runner`,
which writes the generated code into files beside the source, such as `book.g.dart`, and
the team said it would invest in "improving build times and the overall code generation
experience".

**Kotlin: data classes, reflection, and compiler plugins.** `data class Book(...)` makes
the compiler write `toString`, `equals`, `hashCode`, and `copy` from the properties in
the primary constructor, and only those: a property declared in the class body is left
out, so two books that differ only in such a property are equal. `copy(year = 1966)`
uses named arguments with defaults (chapter 5). `compareBy(Book::title, ...)` takes
property references, each a function from a book to one field, and tries them in turn.
Reflection is in a separate library, `kotlin-reflect`, which `kotlinc` adds to the
classpath unless told `-no-reflect`. Without it, `kotlin.reflect.full` doesn't exist,
`unresolved reference 'full'`, and the simpler reflection calls fail as the program runs
with `KotlinReflectionNotSupportedError`. `primaryConstructor!!.parameters` lists the
fields in the order they were declared, while `memberProperties` lists them in
alphabetical order. Annotations are data attached to declarations, which reflection can
read, as in exercise 15.2's `@Column("in_print")`.

To generate code while compiling, Kotlin has **KSP**, Kotlin Symbol Processing, an API
for plugins that read the program's declarations and write new source files, and full
**compiler plugins**, which change the compiler's own output. `kotlinc` ships several:
kotlinx.serialization writes a serializer for each `@Serializable` class, Compose
rewrites `@Composable` functions to build user interfaces, and `power-assert` does what
pytest's assert rewriting does. Compiled with
`-Xplugin=power-assert-compiler-plugin.jar` and told to rewrite `require`, a failed
`require(a + b == 0.3)` prints the expression with every value under it, `0.1`,
`0.30000000000000004`, `0.2`, and `false`, where without the plugin it prints only
`Failed requirement.`

---

## 15.9 Why each language looks the way it does

- **C** got its preprocessor around 1972-3, "partly at the urging of Alan Snyder", as
  Dennis Ritchie wrote, and to have the file inclusion of BCPL and PL/I. "Its original
  version was exceedingly simple, and provided only included files and simple string
  replacements", and it "was originally considered an optional adjunct to the language
  itself", run only when a file asked for it. That history, Ritchie wrote, explains "the
  incomplete integration of the syntax of the preprocessor with the rest of the
  language". Arguments, `#if`, `#`, and `##` came later, and C has relied on the
  preprocessor for constants, generic code, and configuration ever since.
- **C++** kept the preprocessor for compatibility, and has spent decades replacing it
  with features that follow the language's rules: `const` and `inline` functions, then
  templates, then `constexpr` (C++11), `if constexpr` (C++17), `consteval` and modules
  (C++20), and reflection (C++26). Template metaprogramming was discovered rather than
  designed: "In 1994, Erwin Unruh from Siemens presented at a C++ committee meeting a
  program that didn't compile", whose error messages listed the prime numbers below 30.
- **Rust** wanted macros without the C preprocessor's problems, so its macros take
  tokens, match them by grammar, and are hygienic, and a call is always marked with `!`.
  Derive macros written in libraries became stable in Rust 1.15 (2017), and attribute
  and function-like procedural macros in 1.30 (2018), which is what made libraries such
  as serde possible. `const fn` arrived in 1.31 (2018), and has been allowed to do more
  in almost every release since. Rust has no reflection, because it keeps almost no
  description of types in the compiled program.
- **Go** left macros out on purpose, for code that reads the same everywhere and
  compiles quickly. Reflection and struct tags were there from the start, for packages
  such as `encoding/json`, and `go generate`, added in Go 1.4 (2014), moved the rest of
  code generation into ordinary, checked-in files.
- **Python** compiles each module quietly as it is imported, and put its power in the
  running program instead, where every class and function is an object. Decorators
  (Python 2.4, 2004), metaclasses, and `inspect` let a program change its own classes
  and functions as they are made, and `dataclasses` (Python 3.7, 2018) put the most
  common case in the standard library.
- **JavaScript** objects were always open maps, so reflection came first, and ES2015
  added `Proxy` and `Reflect` to intercept what code does with them. Decorators have
  been a proposal for years, and engines still don't run them.
- **TypeScript** erases its types, by design, so its metaprogramming is in the type
  system: types computed from other types and from values. Decorators were an
  experimental TypeScript feature long before the JavaScript proposal settled, and
  TypeScript 5.0 switched to the standard ones.
- **Dart** is compiled ahead of time for Flutter apps, where removing unused code
  matters, so reflection isn't available there, and the team chose code generation, and
  then stopped its work on macros in 2025.
- **Kotlin** writes the most common methods with data classes, keeps reflection in a
  library that a program can leave out, and lets plugins extend the compiler, which is
  how serialization and Compose work.

---

## 15.10 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | `#define SQUARE(x) x * x` | `SQUARE(1 + 2)` is 5: put parentheses around each parameter and the whole body |
| C | Passing a call or `i++` to a macro that uses its argument twice | It runs twice; `-Wall` catches `i++`, but nothing catches a call |
| C | A macro whose body is `{ ... }` | `if (c) M(x); else ...` fails with `'else' without a previous 'if'`; wrap it in `do { ... } while (0)` |
| C | A macro with a local variable, such as `tmp` | A caller's variable of the same name breaks it silently |
| C | `LOG("hi")` with `format, ...` and no other arguments | `expected expression before ')' token` in C17; C23 has `__VA_OPT__` |
| C | Forgetting `#undef X` after an X-macro | The next `#define X` fails with `"X" redefined` under `-Werror` |
| C++ | A comma inside `<>` or `{}` in a macro argument | The macro gets too many arguments; add parentheses |
| C++ | Defaulting `<=>` with a member that has none | The operator is deleted, and the error appears where it is used |
| C++ | Expecting `if constexpr` to hide code outside a template | The dropped branch is still checked |
| C++ | Global registry objects in several files | Their order is unspecified; keep the list in a function-local `static` |
| Rust | `square!(f())` with `$x * $x` | `f` runs twice; bind the value to a name in the macro |
| Rust | `debug_assert!` for a check that must always run | It is gone in release builds; use `assert!` |
| Rust | Deriving `Eq` or `Hash` with an `f64` field | `the trait bound 'f64: Eq' is not satisfied` |
| Rust | A `for` loop in a `const fn` | `error[E0015]`; write `while` |
| Python | `assert` for checks in production code | `python3 -O` removes them all |
| Python | A decorator without `functools.wraps` | The function's `__name__` becomes the wrapper's |
| Python | `@dataclass` without `frozen=True`, in a set | `TypeError: unhashable type` |
| Python | Two `Enum` members with the same value | The second is an alias, silently; add `@unique` |
| Go | A malformed struct tag | Ignored as the program runs; only `go vet` reports it |
| Go | Lowercase fields with `encoding/json` | Left out without a word: `{}` |
| Go | Changing constants without rerunning `go generate` | The generated code is stale; stringer's guard fails the build |
| JavaScript | `===` or a `Set` to compare objects | Compares identity, not fields |
| TypeScript | Running a file with decorators on Node | `SyntaxError: Invalid or unexpected token`; compile it with `tsc` |
| Dart | `dart:mirrors` in a compiled program | `not available on this platform` |
| Kotlin | A property in a data class's body | Left out of `equals`, `hashCode`, `toString`, and `copy` |
| Kotlin | `memberProperties` for the field order | Alphabetical; use `primaryConstructor.parameters` |

---

## 15.11 Idiom check

**C: macros for what functions can't do.** Use `static inline` functions and `enum` or
`const` values first, and macros only for what needs text: stringizing with `#`, pasting
names with `##`, `__LINE__` and `__FILE__`, X-macros for lists, and `#if` for
configuration. Name macros in capitals, put parentheses around every parameter and the
whole body, wrap statement macros in `do { ... } while (0)`, and never pass an argument
with side effects. `cc -E` shows what the compiler sees.

**C++: constexpr, templates, and few macros.** Use `constexpr` variables and functions,
`consteval` where a result must be known while compiling, `if constexpr` in templates,
and `static_assert` to check assumptions. Default `==` and `<=>` with `= default`. Keep
macros for `#expr`, `#include`, and configuration, and X-macros for lists until C++26
reflection is widely available.

**Rust: derive first, `macro_rules!` second.** Derive the standard traits whenever the
fields allow it. Write a `macro_rules!` macro when functions and generics can't do the
job, such as a list of items or a variable number of arguments, and use fragment
specifiers such as `expr` and `ident`, which keep the structure. Use `const fn` and
`const` assertions for tables and checks. Reach for procedural macros, with the `syn`
and `quote` crates, only for a library's derive or attribute.

**Go: plain code, reflection in libraries, and `go generate`.** Write the code out. Use
struct tags with `encoding/json` and similar packages, and run `go vet`, which checks
them. Generate repetitive code with `go generate` and tools such as `stringer`, check
the generated files in, and keep their `Code generated ... DO NOT EDIT.` header. Use
`reflect` in libraries, rarely in application code.

**Python: decorators and dataclasses.** Use `@dataclass`, with `frozen=True` for values,
and `Enum` for fixed sets of names. Write decorators with `functools.wraps`. Use
`inspect` and metaclasses sparingly, mostly in frameworks. Never use `assert` for checks
that must run in production.

**JavaScript: plain objects, with `Proxy` for frameworks.** Use spreading for changed
copies and `Object.keys` or `Object.entries` for generic code over fields. Remember that
`===` and `Set` compare identity. Keep `Proxy` and `Reflect` for libraries that need to
watch or wrap objects.

**TypeScript: compute types, not code.** Use `keyof`, `typeof`, and mapped types to keep
types in step with values, and write the value once, as exercise 15.2 does. Use
decorators only where the build compiles them, and remember that no type exists when the
program runs.

**Dart: records and code generation.** Use records for small values, and classes with
`==` and `hashCode` written by hand, or generated with `freezed` or `json_serializable`
through `build_runner`, for larger ones. Don't use `dart:mirrors` in code that will be
compiled or used in Flutter.

**Kotlin: data classes, with reflection when needed.** Use data classes for values, and
`copy` for changed copies. Use `kotlin-reflect` for frameworks and tools, with
annotations to carry extra data. Use KSP or a compiler plugin, such as
kotlinx.serialization, for code that should be generated while compiling.

---

## 15.12 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Trace calls](exercises/01-trace-calls/) | Writing a wrapper once, as a macro or decorator, and finding a function's name and arguments |
| 2 | [Records from a table](exercises/02-records-from-a-table/) | Writing a list of fields once, and parsing and printing from it |
| 3 | [A macro expander](exercises/03-macro-expander/) | How a text macro is expanded: arguments, rescanning, and stringizing |

```sh
python3 tools/check.py 15                     # your answers, every language
python3 tools/check.py 15 --lang c,python     # some languages
python3 tools/check.py 15 --solutions         # the reference answers
```

**Stretch:** take exercise 15.2 in two languages that work differently, such as C and
Kotlin, and add a field, `rating`, a number with an optional decimal point. Count the
places you changed in each. Then write the whole of exercise 15.2 without any
metaprogramming, field by field. How long is it, and which version would you rather
change in a year?

---

Previous: [Chapter 14: Iterators and functional style](../14-iterators-and-functional-style/) ·
Next: [Chapter 16: Concurrency and parallelism](../16-concurrency-and-parallelism/)
