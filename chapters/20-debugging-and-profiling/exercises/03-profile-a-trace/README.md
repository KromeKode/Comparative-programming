# Exercise 20.3: Profile a trace

An instrumenting profiler, such as cProfile or Callgrind, records each time a function
is entered and each time it returns, and adds it all up into a profile: for each
function, how many calls it got, how long they took in all, and how much of that was in
the function itself. Write the part that adds it up. Given a trace of calls, print the
flat profile.

**Input.** Each line is an event, `TIME enter NAME` or `TIME exit NAME`, with its three
fields separated by spaces or tabs. TIME is the time of the event, 1 to 12 ASCII digits,
and the times never go down. NAME is the function's name, one or more ASCII characters
from `!` to `~`, so it can be `main.(*parser).expr` or `Parser::factor` but has no
spaces. An exit is from the function entered most recently that hasn't exited yet, and
at the end of the trace every call has exited. Blank lines are skipped, but they still
count as lines.

**Output.** A line for each function, `NAME CALLS TOTAL SELF`:

- **CALLS** is how many times the function was entered.
- **TOTAL** is the time from each call's entry to its exit, added up over its calls. A
  call made while another call to the same function is running, by recursion, isn't
  added again, since its time is already in the call around it.
- **SELF** is the time spent in the function itself: each call's time, less the time
  spent in the calls it made, added up over its calls. A recursive call's self time does
  count, since it is a time when that call, and no other, was running.

The lines are in order of self time, highest first, and of name on a tie, by character
code. A trace with no events prints `no calls`.

For example:

```text
0 enter main
5 enter parse
25 exit parse
25 enter eval
30 enter eval
45 exit eval
60 exit eval
70 exit main
```

prints:

```text
eval 2 35 35
parse 1 20 20
main 1 70 15
```

The outer `eval` runs from 25 to 60, and the inner one, which it called, runs from 30 to
45, inside it. So `eval`'s total is 35, not 35 + 15, and its self time is 20 for the
outer call, which is 35 less the 15 in the inner one, and 15 for the inner call, 35 in
all. `main` ran for 70, and 20 of that was in `parse` and 35 in `eval`, so 15 is its
own.

**Problems.** A problem prints one line, and nothing else: no profile of the lines
before it. The program checks the lines in order, and on each line checks its form, then
its time, then the exit, and reports the first problem it finds.

| Problem | Output |
| --- | --- |
| A line isn't three fields of the form above | `error: line N: expected TIME enter NAME or TIME exit NAME` |
| A time is less than the one before | `error: line N: time goes back from A to B` |
| An exit when no call is running | `error: line N: exit from NAME, but nothing is running` |
| An exit from a function that isn't the innermost one running | `error: line N: exit from NAME, but the innermost call is to OTHER` |
| A call is still running at the end of the trace | `error: end of trace: OTHER is still running`, with the innermost one |

Twelve digits keeps every time below 10^12, so a 64-bit integer holds any total, and so
does a JavaScript number, exactly.

```sh
python3 tools/check.py 20 --lang dart
```

**Things to notice:**

- **A stack of calls.** A trace is a walk through the call stack, and reading one needs
  a stack too: an enter pushes a call, and an exit pops it, and checks that it is the
  right one. Adding each call's time to the call below it on the stack, when it exits,
  is all it takes to work out self time.
- **Recursion and total time.** Adding up every call's time would count the inner `eval`
  twice, and a function that recursed ten deep would get a total ten times the time it
  ran. cProfile gets this right in the same way, and shows it in its call counts: `5/1`
  means five calls, of which one was the outermost.
- **Self time adds up, total time doesn't.** The self times add up to the length of the
  trace's calls, 70 in the example, since at each moment exactly one call is running its
  own code. The totals add up to more, since a moment in `parse` is in `main`'s total
  too. This is why a profile sorted by self time tells you where the time goes, and one
  sorted by total time tells you which calls to look inside.
- **Where traces come from.** Real tools write traces of the same shape. Chrome's trace
  event format, which the `chrome://tracing` and Perfetto viewers read, has `"ph": "B"`
  and `"ph": "E"` events, for begin and end. A program that writes one event per call is
  slower than one a sampling profiler watches, but nothing is missed: every call is
  counted, however short.

**Stretch:** print the trace's stacks in the folded form that flame graph tools read,
one line for each different stack, from the outermost call in, separated by `;`, with
the time spent in that stack's innermost call itself. The example gives:

```text
main 15
main;eval 20
main;eval;eval 15
main;parse 20
```

A flame graph draws each line as a bar as wide as its time, on top of the bar for the
stack it was called from, so the widest bars at the top are the functions to make
faster.
