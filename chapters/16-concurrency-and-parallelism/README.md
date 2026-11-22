# Chapter 16: Concurrency and parallelism

Every program so far has done one thing at a time. This chapter is about programs with
several things going on at once: a server with a thousand connections open, a build that
compiles on every core, a game that loads its next level while you play this one. The
languages give you different pieces for it. Threads share memory, processes and isolates
don't, channels carry values from one to another, and async functions take turns on a
single thread. They also make different promises about the mistakes that concurrency
allows. Rust refuses to compile a data race, Go finds one if you ask it to, and in C a
data race is undefined behavior that can pass every test.

**Languages:** all nine · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Concurrency and parallelism, and data races, race conditions, and deadlocks | Transactions and data access (chapter 35), multiplayer networking for games (chapter 48) |
| Starting threads, giving them work, and collecting their results | Complexity and benchmarking (chapter 24), data analysis (chapter 44) |
| Mutexes and atomics for shared state, and lock ordering against deadlock | Transactions and data access (chapter 35), design problems (chapter 31) |
| Channels and queues, for passing values instead of sharing them | HTTP from sockets (chapter 36), WebSockets (chapter 39) |
| Async functions, and the event loop that runs them | REST CRUD API (chapter 38), GUI and desktop apps (chapter 49) |

---

## 16.1 The concept: several things at once

**Concurrency** is having several tasks in progress at once, and **parallelism** is
running several of them at the same instant. Rob Pike put it this way: "Concurrency is
about dealing with lots of things at once. Parallelism is about doing lots of things at
once." A web server on one core is concurrent: it switches between connections whenever
one is waiting for the network. A program that counts primes on four cores is parallel.
Concurrency is a way to structure a program, parallelism is a way to run it faster, and
a program can have either one without the other.

```text
concurrency on one core: the tasks take turns
  core 1   a a a b b a b b b a a

parallelism on two cores: the tasks run at the same time
  core 1   a a a a a a
  core 2   b b b b b b
```

The work is done by three kinds of worker:

- **Threads.** A thread is a sequence of instructions that the operating system
  schedules onto a core. The threads of one process share its memory, so they can read
  and write the same variables, and the operating system can stop a thread between any
  two instructions and run another. C, C++, Rust, Python, and Kotlin have them.
- **Processes, isolates, and workers.** A process has memory of its own, and so do
  Dart's isolates and JavaScript's workers, which are threads that share nothing with
  each other. They cooperate by sending each other copies of values.
- **Tasks.** In an async program, a task is a function that runs until it reaches an
  `await`, where it stops and lets another task run. An **event loop** decides which
  task runs next, and nothing else switches tasks, so on one thread, a task can't be
  interrupted between two statements that have no `await` between them. Go's goroutines
  are in between: they are as cheap as tasks, but the Go runtime runs them on several
  threads at once and can stop one almost anywhere, as an operating system stops a
  thread.

When workers share data, they either share memory or pass messages. With **shared
memory**, they read and write the same data, and use a **lock**, also called a
**mutex**, so that one at a time changes it, or **atomic** operations, which change a
value in one step that no other thread can see half done. With **message passing**, each
piece of data belongs to one worker at a time, and workers send values to each other
through **channels** or queues. Go's documentation puts it as "Do not communicate by
sharing memory; instead, share memory by communicating."

Three kinds of bug are new in this chapter:

- **A data race** is two threads using the same memory at the same time, without a lock
  or an atomic, where at least one of them writes. In C and C++ it is undefined
  behavior. In Java and Kotlin the program keeps running with wrong values, and in Go it
  can also crash. Safe Rust can't express one. It is the aliasing of chapter 9, two
  names for one place, with the two names on two threads.
- **A race condition** is a bug that depends on the order in which tasks happen to run,
  even when each access by itself is safe. Reading a balance, waiting for something, and
  then writing a new balance is a race condition whether the waiting is on a lock, a
  database, or an `await`, as example 04 and exercise 16.2 show.
- **A deadlock** is tasks waiting for each other in a ring, so that none of them can go
  on. Each holds something the next one needs.

---

## 16.2 What each language gives you

| Language | Threads | Shared state | Message passing | Async | A data race is |
| --- | --- | --- | --- | --- | --- |
| C | POSIX threads, `pthread_create`; C11's optional `<threads.h>` | `pthread_mutex_t`; C11's `<stdatomic.h>` | Write your own, with condition variables | None | Undefined behavior; ThreadSanitizer finds it as the program runs |
| C++ | `std::thread`, `std::jthread` (C++20), `std::async` | `std::mutex`, `std::scoped_lock`, `std::atomic` | Write your own, with `std::condition_variable` | Coroutines (C++20), with no task type or event loop in the library | The same as in C |
| Rust | `thread::spawn`, `thread::scope` | `Mutex`, `RwLock`, atomics, shared with `Arc` | `mpsc` channels | `async` and `await`, with no runtime in the library; tokio is the usual one | A compile error in safe code |
| Go | Goroutines, started with `go` | `sync.Mutex`, `sync/atomic` | Channels and `select`, built into the language | Not needed: a goroutine that waits costs little | Wrong values, or a crash; `go run -race` finds it |
| Python | `threading`, but the GIL runs one thread at a time; processes with `concurrent.futures` | `threading.Lock` | `queue.Queue` | `asyncio` | Wrong values, without memory damage |
| JavaScript | Workers, from `node:worker_threads` or the browser's `Worker` | Only `SharedArrayBuffer`, with `Atomics` | `postMessage` | Promises, `async`, `await`, and the event loop | Possible only in a `SharedArrayBuffer` |
| TypeScript | JavaScript's | JavaScript's | JavaScript's | JavaScript's, with typed promises | The same as in JavaScript |
| Dart | Isolates, which share no memory | None between isolates | `SendPort` and `ReceivePort` | `Future`, `Stream`, `async`, and `await` | Impossible, since isolates share nothing |
| Kotlin | Java's threads, thread pools, and virtual threads (Java 21) | `synchronized`, Java's locks and atomics | Java's `BlockingQueue` | `suspend` functions; `launch` and `async` are in the kotlinx.coroutines library | Wrong values, within the rules of the Java memory model |

Read the last column from top to bottom, and the languages sort themselves. C and C++
give you threads and shared memory with no checks at all, and a separate tool to find
the mistakes. Rust gives you the same threads and memory, and checks them while
compiling. Go and Kotlin check nothing while compiling. On the JVM, a race gives wrong
values but can't damage memory, and Go, which stops the program when it catches two
goroutines writing one map, has a race detector built into its toolchain. Python,
JavaScript, and Dart mostly avoid the question: Python's threads take turns,
JavaScript's workers share only what you put in a `SharedArrayBuffer`, and Dart's
isolates share nothing at all. The async column is the other split. Python, JavaScript,
TypeScript, and Dart run async code on an event loop that comes with the language; Rust,
C++, and Kotlin have the syntax but leave the loop to libraries; and Go and C have no
async functions.

---

## 16.3 Threads

Each version counts the primes below 200,000 with four workers, each counting a quarter
of the range, and adds their counts up. Counting primes is work for the processor, with
no waiting, so it gets faster only if the workers really run at the same time.

**C**

```c
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#define LIMIT 200000
#define WORKERS 4

static bool is_prime(long n) {
    if (n < 2) return false;
    for (long d = 2; d * d <= n; d++) {
        if (n % d == 0) return false;
    }
    return true;
}

// A thread runs a function that takes one void * and returns one, so the
// input and the result travel together in a struct.
struct job {
    long start, end;
    long count;
};

static void *count_primes(void *arg) {
    struct job *job = arg;
    job->count = 0;
    for (long n = job->start; n < job->end; n++) {
        if (is_prime(n)) job->count++;
    }
    return NULL;
}

int main(void) {
    pthread_t threads[WORKERS];
    struct job jobs[WORKERS];
    printf("counting primes below %d with %d workers\n", LIMIT, WORKERS);
    for (int i = 0; i < WORKERS; i++) {
        jobs[i] = (struct job){(long)LIMIT * i / WORKERS, (long)LIMIT * (i + 1) / WORKERS, 0};
        if (pthread_create(&threads[i], NULL, count_primes, &jobs[i]) != 0) {
            fprintf(stderr, "can't start thread %d\n", i);
            return 1;
        }
    }

    // pthread_join waits for a thread to finish. Each thread wrote only to its
    // own job, so after the joins, main can read them all.
    long total = 0;
    for (int i = 0; i < WORKERS; i++) {
        pthread_join(threads[i], NULL);
        printf("worker %d: [%ld, %ld) has %ld primes\n", i, jobs[i].start, jobs[i].end,
               jobs[i].count);
        total += jobs[i].count;
    }
    printf("total: %ld primes\n", total);
    return 0;
}
```

**C++**

```cpp
#include <future>
#include <iostream>
#include <vector>

constexpr long limit = 200000;
constexpr int workers = 4;

bool is_prime(long n) {
    if (n < 2) return false;
    for (long d = 2; d * d <= n; d++) {
        if (n % d == 0) return false;
    }
    return true;
}

long count_primes(long start, long end) {
    long count = 0;
    for (long n = start; n < end; n++) {
        if (is_prime(n)) count++;
    }
    return count;
}

int main() {
    std::cout << "counting primes below " << limit << " with " << workers << " workers\n";

    // std::async with std::launch::async runs the function on a new thread
    // and returns a future, which will hold its result.
    std::vector<std::future<long>> results;
    for (int i = 0; i < workers; i++) {
        results.push_back(std::async(std::launch::async, count_primes, limit * i / workers,
                                     limit * (i + 1) / workers));
    }

    // get() waits for the thread to finish and returns what it returned.
    long total = 0;
    for (int i = 0; i < workers; i++) {
        long count = results[i].get();
        std::cout << "worker " << i << ": [" << limit * i / workers << ", "
                  << limit * (i + 1) / workers << ") has " << count << " primes\n";
        total += count;
    }
    std::cout << "total: " << total << " primes\n";
}
```

**Rust**

```rust
use std::thread;

const LIMIT: u64 = 200_000;
const WORKERS: u64 = 4;

fn is_prime(n: u64) -> bool {
    n >= 2 && (2..).take_while(|d| d * d <= n).all(|d| n % d != 0)
}

fn main() {
    println!("counting primes below {LIMIT} with {WORKERS} workers");

    // thread::spawn runs a closure on a new thread. move gives the closure
    // its own copies of start and end, since the thread may outlive them.
    let handles: Vec<_> = (0..WORKERS)
        .map(|i| {
            let (start, end) = (LIMIT * i / WORKERS, LIMIT * (i + 1) / WORKERS);
            let handle = thread::spawn(move || (start..end).filter(|&n| is_prime(n)).count());
            (start, end, handle)
        })
        .collect();

    // join waits for the thread and returns its result, or Err if it panicked.
    let mut total = 0;
    for (i, (start, end, handle)) in handles.into_iter().enumerate() {
        let count = handle.join().expect("a worker panicked");
        println!("worker {i}: [{start}, {end}) has {count} primes");
        total += count;
    }
    println!("total: {total} primes");
}
```

**Go**

```go
package main

import (
	"fmt"
	"sync"
)

const limit = 200000
const workers = 4

func isPrime(n int) bool {
	if n < 2 {
		return false
	}
	for d := 2; d*d <= n; d++ {
		if n%d == 0 {
			return false
		}
	}
	return true
}

func countPrimes(start, end int) int {
	count := 0
	for n := start; n < end; n++ {
		if isPrime(n) {
			count++
		}
	}
	return count
}

func main() {
	fmt.Printf("counting primes below %d with %d workers\n", limit, workers)

	// go starts a function on a new goroutine. Each one writes only its own
	// element of counts, and the WaitGroup counts the goroutines still running.
	counts := make([]int, workers)
	var wg sync.WaitGroup
	for i := range workers {
		wg.Add(1)
		go func() {
			defer wg.Done()
			counts[i] = countPrimes(limit*i/workers, limit*(i+1)/workers)
		}()
	}
	wg.Wait() // waits until every goroutine has called Done

	total := 0
	for i, count := range counts {
		fmt.Printf("worker %d: [%d, %d) has %d primes\n", i, limit*i/workers,
			limit*(i+1)/workers, count)
		total += count
	}
	fmt.Printf("total: %d primes\n", total)
}
```

**Python**

```python
from concurrent.futures import ProcessPoolExecutor

LIMIT = 200_000
WORKERS = 4


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    d = 2
    while d * d <= n:
        if n % d == 0:
            return False
        d += 1
    return True


def count_primes(start: int, end: int) -> int:
    return sum(1 for n in range(start, end) if is_prime(n))


def main() -> None:
    print(f"counting primes below {LIMIT} with {WORKERS} workers")
    starts = [LIMIT * i // WORKERS for i in range(WORKERS)]
    ends = [LIMIT * (i + 1) // WORKERS for i in range(WORKERS)]

    # Python's threads take turns running Python code, so for work like this
    # each worker is a separate process. map sends the arguments to the
    # processes and gives back the results in order.
    with ProcessPoolExecutor(max_workers=WORKERS) as pool:
        counts = list(pool.map(count_primes, starts, ends))

    for i, count in enumerate(counts):
        print(f"worker {i}: [{starts[i]}, {ends[i]}) has {count} primes")
    print(f"total: {sum(counts)} primes")


# A worker process can start by importing this file, as it does on macOS and
# Windows, and then main must not run again.
if __name__ == "__main__":
    main()
```

**JavaScript**

```js
import {
  Worker,
  isMainThread,
  parentPort,
  workerData,
} from "node:worker_threads";

const LIMIT = 200_000;
const WORKERS = 4;

function isPrime(n) {
  if (n < 2) return false;
  for (let d = 2; d * d <= n; d++) {
    if (n % d === 0) return false;
  }
  return true;
}

function countPrimes({ start, end }) {
  let count = 0;
  for (let n = start; n < end; n++) {
    if (isPrime(n)) count++;
  }
  return count;
}

// A worker is a separate JavaScript thread with its own variables. It gets
// its input as a copy, workerData, and answers with a message.
function runWorker(job) {
  const worker = new Worker(new URL(import.meta.url), { workerData: job });
  return new Promise((resolve, reject) => {
    worker.once("message", resolve);
    worker.once("error", reject);
  });
}

// The same file runs on the main thread and in every worker.
if (isMainThread) {
  console.log(`counting primes below ${LIMIT} with ${WORKERS} workers`);
  const jobs = [];
  for (let i = 0; i < WORKERS; i++) {
    jobs.push({
      start: Math.floor((LIMIT * i) / WORKERS),
      end: Math.floor((LIMIT * (i + 1)) / WORKERS),
    });
  }
  // Promise.all waits for every worker, and keeps the results in order.
  const counts = await Promise.all(jobs.map(runWorker));
  for (const [i, { start, end }] of jobs.entries()) {
    console.log(`worker ${i}: [${start}, ${end}) has ${counts[i]} primes`);
  }
  console.log(`total: ${counts.reduce((a, b) => a + b, 0)} primes`);
} else {
  parentPort.postMessage(countPrimes(workerData));
}
```

**TypeScript**

```ts
import {
  Worker,
  isMainThread,
  parentPort,
  workerData,
} from "node:worker_threads";

const LIMIT = 200_000;
const WORKERS = 4;

interface Job {
  start: number;
  end: number;
}

function isPrime(n: number): boolean {
  if (n < 2) return false;
  for (let d = 2; d * d <= n; d++) {
    if (n % d === 0) return false;
  }
  return true;
}

function countPrimes({ start, end }: Job): number {
  let count = 0;
  for (let n = start; n < end; n++) {
    if (isPrime(n)) count++;
  }
  return count;
}

// A worker is a separate JavaScript thread with its own variables. It gets
// its input as a copy, workerData, and answers with a message.
function runWorker(job: Job): Promise<number> {
  const worker = new Worker(new URL(import.meta.url), { workerData: job });
  return new Promise((resolve, reject) => {
    worker.once("message", resolve);
    worker.once("error", reject);
  });
}

// The same file runs on the main thread and in every worker.
if (isMainThread) {
  console.log(`counting primes below ${LIMIT} with ${WORKERS} workers`);
  const jobs: Job[] = [];
  for (let i = 0; i < WORKERS; i++) {
    jobs.push({
      start: Math.floor((LIMIT * i) / WORKERS),
      end: Math.floor((LIMIT * (i + 1)) / WORKERS),
    });
  }
  // Promise.all waits for every worker, and keeps the results in order.
  const counts = await Promise.all(jobs.map(runWorker));
  for (const [i, { start, end }] of jobs.entries()) {
    console.log(`worker ${i}: [${start}, ${end}) has ${counts[i]} primes`);
  }
  console.log(`total: ${counts.reduce((a, b) => a + b, 0)} primes`);
} else {
  parentPort?.postMessage(countPrimes(workerData as Job));
}
```

**Dart**

```dart
import 'dart:isolate';

const limit = 200000;
const workers = 4;

bool isPrime(int n) {
  if (n < 2) return false;
  for (var d = 2; d * d <= n; d++) {
    if (n % d == 0) return false;
  }
  return true;
}

int countPrimes(int start, int end) {
  var count = 0;
  for (var n = start; n < end; n++) {
    if (isPrime(n)) count++;
  }
  return count;
}

Future<void> main() async {
  print('counting primes below $limit with $workers workers');
  final jobs = [
    for (var i = 0; i < workers; i++)
      (start: limit * i ~/ workers, end: limit * (i + 1) ~/ workers),
  ];

  // Isolate.run starts an isolate, a Dart thread with its own memory, runs
  // the function there, and returns its result as a Future. Future.wait
  // waits for all of them, and keeps the results in order.
  final counts = await Future.wait([
    for (final job in jobs) Isolate.run(() => countPrimes(job.start, job.end)),
  ]);
  for (final (i, job) in jobs.indexed) {
    print('worker $i: [${job.start}, ${job.end}) has ${counts[i]} primes');
  }
  print('total: ${counts.reduce((a, b) => a + b)} primes');
}
```

**Kotlin**

```kotlin
import java.util.concurrent.Callable
import java.util.concurrent.Executors

const val LIMIT = 200_000
const val WORKERS = 4

fun isPrime(n: Int): Boolean {
    if (n < 2) return false
    var d = 2
    while (d * d <= n) {
        if (n % d == 0) return false
        d++
    }
    return true
}

fun countPrimes(start: Int, end: Int) = (start until end).count(::isPrime)

fun main() {
    println("counting primes below $LIMIT with $WORKERS workers")
    val jobs = (0 until WORKERS).map { i -> LIMIT * i / WORKERS to LIMIT * (i + 1) / WORKERS }

    // A thread pool runs the tasks it is given on its own threads. submit
    // returns a Future, get waits for its result, and use closes the pool,
    // which waits for every task to finish.
    val counts =
        Executors.newFixedThreadPool(WORKERS).use { pool ->
            jobs
                .map { (start, end) -> pool.submit(Callable { countPrimes(start, end) }) }
                .map { it.get() }
        }
    for ((i, job) in jobs.withIndex()) {
        println("worker $i: [${job.first}, ${job.second}) has ${counts[i]} primes")
    }
    println("total: ${counts.sum()} primes")
}
```

All nine print:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
counting primes below 200000 with 4 workers
worker 0: [0, 50000) has 5133 primes
worker 1: [50000, 100000) has 4459 primes
worker 2: [100000, 150000) has 4256 primes
worker 3: [150000, 200000) has 4136 primes
total: 17984 primes
```

**C: a function and a struct.** `pthread_create` starts a thread that runs a function,
which takes one `void *` and returns one, so each thread's input and its result travel
together in a `struct job`. `pthread_join` waits for a thread to finish, and once it
returns, `main` can read what that thread wrote. Each thread writes only its own job, so
nothing is shared while they run. Forgetting the joins is the classic mistake: when
`main` returns, the process ends, and its threads with it, wherever they were. C11 added
threads to the C library, as `thrd_create` and `thrd_join` in `<threads.h>`, but made
them optional, and most C code uses POSIX threads. The build has no `-pthread` flag,
because since glibc 2.34 the thread functions are in the C library itself.

**C++: futures.** `std::async(std::launch::async, f, args...)` runs `f` on a new thread
and returns a `std::future`, and `get` waits for the thread and returns what `f`
returned, or throws what it threw. Without `std::launch::async`, the library may instead
run `f` later, on the thread that calls `get`. `std::thread` is the lower-level tool,
and it has a trap: a `std::thread` must be joined or detached before it is destroyed, or
the program stops with `terminate called without an active exception`. C++20's
`std::jthread` joins in its destructor, so the next two examples use it.

**Rust: closures that own what they use.** `thread::spawn` takes a closure, runs it on a
new thread, and returns a `JoinHandle`, whose `join` returns the closure's result, or an
`Err` if the thread panicked. The new thread might outlive the function that started it,
so the closure must own everything it uses, which is what `move` does. Without `move`,
the compiler stops with `error[E0373]`: the closure "may outlive the current function,
but it borrows `end`, which is owned by the current function". Whatever the closure
takes must also be `Send`, safe to hand to another thread. An `Rc`, whose reference
count isn't atomic, isn't, and moving one into a thread is
`error[E0277]: 'Rc<i32>' cannot be sent between threads safely`. When threads need to
borrow instead, `thread::scope`, from Rust 1.63, waits for every thread started in it
before it returns, so the borrowed values outlive the threads. Example 02 and exercise
16.1 use it.

**Go: goroutines.** `go f()` runs `f` on a new goroutine and carries on at once. A
goroutine starts with a stack of a few kilobytes that grows when it needs to, and the Go
runtime spreads goroutines over a few operating system threads, so a program can have a
hundred thousand of them. `go` gives back no result, so each goroutine writes its count
into its own element of the slice `counts`. A `sync.WaitGroup` counts the goroutines
that are still running: `Add(1)` before starting one, `Done` when it finishes, and
`Wait` until the count is zero. Go 1.25 adds `wg.Go(f)`, which does all three. Before Go
1.22, every iteration of a loop shared one `i`, and goroutines that captured it all saw
whatever value it had last. Since 1.22, each iteration has its own.

**Python: processes, because of the GIL.** CPython has a **global interpreter lock**,
the GIL, which lets one thread at a time run Python code. Threads still help a program
that spends its time waiting, on files, the network, or `time.sleep`, because a thread
lets go of the GIL while it waits. For counting primes they don't help at all. With a
pool of threads instead, this example took 0.43 seconds with one thread and 0.49 with
four, and with processes, 0.45 seconds with one and 0.18 with four. So it uses
`ProcessPoolExecutor`, which starts worker processes, each with its own interpreter and
its own GIL. `map` sends each process its arguments by **pickling** them, turning them
into bytes, and the results come back the same way, in order. How a worker process
starts depends on the system. Linux has used `fork`, which copies the running process,
and macOS and Windows use `spawn`, which starts a new interpreter that imports the main
file, as does `forkserver`, Linux's default from Python 3.14. Importing must not start
another pool, which is why `main` runs only under `if __name__ == "__main__":`. Without
that guard, the import tries to, and fails with a `RuntimeError`: "An attempt has been
made to start a new process before the current process has finished its bootstrapping
phase." Python 3.13 also comes in a build without the GIL, which Python 3.14 supports
officially, and in which threads run Python code in parallel.

**JavaScript and TypeScript: workers.** A Node worker is a separate JavaScript engine on
a thread of its own, with its own variables and its own event loop. The same file runs
on the main thread and in every worker, and `isMainThread` says which one it is. The
worker receives its input as `workerData` and answers with `postMessage`, which arrives
on the main thread as a `message` event. `runWorker` wraps that in a promise, and
`Promise.all` waits for all four and keeps their results in order. Everything sent
either way is copied with the **structured clone** algorithm, which copies numbers,
strings, arrays, objects, `Map`s, and `Set`s, but not functions: putting one in
`workerData` throws `DataCloneError: (n) => n * n could not be cloned.` Browsers have
the same thing as `Worker`. The TypeScript version declares that `runWorker` returns
`Promise<number>`, which TypeScript has to take on trust, since a message can be
anything.

**Dart: isolates.** `Isolate.run`, from Dart 2.19, starts an isolate, runs a function in
it, and returns its result as a `Future`. An isolate has its own memory, so the closure,
and everything it captures, is copied into the new isolate, and the result is copied
back. Some objects can't be copied, such as a `ReceivePort` or an open socket. A closure
that captures one fails with
`Invalid argument(s): Illegal argument in isolate message: object is unsendable`, and
the trap is that closures in one function share their captured variables, so a closure
can fail because another closure beside it uses a port. Isolates are much cheaper to
start than processes, since all of them live in one process, but each has a heap of its
own, with its own copy of every top-level variable.

**Kotlin: Java's thread pools.** Kotlin on the JVM uses Java's threads, and
`java.util.concurrent` for the rest. `Executors.newFixedThreadPool(WORKERS)` makes a
pool of four threads, which take tasks from a queue. `submit` gives the pool a task and
returns a `Future`, and `get` waits for its result. A pool keeps its threads for the
next task, which is cheaper than starting a thread for each one, and `use` shuts it
down, waiting for its tasks, at the end of the block. Java 21 added **virtual threads**,
which the JVM schedules onto a few operating system threads, as Go does with goroutines,
and `Executors.newVirtualThreadPerTaskExecutor()` starts one for each task. Kotlin's own
lightweight tasks are coroutines, but its standard library has only their foundations.
`launch`, `async`, and the dispatchers that run coroutines on threads are in the
kotlinx.coroutines library, which this book's plain `kotlinc` doesn't include, so the
examples use Java's tools.

---

## 16.4 Shared state: locks and atomics

Each version has four threads add 1 to a shared count, 100,000 times each, once with a
lock around the addition and once with an atomic count.

**C**

```c
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

#define THREADS 4
#define TIMES 100000

// count += 1 reads count, adds 1, and writes it back. Two threads that do it
// at once can both read the same value, and one of the additions is lost.
// That is a data race, and in C it is undefined behavior.

static long locked_count = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// A mutex lets one thread at a time run the code between lock and unlock.
static void *add_with_mutex(void *arg) {
    (void)arg;
    for (int i = 0; i < TIMES; i++) {
        pthread_mutex_lock(&lock);
        locked_count += 1;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

// An atomic variable can be read, changed, and written as one step.
static atomic_long atomic_count = 0;

static void *add_with_atomic(void *arg) {
    (void)arg;
    for (int i = 0; i < TIMES; i++) atomic_fetch_add(&atomic_count, 1);
    return NULL;
}

static void run(void *(*work)(void *)) {
    pthread_t threads[THREADS];
    for (int i = 0; i < THREADS; i++) pthread_create(&threads[i], NULL, work, NULL);
    for (int i = 0; i < THREADS; i++) pthread_join(threads[i], NULL);
}

int main(void) {
    printf("%d threads add 1, %d times each\n", THREADS, TIMES);
    run(add_with_mutex);
    printf("with a mutex: %ld\n", locked_count);
    run(add_with_atomic);
    printf("with an atomic: %ld\n", atomic_load(&atomic_count));
    return 0;
}
```

**C++**

```cpp
#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

constexpr int threads = 4;
constexpr int times = 100000;

// Runs work on several threads at once. A std::jthread joins its thread when
// it is destroyed, so the vector's destructor waits for all of them.
void run(const std::function<void()>& work) {
    std::vector<std::jthread> workers;
    for (int i = 0; i < threads; i++) workers.emplace_back(work);
}

int main() {
    std::cout << threads << " threads add 1, " << times << " times each\n";

    // A lock_guard locks the mutex when it is made and unlocks it when it
    // goes out of scope, even if the code between throws.
    long locked_count = 0;
    std::mutex lock;
    run([&] {
        for (int i = 0; i < times; i++) {
            std::lock_guard guard(lock);
            locked_count += 1;
        }
    });
    std::cout << "with a mutex: " << locked_count << "\n";

    // ++ on a std::atomic reads, adds, and writes as one step.
    std::atomic<long> atomic_count = 0;
    run([&] {
        for (int i = 0; i < times; i++) ++atomic_count;
    });
    std::cout << "with an atomic: " << atomic_count << "\n";
}
```

**Rust**

```rust
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, Mutex};
use std::thread;

const THREADS: usize = 4;
const TIMES: u64 = 100_000;

fn main() {
    println!("{THREADS} threads add 1, {TIMES} times each");

    // A Mutex owns the value it protects, and lock() is the only way to reach
    // it. Arc is a reference count that threads can share, so each thread
    // gets its own handle to the one Mutex.
    let locked_count = Arc::new(Mutex::new(0));
    let handles: Vec<_> = (0..THREADS)
        .map(|_| {
            let count = Arc::clone(&locked_count);
            thread::spawn(move || {
                for _ in 0..TIMES {
                    // The guard unlocks the mutex when it goes out of scope.
                    *count.lock().unwrap() += 1;
                }
            })
        })
        .collect();
    for handle in handles {
        handle.join().unwrap();
    }
    println!("with a mutex: {}", locked_count.lock().unwrap());

    // thread::scope waits for every thread spawned in it before it returns,
    // so they can borrow local variables, and no Arc is needed.
    let atomic_count = AtomicU64::new(0);
    thread::scope(|scope| {
        for _ in 0..THREADS {
            scope.spawn(|| {
                for _ in 0..TIMES {
                    atomic_count.fetch_add(1, Ordering::Relaxed);
                }
            });
        }
    });
    println!("with an atomic: {}", atomic_count.load(Ordering::Relaxed));
}
```

**Go**

```go
package main

import (
	"fmt"
	"sync"
	"sync/atomic"
)

const threads = 4
const times = 100000

// run starts work on several goroutines and waits for all of them.
func run(work func()) {
	var wg sync.WaitGroup
	for range threads {
		wg.Add(1)
		go func() {
			defer wg.Done()
			work()
		}()
	}
	wg.Wait()
}

func main() {
	fmt.Printf("%d goroutines add 1, %d times each\n", threads, times)

	// Lock and Unlock let one goroutine at a time run the code between them.
	lockedCount := 0
	var mu sync.Mutex
	run(func() {
		for range times {
			mu.Lock()
			lockedCount++
			mu.Unlock()
		}
	})
	fmt.Println("with a mutex:", lockedCount)

	// Add reads, adds, and writes as one step.
	var atomicCount atomic.Int64
	run(func() {
		for range times {
			atomicCount.Add(1)
		}
	})
	fmt.Println("with an atomic:", atomicCount.Load())
}
```

**Python**

```python
import threading

THREADS = 4
TIMES = 100_000

locked_count = 0
lock = threading.Lock()


def add_with_lock() -> None:
    global locked_count
    for _ in range(TIMES):
        # with lock: holds the lock for the block, and releases it on the way
        # out, even if the block raises.
        with lock:
            locked_count += 1


def main() -> None:
    print(f"{THREADS} threads add 1, {TIMES} times each")
    threads = [threading.Thread(target=add_with_lock) for _ in range(THREADS)]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()
    print(f"with a lock: {locked_count}")
    # Python has no atomic integers. Even with the GIL, which lets only one
    # thread run Python code at a time, += is several steps, and a thread
    # can be paused between them.


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
import { Worker, isMainThread, workerData } from "node:worker_threads";

const THREADS = 4;
const TIMES = 100_000;

// Every value sent to a worker is copied, except a SharedArrayBuffer, which
// is memory that every thread it is sent to can see. An Int32Array reads it
// as 32-bit integers.
function runWorker(shared) {
  const worker = new Worker(new URL(import.meta.url), { workerData: shared });
  return new Promise((resolve, reject) => {
    worker.once("exit", () => resolve());
    worker.once("error", reject);
  });
}

if (isMainThread) {
  console.log(`${THREADS} threads add 1, ${TIMES} times each`);
  const shared = new SharedArrayBuffer(4);
  await Promise.all(Array.from({ length: THREADS }, () => runWorker(shared)));
  console.log(`with an atomic: ${Atomics.load(new Int32Array(shared), 0)}`);
} else {
  // Atomics.add reads, adds, and writes as one step. JavaScript has no
  // mutex, but Atomics.wait and Atomics.notify are enough to build one.
  const count = new Int32Array(workerData);
  for (let i = 0; i < TIMES; i++) Atomics.add(count, 0, 1);
}
```

**TypeScript**

```ts
import { Worker, isMainThread, workerData } from "node:worker_threads";

const THREADS = 4;
const TIMES = 100_000;

// Every value sent to a worker is copied, except a SharedArrayBuffer, which
// is memory that every thread it is sent to can see. An Int32Array reads it
// as 32-bit integers.
function runWorker(shared: SharedArrayBuffer): Promise<void> {
  const worker = new Worker(new URL(import.meta.url), { workerData: shared });
  return new Promise((resolve, reject) => {
    worker.once("exit", () => resolve());
    worker.once("error", reject);
  });
}

if (isMainThread) {
  console.log(`${THREADS} threads add 1, ${TIMES} times each`);
  const shared = new SharedArrayBuffer(4);
  await Promise.all(Array.from({ length: THREADS }, () => runWorker(shared)));
  console.log(`with an atomic: ${Atomics.load(new Int32Array(shared), 0)}`);
} else {
  // Atomics.add reads, adds, and writes as one step. JavaScript has no
  // mutex, but Atomics.wait and Atomics.notify are enough to build one.
  const count = new Int32Array(workerData as SharedArrayBuffer);
  for (let i = 0; i < TIMES; i++) Atomics.add(count, 0, 1);
}
```

**Dart**

```dart
import 'dart:isolate';

const isolates = 4;
const times = 100000;

// Each isolate has its own copy of every top-level variable, starting from
// its initial value. Isolates share no memory, so they can't race on it.
var count = 0;

int addMany() {
  for (var i = 0; i < times; i++) {
    count += 1;
  }
  return count;
}

Future<void> main() async {
  print('$isolates isolates add 1, $times times each');
  final results = await Future.wait([
    for (var i = 0; i < isolates; i++) Isolate.run(addMany),
  ]);
  print('each isolate counted: ${results.join(', ')}');
  print("main's count: $count");
  // The only way to combine their work is to send the results back.
  print('added up: ${results.reduce((a, b) => a + b)}');
}
```

**Kotlin**

```kotlin
import java.util.concurrent.atomic.AtomicLong
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.thread
import kotlin.concurrent.withLock

const val THREADS = 4
const val TIMES = 100_000

// Starts work on several threads, and waits for all of them.
fun run(work: () -> Unit) {
    val threads = List(THREADS) { thread { work() } }
    threads.forEach { it.join() }
}

fun main() {
    println("$THREADS threads add 1, $TIMES times each")

    // withLock holds the lock while the block runs, and releases it after,
    // even if the block throws.
    var lockedCount = 0L
    val lock = ReentrantLock()
    run { repeat(TIMES) { lock.withLock { lockedCount++ } } }
    println("with a lock: $lockedCount")

    // incrementAndGet reads, adds, and writes as one step.
    val atomicCount = AtomicLong()
    run { repeat(TIMES) { atomicCount.incrementAndGet() } }
    println("with an atomic: $atomicCount")
}
```

The C, C++, and Rust versions print the same, and the others have their own:

*C, C++ and Rust:*

```text
4 threads add 1, 100000 times each
with a mutex: 400000
with an atomic: 400000
```

*Go:*

```text
4 goroutines add 1, 100000 times each
with a mutex: 400000
with an atomic: 400000
```

*Python:*

```text
4 threads add 1, 100000 times each
with a lock: 400000
```

*JavaScript and TypeScript:*

```text
4 threads add 1, 100000 times each
with an atomic: 400000
```

*Dart:*

```text
4 isolates add 1, 100000 times each
each isolate counted: 100000, 100000, 100000, 100000
main's count: 0
added up: 400000
```

*Kotlin:*

```text
4 threads add 1, 100000 times each
with a lock: 400000
with an atomic: 400000
```

**Without either.** `count += 1` is three steps: read the count, add 1, and write it
back. When two threads read the same value, both write that value plus 1, and one of the
additions is lost. With the lock and the atomic taken out, where the right answer is
400,000, three runs each gave 107,828, 126,489, and 220,998 in C compiled without
optimization, 309,764, 310,958, and 340,527 in Go, 159,887, 112,264, and 121,104 in
Kotlin, and 124,893, 289,657, and 277,235 in JavaScript with a `SharedArrayBuffer`. The
same C compiled with `-O2` printed 400,000 in 20 runs out of 20, because GCC turned each
thread's loop into one addition of 100,000. The compiler may assume that no other thread
touches `count` without synchronization, since that would be a data race, and a data
race is undefined behavior. So the racy program passes its test, until a change
elsewhere changes what the compiler does. ThreadSanitizer, `cc -fsanitize=thread`,
watches every access as the program runs, and finds the race whether or not the answer
comes out right. Its report, with the paths shortened, starts:

```text
WARNING: ThreadSanitizer: data race (pid=12751)
  Read of size 8 at 0x5595107fd018 by thread T2:
    #0 add racy.c:8
  Previous write of size 8 at 0x5595107fd018 by thread T1:
    #0 add racy.c:8
  Location is global 'count' of size 8 at 0x5595107fd018
```

Go's race detector does the same job for Go, with `go run -race`, which reports
`WARNING: DATA RACE` and `Found 2 data race(s)` for the racy version. `go vet` says
nothing about it.

**A lock** lets one thread at a time run the code between locking and unlocking. C calls
`pthread_mutex_lock` and `pthread_mutex_unlock`, and forgetting the unlock, on an early
`return`, leaves every other thread waiting forever. C++'s `std::lock_guard` locks in
its constructor and unlocks in its destructor, so the mutex is unlocked however the
block ends, which is the RAII of chapter 10. Python's `with lock:` and Kotlin's
`lock.withLock { }` do the same for a block. Go has no destructors, and puts
`defer mu.Unlock()` right after `mu.Lock()` when the locked code is longer than one
line.

**An atomic** is a variable whose read, change, and write happen as one step. C's
`atomic_fetch_add`, C++'s `++` on a `std::atomic<long>`, Rust's `fetch_add`, Go's
`atomic.Int64`, Kotlin's `AtomicLong`, and JavaScript's `Atomics.add` compile to
instructions that the processor makes atomic, such as `lock addq $0x1`, which GCC writes
for the C version on x86. An atomic is faster than a lock for a single number, but it
protects only that number. Two atomic balances don't make a transfer between them
atomic, which is why exercise 16.2 uses locks.

**Rust: the lock owns the data.** In every other language here, the lock and the count
are two separate variables, and only the programmer knows that one guards the other.
Rust's `Mutex<i32>` holds the count inside it, and `lock()` is the only way to reach it.
It returns, inside a `Result`, a guard that gives access to the count and unlocks when
it goes out of scope. The `Result` is `Err` if another thread panicked while it held the
lock, since the data may be half changed, and the example calls `unwrap`. The four
threads need to share the one `Mutex`, so it is inside an `Arc`, an atomically counted
`Rc`. The atomic version uses `thread::scope` instead, so its threads can borrow
`atomic_count` from `main`, and no `Arc` is needed. Without the atomic, the same scoped
threads adding to a plain local count don't compile:
`error[E0499]: cannot borrow 'count' as mutable more than once at a time`. That is the
borrowing rule of chapter 10, one writer or many readers, and it is the same rule that
rules out data races. `Ordering::Relaxed` says that the count needs to be atomic, but
needn't put other memory accesses in order around it, which is enough for a counter. C's
`atomic_fetch_add` and C++'s `++` use the strictest order, `seq_cst`, by default.

**Python: the GIL isn't a lock for you.** Without the lock, the Python version printed
400,000 in 5 runs out of 5, because CPython 3.13 happens not to switch threads in the
middle of `count += 1` on a global. Nothing promises that. With the read and the write
split by `time.sleep(0)`, which lets another thread run, the total was 100,001, and in
the build without the GIL, threads really run in parallel. Python has no atomic
integers, so the lock is the tool.

**JavaScript: shared memory, only by request.** A worker gets copies of what it is sent,
except a `SharedArrayBuffer`, which is the same memory on every thread it is sent to. An
`Int32Array` over it reads it as 32-bit integers, and `Atomics.add` adds to one of them
as a single step. JavaScript has no mutex, but `Atomics.wait` and `Atomics.notify` are
enough to build one. Browsers allow `SharedArrayBuffer` only on pages that opt into
cross-origin isolation, a rule added after the Spectre attacks of 2018.

**Dart: nothing to share.** Each isolate has its own copy of every top-level variable,
so each counted to 100,000 in its own `count`, and `main`'s stayed 0. The isolates can't
race, and also can't share the count: the only way to combine their work is to send the
results back and add them up.

---

## 16.5 Message passing: channels and queues

Each version has a producer that sends the numbers 1 to 1000 into a channel, three
workers that take numbers out, square them, and send the squares into a second channel,
and a main thread that adds the squares up. Each number goes to exactly one worker.

**C**

```c
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

#define COUNT 1000
#define WORKERS 3
#define CAPACITY 16

// C has no channels, so here is one: a ring buffer, a mutex that guards it,
// and two condition variables, which let a thread sleep until another one
// says that something has changed.
struct channel {
    long items[CAPACITY];
    int head, size;
    bool closed;
    pthread_mutex_t lock;
    pthread_cond_t not_empty, not_full;
};

static void channel_init(struct channel *ch) {
    *ch = (struct channel){.head = 0};
    pthread_mutex_init(&ch->lock, NULL);
    pthread_cond_init(&ch->not_empty, NULL);
    pthread_cond_init(&ch->not_full, NULL);
}

static void channel_send(struct channel *ch, long item) {
    pthread_mutex_lock(&ch->lock);
    // pthread_cond_wait unlocks the mutex while it sleeps, and locks it again
    // before it returns. It can wake for no reason, so the test is a loop.
    while (ch->size == CAPACITY) pthread_cond_wait(&ch->not_full, &ch->lock);
    ch->items[(ch->head + ch->size) % CAPACITY] = item;
    ch->size++;
    pthread_cond_signal(&ch->not_empty);
    pthread_mutex_unlock(&ch->lock);
}

// Returns false once the channel is closed and empty.
static bool channel_receive(struct channel *ch, long *item) {
    pthread_mutex_lock(&ch->lock);
    while (ch->size == 0 && !ch->closed) pthread_cond_wait(&ch->not_empty, &ch->lock);
    bool got = ch->size > 0;
    if (got) {
        *item = ch->items[ch->head];
        ch->head = (ch->head + 1) % CAPACITY;
        ch->size--;
        pthread_cond_signal(&ch->not_full);
    }
    pthread_mutex_unlock(&ch->lock);
    return got;
}

static void channel_close(struct channel *ch) {
    pthread_mutex_lock(&ch->lock);
    ch->closed = true;
    pthread_cond_broadcast(&ch->not_empty);  // wake every waiting receiver
    pthread_mutex_unlock(&ch->lock);
}

static struct channel jobs, results;
static atomic_int workers_left = WORKERS;

static void *produce(void *arg) {
    (void)arg;
    for (long n = 1; n <= COUNT; n++) channel_send(&jobs, n);
    channel_close(&jobs);
    return NULL;
}

static void *work(void *arg) {
    (void)arg;
    long n;
    while (channel_receive(&jobs, &n)) channel_send(&results, n * n);
    // The last worker to finish closes the results.
    if (atomic_fetch_sub(&workers_left, 1) == 1) channel_close(&results);
    return NULL;
}

int main(void) {
    channel_init(&jobs);
    channel_init(&results);
    pthread_t producer, workers[WORKERS];
    pthread_create(&producer, NULL, produce, NULL);
    for (int i = 0; i < WORKERS; i++) pthread_create(&workers[i], NULL, work, NULL);

    long received = 0, sum = 0, square;
    while (channel_receive(&results, &square)) {
        received++;
        sum += square;
    }
    pthread_join(producer, NULL);
    for (int i = 0; i < WORKERS; i++) pthread_join(workers[i], NULL);
    printf("%d workers squared 1 to %d\n", WORKERS, COUNT);
    printf("results: %ld\n", received);
    printf("sum: %ld\n", sum);
    return 0;
}
```

**C++**

```cpp
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

// The standard library has no channel, but it has the parts: a queue, a
// mutex that guards it, and condition variables, which let a thread sleep
// until another one says that something has changed.
template <typename T>
class Channel {
public:
    explicit Channel(std::size_t capacity) : capacity_(capacity) {}

    void send(T item) {
        std::unique_lock lock(mutex_);
        // wait unlocks the mutex while it sleeps, and returns, locked, once
        // the condition is true.
        not_full_.wait(lock, [&] { return items_.size() < capacity_; });
        items_.push(std::move(item));
        not_empty_.notify_one();
    }

    // Returns nothing once the channel is closed and empty.
    std::optional<T> receive() {
        std::unique_lock lock(mutex_);
        not_empty_.wait(lock, [&] { return !items_.empty() || closed_; });
        if (items_.empty()) return std::nullopt;
        T item = std::move(items_.front());
        items_.pop();
        not_full_.notify_one();
        return item;
    }

    void close() {
        std::lock_guard lock(mutex_);
        closed_ = true;
        not_empty_.notify_all();  // wake every waiting receiver
    }

private:
    std::mutex mutex_;
    std::condition_variable not_empty_, not_full_;
    std::queue<T> items_;
    std::size_t capacity_;
    bool closed_ = false;
};

constexpr long count = 1000;
constexpr int workers = 3;

int main() {
    Channel<long> jobs(16), results(16);
    std::atomic<int> workers_left = workers;

    std::jthread producer([&] {
        for (long n = 1; n <= count; n++) jobs.send(n);
        jobs.close();
    });

    std::vector<std::jthread> pool;
    for (int i = 0; i < workers; i++) {
        pool.emplace_back([&] {
            while (auto n = jobs.receive()) results.send(*n * *n);
            // The last worker to finish closes the results.
            if (--workers_left == 0) results.close();
        });
    }

    long received = 0, sum = 0;
    while (auto square = results.receive()) {
        received++;
        sum += *square;
    }
    std::cout << workers << " workers squared 1 to " << count << "\n";
    std::cout << "results: " << received << "\n";
    std::cout << "sum: " << sum << "\n";
}
```

**Rust**

```rust
use std::sync::mpsc;
use std::sync::{Arc, Mutex};
use std::thread;

const COUNT: u64 = 1000;
const WORKERS: usize = 3;

fn main() {
    // mpsc means many producers, one consumer: a Sender can be cloned, and
    // the Receiver can't. sync_channel(16) holds up to 16 values, and a send
    // to a full channel waits for room.
    let (job_sender, job_receiver) = mpsc::sync_channel::<u64>(16);
    let (result_sender, result_receiver) = mpsc::sync_channel::<u64>(16);

    // Several workers take jobs from one Receiver, so they share it behind a
    // Mutex, and each one locks it to take a job.
    let job_receiver = Arc::new(Mutex::new(job_receiver));
    for _ in 0..WORKERS {
        let jobs = Arc::clone(&job_receiver);
        let results = result_sender.clone();
        thread::spawn(move || {
            loop {
                // The lock is released at the end of this statement, before
                // the work starts. recv returns Err once every Sender is gone.
                let job = jobs.lock().unwrap().recv();
                let Ok(n) = job else { break };
                results.send(n * n).unwrap();
            }
        });
    }
    // Only the workers' clones should keep the results channel open.
    drop(result_sender);

    thread::spawn(move || {
        for n in 1..=COUNT {
            job_sender.send(n).unwrap();
        }
        // job_sender is dropped here, which closes the channel.
    });

    // The loop ends when every Sender of results has been dropped.
    let (mut received, mut sum) = (0, 0);
    for square in result_receiver {
        received += 1;
        sum += square;
    }
    println!("{WORKERS} workers squared 1 to {COUNT}");
    println!("results: {received}");
    println!("sum: {sum}");
}
```

**Go**

```go
package main

import (
	"fmt"
	"sync"
)

const count = 1000
const workers = 3

func main() {
	// A channel carries values from one goroutine to another. These hold up
	// to 16 values, and a send to a full channel waits for room.
	jobs := make(chan int, 16)
	results := make(chan int, 16)

	// The producer sends the numbers, then closes the channel to say that
	// no more are coming.
	go func() {
		for n := 1; n <= count; n++ {
			jobs <- n
		}
		close(jobs)
	}()

	// Each worker takes numbers until jobs is closed and empty. No number
	// goes to two workers.
	var wg sync.WaitGroup
	for range workers {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for n := range jobs {
				results <- n * n
			}
		}()
	}

	// When every worker has finished, nothing else will send a result.
	go func() {
		wg.Wait()
		close(results)
	}()

	received, sum := 0, 0
	for square := range results {
		received++
		sum += square
	}
	fmt.Printf("%d workers squared 1 to %d\n", workers, count)
	fmt.Println("results:", received)
	fmt.Println("sum:", sum)
}
```

**Python**

```python
import queue
import threading

COUNT = 1000
WORKERS = 3


def main() -> None:
    # A Queue is safe to use from several threads. With a maxsize, put waits
    # while the queue is full.
    jobs: queue.Queue[int] = queue.Queue(maxsize=16)
    results: queue.Queue[int] = queue.Queue(maxsize=16)

    def produce() -> None:
        for n in range(1, COUNT + 1):
            jobs.put(n)
        # shutdown (Python 3.13) closes the queue: once it is empty, get
        # raises ShutDown instead of waiting.
        jobs.shutdown()

    def work() -> None:
        while True:
            try:
                n = jobs.get()
            except queue.ShutDown:
                return
            results.put(n * n)

    workers = [threading.Thread(target=work) for _ in range(WORKERS)]

    def close_results() -> None:
        for worker in workers:
            worker.join()
        results.shutdown()

    threading.Thread(target=produce).start()
    for worker in workers:
        worker.start()
    threading.Thread(target=close_results).start()

    received = total = 0
    while True:
        try:
            square = results.get()
        except queue.ShutDown:
            break
        received += 1
        total += square
    print(f"{WORKERS} workers squared 1 to {COUNT}")
    print(f"results: {received}")
    print(f"sum: {total}")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
import { Worker, isMainThread, parentPort } from "node:worker_threads";

const COUNT = 1000;
const WORKERS = 3;

if (isMainThread) {
  // Workers share no memory with the main thread. postMessage sends a copy
  // of a value, and the other side gets it as a "message" event. Only the
  // main thread hands out jobs, so no two workers can get the same one.
  let next = 1;
  let received = 0;
  let sum = 0;

  function startWorker() {
    const worker = new Worker(new URL(import.meta.url));
    worker.on("message", (square) => {
      if (square !== null) {
        received++;
        sum += square;
      }
      // Each answer asks for the next job, so a faster worker gets more.
      worker.postMessage(next <= COUNT ? next++ : null);
    });
    return new Promise((resolve, reject) => {
      worker.once("exit", () => resolve());
      worker.once("error", reject);
    });
  }

  await Promise.all(Array.from({ length: WORKERS }, startWorker));
  console.log(`${WORKERS} workers squared 1 to ${COUNT}`);
  console.log(`results: ${received}`);
  console.log(`sum: ${sum}`);
} else if (parentPort) {
  // A worker asks for its first job with null, then answers each job with
  // its result. null means stop, and closing the port lets the worker end.
  const port = parentPort;
  port.on("message", (n) => {
    if (n === null) port.close();
    else port.postMessage(n * n);
  });
  port.postMessage(null);
}
```

**TypeScript**

```ts
import { Worker, isMainThread, parentPort } from "node:worker_threads";

const COUNT = 1000;
const WORKERS = 3;

if (isMainThread) {
  // Workers share no memory with the main thread. postMessage sends a copy
  // of a value, and the other side gets it as a "message" event. Only the
  // main thread hands out jobs, so no two workers can get the same one.
  let next = 1;
  let received = 0;
  let sum = 0;

  function startWorker(): Promise<void> {
    const worker = new Worker(new URL(import.meta.url));
    worker.on("message", (square: number | null) => {
      if (square !== null) {
        received++;
        sum += square;
      }
      // Each answer asks for the next job, so a faster worker gets more.
      worker.postMessage(next <= COUNT ? next++ : null);
    });
    return new Promise((resolve, reject) => {
      worker.once("exit", () => resolve());
      worker.once("error", reject);
    });
  }

  await Promise.all(Array.from({ length: WORKERS }, startWorker));
  console.log(`${WORKERS} workers squared 1 to ${COUNT}`);
  console.log(`results: ${received}`);
  console.log(`sum: ${sum}`);
} else if (parentPort) {
  // A worker asks for its first job with null, then answers each job with
  // its result. null means stop, and closing the port lets the worker end.
  const port = parentPort;
  port.on("message", (n: number | null) => {
    if (n === null) port.close();
    else port.postMessage(n * n);
  });
  port.postMessage(null);
}
```

**Dart**

```dart
import 'dart:isolate';

const count = 1000;
const workers = 3;

// Runs in a worker isolate. Isolates share no memory, and talk through
// ports: a message sent to a SendPort arrives, copied, at its ReceivePort.
// The worker sends back a port for its jobs, then the square of each job,
// and stops at null.
void work(SendPort replies) {
  final jobs = ReceivePort();
  replies.send(jobs.sendPort);
  jobs.listen((job) {
    if (job == null) {
      jobs.close(); // with nothing left to listen to, the isolate ends
    } else {
      replies.send((job as int) * job);
    }
  });
}

Future<void> main() async {
  // Only the main isolate hands out jobs, so no two workers get the same one.
  var next = 1, received = 0, sum = 0;

  Future<void> drive() async {
    final replies = ReceivePort();
    await Isolate.spawn(work, replies.sendPort);
    late SendPort jobs;
    // A ReceivePort is a Stream, and leaving the loop closes it.
    await for (final message in replies) {
      if (message is SendPort) {
        jobs = message;
      } else {
        received++;
        sum += message as int;
      }
      if (next > count) {
        jobs.send(null);
        break;
      }
      jobs.send(next++);
    }
  }

  await Future.wait([for (var i = 0; i < workers; i++) drive()]);
  print('$workers workers squared 1 to $count');
  print('results: $received');
  print('sum: $sum');
}
```

**Kotlin**

```kotlin
import java.util.concurrent.ArrayBlockingQueue
import kotlin.concurrent.thread

const val COUNT = 1000L
const val WORKERS = 3

// Java's queues can't be closed, so a special value says "stop".
const val STOP = -1L

fun main() {
    // A BlockingQueue is safe to use from several threads. put waits while
    // the queue is full, and take waits while it is empty.
    val jobs = ArrayBlockingQueue<Long>(16)
    val results = ArrayBlockingQueue<Long>(16)

    thread {
        for (n in 1..COUNT) jobs.put(n)
        repeat(WORKERS) { jobs.put(STOP) } // one for each worker
    }

    repeat(WORKERS) {
        thread {
            while (true) {
                val n = jobs.take()
                if (n == STOP) break
                results.put(n * n)
            }
            results.put(STOP) // this worker is done
        }
    }

    var received = 0
    var sum = 0L
    var finished = 0
    while (finished < WORKERS) {
        val square = results.take()
        if (square == STOP) {
            finished++
        } else {
            received++
            sum += square
        }
    }
    println("$WORKERS workers squared 1 to $COUNT")
    println("results: $received")
    println("sum: $sum")
}
```

All nine print:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
3 workers squared 1 to 1000
results: 1000
sum: 333833500
```

**Go: channels in the language.** `make(chan int, 16)` makes a channel that holds up to
16 values. `jobs <- n` sends, and waits while the channel is full, and `<-jobs`
receives, and waits while it is empty. A channel made without a size holds nothing, so
every send waits for a receive, and the two goroutines meet. `close(jobs)` says that no
more values are coming, and `for n := range jobs` takes values until the channel is
closed and empty. Closing is what ends the workers' loops, and the WaitGroup and one
more goroutine close `results` once every worker is done. A channel with room for 16
values is a lazy pipeline from chapter 14, stretched between goroutines: the producer
can get only 16 numbers ahead of the workers before it has to wait, which is called
**backpressure**. Without `close(jobs)`, every goroutine ends up waiting for a value
that never comes, and Go stops the program with
`fatal error: all goroutines are asleep - deadlock!`. Go's `select`, not used here,
waits on several channels at once and takes whichever is ready first.

**Rust: closing by dropping.** `mpsc::sync_channel(16)` returns a `SyncSender` and a
`Receiver` for a channel of 16, and `mpsc::channel()` makes one with no limit. `mpsc`
stands for multiple producers, single consumer: a sender can be cloned, and the receiver
can't. Several workers taking jobs from one receiver share it behind
`Arc<Mutex<Receiver>>`, and each locks it just long enough to take a job. There is no
`close`. A channel closes when every sender has been dropped, and `recv` then returns
`Err`, which ends the loop. So `main` drops its own `result_sender` once the workers
have their clones, and if it didn't, `for square in result_receiver` would wait forever
for a sender that would never send.

**C and C++: a channel by hand.** Neither library has a channel, so each version builds
one from a queue, a mutex, and two **condition variables**: `not_empty`, which receivers
wait on, and `not_full`, which senders wait on. Waiting on a condition variable unlocks
the mutex while the thread sleeps, and locks it again before the thread carries on. A
waiting thread can also wake when nobody signaled it, so the C version waits in a
`while` loop that checks the condition again, and C++'s `wait(lock, predicate)` has the
loop built in. Closing sets a flag and wakes every waiting thread with a broadcast. The
C++ `Channel<T>` returns `std::optional<T>` from `receive`, which is empty once the
channel is closed and drained, so `while (auto n = jobs.receive())` reads like Go's
`range`.

**Python and Kotlin: queues.** Python's `queue.Queue` is safe to use from several
threads, with `put` and `get` that wait. Python 3.13 added `shutdown`, which closes a
queue: once it is empty, `get` raises `queue.ShutDown` instead of waiting. Java's
`ArrayBlockingQueue` has `put` and `take`, which wait, but no way to close it, so the
Kotlin version sends a special value, `STOP`, once for each worker. That is a **poison
pill**, and it was also the Python way before 3.13. kotlinx.coroutines has a `Channel`
with `close`, much like Go's.

**JavaScript and Dart: the main thread hands out the work.** Workers and isolates have
no channel that several of them can take from, so the main thread keeps the next number
and gives one to each worker that asks. A worker asks for its first job, answers each
job with its square, and gets the next job in reply, so a faster worker gets more of
them. When the numbers run out, the main thread replies `null`, and the worker closes
its port, which lets it end. Dart's version starts with a handshake: `Isolate.spawn`
gives the new isolate a `SendPort` to reply on, and the isolate's first message is a
`SendPort` of its own, for its jobs. A `ReceivePort` is a `Stream`, so `drive` reads it
with `await for`, and leaving the loop closes it.

---

## 16.6 Async and await: many tasks, one thread

Each version runs three tasks that each print their steps and pause after each one, and
gathers their results. Then four tasks add 1 to a shared count, once with no pause
between reading the count and writing it, and once with a pause in the middle. Go and C
aren't here. Go doesn't need async functions, since a goroutine that waits for the
network costs a few kilobytes and the runtime runs another one, and C has no async
functions at all.

**C++**

```cpp
#include <coroutine>
#include <deque>
#include <exception>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

// C++20 has coroutines, with co_await and co_return, but the standard library
// has no event loop and no task type to run them with. Libraries supply those.
// This is a small one, to show what they do.

// The coroutines that are ready to run, in the order they will run.
std::deque<std::coroutine_handle<>> ready;

// Awaiting a Pause suspends the coroutine, and puts it at the back of the line.
struct Pause {
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> handle) const { ready.push_back(handle); }
    void await_resume() const noexcept {}
};

// A coroutine that returns a T. The compiler finds promise_type, and calls
// its functions when the coroutine starts, returns, and finishes.
template <typename T>
class Task {
public:
    struct promise_type {
        std::optional<T> result;

        Task get_return_object() {
            return Task(std::coroutine_handle<promise_type>::from_promise(*this));
        }
        // Start suspended, so that nothing runs until run_all resumes it.
        std::suspend_always initial_suspend() noexcept { return {}; }
        // Stay suspended at the end, so that the result is still there.
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(T value) { result = std::move(value); }
        void unhandled_exception() { std::terminate(); }
    };

    explicit Task(std::coroutine_handle<promise_type> handle) : handle(handle) {}
    Task(Task&& other) noexcept : handle(std::exchange(other.handle, {})) {}
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task& operator=(Task&&) = delete;
    // The coroutine's frame, with its locals, is on the heap until this.
    ~Task() {
        if (handle) handle.destroy();
    }

    std::coroutine_handle<promise_type> handle;
};

// Runs the tasks on this one thread until all of them finish, and returns
// their results in order.
template <typename T>
std::vector<T> run_all(std::vector<Task<T>> tasks) {
    for (const auto& task : tasks) ready.push_back(task.handle);
    while (!ready.empty()) {
        auto next = ready.front();
        ready.pop_front();
        next.resume();  // runs the coroutine up to its next co_await
    }
    std::vector<T> results;
    for (const auto& task : tasks) results.push_back(*task.handle.promise().result);
    return results;
}

// name is a string_view of a string literal, which lasts as long as the program.
// A view of a temporary string would dangle after the first co_await.
Task<int> task(std::string_view name, int steps) {
    for (int step = 1; step <= steps; step++) {
        std::cout << name << ": step " << step << "\n";
        // co_await is the only place a task can stop and let another one run.
        co_await Pause{};
    }
    std::cout << name << ": done\n";
    co_return steps;
}

// These return the count they wrote. count is a reference to main's int,
// which outlives the tasks.
Task<int> add_at_once(int& count) {
    count += 1;  // no co_await, so no other task can run in the middle
    co_return count;
}

Task<int> add_with_await(int& count) {
    int value = count;
    co_await Pause{};  // every other task reads the old value here
    count = value + 1;
    co_return count;
}

int main() {
    std::cout << "three tasks, each pausing at every await\n";
    std::vector<Task<int>> tasks;
    tasks.push_back(task("a", 2));
    tasks.push_back(task("b", 3));
    tasks.push_back(task("c", 1));
    auto results = run_all(std::move(tasks));
    std::cout << "results: " << results[0] << ", " << results[1] << ", " << results[2] << "\n";

    std::cout << "4 tasks add 1 to a shared count\n";
    int count = 0;
    std::vector<Task<int>> adds;
    for (int i = 0; i < 4; i++) adds.push_back(add_at_once(count));
    run_all(std::move(adds));
    std::cout << "with no await between reading and writing: " << count << "\n";
    count = 0;
    adds.clear();
    for (int i = 0; i < 4; i++) adds.push_back(add_with_await(count));
    run_all(std::move(adds));
    std::cout << "with an await between reading and writing: " << count << "\n";
}
```

**Rust**

```rust
use std::cell::Cell;
use std::collections::VecDeque;
use std::future::Future;
use std::pin::Pin;
use std::sync::{Arc, Mutex};
use std::task::{Context, Poll, Wake, Waker};

// Rust has async, await, and the Future trait, but its standard library has no
// event loop to run futures on. Programs use a crate such as tokio for that.
// This is a small one, to show what such a loop does.

// A future that isn't ready the first time it is polled. It asks to be polled
// again, which puts its task at the back of the line.
struct Pause {
    paused: bool,
}

impl Future for Pause {
    type Output = ();

    fn poll(mut self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<()> {
        if self.paused {
            return Poll::Ready(());
        }
        self.paused = true;
        cx.waker().wake_by_ref();
        Poll::Pending
    }
}

fn pause() -> Pause {
    Pause { paused: false }
}

// Waking a task puts its number at the back of the queue of tasks that are
// ready to run. A Waker has to be Send and Sync, even on one thread, so the
// queue is behind an Arc and a Mutex.
struct TaskWaker {
    id: usize,
    ready: Arc<Mutex<VecDeque<usize>>>,
}

impl Wake for TaskWaker {
    fn wake(self: Arc<Self>) {
        self.ready.lock().unwrap().push_back(self.id);
    }
}

// A future on the heap, pinned there because it can hold references into
// itself. 'a is how long the things it borrows live.
type Task<'a, T> = Pin<Box<dyn Future<Output = T> + 'a>>;

fn spawn<'a, T>(future: impl Future<Output = T> + 'a) -> Task<'a, T> {
    Box::pin(future)
}

// Runs the tasks on this one thread until all of them finish, and returns
// their results in order.
fn run_all<T>(mut tasks: Vec<Task<'_, T>>) -> Vec<T> {
    let ready = Arc::new(Mutex::new((0..tasks.len()).collect::<VecDeque<_>>()));
    let wakers: Vec<Waker> = (0..tasks.len())
        .map(|id| {
            let ready = Arc::clone(&ready);
            Waker::from(Arc::new(TaskWaker { id, ready }))
        })
        .collect();
    let mut results: Vec<Option<T>> = tasks.iter().map(|_| None).collect();
    loop {
        // The lock is released at the end of this statement, before the poll,
        // which can wake the task and lock the queue again.
        let next = ready.lock().unwrap().pop_front();
        let Some(id) = next else { break };
        // poll runs the task up to its next await that isn't ready.
        let mut cx = Context::from_waker(&wakers[id]);
        if let Poll::Ready(result) = tasks[id].as_mut().poll(&mut cx) {
            results[id] = Some(result);
        }
    }
    results
        .into_iter()
        .map(|result| result.expect("a task never finished"))
        .collect()
}

async fn task(name: &str, steps: u32) -> u32 {
    for step in 1..=steps {
        println!("{name}: step {step}");
        // await is the only place a task can stop and let another one run.
        pause().await;
    }
    println!("{name}: done");
    steps
}

async fn add_at_once(count: &Cell<u32>) {
    count.set(count.get() + 1); // no await, so no other task can run in the middle
}

async fn add_with_await(count: &Cell<u32>) {
    let value = count.get();
    pause().await; // every other task reads the old value here
    count.set(value + 1);
}

fn main() {
    println!("three tasks, each pausing at every await");
    // Calling an async fn runs none of it. It returns a future, which runs
    // only when something polls it.
    let results = run_all(vec![
        spawn(task("a", 2)),
        spawn(task("b", 3)),
        spawn(task("c", 1)),
    ]);
    let results: Vec<String> = results.iter().map(u32::to_string).collect();
    println!("results: {}", results.join(", "));

    println!("4 tasks add 1 to a shared count");
    // The tasks share count through a Cell, which can be changed through a
    // shared reference, and can't be sent to another thread.
    let count = Cell::new(0);
    run_all((0..4).map(|_| spawn(add_at_once(&count))).collect());
    println!("with no await between reading and writing: {}", count.get());
    let count = Cell::new(0);
    run_all((0..4).map(|_| spawn(add_with_await(&count))).collect());
    println!("with an await between reading and writing: {}", count.get());
}
```

**Python**

```python
import asyncio
from dataclasses import dataclass


async def task(name: str, steps: int) -> int:
    for step in range(1, steps + 1):
        print(f"{name}: step {step}")
        # await is the only place a task can stop and let another one run.
        # sleep(0) waits for nothing, so this task goes to the back of the
        # event loop's queue, behind the tasks that are ready.
        await asyncio.sleep(0)
    print(f"{name}: done")
    return steps


@dataclass
class Counter:
    value: int = 0


async def add_at_once(count: Counter) -> None:
    count.value += 1  # no await, so no other task can run in the middle


async def add_with_await(count: Counter) -> None:
    value = count.value
    await asyncio.sleep(0)  # every other task reads the old value here
    count.value = value + 1


async def main() -> None:
    print("three tasks, each pausing at every await")
    # gather runs the tasks at the same time, on this one thread, and
    # returns their results in the order they were given.
    results = await asyncio.gather(task("a", 2), task("b", 3), task("c", 1))
    print("results:", ", ".join(str(result) for result in results))

    print("4 tasks add 1 to a shared count")
    count = Counter()
    await asyncio.gather(*(add_at_once(count) for _ in range(4)))
    print(f"with no await between reading and writing: {count.value}")
    count = Counter()
    await asyncio.gather(*(add_with_await(count) for _ in range(4)))
    print(f"with an await between reading and writing: {count.value}")


if __name__ == "__main__":
    # asyncio.run starts an event loop, runs main until it finishes, and
    # closes the loop.
    asyncio.run(main())
```

**JavaScript**

```js
// Resolves on a later turn of the event loop. A task that awaits it goes to
// the back of the line, behind everything that is ready to run.
const pause = () => new Promise((resolve) => setTimeout(resolve, 0));

async function task(name, steps) {
  for (let step = 1; step <= steps; step++) {
    console.log(`${name}: step ${step}`);
    // await is the only place a task can stop and let another one run.
    await pause();
  }
  console.log(`${name}: done`);
  return steps;
}

async function addAtOnce(count) {
  count.value += 1; // no await, so no other task can run in the middle
}

async function addWithAwait(count) {
  const value = count.value;
  await pause(); // every other task reads the old value here
  count.value = value + 1;
}

console.log("three tasks, each pausing at every await");
// Calling an async function runs it up to its first await, and returns a
// Promise. Promise.all waits for all of them, and keeps the results in order.
const results = await Promise.all([task("a", 2), task("b", 3), task("c", 1)]);
console.log(`results: ${results.join(", ")}`);

console.log("4 tasks add 1 to a shared count");
const atOnce = { value: 0 };
await Promise.all([1, 2, 3, 4].map(() => addAtOnce(atOnce)));
console.log(`with no await between reading and writing: ${atOnce.value}`);
const withAwait = { value: 0 };
await Promise.all([1, 2, 3, 4].map(() => addWithAwait(withAwait)));
console.log(`with an await between reading and writing: ${withAwait.value}`);
```

**TypeScript**

```ts
// Resolves on a later turn of the event loop. A task that awaits it goes to
// the back of the line, behind everything that is ready to run.
const pause = (): Promise<void> =>
  new Promise((resolve) => setTimeout(resolve, 0));

async function task(name: string, steps: number): Promise<number> {
  for (let step = 1; step <= steps; step++) {
    console.log(`${name}: step ${step}`);
    // await is the only place a task can stop and let another one run.
    await pause();
  }
  console.log(`${name}: done`);
  return steps;
}

interface Counter {
  value: number;
}

async function addAtOnce(count: Counter): Promise<void> {
  count.value += 1; // no await, so no other task can run in the middle
}

async function addWithAwait(count: Counter): Promise<void> {
  const value = count.value;
  await pause(); // every other task reads the old value here
  count.value = value + 1;
}

console.log("three tasks, each pausing at every await");
// Calling an async function runs it up to its first await, and returns a
// Promise. Promise.all waits for all of them, and keeps the results in order.
const results = await Promise.all([task("a", 2), task("b", 3), task("c", 1)]);
console.log(`results: ${results.join(", ")}`);

console.log("4 tasks add 1 to a shared count");
const atOnce: Counter = { value: 0 };
await Promise.all([1, 2, 3, 4].map(() => addAtOnce(atOnce)));
console.log(`with no await between reading and writing: ${atOnce.value}`);
const withAwait: Counter = { value: 0 };
await Promise.all([1, 2, 3, 4].map(() => addWithAwait(withAwait)));
console.log(`with an await between reading and writing: ${withAwait.value}`);
```

**Dart**

```dart
// Completes on a later turn of the event loop. A task that awaits it goes to
// the back of the line, behind everything that is ready to run.
Future<void> pause() => Future(() {});

Future<int> task(String name, int steps) async {
  for (var step = 1; step <= steps; step++) {
    print('$name: step $step');
    // await is the only place a task can stop and let another one run.
    await pause();
  }
  print('$name: done');
  return steps;
}

class Counter {
  int value = 0;
}

Future<void> addAtOnce(Counter count) async {
  count.value += 1; // no await, so no other task can run in the middle
}

Future<void> addWithAwait(Counter count) async {
  final value = count.value;
  await pause(); // every other task reads the old value here
  count.value = value + 1;
}

Future<void> main() async {
  print('three tasks, each pausing at every await');
  // Calling an async function runs it up to its first await, and returns a
  // Future. Future.wait waits for all of them, and keeps the results in order.
  final results = await Future.wait([task('a', 2), task('b', 3), task('c', 1)]);
  print('results: ${results.join(', ')}');

  print('4 tasks add 1 to a shared count');
  final atOnce = Counter();
  await Future.wait([for (var i = 0; i < 4; i++) addAtOnce(atOnce)]);
  print('with no await between reading and writing: ${atOnce.value}');
  final withAwait = Counter();
  await Future.wait([for (var i = 0; i < 4; i++) addWithAwait(withAwait)]);
  print('with an await between reading and writing: ${withAwait.value}');
}
```

**Kotlin**

```kotlin
import kotlin.coroutines.Continuation
import kotlin.coroutines.EmptyCoroutineContext
import kotlin.coroutines.createCoroutine
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine

// Kotlin has suspend functions, and its standard library has the pieces to
// start and resume them, but no event loop. The kotlinx.coroutines library
// supplies one, with launch, async, and delay. This is a small one, to show
// what such a loop does.

// The coroutines that are ready to run, in the order they will run.
val ready = ArrayDeque<Continuation<Unit>>()

// Suspends the coroutine, and puts the rest of it at the back of the line.
suspend fun pause() = suspendCoroutine<Unit> { rest -> ready.addLast(rest) }

// Runs the tasks on this one thread until all of them finish, and returns
// their results in order.
fun <T> runAll(tasks: List<suspend () -> T>): List<T> {
    val results = MutableList<T?>(tasks.size) { null }
    for ((i, task) in tasks.withIndex()) {
        // createCoroutine makes a coroutine that hasn't started. The
        // Continuation passed to it gets the result when it finishes.
        val start =
            task.createCoroutine(
                Continuation(EmptyCoroutineContext) { result ->
                    results[i] = result.getOrThrow()
                }
            )
        ready.addLast(start)
    }
    while (ready.isNotEmpty()) {
        ready.removeFirst().resume(Unit) // runs the coroutine up to its next pause
    }
    return results.map { it!! }
}

suspend fun task(name: String, steps: Int): Int {
    for (step in 1..steps) {
        println("$name: step $step")
        // Calling a suspend function is the only place a task can stop and
        // let another one run. Kotlin has no await keyword.
        pause()
    }
    println("$name: done")
    return steps
}

class Counter {
    var value = 0
}

suspend fun addAtOnce(count: Counter) {
    count.value += 1 // no suspend call, so no other task can run in the middle
}

suspend fun addWithAwait(count: Counter) {
    val value = count.value
    pause() // every other task reads the old value here
    count.value = value + 1
}

fun main() {
    println("three tasks, each pausing at every await")
    val results =
        runAll(listOf(suspend { task("a", 2) }, suspend { task("b", 3) }, suspend { task("c", 1) }))
    println("results: ${results.joinToString(", ")}")

    println("4 tasks add 1 to a shared count")
    val atOnce = Counter()
    runAll(List(4) { suspend { addAtOnce(atOnce) } })
    println("with no await between reading and writing: ${atOnce.value}")
    val withAwait = Counter()
    runAll(List(4) { suspend { addWithAwait(withAwait) } })
    println("with an await between reading and writing: ${withAwait.value}")
}
```

All seven print:

*C++, Rust, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
three tasks, each pausing at every await
a: step 1
b: step 1
c: step 1
a: step 2
b: step 2
c: done
a: done
b: step 3
b: done
results: 2, 3, 1
4 tasks add 1 to a shared count
with no await between reading and writing: 4
with an await between reading and writing: 1
```

**The order is fixed.** Everything runs on one thread, and a task stops only at an
`await`, so the output is the same on every run. `a` prints its first step and pauses,
which sends it to the back of the line, behind `b` and `c`. Then `b` and `c` take their
turns, and the three take turns until each one runs out of steps. `c` has one step and
finishes first, but the results still come back in the order the tasks were given: 2, 3,
1. In the second part, `add_at_once` has no `await`, so each task reads and writes the
count without a break, and the total is 4. `add_with_await` pauses between reading and
writing, so all four tasks read 0 before any of them writes, and the total is 1. That is
a race condition with no threads at all, and it is why exercise 16.2 needs locks in
JavaScript and Dart too.

**Python: asyncio.** `async def` makes a **coroutine function**: calling it runs
nothing, and returns a coroutine object that runs when something awaits it.
`asyncio.run(main())` starts an event loop, runs `main` until it finishes, and closes
the loop. `asyncio.gather` wraps each coroutine in a task, which the loop starts on its
next turn, and returns their results in order. `await asyncio.sleep(0)` waits for
nothing, but goes through the loop, so the task goes to the back of the line. An `await`
pauses a function the way chapter 14's `yield` pauses a generator, and before Python
3.5, asyncio's coroutines were generators, awaited with `yield from`. The event loop is
the code that resumes them. Forgetting an `await` is the common mistake.
`value = task()` makes a coroutine and never runs it, and `print(value)` prints
`<coroutine object task at 0x...>`. Python warns with
`RuntimeWarning: coroutine 'task' was never awaited`, but only once the coroutine is
thrown away, and mypy, with `--strict`, finds no problem once the coroutine is stored in
a variable.

**JavaScript and TypeScript: promises.** An `async` function returns a promise, and runs
at once, up to its first `await`. So `task("a", 2)` prints `a: step 1` before the next
task is even called, and `Promise.all` then waits for all of the promises. An ES module
can `await` at its top level, which is why there is no `main`. `pause` makes a promise
that `setTimeout` resolves on a later turn of the event loop. JavaScript's event loop
has two kinds of queue: **tasks**, such as timers and I/O, and **microtasks**, such as
the rest of an async function after an `await` on a promise that has already resolved.
The loop runs every waiting microtask before the next task, so a function that awaits
`null` resumes before a `setTimeout` of 0 that was set earlier, and `pause` sends a task
to the back of the whole line. A forgotten `await` here gives a promise instead of a
value: `console.log(task())` prints `Promise { <pending> }`, and in a template string it
is `[object Promise]`. TypeScript's types make that easier to see, since `task()` has
the type `Promise<number>`, not `number`. Node's usual way to read stdin, which chapter
1 left for this chapter, works on the same loop: the input arrives as events, and
`for await (const line of createInterface({ input: process.stdin }))`, with
`createInterface` from `node:readline`, reads it one line at a time.

**Dart: futures.** Dart works like JavaScript. An `async` function returns a `Future`,
and runs up to its first `await`. `Future(() {})` completes on a later turn of the event
loop, and Dart also has a microtask queue, which runs first, with `scheduleMicrotask`
and `Future.microtask`. `Future.wait` waits for a list of futures and keeps their
results in order. The analyzer doesn't complain about a forgotten `await` unless the
`unawaited_futures` lint is turned on, and `print(task())` prints
`Instance of 'Future<int>'`. An `async*` function, which chapter 14 mentioned, is a
`sync*` generator for values that arrive over time: it returns a `Stream`, and
`await for` reads one, as `drive` in example 03 reads a `ReceivePort`.

**Rust: futures that do nothing until polled.** Rust has `async fn` and `.await`, and
the `Future` trait in its standard library, but no event loop to run futures on.
Programs use a library for that, usually tokio, so this version has a small loop of its
own, to show what such a library does. Calling `task("a", 2)` runs none of it, and
returns a future: a value that holds the function's local variables and where it
stopped. `run_all` **polls** each future, which runs it up to an `.await` that isn't
ready. `Pause` is a future that isn't ready the first time it is polled. It calls
`wake_by_ref` on the waker it was given, which puts its task back on the queue, and
returns `Poll::Pending`. A future can hold references into itself, so it mustn't move
once it has been polled, which is what `Pin<Box<...>>` promises. A `Waker` has to be
`Send` and `Sync`, even on one thread, since a real one is usually woken from another
thread, so the queue is behind an `Arc` and a `Mutex`. The shared count is a `Cell`,
which allows changes through a shared reference, and can't be sent to another thread.

**C++: coroutines without a library.** A C++20 function that uses `co_await` or
`co_return` is a coroutine, and its return type decides how it behaves. The compiler
looks for that type's `promise_type`, and calls its functions when the coroutine starts,
returns a value, and finishes. Here, `initial_suspend` returns `std::suspend_always`, so
a `Task` doesn't run until `run_all` resumes it, and `final_suspend` does too, so its
result is still there afterward. The coroutine's local variables live in a **frame** on
the heap, which `~Task` destroys. `co_await Pause{}` calls `Pause`'s `await_suspend`
with the coroutine's handle, which puts it at the back of `ready`. Parameters are the
trap. A coroutine keeps its parameters in its frame, but a reference parameter keeps
only the reference. Declaring `task` with `const std::string& name` and calling
`task("a", 2)` makes a temporary string that is destroyed at the end of the statement,
before the coroutine runs, and AddressSanitizer reports `stack-use-after-scope` when it
prints `name`. The example takes a `std::string_view` of a string literal, which lasts
as long as the program.

**Kotlin: suspend functions.** A `suspend fun` can pause, and can be called only from
another `suspend fun` or a coroutine. Calling `pause()` from plain `main` is an error:
"suspend function 'suspend fun pause(): Unit' can only be called from a coroutine or
another suspend function." Kotlin has no `await` keyword: calling a suspend function is
where a task can stop. The compiler turns each suspend function into a state machine,
and passes it a **continuation**, an object that resumes the function where it stopped.
`suspendCoroutine` hands that continuation to its block, which here puts it on the
`ready` queue, and the function stays stopped until something calls `resume`.
`createCoroutine` makes a coroutine from a suspend lambda without starting it. The
kotlinx.coroutines library builds `launch`, `async`, `delay`, and its dispatchers on
these same few functions, and chapter 14's `sequence { }` is built on them too: its
block is a suspend lambda, and `yield` suspends it.

**The color of a function.** In Python, JavaScript, TypeScript, Dart, Rust, C++, and
Kotlin, a function that waits has to be async, or suspend, and so does every function
that calls it, up to the event loop. Bob Nystrom, who works on Dart, called this the
color of a function, in a 2015 essay: async functions are one color, ordinary ones
another, and an ordinary function can't call an async one and wait for it. Go has no
colors, because any function can wait, and a waiting goroutine lets the runtime run
others. Java's virtual threads give the JVM the same thing.

---

## 16.7 Which tool for which job

| The work | Use |
| --- | --- |
| Many connections or files, mostly waiting | Async in Python, JavaScript, TypeScript, Dart, Rust (tokio), and Kotlin (kotlinx.coroutines); goroutines in Go; virtual threads on the JVM; an event library such as libuv in C and C++ |
| Computation that can be split up | Threads in C, C++, Rust, Go, and Kotlin; processes in Python; workers in JavaScript and TypeScript; isolates in Dart |
| A user interface that has to keep responding | The UI's own event loop for the UI, and a thread, worker, or isolate for long computations |
| A pipeline of stages | Channels in Go and Rust, queues in Python, Kotlin, C, and C++, and messages between workers or isolates |

Waiting and computing need different tools. A program that mostly waits, for the
network, a disk, or a database, needs many tasks that cost little, and one thread can
run thousands of them, because each one spends nearly all its time stopped at an
`await`. A program that mostly computes needs one worker for each core, and more workers
than cores only take turns. A thread that computes for a long time without an `await`
stops every other task on its event loop, which is why a long calculation on a browser's
main thread freezes the page.

Parallel work also has a cost to start and to combine. On the machine that ran this
book's examples, starting a thread in C and joining it took about 50 microseconds, a
Dart isolate about 0.3 milliseconds, a Python worker process about 4 milliseconds with
`fork` and 60 with `spawn`, and a Node worker about 40 milliseconds, and then a process,
an isolate, or a worker needs its input copied to it, and its results copied back. Some
of the work can't be split at all, such as reading the input and printing the answer,
and that part sets a limit. If a tenth of the work runs on one core, the program can
never be more than ten times faster, however many cores it gets. That is **Amdahl's
law**, from 1967.

---

## 16.8 Why each language looks the way it does

- **C** left threads to the operating system for decades. POSIX threads came in 1995, as
  a library with a C interface, and C itself had no rules for what one thread sees of
  another's writes until C11, which added a memory model, `<stdatomic.h>`, and an
  optional `<threads.h>`.
- **C++** got its memory model, `std::thread`, `std::mutex`, and `std::atomic` in C++11,
  all from the same work as C11's. C++20 added `std::jthread`, and coroutines as a
  language feature with no task types, leaving those to libraries. C++26 adds
  `std::execution`, a standard way to describe and run asynchronous work.
- **Rust** found that its ownership rules, one writer or many readers, are also the
  rules that prevent data races, and added two traits, `Send` and `Sync`, so the
  compiler can check which types may cross threads. The Rust book calls the result
  "fearless concurrency". `async` and `.await` arrived in Rust 1.39 (2019), without a
  runtime in the standard library, because a server, a phone, and a microcontroller need
  different ones.
- **Go** was designed at Google for servers, with goroutines and channels from the
  start. Its channels come from Tony Hoare's communicating sequential processes, CSP
  (1978), and the race detector arrived in Go 1.1 (2013).
- **Python** has had the GIL for as long as it has had threads. The GIL keeps the
  interpreter simple and single-threaded code fast, and makes C extensions easy to
  write, at the price of parallel threads. `asyncio` arrived in Python 3.4 (2014), and
  the keywords `async` and `await` in 3.5 (2015). PEP 703 brought a build without the
  GIL, experimental in 3.13 and supported in 3.14.
- **JavaScript** has run on one thread in the browser since 1995, with an event loop for
  clicks, timers, and network replies, so two pieces of JavaScript never run at the same
  moment. Callbacks came first, then promises in ES2015 and `async` and `await` in
  ES2017. Workers came later, and share nothing unless asked to.
- **TypeScript** adds types to JavaScript's concurrency without changing it.
  `Promise<T>` says what an `await` gives back.
- **Dart** was designed for user interfaces, and Flutter runs the Dart code of its UI on
  one thread, so UI code needs no locks. Isolates do the heavy work, with no shared
  memory to get wrong. `async` and `await` arrived in Dart 1.9 (2015).
- **Kotlin** took threads, locks, and the `java.util.concurrent` library from Java, and
  added coroutines in Kotlin 1.3 (2018): the compiler supports `suspend`, and the
  kotlinx.coroutines library does the rest, with **structured concurrency**, in which a
  coroutine can't outlive the scope that started it.

---

## 16.9 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | Threads that change one variable without a lock or an atomic | Undefined behavior; at `-O2` it can pass every test; build with `-fsanitize=thread` to find it |
| C | `return` from `main` before joining the threads | The process ends, and the threads with it, wherever they were |
| C | `pthread_cond_wait` in an `if` instead of a `while` | A thread can wake when nothing changed, and carry on as if it had |
| C, C++ | Locking a mutex that the same thread already holds | Undefined behavior; on Linux the thread waits forever |
| C++ | A `std::thread` destroyed without `join` or `detach` | `terminate called without an active exception`; use `std::jthread` |
| C++ | `std::async(f)` without `std::launch::async` | `f` may run later, on the thread that calls `get` |
| C++ | A reference parameter in a coroutine, called with a temporary | It dangles after the first suspension; ASan reports `stack-use-after-scope` |
| Rust | An `Rc` in a thread | `error[E0277]: 'Rc<i32>' cannot be sent between threads safely`; use `Arc` |
| Rust | A closure for `thread::spawn` that borrows | `error[E0373]`; add `move`, or use `thread::scope` |
| Rust | Locking the same `Mutex` twice on one thread, or two in different orders | It compiles; Rust doesn't prevent deadlocks |
| Go | Writing a map from several goroutines | `fatal error: concurrent map writes`; use a mutex |
| Go | Forgetting to `close` a channel that someone ranges over | `fatal error: all goroutines are asleep - deadlock!` |
| Go | A data race on a plain variable | Wrong values, and `go vet` is silent; test with `-race` |
| Python | Threads for computation | No faster, since the GIL runs one thread at a time; use processes |
| Python | `ProcessPoolExecutor` without `if __name__ == "__main__":` | Fails under `spawn` and `forkserver` with `RuntimeError: An attempt has been made to start a new process ...` |
| Python | Calling a coroutine function without `await` | Nothing runs; `RuntimeWarning: coroutine 'task' was never awaited` |
| Python | `time.sleep` in a coroutine | Every task on the loop waits: three tasks that sleep 0.2 seconds take 0.6; use `await asyncio.sleep` |
| JavaScript | `array.forEach(async (x) => { ... })` | `forEach` doesn't wait for the promises; use `for ... of` with `await`, or `Promise.all` |
| JavaScript | A forgotten `await` | A `Promise { <pending> }` where a value should be |
| JavaScript | Sending a function to a worker | `DataCloneError: ... could not be cloned.` |
| JavaScript | An `await` between reading and writing shared state | A race condition, with no threads; guard it with an async lock |
| TypeScript | Trusting the type of a message from a worker | It is `any`, whatever the promise says |
| Dart | A forgotten `await` | No warning unless the `unawaited_futures` lint is on |
| Dart | A closure for `Isolate.run` beside one that captures a port | `object is unsendable`, since closures in a function share their captures |
| Dart | Expecting an isolate to change a variable in `main` | It changes its own copy |
| Kotlin | `@Volatile var count` and `count++` from several threads | 195,111 of 400,000; volatile makes each read and write visible to other threads, not `++` atomic; use `AtomicLong` |
| Kotlin | Calling a `suspend fun` from ordinary code | `error: suspend function ... can only be called from a coroutine or another suspend function.` |
| Kotlin | A `HashMap` that several threads put keys into | Keys go missing, 139,495 of 200,000 in one run, and some runs never finish; use `ConcurrentHashMap` |

---

## 16.10 Idiom check

**C: pthreads, with a sanitizer.** Give each thread a struct of its own for its input
and its result, join every thread, and keep shared data behind a `pthread_mutex_t` that
a comment ties to it. Use `<stdatomic.h>` for counters and flags. Build and test with
`-fsanitize=thread`, since a data race can pass every ordinary test.

**C++: jthread, scoped_lock, and atomics.** Use `std::jthread`, or `std::async` with
`std::launch::async` for a result. Lock with `std::lock_guard`, or `std::scoped_lock`
for several mutexes at once, and never call `lock` and `unlock` by hand. Use
`std::atomic` for single values, and pass values to coroutines, not references.

**Rust: let the compiler check it.** Use `thread::scope` when threads can borrow, and
`thread::spawn` with `move`, `Arc`, and `Mutex` when they can't. Keep the data inside
its `Mutex`. Use `mpsc` channels to hand values between threads. For async code, use a
runtime such as tokio, and don't block inside a future.

**Go: goroutines and channels.** Start goroutines with `go`, and wait for them with a
`WaitGroup`. Share by communicating: pass values through channels, close a channel from
the sending side, and range over it on the receiving side. Use a `sync.Mutex` for state
that has to be shared, keep it beside the data in a struct, and run tests with `-race`.

**Python: asyncio for waiting, processes for computing.** Use `asyncio` for many
connections, and `concurrent.futures` for a pool: `ThreadPoolExecutor` for waiting, and
`ProcessPoolExecutor` for computing. Guard shared state with `threading.Lock`, pass work
through `queue.Queue`, and put the program's start under `if __name__ == "__main__":`.

**JavaScript: async and await, and workers for heavy work.** Use `async` and `await`,
`Promise.all` for several things at once, and `for ... of` with `await` for one after
another. Move long computations into a worker, and send it data with `postMessage`. Keep
`SharedArrayBuffer` and `Atomics` for when copying costs too much.

**TypeScript: typed promises.** Give async functions a return type of `Promise<T>`, and
check what arrives from a worker before trusting its type. Everything else is
JavaScript's.

**Dart: futures, and isolates for heavy work.** Use `async` and `await`, `Future.wait`
for several futures, and `Stream`s for a sequence of events. Run long computations with
`Isolate.run`, and keep the closure small. Turn on the `unawaited_futures` lint.

**Kotlin: kotlinx.coroutines in real projects.** Use `launch` and `async` from
kotlinx.coroutines, with structured concurrency, for most concurrent code, and
`Dispatchers.Default` for computation. Without the library, use `java.util.concurrent`:
thread pools, `ConcurrentHashMap`, `AtomicLong`, `withLock`, and virtual threads for
many tasks that wait.

---

## 16.11 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Count words in parallel](exercises/01-word-counts/) | Dividing work among threads, processes, workers, or isolates, and combining their results in order |
| 2 | [Transfers between accounts](exercises/02-transfers/) | A lock for each account, lock ordering against deadlock, and async locks |
| 3 | [An event loop](exercises/03-event-loop/) | How async tasks are run: a ready queue, sleeping tasks, waiting, and deadlock |

```sh
python3 tools/check.py 16                     # your answers, every language
python3 tools/check.py 16 --lang go,python    # some languages
python3 tools/check.py 16 --solutions         # the reference answers
```

**Stretch:** take exercise 16.2 in Go and rewrite it with no locks at all, as Go's
proverb suggests: one goroutine owns all the balances, and the workers send it transfers
through a channel. What happens to the deadlock, and to the parallelism? Then do the
same in Python with a `queue.Queue`, and compare the two.

---

Previous: [Chapter 15: Macros and metaprogramming](../15-macros-and-metaprogramming/) ·
Next: [Chapter 17: Version control with Git](../17-version-control-with-git/)
