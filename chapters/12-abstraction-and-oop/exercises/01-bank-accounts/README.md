# Exercise 12.1: Bank accounts

Your program runs a small bank. Each line of input is one command.

| Command | What it does |
| --- | --- |
| `open KIND NAME` | Opens a `checking` or `savings` account for NAME, with a balance of 0 |
| `deposit NAME AMOUNT` | Adds AMOUNT to the balance |
| `withdraw NAME AMOUNT` | Takes AMOUNT from the balance, if the account's rules allow it |
| `month` | Ends the month for every account, in the order they were opened |

The two kinds of account have different rules:

| | Checking | Savings |
| --- | --- | --- |
| Withdrawals | May take the balance down to -100, and no lower | May not take the balance below 0, and at most 2 a month |
| Month end | If the balance is below 0, charge a fee of 5 | Add 1% of the balance as interest, rounded down, and start a new count of withdrawals |

Print one line for each command, or `error: ` and the reason:

| Input | Output |
| --- | --- |
| `open checking alice` | `alice: opened a checking account` |
| `deposit alice 100` | `alice: deposited 100, balance 100` |
| `withdraw alice 150` | `alice: withdrew 150, balance -50` |
| `withdraw alice 60` | `error: alice: over the overdraft limit of 100` |
| `open savings bob` | `bob: opened a savings account` |
| `deposit bob 250` | `bob: deposited 250, balance 250` |
| `withdraw bob 300` | `error: bob: insufficient funds` |
| `month` | `alice: fee 5, balance -55` |
| | `bob: interest 2, balance 252` |

The other errors are `error: unknown account kind: KIND`,
`error: NAME already has an account`, `error: no account named NAME`, and
`error: NAME: only 2 withdrawals a month`. Check the kind before the name, and a savings
account's count before its balance. A withdrawal that fails does not count toward the 2.
At month end, a checking account with a balance of 0 or more prints `no fee` in place of
`fee 5`, and the fee may take a balance below -100.

Every line is a well-formed command, and amounts are whole numbers from 1 to
1,000,000,000,000. Skip blank lines, and allow any number of spaces between words. The
tests use ASCII only, with at most 100 accounts and names under 32 characters.

Write a shared account type with `deposit` and `withdraw`, and give each kind of account
its own withdrawal rule and month end. The code that reads commands must never check
which kind of account it has.

```sh
python3 tools/check.py 12 --lang go
```

**Things to notice:**

- **The base class runs the steps, and each kind fills one in.** In the reference
  solutions with classes, `withdraw` is written once in `Account` and calls
  `check_withdrawal`, which each kind overrides. `Savings` also overrides `withdraw` to
  count, and calls the base version first: `super().withdraw` in Python,
  `Account::withdraw` in C++, and `super.withdraw` in JavaScript, TypeScript, Dart, and
  Kotlin. Kotlin methods are final unless marked `open`, so `Account` has to say
  `open fun withdraw`. Without it, the override is an error:
  `'withdraw' in 'Account' is final and cannot be overridden.`
- **An abstract class cannot be made by itself.** In Python, `Account("x")` raises a
  `TypeError` that begins `Can't instantiate abstract class Account` and names both
  missing methods. TypeScript, Dart, and C++ refuse it at compile time. JavaScript has
  no abstract methods, so the JavaScript `Account` gives each one a body that throws:
  `Gold must define monthEnd`.
- **Without inheritance, split what is shared from what differs.** The Rust solution has
  one `Account` struct, which holds the name, the balance, and a `Box<dyn Rules>` for
  the part that differs. `Rules` gives `after_withdrawal` a default body that does
  nothing, so only `Savings` writes one. The C solution puts the shared struct first in
  each kind's struct, as in section 12.4, and a `NULL` in the table of functions means
  "do nothing after a withdrawal". Go embeds a `base` struct, so `Name`, `Balance`, and
  `Deposit` are written once, but each kind writes its own `Withdraw`: a method of
  `base` cannot call a method of the struct that embeds it.
- **The kinds are a table.** `open` finds the kind in a map from its name to a way to
  make one: the class itself in Python, JavaScript, and TypeScript, `Checking.new` in
  Dart, `::Checking` in Kotlin, and a function in Go. Rust uses a `match`, and C and C++
  a chain of `if`s. A third kind needs its own class and one more entry in the table,
  and nothing else changes. In Rust, an `enum` with a `match` in each method would also
  work, since both kinds are known up front (chapter 8).
- **Money in whole numbers.** Interest is rounded down by dividing whole numbers: `//`
  in Python, `~/` in Dart, `Math.floor` in JavaScript and TypeScript, and `/` in the
  rest. A balance of 1,000,000,000,000 does not fit in 32 bits, so C and C++ use
  `long long`, Rust `i64`, Go `int64`, and Kotlin `Long`. Dart's `int` is 64 bits, and a
  JavaScript number holds every whole number up to 2^53 exactly.

**Stretch:** add a third kind, `student`: a checking account with an overdraft limit of
20 and no fee. Count how many places in your solution had to change. Then add
`transfer FROM TO AMOUNT`, which deposits into TO only if the withdrawal from FROM
works.
