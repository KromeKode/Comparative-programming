# Exercise 16.2: Transfers between accounts

Move money between bank accounts from several workers at once, without losing any of it.
Each account has a lock of its own, so that transfers between different accounts can run
at the same time, and every transfer takes the locks of both its accounts.

The first line of input is the number of workers, from 1 to 16. The second is the
starting balance of each account, and the accounts are numbered from 0. Each line after
that is a transfer of three whole numbers: the account it comes from, the account it
goes to, and the amount. For example:

```text
3
100 50 75 0
0 1 30
2 0 10
1 3 20
3 2 5
1 2 60
0 3 15
```

prints:

```text
3 workers, 4 accounts, 6 transfers
worker 0: 2 transfers, 35 moved
worker 1: 2 transfers, 70 moved
worker 2: 2 transfers, 35 moved
account 0: 65
account 1: 0
account 2: 130
account 3: 30
total: 225
```

The rules are:

- **Shares.** Read every line before the workers start, and number the transfers without
  a problem from 0. With W workers, worker 0 applies transfers 0, W, 2W, and so on, and
  worker 1 applies transfers 1, W + 1, 2W + 1, and so on. Start all the workers, and
  wait for them to finish.
- **A transfer.** Take the locks of both accounts, read both balances, yield, write the
  first balance less the amount and the second balance plus the amount, and release the
  locks. Yielding lets another worker run in the middle, as it could while a real bank
  waits on its database, so a mistake with the locks shows up in the tests instead of
  once a month.
- **Overdrafts.** A balance can go below 0. Every transfer is applied, and in any order
  the final balances come out the same.
- **A transfer to the same account** counts as a transfer, and its amount counts as
  moved, but it leaves the balance as it was.
- **Output.** The number of workers, accounts, and transfers, then a line for each
  worker with its transfers and the sum of their amounts, then each account's final
  balance, and the total of all the balances. Write `1 worker`, `1 account`, and
  `1 transfer`.

| Language | A worker is | To yield |
| --- | --- | --- |
| C | A thread from `pthread_create` | `sched_yield()` |
| C++ | A `std::jthread` | `std::this_thread::yield()` |
| Rust | A scoped thread from `thread::scope` | `thread::yield_now()` |
| Go | A goroutine | `runtime.Gosched()` |
| Python | A `threading.Thread` | `time.sleep(0)` |
| Kotlin | A thread from `thread { }` | `Thread.yield()` |
| JavaScript and TypeScript | An async function | `await new Promise((resolve) => setImmediate(resolve))` |
| Dart | An async function | `await Future(() {})` |

JavaScript, TypeScript, and Dart run every worker on one thread, so no two of them run
at the same moment, but a worker that awaits lets the others run, and they can read a
balance it is about to write. Their locks are async: taking one returns a promise, or a
Future, that completes once the lock is free. None of the three has one in its standard
library, so write a small `Lock` class.

A line with a problem prints one line. A problem in the first two lines stops the
program. A transfer with a problem is left out, and the others are still applied.

| Problem | Output |
| --- | --- |
| The first line isn't a whole number from 1 to 16 | `error: line 1 must be the number of workers, from 1 to 16` |
| The second line isn't one or more whole numbers | `error: line 2 must be the starting balances` |
| A transfer that isn't three whole numbers | `error: line 5: a transfer is three numbers: from, to, and amount` |
| An account that doesn't exist | `error: line 7: no account 3` |

Lines are numbered from 1. Skip blank lines after the second, and allow any number of
spaces between numbers. If both accounts of a transfer don't exist, report the first.
The tests use numbers under a million and fewer than 100,000 transfers.

```sh
python3 tools/check.py 16 --lang c
```

**Things to notice:**

- **Deadlock.** If every transfer takes the lock of the account it comes from and then
  the lock of the account it goes to, a transfer from 0 to 1 and a transfer from 1 to 0
  can each take their first lock and wait forever for the other's. The cure is to take
  the locks in one order everywhere, lower account first, so that no transfer waits for
  a lock lower than one it holds, and a ring of transfers, each waiting for the next,
  can't form. C++'s `std::scoped_lock` takes several locks at once without deadlock,
  whatever order other threads take them in, so the C++ solution doesn't sort them.
- **How a deadlock ends.** Go notices when every goroutine is blocked, and stops with
  `fatal error: all goroutines are asleep - deadlock!` and exit code 2, but only if
  every goroutine is stuck, so a deadlock in a server that is also waiting for requests
  goes unnoticed. Node stops when nothing is left to run, and prints
  `Warning: Detected unsettled top-level await` with exit code 13. Dart's program ends
  quietly, with no output after the deadlock and exit code 0. C, C++, Rust, Python, and
  Kotlin wait forever, and the test fails when it times out.
- **A transfer to the same account.** Taking a lock twice waits for itself forever in
  Go, Python, JavaScript, TypeScript, and Dart. In C and C++ it is undefined behavior,
  and Rust's documentation promises only that the second `lock` won't return, and on
  Linux all three wait forever. Kotlin's `ReentrantLock` lets a thread take a lock it
  holds, so it carries on, but reading both balances and then writing both gives the
  account `balance - amount` and then overwrites it with `balance + amount`, so the
  transfer makes money. This is the aliasing of chapter 9: two names, `source` and
  `target`, for one place.
- **Where the lock lives.** In Rust, each balance is inside its own `Mutex<i64>`, and
  the only way to reach it is through the guard that `lock` returns, so there is no way
  to forget a lock. In C, C++, Go, Python, and Kotlin, the lock and the balance are
  separate, and only a comment says which lock guards which balance. Go puts them in one
  struct, with the `sync.Mutex` embedded, which is the usual way to say it.
- **The GIL doesn't help.** Python's threads run one at a time, but `time.sleep(0)`
  gives up the GIL between the read and the write, so without the locks the totals come
  out wrong.
- **Async needs locks too.** With no locks, the JavaScript solution prints a total of
  275 for the example instead of 225, every time. One thread means no data races, but an
  `await` between a read and a write is a race condition all the same.

**Stretch:** some transfers are refused. Leave out a transfer that would take its first
account below 0, and count it as refused instead of moved. Now the order of the
transfers matters. Which outputs still come out the same on every run, and what would
you have to change to make the others repeatable?
