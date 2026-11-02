# Exercise 13.2: Typed settings

Your program keeps four settings, each with its own type and default value:

| Setting | Type | Default | Written as |
| --- | --- | --- | --- |
| `volume` | Whole number | `5` | An optional minus sign and at least one digit, fitting in 32 bits |
| `muted` | Boolean | `false` | `true` or `false` |
| `name` | Text | `guest` | Any words |
| `tags` | List of words | `[]` | Any words, shown as `[red, green]` |

Each line of input is one command:

| Command | What it does |
| --- | --- |
| `set NAME VALUE` | Sets the setting from VALUE, the rest of the line, and prints it |
| `get NAME` | Prints the setting as `NAME = VALUE` |
| `reset NAME` | Puts the default back, and prints the setting |
| `list` | Prints every setting in the order above, with ` (default)` after each one whose value equals its default |
| `summary` | Prints `NAME hears the sound at V%`, where V is the volume times 10, or `NAME has muted the sound` when muted is true. NAME is the value of `name` |

| Input | Output |
| --- | --- |
| `set volume 7` | `volume = 7` |
| `set muted yes` | `error: muted needs true or false, not "yes"` |
| `set name Ada  Lovelace` | `name = Ada Lovelace` |
| `summary` | `Ada Lovelace hears the sound at 70%` |
| `set tags red green` | `tags = [red, green]` |
| `set volume 3000000000` | `error: volume needs a whole number, not "3000000000"` |
| `set volume 5` | `volume = 5` |
| `list` | `volume = 5 (default)` |
| | `muted = false (default)` |
| | `name = Ada Lovelace` |
| | `tags = [red, green]` |

The other error is `error: no setting named "NAME"`. A `set` that fails changes nothing.
VALUE is the rest of the line with its words joined by single spaces, so
`set name Ada  Lovelace` stores `Ada Lovelace`. A whole number has no plus sign, and
leading zeros are allowed, so `007` sets 7. Every line is a well-formed command, and
every `set` has a value. Skip blank lines, and allow any number of spaces between words.
The tests use ASCII only, with lines under 256 characters, and run `summary` only when
the volume times 10 fits in 32 bits.

Write a generic `Setting<T>` that holds a T, its default, a function that reads a T from
text, and a function that shows a T. Then make two things work at once. The code that
runs commands keeps all four settings in one list, and never asks which type a setting
holds. And `summary` reads `volume` as a number and `muted` as a boolean, with no casts
and no parsing.

```sh
python3 tools/check.py 13 --lang kotlin
```

**Things to notice:**

- **One list, four different T's.** `Setting<Int>` and `Setting<Boolean>` are different
  types, so `[volume, muted]` has no obvious item type. Kotlin's answer is a star
  projection, `List<Setting<*>>`: a setting of some type it doesn't know, whose members
  that don't take a T can still be called. Dart's generics are covariant, so every
  `Setting<int>` is also a `Setting<Object>`. Rust, Go, TypeScript, and C++ declare an
  interface that leaves T out, `AnySetting`, and every `Setting<T>` implements it: a
  trait used as `&dyn AnySetting`, a Go interface, a TypeScript interface, and an
  abstract C++ class that the `Setting` template derives from. Python's version is a
  `Protocol`. In C, each setting holds a `void *` that points at a typed variable, and
  nothing checks that its functions agree about what that is.
- **Typed handles.** The same four settings are also fields of a `Settings` type, or
  constants in TypeScript and JavaScript, each with its full type. `summary` reads
  `muted.value` as a boolean and `volume.value` as a number, while `list` goes through
  the shared list. Both lead to the same four objects. In C++, the list holds pointers
  into the `Settings` object itself, so the solution deletes its copy constructor: a
  copy's list would still point into the original.
- **Equality has to come from somewhere.** `list` compares each value with its default.
  Rust asks for `T: PartialEq` and C++ for `std::equality_comparable T`, and a
  `Vec<String>` or `std::vector<std::string>` compares item by item. So does a Python or
  Kotlin list. But Go's `comparable` constraint leaves out slices, and `==` on two
  arrays in JavaScript, TypeScript, and Dart asks whether they are the same array:
  `[1] == [1]` is `false`. So in those languages each setting can bring its own `equal`
  function, such as Go's `slices.Equal[[]string]`.
- **Parsing belongs to the setting, not to the type.** It is tempting to require
  `T: FromStr` in Rust, but `impl FromStr for Vec<String>` is refused with
  `error[E0117]`, because neither the trait nor the type is yours. That is the orphan
  rule from section 13.5. Keeping a `parse` function in each setting avoids the problem,
  and lets two settings of the same type read text in different ways.
- **32 bits.** `3000000000` must be refused. Rust's `parse::<i32>`, C++'s `from_chars`
  into an `int`, Go's `strconv.ParseInt(text, 10, 32)`, and Kotlin's `toIntOrNull` all
  report a number that doesn't fit. Python, JavaScript, TypeScript, and Dart read it
  without complaint, so their solutions check the range themselves.

**Stretch:** add a fifth setting, `theme`, which accepts only `light` or `dark` and
starts as `light`. Give it a type of its own, such as an enum. What did you change
besides adding the setting?
