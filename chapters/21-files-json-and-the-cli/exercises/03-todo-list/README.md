# Exercise 21.3: A to-do list

Write `todo`, which keeps a to-do list in a file, `todo.json`, in the current folder.

```text
usage: todo add TEXT...
       todo list
       todo done ID
       todo remove ID
```

```text
$ todo add buy milk
added 1: buy milk
$ todo add call Ada
added 2: call Ada
$ todo done 2
done 2: call Ada
$ todo list
1 [ ] buy milk
2 [x] call Ada
$ todo remove 1
removed 1: buy milk
```

**Commands.**

- `add` adds a task whose text is its arguments, with a space between each two. The
  task's id is one more than the largest id in the list, or 1 if there are no tasks, and
  the task isn't done.
- `list` prints each task, in the order of the file: its id, `[x]` if it's done or `[ ]`
  if it isn't, and its text.
- `done` marks the task with that ID done, and `remove` takes it out of the list.

An ID is digits. When there's no `todo.json`, the list is empty, and `add` makes the
file. `list` never writes it.

**The file.** After the commands above, `todo.json` holds:

```json
{
  "version": 1,
  "tasks": [
    {
      "id": 2,
      "text": "call Ada",
      "done": true
    }
  ]
}
```

That's JSON as `JSON.stringify(list, null, 2)` writes it, with a newline at the end: two
spaces for each level, the keys in this order, and each string written as in exercise
21.2. With no tasks, `"tasks"` is `[]`.

The file that `todo` reads must be the same: an object with two keys, `version`, which
is 1, and `tasks`, an array of tasks. Each task is an object with three keys: `id`, a
whole number from 1 up that no other task has; `text`, a string; and `done`, which is
`true` or `false`.

**Writing.** `todo` writes the new list to another file in the same folder, and then
renames that file to `todo.json`. A rename replaces a file in one step, so that if the
program stops halfway, `todo.json` holds the old list or the new one, and never half of
one. When `todo` is done, nothing but `todo.json` is left in the folder.

**Problems.** The command line is checked before `todo.json` is read, and when anything
goes wrong, `todo` leaves `todo.json` as it was.

| Problem | stderr | Status |
| --- | --- | --- |
| A command line that isn't one of the four above | a usage message | 2 |
| No task has the ID | `todo: no task ID`, with the ID as it was given | 1 |
| The file can't be read or written | `todo: todo.json: ` and the system's message | 2 |
| The file isn't JSON | `todo: todo.json: not valid JSON` | 2 |
| The file is from a later `todo` | `todo: todo.json: unknown version 2` | 2 |
| The file isn't a to-do list | `todo: todo.json: not a to-do list` | 2 |

`todo.json` is from a later `todo` when it holds an object whose `version` is a number
other than 1, whatever else the file holds, since a later `todo` may keep other things
in it. Otherwise, `todo` exits with 0.

The tests check a usage message's status, and not its words, so that an argument library
from section 21.3 can write it. The starters for Rust, Kotlin, C++, and C come with the
build files that download serde and serde_json, kotlinx.serialization, nlohmann/json,
and cJSON.

```sh
python3 tools/check.py 21 --lang go
```

**Things to notice:**

- **A missing field.** Go's `json.Unmarshal` leaves a field that the JSON doesn't have
  as it was, so that a task without `done` comes out as not done. A pointer field stays
  `nil` when its key is missing, which shows it. serde and kotlinx.serialization say
  that a missing field is an error, unless the field has a default.
- **A key that nothing reads.** serde and Go's `json.Unmarshal` ignore a key that the
  type has no field for, unless `#[serde(deny_unknown_fields)]` or a `json.Decoder`'s
  `DisallowUnknownFields` says not to. kotlinx.serialization says that it's an error,
  unless `ignoreUnknownKeys` is set.
- **A number that isn't one.** kotlinx.serialization reads `"1"`, in quotes, as the
  number 1, and `"true"` as true, and has no setting to stop it. nlohmann's `get<int>()`
  reads `true` as 1. In Python, `True` is an `int`, and `isinstance(True, int)` is true.
- **The order of the keys.** A Python `dict` and a Dart map keep their keys in the order
  they were added, and so does a JavaScript object, but for keys such as `"10"`, which
  go first, as in exercise 21.2; and serde, Go, and kotlinx.serialization write a
  struct's or class's fields in the order they were declared. nlohmann's `json` sorts
  its keys, and its `ordered_json` keeps them in order. Go's `json.Marshal` writes a
  `nil` slice as `null`, not `[]`.

**Stretch:** read the list's file name from the environment variable `TODO_FILE`, when
it's set, so that a list can be kept anywhere. The new file still has to be in the same
folder as the list, for the rename.
