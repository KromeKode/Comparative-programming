import sys
from functools import cache


@cache  # a decorator: it wraps fib in a function that remembers every answer
def fib(n: int) -> int:
    if n < 2:
        return n
    return fib(n - 1) + fib(n - 2)


def main() -> None:
    for line in sys.stdin:
        text = line.strip()
        if text:
            n = int(text)
            print(f"fib({n}) = {fib(n)}")


if __name__ == "__main__":
    main()
