# Exercise 11.3: Retry, skip, or stop

Your program downloads a batch of files from a pretend server. Each line of input is
one file: its name, then the server's answer to each attempt, in order. If the program
tries more times than there are answers, the server keeps giving its last answer.

| Answer | Error | What to do |
| --- | --- | --- |
| `ok` | | The download worked |
| `timeout` | `timed out` | Try again |
| `busy` | `server busy` | Try again |
| `missing` | `not found` | Skip this file and go on to the next |
| `auth` | `login refused` | Stop the whole batch, because every other file would fail too |

Try each file at most 3 times. Before trying again, print `NAME: ERROR, retrying in
Ns`, where the wait starts at 1 second and doubles each time. The program only prints
the wait; it does not sleep. Then print how the file ended:

| Ending | Output |
| --- | --- |
| It worked | `a.txt: ok after 1 attempt` or `a.txt: ok after 3 attempts` |
| A file to skip | `a.txt: skipped: not found` |
| Three errors in a row that are worth retrying | `a.txt: failed: gave up after 3 attempts: server busy` |
| A reason to stop | `a.txt: stopped: login refused` |

A "gave up" line names the error from the last attempt. When the batch stops, the
files after it are not tried. At the end, print
`downloaded N, skipped N, failed N, not tried N`, where a stopped file counts as
failed. Skip blank lines, and allow any number of spaces between words.

| Input | Output |
| --- | --- |
| `index.html ok` | `index.html: ok after 1 attempt` |
| `style.css timeout ok` | `style.css: timed out, retrying in 1s` |
| | `style.css: ok after 2 attempts` |
| `script.js busy busy busy` | `script.js: server busy, retrying in 1s` |
| | `script.js: server busy, retrying in 2s` |
| | `script.js: failed: gave up after 3 attempts: server busy` |
| `font.woff auth` | `font.woff: stopped: login refused` |
| `icon.svg ok` | `downloaded 2, skipped 0, failed 2, not tried 1` |

Write a `download` function for one attempt and a `download_with_retries` function
that calls it, and give the errors a type. The decision to retry, skip, or stop must
come from the error's type, never from its message.

```sh
python3 tools/check.py 11 --lang go
```

**Things to notice:**

- **The error decides, so put the decision on the error.** Most reference solutions
  ask the error itself: a `retry` field or method in Python, C++, JavaScript,
  TypeScript, Dart, and Kotlin, `is_retryable` and `is_fatal` in Rust and C. Go
  checks with `errors.Is` against sentinel errors, so `isRetryable` is a function.
  Go's own `net.Error` once had a `Temporary()` method for this, and it is now
  deprecated: "Temporary errors are not well-defined." Name what the caller will do
  instead.
- **Giving up is a new error that wraps the last one.** `GaveUp` carries the attempt
  count and the final error, and its message ends with the final error's message. Go
  keeps the final error with an `Unwrap` method, JavaScript, TypeScript, and Kotlin as
  the `cause`, Python with `raise ... from`, and Rust, C, and Dart in a field. The C++
  solution keeps only the message, because copying the error into a `DownloadError`
  field would cut it down to the base class.
- **Wrapping changes what a check finds.** In Go, `errors.Is(err, ErrTimedOut)` is
  true for a `GaveUpError` that wraps a timeout, so ask whether it gave up before you
  ask what the error was. Python and C++ have the same trap the other way round: an
  `except DownloadError` or `catch (const DownloadError &)` that comes first also
  catches `GaveUp`.
- **Rethrow without changing the error.** When `download_with_retries` meets an error
  it does not retry, it passes it up as it is: `raise` in Python, `throw;` in C++,
  `rethrow` in Dart, `throw e` in Kotlin, and `throw error` in JavaScript and
  TypeScript. In C++, `throw error;` would throw a copy of the base class and lose the
  real type.
- **A `break` inside a `switch` leaves the `switch`.** In Go and Dart, stopping the
  batch from inside a `switch` takes a labeled loop and `break batch`. In Kotlin and
  Rust, a `break` inside `when` or `match` leaves the loop.

**Stretch:** real servers sometimes say how long to wait, with a `Retry-After` header.
Add an answer `busy:5` that means "busy, and wait 5 seconds", and use that wait
instead of the doubled one.
