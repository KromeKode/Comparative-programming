# Exercise 9.3: A growable array in C

C has no built-in list that grows. This exercise builds one by hand: a small header
struct that holds a pointer to a block of ints on the heap, how many are in use, and
how many fit. It is the same design as a C++ `std::vector`, a Rust `Vec`, and a Go
slice. This exercise is C only, because every other language in the book already
has one.

Each line of input is a command:

| Command | Effect |
| --- | --- |
| `push N` | Add N at the end. If the array is full, first grow it and print `grow: OLD -> NEW`. |
| `pop` | Remove the last item and print `pop: N`. |
| `get I` | Print `get I: N`, the item at index I. |
| `set I N` | Replace the item at index I with N. Print nothing. |
| `print` | Print the items as `[10, 20, 30] (length 3, capacity 4)`. |

The array starts empty with capacity 0. The first push grows it to 4, and each later
growth doubles it. Indexes count from 0, and `pop` never shrinks the capacity.

If a command is wrong, print one of these errors, leave the array as it was, and carry
on. For `set`, check the index before N.

| Problem | Error |
| --- | --- |
| An unknown command | `error: unknown command "insert"` |
| The wrong number of values | `error: push takes 1 value, got 0`, `error: set takes 2 values, got 1`, `error: print takes 0 values, got 1` |
| A value that is not a whole `int` | `error: "1.5" is not an int` |
| An index that is not in use | `error: index 3 is out of range (length 2)` |
| `pop` on an empty array | `error: pop from an empty array` |

A whole `int` is an optional `+` or `-` and digits, from -2147483648 to 2147483647. Skip
blank lines, and allow any number of spaces between the parts of a line. Lines are
shorter than 1,000 characters.

| Input | Output |
| --- | --- |
| `push 10` | `grow: 0 -> 4` |
| `push 20` | |
| `push 30` | |
| `print` | `[10, 20, 30] (length 3, capacity 4)` |
| `get 1` | `get 1: 20` |
| `pop` | `pop: 30` |

```sh
python3 tools/check.py 09 --lang c
```

**Things to notice:**

- **`realloc` does the moving.** It finds a bigger block, copies the items into it, and
  frees the old one, or grows the block where it is if there is room. Either way, a
  pointer into the old block may now be dangling. That is why C++ warns that growing a
  vector invalidates pointers into it, and why Rust will not let you hold a reference
  into a `Vec` across a push (section 9.4).
- **Never write `a->items = realloc(a->items, size)`.** When `realloc` fails it returns
  `NULL` and leaves the old block alone, and that line has just thrown away the only
  pointer to it. Keep the result in a temporary until you know it is not `NULL`.
- **Doubling keeps pushes cheap.** Growing by one each time would copy every item on
  every push. With doubling, N pushes copy items fewer than 2N times in all, so a push
  costs a constant amount on average, which is called amortized constant time
  (chapter 23).
- **Read numbers with `strtol`, and check all three ways it can fail:** nothing was
  read (`end == text`), something was left over (`*end != '\0'`), or the number was
  too big (`errno == ERANGE`). A `long` is wider than an `int` on 64-bit Linux and
  macOS, so check the `int` range as well.
- **One `malloc` or `realloc` block, one `free`.** Free the items when the input ends.
  Build with `cc -g -fsanitize=address` while you work: AddressSanitizer stops the
  program at the first read or write past the end of the block, and on Linux it also
  reports any block that was never freed. Chapter 10 covers leaks in depth.

**Stretch:** make `pop` halve the capacity when the length falls to a quarter of it,
printing `shrink: OLD -> NEW`. Why a quarter and not a half? Picture a full array of 8
items under a shrink-at-half rule, then push one item and pop it, over and over.
