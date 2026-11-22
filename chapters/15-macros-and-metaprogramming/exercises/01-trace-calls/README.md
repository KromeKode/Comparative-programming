# Exercise 15.1: Trace calls

Write three recursive functions, `fib`, `gcd`, and `power`, and one piece of code that
wraps them, so that every call can be traced. Each line of input is a command, a
function name, and the function's arguments, which are whole numbers.

| Command | Output |
| --- | --- |
| `trace NAME ARGS` | A line for each call, as it starts and as it returns, then a summary |
| `count NAME ARGS` | Only the summary: the call, its result, and how many calls it took |

A call prints as `name(a, b)` when it starts, and its result prints as `-> value` when
it returns, both indented two spaces for each call that is still running around it. The
summary counts every call, the first one included.

For example, `trace gcd 48 18` prints:

```text
gcd(48, 18)
  gcd(18, 12)
    gcd(12, 6)
      gcd(6, 0)
      -> 6
    -> 6
  -> 6
-> 6
gcd(48, 18) = 6 in 4 calls
```

`count`, and every line with a mistake in it, print one line:

| Input | Output |
| --- | --- |
| `count fib 20` | `fib(20) = 6765 in 21891 calls` |
| `count fib 0` | `fib(0) = 0 in 1 call` |
| `trace fib 1 2` | `fib takes 1 argument, got 2` |
| `count sqrt 16` | `unknown function: sqrt` |
| `print fib 3` | `unknown command: print` |

The functions are:

- `fib(n)` is `n` when `n` is below 2, and otherwise `fib(n - 1) + fib(n - 2)`.
- `gcd(a, b)` is `a` when `b` is 0, and otherwise `gcd(b, a % b)`.
- `power(base, exponent)` is 1 when `exponent` is 0. When `exponent` is even, it is
  `half * half`, where `half` is `power(base, exponent / 2)`, and otherwise it is
  `base * power(base, exponent - 1)`.

Check the command first, then the name, then the number of arguments. Skip blank lines,
and allow any number of spaces between words. Every line that isn't blank has a command
and a name. The tests use arguments that are 0 or more, and results below 2^53, so
JavaScript's numbers hold them exactly.

Write the tracing once, as a wrapper, macro, or decorator, and apply it to each
function. The functions themselves should read like ordinary recursive functions, and
their recursive calls must be traced too. Where your language can, find each function's
name and its number of arguments from the function itself, rather than writing them
again in a table.

```sh
python3 tools/check.py 15 --lang python
```

**Things to notice:**

- **Where the wrapper goes in.** Python's `@traced` replaces `fib` with the wrapper when
  the `def` runs, and the recursive call `fib(n - 1)` looks the global name up as it
  runs, so it finds the wrapper. C's `TRACED_1` and `TRACED_2` and Rust's `traced!` take
  the function's name, parameters, and body, and write a wrapper of that name around a
  body of their own. JavaScript and TypeScript wrap every method of an object with
  `traceAll`, and each body runs with `this` set to the wrapped object, so
  `this.fib(n - 1)` is traced. C++, Go, Dart, and Kotlin write each function by hand,
  and its body is a lambda or closure passed to `traced`.
- **Where the name comes from.** The C and Rust macros turn the name into a string with
  `#name` and `stringify!`. C++ passes `__func__`, which holds the name of the function
  it appears in. Python reads `__name__`, JavaScript and TypeScript use the object's
  keys, and Kotlin reads `name` from the reference `::fib`. Go's `trace` looks for its
  caller on the call stack with `runtime.Caller`, and asks the runtime for that
  function's name. Dart can't find a function's name as the program runs, so each
  function passes its own as a string.
- **Where the number of arguments comes from.** Python asks `inspect.signature`,
  JavaScript and TypeScript read a function's `length`, Go asks `reflect` with `NumIn`
  and calls with `Call`, and Kotlin reads `parameters` and calls with `call`, both from
  `kotlin-reflect`. C++ works it out while compiling: `sizeof...(Args)` counts the
  parameters, and `std::index_sequence` unpacks a vector into that many arguments.
  Rust's macro counts the parameter names, and then uses the same names as a slice
  pattern, `let &[n] = args`. Dart keeps function types when the program runs, so
  `function is int Function(int)` tells one argument from two. In C, the macro's own
  name, `TRACED_1` or `TRACED_2`, says how many.
- **Keeping the wrapper's name.** A wrapper is a new function, so it has its own name
  and number of parameters. Python's `functools.wraps` copies `__name__` and the
  docstring from the original, and without it, `fib.__name__` is `wrapper`. The
  JavaScript and TypeScript solutions do the same by hand, with
  `Object.defineProperty(wrapper, "length", ...)`, since a wrapper that takes `...args`
  has a `length` of 0.
- **A trap in JavaScript.** `const fib = traced(function fib(n) { ... fib(n - 1) ... })`
  traces only the first call. Inside a named function expression, the name means the
  function itself, not the variable, so `fib(10)` returns 55 after one traced call.
  TypeScript refuses the other way round, wrapping a function declaration in place, with
  `error TS2630: Cannot assign to 'fib' because it is a function.`

**Stretch:** add a command `memo` that works like `count`, but has the wrapper remember
each result, and return a remembered one without running the body. How many calls does
`memo fib 20` count, and how many times does the body run? Python's `functools.cache` is
a decorator that does this.
