# Exercise 9.2: Snapshots

This program is a tiny paint program with an undo slot. The first line of input is
`W H`, the width and height of a grid, each from 1 to 50. Every cell starts as `.`.
Each line after it is a command:

| Command | Effect |
| --- | --- |
| `paint X Y C` | Set the cell in column X, row Y to the character C. Columns count from 1 on the left, rows from 1 at the top. |
| `fill C` | Set every cell to C. |
| `save` | Remember a copy of the grid. Saving again replaces the copy. |
| `restore` | Make the grid a copy of the saved one. The saved copy stays, so `restore` can run again. |
| `show` | Print the grid, one row per line, then an empty line. |

If a command is wrong, print one of these errors, leave the grid as it was, and carry
on. Check in this order: the command, the number of values, X, Y, whether the cell is
on the grid, then C.

| Problem | Error |
| --- | --- |
| An unknown command | `error: unknown command "zoom"` |
| The wrong number of values | `error: paint takes 3 values, got 2`, `error: fill takes 1 value, got 0`, `error: show takes 0 values, got 1` |
| X or Y is not all digits | `error: "-1" is not a number` |
| The cell is off the grid | `error: (5, 1) is outside the 4 by 3 grid` |
| C is not a single character | `error: "ab" is not one character` |
| `restore` before any `save` | `error: nothing saved` |

The off-the-grid error repeats X and Y exactly as they were written. A number too big
for any integer type is off the grid too. Skip blank lines, and allow any number of
spaces between the parts of a line. The first line is always valid, and the tests use
ASCII only.

| Input | Output |
| --- | --- |
| `3 2` | |
| `paint 1 1 a` | |
| `save` | |
| `paint 2 2 b` | |
| `show` | `a..` then `.b.` then an empty line |
| `restore` | |
| `show` | `a..` then `...` then an empty line |

```sh
python3 tools/check.py 09 --lang python
```

**Things to notice:**

- **Three aliasing bugs are waiting, and the tests catch each one.** First, a grid built
  by repeating one row, such as Python's `[["."] * w] * h`, JavaScript's
  `Array(h).fill(row)`, Dart's `List.filled(h, row)`, or Kotlin's `List(h) { row }`, has
  one row in every slot, so painting one cell paints a whole column. Second, a shallow
  copy of the grid shares its rows, so painting after `save` changes the snapshot too.
  Third, `grid = saved` on restore makes both names point at one grid, so the next
  paint changes the snapshot, and a second `restore` brings back the wrong picture.
- **Value semantics gets all three right without trying.** In C, put the cells in an
  array inside a struct, and `saved = grid` copies every cell. In C++, a
  `std::vector<std::string>` copies its rows on assignment. Rust's `vec![row; h]`
  clones the row for each slot, and `saved = grid` would move the grid away, so the
  compiler pushes you to write `grid.clone()`.
- **Go sits in the middle.** A `[][]rune` is a slice of slice headers, so
  `slices.Clone(grid)` and `copy` share the rows. Clone each row as well. A fixed-size
  array such as `[50][50]rune` would copy on assignment, like C's.
- **Deep copies by hand.** Python has `copy.deepcopy` and JavaScript and TypeScript have
  `structuredClone`, but a list comprehension or a `map` that copies each row says
  exactly what is copied. Dart and Kotlin have no general deep copy, so copying each row
  is the only way.

**Stretch:** replace the single slot with an undo stack. `save` pushes a copy, and
`undo` pops the most recent copy back into the grid, printing `error: nothing to undo`
when the stack is empty.
