# Exercise 19.2: Test an expiring cache

Each language's folder has a cache that forgets each value a fixed number of seconds
after it was put, and a test file with one test in it. The cache doesn't read the time
itself: it asks a clock, which it is given when it is made. The test that is there gives
it the computer's clock, which is enough to see a value come back, but not to see one
expire, unless the test waits for it. Write a fake clock, whose time the test sets, and
with it the rest of the tests. As in exercise 19.1, the checker runs them on the code as
given, where they must all pass, and then on six copies of it, each with one bug put in,
where they must fail.

For example, with a ttl, a time to live, of 10 seconds:

```text
the clock says   the test does         and gets
100              put("a", "1")
109              get("a")              "1"
110              get("a")              nothing: "a" expired at 100 + 10
```

This is everything the cache promises, and so everything your tests may rely on:

- **The clock.** The cache reads the time, in whole seconds, from its clock, whenever it
  needs to know it. The clock never goes back, so a test may move a fake clock forward
  or leave it where it is, but not move it back: what the cache does then isn't
  promised.
- **Put.** `put(key, value)` stores `value` under `key`, replacing any value that is
  there. An entry put when the clock says t is live until the clock says t + ttl: it is
  there at t + ttl − 1, and gone at t + ttl. Putting a key that is already there starts
  its ttl again.
- **Get.** `get(key)` gives the value of a live entry, and nothing for a key that was
  never put or has expired. Getting an entry doesn't change when it expires.
- **Size.** The size is how many entries are live.
- **Keys.** Each key has its own entry: putting one key, or one key expiring, doesn't
  change the others.
- **The ttl.** `ttl` is at least 1. When the cache frees the memory of an expired entry
  isn't promised, only that `get` and the size don't count it.

These are the files and the cache in each language, and how to run your tests yourself:

| Language | Write the tests in | The cache | Run the tests |
| --- | --- | --- | --- |
| C | `test_cache.c` | `cache_new(ttl, clock)`, `cache_put`, `cache_get`, which returns `NULL` for nothing, `cache_size`, and `cache_free` | `cc -std=c17 -Wall -Wextra cache.c test_cache.c -o tests && ./tests` |
| C++ | `cache_test.cpp` | `cache::Cache cache(ttl, clock)`, with `put`, `get`, which returns a `std::optional<std::string>`, and `size` | `c++ -std=c++20 -Wall -Wextra cache.cpp cache_test.cpp -o tests && ./tests` |
| Rust | `src/tests.rs` | `Cache::new(ttl, &clock)`, with `put`, `get`, which returns an `Option<&str>`, and `len` | `cargo test` |
| Go | `cache_test.go` | `New(ttl, clock)`, with `Put`, `Get`, which returns the value and whether there is one, and `Len` | `go test` |
| Python | `test_cache.py` | `Cache(ttl, clock)`, with `put`, `get`, which returns `None` for nothing, and `len(cache)` | `uv run pytest` |
| JavaScript | `cache.test.js` | `new Cache(ttl, clock)`, with `put`, `get`, which returns `undefined` for nothing, and `size` | `node --test` |
| TypeScript | `cache.test.ts` | The same as JavaScript | `node --test`, and `tsc` to check the types |
| Dart | `test/cache_test.dart` | `Cache(ttl, clock)`, with `put`, `get`, which returns `null` for nothing, and `length` | `dart test` |
| Kotlin | `src/test/kotlin/CacheTest.kt` | `Cache(ttl, clock)`, with `put`, `get`, which returns `null` for nothing, and `size` | `gradle test` |

And this is what a clock is in each language, so what your fake clock has to be:

| Language | A clock is |
| --- | --- |
| C | A `struct cache_clock`: a function, `now`, and a `void *context` that the cache passes to it, which can point at the time your test sets |
| C++ | An object of a class derived from `cache::Clock`, which overrides `now() const`. The cache keeps a reference to it, so it must outlive the cache |
| Rust | A value of a type that implements the `Clock` trait. The cache borrows it, and `now` takes `&self`, so the time has to be in something that can be changed through a shared reference, such as a `Cell<u64>` |
| Go | A value of any type with a `Now() int64` method. Give the cache a pointer, so that the test changes the clock the cache has, and not a copy |
| Python | Any object with a `now()` method that returns an `int`: `Clock` is a `Protocol`, so your class doesn't have to name it |
| JavaScript | Any object with a `now()` method |
| TypeScript | Any object with a `now(): number` method, which is what the `Clock` interface asks for |
| Dart | An object of a class that implements `Clock` |
| Kotlin | Anything that implements `Clock`, which is a `fun interface`, so a lambda will do: `Cache(10) { time }` reads a `var time` each time the cache asks |

The bugs are in the `tests/` folder, as in exercise 19.1. Write your tests before you
read them, and if the checker says your tests missed one, read that one and think about
what test would have caught it without knowing it was there.

```sh
python3 tools/check.py 19 --lang go
```

When every bug is caught, the checker prints
`passed on the code as given, caught 6 of 6 bugs` for this exercise.

**Things to notice:**

- **Why the clock is passed in.** If the cache called `time(NULL)` or `Date.now()`
  itself, a test would have to wait ten seconds to see an entry expire, and could still
  never ask what happens at exactly t + ttl − 1, since the real clock doesn't stop
  there. Giving the cache its clock is the one change that makes the time something a
  test can set. This is called **dependency injection**, and the clock is the
  dependency. It is the same move as the thermostat's sensor in section 19.6, with time
  as the thing outside the code.
- **Where the bugs are.** Most of the six are about time and about more than one
  operation: the second an entry expires, a get before it does, a second put of the same
  key, and a second key. Tests of one put and one get, at one time, catch none of them.
  Each test is a short story told with the clock: put, move the clock, look, move it
  again, and look again.
- **Each language's fake.** The fakes differ in the ways the languages do. C passes the
  time through `context`, the pointer that every C callback carries for this reason.
  Rust needs a `Cell`, because the cache holds a shared reference to the clock while the
  test changes it, and Go a pointer, for the same reason. Python, JavaScript, and
  TypeScript take any object with the right method, and Kotlin takes a lambda, since
  `Clock` has one function.
- **Other ways to fake time.** Code that reads the clock itself can still be tested, by
  replacing the clock from outside: `unittest.mock.patch("time.time")` or the
  time-machine library in Python, `mock.timers` in `node:test`, and `package:clock` in
  Dart, which code reads through a zone that a test can replace. They work without
  changing the code, which is why they exist, but a test that patches `time.time` breaks
  the day the code switches to `time.monotonic`, and a clock that is passed in says in
  the code's own signature what it depends on.

**Stretch:** test the cache against a **model**: a much simpler version of it that is
obviously right, such as a map from each key to its value and the time it was put, which
never forgets anything. With a fixed seed, make a long random sequence of puts, gets,
and moves of the clock, run it on both, and check after every step that `get` and the
size agree with what the model says. Which of the six bugs does it catch on its own?
This is **model-based testing**, which is how property-based tools such as Hypothesis
(its stateful testing), proptest (with proptest-state-machine), and fast-check (its
commands) test things that have state.
