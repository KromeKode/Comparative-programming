# Exercise 16.3: An event loop

Write the loop that runs async tasks, the part that `asyncio`, Node, and Dart give you
and that `await` hands control back to. It runs on one thread, with a clock that only
moves when the loop moves it, so a task that sleeps for an hour takes no time at all.

Each line of input that starts with `task` defines a task, and the lines after it, up to
the next `task`, are its steps. A step is one of these:

| Step | What it does |
| --- | --- |
| `print TEXT` | Prints `[TIME] NAME: TEXT`, with the clock's time and the task's name |
| `sleep N` | Stops the task until the clock reaches N later than now |
| `wait NAME` | Stops the task until the task called NAME has finished |

For example:

```text
task fetch
  print start the download
  sleep 30
  print downloaded
task parse
  print wait for the download
  wait fetch
  print parsing
  sleep 5
  print parsed
task tick
  sleep 10
  print tick
  sleep 10
  print tick
  sleep 10
  print tick
```

prints:

```text
[0] fetch: start the download
[0] parse: wait for the download
[10] tick: tick
[20] tick: tick
[30] fetch: downloaded
[30] fetch: done
[30] tick: tick
[30] tick: done
[30] parse: parsing
[35] parse: parsed
[35] parse: done
[35] all done
```

The loop keeps a line of the tasks that are ready to run. The rules are:

- **Starting.** The clock starts at 0, and every task is ready, in the order the tasks
  are defined.
- **Running.** The loop takes the first task in the line, and runs its steps until one
  of them stops it, or it runs out of steps. Then it takes the next task.
- **Sleeping.** `sleep 0` puts the task at the back of the line, and lets the others run
  first. A longer sleep takes the task out of the line until its time comes.
- **Waking.** When the line is empty, the clock jumps to the time when the next sleeping
  task wakes, and every task that wakes at that time joins the line, in the order they
  went to sleep.
- **Waiting.** `wait` for a task that has finished carries on at once. Otherwise the
  task stops, and when the other task finishes, every task waiting for it joins the back
  of the line, in the order they began waiting.
- **Finishing.** After a task's last step, print `[TIME] NAME: done`. When nothing is
  ready or sleeping, print `[TIME] all done`, or, if some tasks haven't finished,
  `[TIME] deadlock: ` and, for each of them in the order they are defined,
  `NAME waits for OTHER`, separated by commas.

Read every line before running anything, since a `wait` can name a task defined further
down. A line with a problem prints one line, before the tasks run, and is left out. The
others still run.

| Problem | Output |
| --- | --- |
| A step before the first `task` | `error: line 1: a step before any task` |
| A step that isn't `print`, `sleep`, or `wait` | `error: line 4: unknown step: jump` |
| A `sleep` that isn't followed by a whole number | `error: line 5: sleep takes a whole number` |
| A `wait` for a task that doesn't exist | `error: line 7: no task named nobody` |

The first word of a line, after removing spaces from both ends, says what the line is,
and the rest of the line, with spaces removed from both ends, is the name, the text, the
time, or the task to wait for. Lines are numbered from 1, and blank lines are skipped.
Task names are single words, and all different. The tests use fewer than 100 tasks,
fewer than 100 steps in each, and lines under 200 characters.

```sh
python3 tools/check.py 16 --lang dart
```

**Things to notice:**

- **Two queues.** The loop needs a first-in, first-out line of ready tasks, and the
  sleeping tasks in order of when they wake. Python's `asyncio` keeps the same two
  things: a `deque` of callbacks that are ready, and a heap of timers. A heap gives the
  smallest item first. Python has `heapq`, which works on a list of tuples, C++ has
  `std::priority_queue`, Rust `BinaryHeap`, Go `container/heap`, and Kotlin
  `java.util.PriorityQueue`. Rust's and C++'s give the largest item first, so the Rust
  solution wraps each item in `Reverse` and the C++ solution passes `std::greater<>`. C,
  JavaScript, and TypeScript have no heap in their standard libraries, and their
  solutions search a list. Dart's is in `package:collection`, outside the core
  libraries, and its solution uses a `SplayTreeSet`, which keeps its items sorted.
- **Ties.** Two tasks can wake at the same time. The heap holds `(time, order, task)`,
  where `order` counts the sleeps, so tasks that wake at the same time come out in the
  order they went to sleep. A heap on its own doesn't keep equal items in order.
- **Where a task stops.** Each task remembers the index of its next step, so the loop
  can carry on where it stopped. That index is what a compiler keeps for a real async
  function, along with its local variables. Python and JavaScript turn the function into
  an object that holds them, C++20 puts them in a coroutine frame on the heap, and Rust
  and Kotlin turn the function into a state machine with one state for each `await` or
  suspension point.
- **Three kinds of step.** C uses an `enum` and a `union`, and the `kind` says which
  member of the union holds the step. C++ uses `std::variant`, Rust an `enum` with data,
  Dart a `sealed class`, Kotlin a `sealed interface`, and TypeScript a union of object
  types with a `kind` field. Go has no sum types, so each step is its own struct, and a
  type switch tells them apart. Python matches on dataclasses with `match`.
- **Deadlock is easy to see here.** This loop knows every task and what each one waits
  for, so it can say which tasks are stuck. Real event loops can't always tell. Node,
  for example, just stops when nothing is left to run.

**Stretch:** add a step `spawn NAME`, which starts a new copy of the task called NAME,
putting it at the back of the line. A spawned copy prints with its own name, `NAME#2`
for the second copy and so on, and `wait NAME` waits for the first one. How does the
loop's data change once tasks can be created while it runs?
