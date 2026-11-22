# Exercise 16.1: Count words in parallel

Count the words in a text with several workers at once, each counting its own share of
the lines, and then add their counts up. This is the shape called map-reduce: each
worker maps its lines to a table of counts, and the main thread reduces the tables to
one.

The first line of input is two whole numbers: how many workers to use, from 1 to 16, and
how many of the most common words to print. The lines after it are the text. For
example:

```text
4 3
The river ran past the mill, and the mill turned all day.
A miller stood at the door and watched the water go by.
The water was cold, and the wheel was old,
and the miller was older than both.
At noon the miller ate bread by the river.
At night the wheel stopped, and the river ran on.
Nobody asked the river where it went.
The miller never asked the wheel why it turned.
The mill is gone now, and the river runs on.
```

prints:

```text
4 workers, 9 lines
worker 0: 3 lines, 33 words
worker 1: 2 lines, 16 words
worker 2: 2 lines, 17 words
worker 3: 2 lines, 19 words
85 words, 42 different
the: 17
and: 6
river: 5
```

The rules are:

- **Shares.** With L lines of text and W workers, each worker gets L / W lines, rounded
  down, and the first L % W workers get one more. Worker 0 gets the first lines, worker
  1 the lines after those, and so on, so a worker can get no lines at all.
- **Words.** A word is a run of the letters A to Z, in either case, and is counted in
  lower case. Everything else separates words, so `don't` is the two words `don` and
  `t`, and `abc123def` is `abc` and `def`. Blank lines count as lines.
- **Output.** First the number of workers and lines, then a line for each worker, in
  order, with its lines and words, then the total number of words and how many different
  words there are. Last come the most common words with their counts, most common first,
  and in alphabetical order when they tie. If there are fewer different words than were
  asked for, print them all. Write `1 worker`, `1 line`, and `1 word`.
- **Workers.** Run each worker on a thread of its own, or the closest thing your
  language has, and give each one a table of its own. Wait for all of them, and then add
  their tables up on the main thread. The workers don't share a table, and don't print.

| Language | A worker is |
| --- | --- |
| C | A thread from `pthread_create` |
| C++ | A thread from `std::async` or `std::jthread` |
| Rust | A scoped thread from `thread::scope` |
| Go | A goroutine |
| Python | A process from `ProcessPoolExecutor`, since Python's threads take turns running Python code |
| JavaScript and TypeScript | A worker thread from `node:worker_threads` |
| Dart | An isolate from `Isolate.run` |
| Kotlin | A thread from a pool made by `Executors` |

A problem with the first line prints one line, and nothing else.

| Problem | Output |
| --- | --- |
| It isn't two whole numbers, or there is no first line | `error: the first line must be two numbers, workers and top` |
| The number of workers isn't from 1 to 16 | `error: workers must be from 1 to 16` |

The tests use ASCII text, lines under 1000 characters, and numbers under 1000.

```sh
python3 tools/check.py 16 --lang go
```

**Things to notice:**

- **Sharing the lines.** C gives each thread a pointer to its first line and a count,
  C++ gives it a `std::span`, Go a slice, and Kotlin a `subList`, and all four are views
  of the main thread's lines, not copies. Rust's scoped threads borrow slices of
  `main`'s vector, which the borrow checker allows because `thread::scope` waits for
  every thread before it returns, so the lines outlive the threads. Reading shared
  memory is safe as long as nothing writes to it. Python's processes, JavaScript's
  workers, and Dart's isolates have no memory in common with the main thread, so each
  worker gets a copy of its lines, and its table comes back as another copy.
- **Results in order.** The workers finish in any order, but the output is in worker
  order. C reads each worker's struct after `pthread_join`, and each Go goroutine writes
  its own element of a slice, so neither needs a lock. The others keep a list of handles
  in the order the work was given, and wait on each one: `std::future::get`,
  `JoinHandle::join`, `pool.map`, `Promise.all`, `Future.wait`, and `invokeAll`.
- **Merging tables.** Python's `Counter` adds with `+=`, Kotlin's
  `merge(word, count, Int::plus)` adds or inserts, Dart's `update` takes an `ifAbsent`,
  and Rust's `entry(word).or_insert(0)` gives a place to add to. In C++ and Go,
  `total[word] += count` works on a missing key, which C++ inserts as 0 and Go reads as
  its zero value. C has no hash table in its library, so the solution has a small one.
- **Is it faster?** Not for the tests. The largest, 3,000 lines, takes about as long
  with one worker as with four: about 4 milliseconds in C, 65 in Python, and 110 in
  JavaScript, most of it starting the program. Starting a thread takes about 50
  microseconds, a Dart isolate about 0.3 milliseconds, a Python worker process 4
  milliseconds with `fork` and 60 with `spawn`, and a Node worker about 40, and then the
  lines are copied to it. Parallel work pays off when each worker has much more to do
  than that.

**Stretch:** instead of dividing the lines up before the workers start, put the lines in
a channel or queue, and have each worker take the next line when it is ready. Which
lines of the output can you still print, and why do the others change from run to run?
Then time it with 1 worker and with 4 on a text of a million lines.
