# Exercise 1.2: FizzBuzz

Read a whole number `n` (0 ≤ n ≤ 10,000) from the first line of stdin. Spaces around
it are allowed. Then print the numbers from 1 to `n`, one per line, except:

- multiples of 3 print `Fizz`
- multiples of 5 print `Buzz`
- multiples of both print `FizzBuzz`

If `n` is 0, print nothing.

Input `15` gives:

```text
1
2
Fizz
4
Buzz
Fizz
7
8
Fizz
Buzz
11
Fizz
13
14
FizzBuzz
```

```sh
python3 tools/check.py 01 --lang go
```

**Things to notice:** compare how each language turns the text `"15"` into the number
15. Which ones crash on bad input, which return an error value, and which quietly
return a special value such as `NaN`? Look at the Rust solution's `match` on a tuple
afterwards; chapter 4 covers it.
