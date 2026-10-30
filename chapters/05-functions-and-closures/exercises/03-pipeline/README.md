# Exercise 5.3: Function pipeline

Each line of input is a starting whole number followed by zero or more steps. A step
is an operation and a whole number:

| Step | Result |
| --- | --- |
| `add n` | `x + n` |
| `sub n` | `x - n` |
| `mul n` | `x * n` |
| `min n` | the smaller of `x` and `n` |
| `max n` | the larger of `x` and `n` |

Apply the steps from left to right, and print:

```text
<start> -> <result>
```

Skip blank lines.

| Input | Output | Because |
| --- | --- | --- |
| `5 add 3 mul 2 max 20` | `5 -> 20` | 5 + 3 = 8, 8 * 2 = 16, and 20 is larger |
| `2 add 3 mul 4` | `2 -> 20` | (2 + 3) * 4 |
| `2 mul 4 add 3` | `2 -> 11` | 2 * 4 + 3 |
| `7` | `7 -> 7` | no steps |

Do it in two stages. First turn each step into a function: `make_step("add", 3)`
returns a function that adds 3. Collect those functions in a list. Then run the start
value through the list. Web frameworks build their middleware the same way: a list of
functions, each made with its own settings, applied in order.

```sh
python3 tools/check.py 05 --lang rust
```

**Things to notice:**

- **Each step captures its own `n`.** Every call to `make_step` returns a new closure
  with its own copy of `n`: `[n](long long x) { return x + n; }` in C++,
  `move |x| x + n` in Rust, `lambda x: x + n` in Python, `{ x -> x + n }` in Kotlin.
- **A list of functions needs one element type.** In Go, every `func(int) int` has the
  same type, so a `[]func(int) int` just works, and the same goes for Kotlin's
  `List<(Long) -> Long>`. In Rust, every closure has its own
  unnamed type, so the five arms of a `match` disagree. Boxing them as
  `Box<dyn Fn(i64) -> i64>` gives them one type. C++'s `std::function` does the same
  job.
- **C has function pointers, not closures.** A function pointer cannot remember `n`,
  so store each step as a struct holding the pointer and its `n`. That pair is what
  the other languages build for you when they make a closure.
- **Big numbers.** `2 mul 1000000 mul 1000000` is 2 * 10^12, which needs 64 bits.
- **Reading the words in pairs.** Python can pair them with
  `zip(words[1::2], words[2::2])`, Rust with `rest.chunks(2)`, and Kotlin with
  `words.drop(1).chunked(2)`. The other languages step an index by 2.

**Stretch:** write `compose_all(steps)`, which turns the list into one function, and
call it once per line. In JavaScript that is a `reduce` that starts from `(x) => x`:
`steps.reduce((run, step) => (x) => step(run(x)), (x) => x)`.
