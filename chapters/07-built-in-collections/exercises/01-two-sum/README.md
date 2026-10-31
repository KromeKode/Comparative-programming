# Exercise 7.1: Two Sum

Each line of input holds a target, a colon, and a list of whole numbers:

```text
9: 2 7 11 15
```

Find two different positions `i` and `j`, with `i < j`, whose numbers add up to the
target, and print `i j`. Positions count from 0. Here `2 + 7 = 9`, so the answer is
`0 1`. If no pair works, print `none`. Skip blank lines.

When several pairs work, print the one with the **smallest `j`**, and for that `j` the
**smallest `i`**. In other words, read the numbers from left to right and stop at the
first number that completes a pair.

| Input | Output |
| --- | --- |
| `9: 2 7 11 15` | `0 1` |
| `6: 3 2 4` | `1 2` |
| `6: 3 3` | `0 1` |
| `6: 3` | `none` |
| `10: 5 1 9 5 5` | `1 2` (`5 + 5` also works, but its `j` is 3) |

Every number in a list fits in a 32-bit signed integer. The target and the sums may
not: `2147483647 + 2147483647` is 4294967294.

```sh
python3 tools/check.py 07 --lang go
```

**Things to notice:**

- **A map turns two loops into one.** The obvious answer tries every pair: for each
  `j`, look at every `i` before it. That does about n²/2 additions. Instead, walk the
  list once and keep a map from each number seen so far to the first position it
  appeared at. At position `j`, the partner you need is `target - numbers[j]`, and one
  map lookup says whether you have seen it. Look it up *before* you add `numbers[j]`,
  or `6: 3` would pair the 3 with itself.
- **Keep the first index.** A number can repeat, and the rule wants the smallest `i`.
  Only store a number the first time you see it: `setdefault` in Python, `entry(n).or_insert(j)`
  in Rust, `try_emplace` in C++, `putIfAbsent` in Dart and Kotlin, and a `has` check in
  JavaScript and Go. The test `04-first-pair` catches a map that keeps the last index.
- **Use 64-bit integers.** `target - numbers[j]` can fall outside the 32-bit range, and
  so can the target itself. The test `05-big-numbers` has such values. A Kotlin `Int`
  cannot even parse `4294967294`; a `Long` can. JavaScript and TypeScript are fine:
  a `number` holds every integer up to 2^53 exactly.
- **C has no map.** The C solution builds a small hash table with open addressing:
  hash the number to a slot, and if the slot holds a different number, try the next
  one. Chapter 24 covers hash tables properly.

**Stretch:** make a big input and time both approaches. This prints a line with 10,000
numbers whose only pair is the two 1s at the end:

```sh
python3 -c "import random; print('2:', *random.choices(range(0, 10**9, 4), k=10000), 1, 1)" > big.in
```

On the machine that tested this book, a Python double loop took about 5 seconds on it,
and the map version took 0.03 seconds. With a million numbers, the map version took
about half a second. The double loop would need about 10,000 times as long as before,
which is well over half a day.
