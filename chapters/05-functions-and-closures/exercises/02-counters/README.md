# Exercise 5.2: Counter factory

A **counter factory** is a function that makes counters. Each counter is itself a
function: call it, and it returns its current value and then moves on by a step. The
counter remembers where it is between calls without any global variable. That memory
is what makes it a closure.

Each line of input is a command:

| Command | What it does | Prints |
| --- | --- | --- |
| `new <name> <start> <step>` | Makes a counter that starts at `<start>` and adds `<step>` after each call. If `<name>` already has a counter, the new one replaces it. | nothing |
| `next <name>` | Calls the counter named `<name>`. | `<name>: <value>`, or `<name>: no such counter` |

Skip blank lines. The start and step are whole numbers. They can be negative, and a
step can be as large as 10^12.

Input:

```text
new a 0 1
new b 100 -10
next a
next b
next a
next b
next c
```

Output:

```text
a: 0
b: 100
a: 1
b: 90
c: no such counter
```

Write `make_counter(start, step)`, which returns a counter function, and keep the
counters in a map from name to function.

```sh
python3 tools/check.py 05 --lang python
```

**Things to notice:**

- **What the closure keeps.** `make_counter` has returned by the time the counter is
  called, but the counter still needs `next` and `step`. Go, Python, JavaScript,
  TypeScript, Dart, and Kotlin keep captured variables alive for as long as the closure
  is.
  C++ and Rust copy or move them into the closure itself.
- **Changing what was captured.** In Python, `next_value += step` inside the counter
  makes a new local variable unless you first write `nonlocal next_value`. A C++ lambda
  cannot change its copies unless it is marked `mutable`. A Rust closure that changes
  its state is `FnMut`, so the map holds `Box<dyn FnMut() -> i64>` and you call one
  through `get_mut`.
- **C has no closures.** Keep each counter's state in a struct, and the structs in an
  array. The struct holds the same data a closure would. The difference is that you
  find it and pass it around yourself.
- **Same settings, separate state.** Two counters made with the same start and step do
  not share anything, because each call to `make_counter` makes fresh variables. The
  test `05-same-settings-separate-state` checks this.
- **Big steps.** Three steps of 10^12 pass 2^31 quickly. Use `long long` in C and C++,
  `i64` in Rust, and `Long` in Kotlin. Go's `int` is 64 bits on every platform the book uses.
- **Splitting the commands.** This is chapter 4's list patterns again. Python's
  `case ["new", name, start, step]:`, Rust's `["new", name, start, step] =>`, and
  Dart's `case ['new', final name, final start, final step]` check the length and bind
  the words in one step. Kotlin's `val (_, name, start, step) = words` binds them but
  does not check the length: on a shorter list it throws `IndexOutOfBoundsException`,
  so check `words.size` first.

**Stretch:** add `reset <name>`, which puts a counter back to its start. Now the
counter needs two operations on the same state. Can `make_counter` return two closures
that share it?
