# Exercise 8.3: A robot on a grid

A robot stands at (0, 0) on an endless grid, facing north. Each line of input is a
program for a new robot: a string of commands, one character each.

| Command | Effect |
| --- | --- |
| `L` | Turn left 90 degrees |
| `R` | Turn right 90 degrees |
| `F` | Move one square forward |

North is toward +y and east is toward +x. After running a line, print where the robot
ends up and which way it faces: `x y direction`, with the direction in lowercase. If a
line contains any character other than `L`, `R`, or `F`, uppercase only, run none of it
and print `error: unknown command 'X' at position N` for the first such character,
counting positions from 1. A space counts as an unknown command. Skip empty lines. The
tests use ASCII only.

| Input | Output |
| --- | --- |
| `F` | `0 1 north` |
| `FFRFF` | `2 2 east` |
| `RRFF` | `0 -2 south` |
| `LFFLFFF` | `-2 -3 south` |
| `FFLB` | `error: unknown command 'B' at position 4` |

```sh
python3 tools/check.py 08 --lang kotlin
```

**Things to notice:**

- **An enum for the direction, a struct for the robot.** The robot is three values that
  change together, so they belong in one struct. Giving the direction its own enum
  type means a robot cannot face `5` or `"nrth"`, except in C, C++, and Go, where an
  enum is an integer underneath and can hold any number.
- **Put the directions in clockwise order.** Then turning right adds 1 to the
  direction's number and turning left subtracts 1, wrapping around after 4. Watch the
  sign of `%` (section 3.2): `-1 % 4` is `-1` in C, C++, Rust, Go, JavaScript,
  TypeScript, and Kotlin, but `3` in Python and Dart. Add 4 first, or use Rust's
  `rem_euclid` or Kotlin's `mod`. The Rust solution avoids the arithmetic: a `match`
  turns right, and turning left is three right turns.
- **Data can live on the enum.** Kotlin's and Dart's enums take constructor arguments,
  so each direction can carry its own step, such as `NORTH(0, 1)`. Python's enum values
  can be the step itself. Rust, C++, C, and Go keep the steps in a `match`, a `switch`,
  or an array indexed by the enum.
- **Default values differ.** Go's zero value for the robot struct is already right,
  because `north` is 0. C++, Dart, and Kotlin set defaults on the fields themselves.
  Field defaults in Rust are still experimental (`error[E0658]: default values on
  fields are experimental`), so its solution writes a `new` function.
- **Check the whole line before running it.** Validating first means a program with a
  bad command never moves the robot halfway.

**Stretch:** also print how many different squares each robot stood on, counting
(0, 0). Keep the squares in a set of (x, y) pairs, as in section 7.6.
