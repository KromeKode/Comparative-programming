# Exercise 21.2: Compare JSON files

Write `jsondiff`, which reads two JSON files and prints how the second differs from the
first.

```text
usage: jsondiff OLD NEW
```

Here are two versions of a program's settings, `old.json`:

```json
{
  "name": "notes",
  "version": 3,
  "debug": true,
  "tags": ["text", "markdown"],
  "server": {"host": "localhost", "port": 8080}
}
```

and `new.json`:

```json
{
  "name": "notes",
  "version": 4,
  "tags": ["text", "markdown", "sync"],
  "server": {"host": "localhost", "port": 8443, "tls": true}
}
```

```text
$ jsondiff old.json new.json
- $.debug: true
~ $.server.port: 8080 -> 8443
+ $.server.tls: true
+ $.tags[2]: "sync"
~ $.version: 3 -> 4
```

**Differences.** Each line is one difference: `-` for a value that only OLD has, `+` for
one that only NEW has, and `~` for one that changed, with its old value and then its new
one. `jsondiff` compares the value in OLD with the value in NEW, and goes on into them:

- Two objects: it takes each key that either one has, in order of the keys' characters'
  codes, so that `"B"` comes before `"a"`, and `"10"` before `"9"`. A key that only one
  of them has is a `-` or a `+` line. The values of a key that both have are compared in
  the same way as the whole files were.
- Two arrays: it compares the items at each index, from 0. When one array is longer,
  each of its extra items is a `-` or a `+` line.
- Any other two values: they are a `~` line, unless they are the same kind of value,
  such as two numbers, and the same value of that kind. So `1` and `"1"` are different,
  and so are `true` and `1`, and `[]` and `{}`.

**Paths.** Each line's path says where its value is. `$` is the whole file. After a
path, `.name` is the value of the key `name` in the object there, and `[2]` is the item
at index 2 of the array there. A key is written after a `.` when it's an ASCII letter or
`_`, followed by ASCII letters, digits, and `_`s; any other key is written in brackets,
as a JSON string, as in `["first name"]`.

**Values.** A value is written as JSON, on one line, with no spaces, and with each
object's keys in the order above. In a string, `"` and `\` are written `\"` and `\\`;
backspace, form feed, newline, carriage return, and tab are `\b`, `\f`, `\n`, `\r`, and
`\t`; the other characters below U+0020 are `\u` and four hex digits, in lowercase, as
in `\u001b`; and every other character is written as it is, in UTF-8.

**The files.** When a file is JSON, you can count on three things about it: it's UTF-8,
no object in it has the same key twice, and each number in it is a whole number from
−(2^53 − 1) to 2^53 − 1, written as digits, with a `-` in front of a negative one. A
file is JSON if it holds one JSON value, as RFC 8259 defines it, with nothing else but
white space before and after it.

**Problems.** `jsondiff` reads OLD and then NEW, and stops at the first problem, with a
message on stderr.

| Problem | stderr |
| --- | --- |
| A file that can't be read | `jsondiff: NAME: ` and the system's message, as in 21.1 |
| A file that isn't JSON | `jsondiff: NAME: not valid JSON` |
| Not two arguments | `usage: jsondiff OLD NEW` |

**Exit status.** `jsondiff` exits with 0 if the files hold the same value, 1 if they
differ, and 2 if anything went wrong. These are `diff`'s.

The starters for Rust, Kotlin, C++, and C come with the build files that download their
JSON libraries: serde_json, kotlinx.serialization, nlohmann/json, and cJSON. The others
have one in their standard library.

```sh
python3 tools/check.py 21 --lang go
```

**Things to notice:**

- **What a parser takes.** Python's `json.loads` reads `NaN`, `Infinity`, and
  `-Infinity`, as JavaScript writes them, and gives each to its `parse_constant`
  argument, if there is one. kotlinx's `parseToJsonElement` takes any word outside
  quotes, such as `NaN`, `yes`, or `01`, as a value whose `isString` is false.
  `cJSON_Parse` reads one value and ignores what comes after it, so that
  `{"a": 1} {"a": 2}` is `{"a": 1}`; `cJSON_ParseWithOpts` can say no. Go's
  `json.Unmarshal` checks the whole of its input, but a `json.Decoder` reads one value
  and leaves the rest. cJSON and kotlinx also read `01` as a number and a tab inside
  quotes, which JSON doesn't allow; the tests don't try those, and the solutions don't
  check for them.
- **The order of the keys.** A Python `dict`, a JavaScript object, a Dart `Map`, a
  kotlinx `JsonObject`, and cJSON's list of members keep the keys in the order of the
  file, and a Go map in no order at all. serde_json's `Map` and nlohmann's `json` keep
  them sorted. A JavaScript object puts the keys that look like array indexes first, in
  the order of their numbers, so that `Object.keys` gives `"9"` before `"10"`. And
  `"constructor" in object` is true for every object that `JSON.parse` makes, because
  `in` looks at what an object inherits too.
- **Being the same.** In Python, `True == 1` and `0 == False`, because `bool` is a kind
  of `int`. JavaScript's `===` says that two objects are the same only if they are one
  object. Go's `==` stops the program with a panic when it compares two maps held in
  `any`s.
- **Writing JSON.** Go's `json.Marshal` writes `<`, `>`, and `&` as `\u003c`, `\u003e`,
  and `\u0026`, and U+2028 and U+2029 as `\u2028` and `\u2029`, so that its JSON is safe
  in a web page's `<script>`; `SetEscapeHTML(false)` stops the first three, but not the
  others. Python's `json.dumps` writes `é` as `\u00e9` unless `ensure_ascii` is false.
  `cJSON_Print` writes 9007199254740991 as `9.00719925474099e+15`, which is
  9007199254740990, because cJSON counts that as close enough.

**Stretch:** an item put at the front of an array makes every item after it a `~` line,
as test 20 shows. Compare arrays the way `diff` compares lines: find the longest run of
items, not necessarily next to each other, that both arrays have in the same order, and
print the rest as `-` and `+` lines.
