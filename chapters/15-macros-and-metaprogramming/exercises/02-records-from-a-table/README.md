# Exercise 15.2: Records from a table

Read a table of books and turn each row into a `Book` record. The first line that isn't
blank is a header of column names, separated by commas, and each line after it is a row
of values in the same order. The columns can come in any order, and any of them can be
left out.

| Field | Kind | Default |
| --- | --- | --- |
| `title` | Text | `""` |
| `author` | Text | `""` |
| `year` | Whole number | `0` |
| `pages` | Whole number | `0` |
| `in_print` | `true` or `false` | `false` |

Print each good row as a `Book` with every field, in the order of the table above, with
text in double quotes. A field whose column is missing has its default. After the last
row, print how many books there were and how many bad rows.

```text
year,title,author
1949,Nineteen Eighty-Four,George Orwell
1818,Frankenstein,Mary Shelley
```

prints:

```text
Book(title="Nineteen Eighty-Four", author="George Orwell", year=1949, pages=0, in_print=false)
Book(title="Frankenstein", author="Mary Shelley", year=1818, pages=0, in_print=false)
2 books, 0 bad rows
```

A bad row prints one line instead of a book, and the rows after it carry on. Rows are
counted from 1, after the header, and blank lines don't count.

| Problem | Output |
| --- | --- |
| Too few or too many values | `row 5: expected 4 values, got 2` |
| A whole number that isn't one | `row 2: year: "MCMLXXI" is not a whole number` |
| A flag that isn't `true` or `false` | `row 3: in_print: "yes" is not true or false` |

Check a row's values from left to right, and report the first problem. A whole number is
an optional `-` followed by one or more digits, so `+12`, `2001.5`, and an empty value
are all bad, and `0042` is 42. Only `true` and `false`, in lower case, are flags.

A problem in the header stops the program, after one line: `unknown column: isbn` for a
name that isn't a field, or `repeated column: year` for a name used twice. Check the
names from left to right.

Remove spaces from both ends of every name and value, and skip blank lines. Values never
contain commas or quotes. The tests use lines under 1000 characters, and numbers below
2^53 in size, so JavaScript's numbers hold them exactly.

Write the list of fields once. Everything else, finding a field by its column name,
parsing a value by its field's kind, starting from the defaults, and printing a book,
should come from that list, so that adding a field means changing one place. Use the
tools of this chapter: a macro, reflection, or a description of the fields that the
program reads as it runs.

```sh
python3 tools/check.py 15 --lang go
```

**Things to notice:**

- **Where the list of fields lives.** C writes it as an X-macro, `BOOK_FIELDS(X)`, and
  expands it into the struct, an enum, the names, a switch that sets a field, and the
  printer. Rust's `record!` macro takes the struct definition and writes it back out
  with `FIELDS`, `set`, and `describe` added. Python's `@dataclass` and Kotlin's
  `data class` are the list, and the code reads them with `dataclasses.fields` and with
  `primaryConstructor`, from `kotlin-reflect`. Go's struct is the list too, read with
  `reflect`, and each field's tag, such as `` `column:"in_print"` ``, gives its column
  name. JavaScript, TypeScript, and Dart write a book with every field at its default,
  and the type of each default says how to parse its column. C++ can't list a struct's
  members until C++26, so its list pairs each name with a pointer to the member,
  `Field{"year", &Book::year}`, and the compiler checks the members but not the names.
- **Picking the parser by type.** The parser for a field is picked by its type: as the
  program runs in Python, Go, Kotlin, JavaScript, TypeScript, and Dart, and while
  compiling in Rust and C++. Rust's macro calls `<$type as Field>::parse`, so a field of
  a type with no `Field` implementation doesn't compile. C++ picks among overloads of
  `parse` by the member's type, inside a fold expression over a tuple. C pastes the kind
  into the function's name: `parse_##kind` becomes `parse_whole`.
- **Defaults.** C's `struct book book = {0}` sets every field to zero, which is `""`, 0,
  and `false`, so the defaults come for free. Rust derives `Default`, and Go's zero
  values are the defaults too. Kotlin's `callBy` builds a `Book` from the arguments in a
  map, and uses the default of every parameter left out of it.
- **A type made from a value.** TypeScript's solution writes the fields once, as the
  default book, and `type Book = typeof DEFAULT_BOOK` gets the type from it. The
  compiler still can't follow a field chosen by a name read from the input, so
  `book[name] = ...` needs the book typed as a `Record<Column, Value>`.
- **Library parsers accept more than the rule.** Python's `int` accepts `+12`, ` 12 `,
  and `1_000`, and JavaScript's `Number` turns `""` into 0, `1e3` into 1000, and `0x10`
  into 16. Rust, Go, Kotlin, and Dart all accept `+12`, and Dart's `int.parse` takes
  `0x10` too. So every solution checks the text before it converts it. Flags have the
  same trap: Python's `bool("false")` and JavaScript's `Boolean("false")` are both true,
  since any text that isn't empty is.

**Stretch:** add a field, `rating`, a number with an optional decimal point, such as
`4.5`. How many places in your solution did you change? Then add a command after the
table, such as `sort year`, that sorts the books by a field named in the input.
