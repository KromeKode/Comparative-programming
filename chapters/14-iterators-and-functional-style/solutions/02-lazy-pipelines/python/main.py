import sys
from collections.abc import Callable, Iterator
from itertools import count, dropwhile, islice, repeat, takewhile


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    d = 2
    while d * d <= n:
        if n % d == 0:
            return False
        d += 1
    return True


# The sources are generators, so the endless ones cost nothing until asked.
def primes() -> Iterator[int]:
    return (n for n in count(2) if is_prime(n))


def fibonacci() -> Iterator[int]:
    a, b = 0, 1
    while True:
        yield a
        a, b = b, a + b


def source(words: list[str]) -> Iterator[int]:
    match words:
        case ["naturals"]:
            return count(1)
        case ["primes"]:
            return primes()
        case ["fibonacci"]:
            return fibonacci()
        case ["range", low, high]:
            return iter(range(int(low), int(high) + 1))
        case ["repeat", n]:
            return repeat(int(n))
    raise ValueError(f"unknown source {words}")


def predicate(words: list[str]) -> Callable[[int], bool]:
    match words:
        case ["even"]:
            return lambda x: x % 2 == 0
        case ["odd"]:
            return lambda x: x % 2 != 0
        case ["prime"]:
            return is_prime
        case ["above", n]:
            return lambda x: x > int(n)
        case ["below", n]:
            return lambda x: x < int(n)
    raise ValueError(f"unknown predicate {words}")


def function(words: list[str]) -> Callable[[int], int]:
    match words:
        case ["square"]:
            return lambda x: x * x
        case ["negate"]:
            return lambda x: -x
        case ["add", n]:
            return lambda x: x + int(n)
        case ["mul", n]:
            return lambda x: x * int(n)
    raise ValueError(f"unknown function {words}")


# Each stage wraps the iterator it is given in a new lazy one.
def stage(items: Iterator[int], words: list[str]) -> Iterator[int]:
    match words:
        case ["map", *rest]:
            return map(function(rest), items)
        case ["filter", *rest]:
            return filter(predicate(rest), items)
        case ["takewhile", *rest]:
            return takewhile(predicate(rest), items)
        case ["dropwhile", *rest]:
            return dropwhile(predicate(rest), items)
        case ["take", n]:
            return islice(items, int(n))
        case ["skip", n]:
            return islice(items, int(n), None)
        case ["step", n]:
            return islice(items, 0, None, int(n))
    raise ValueError(f"unknown stage {words}")


def run(line: str) -> str:
    parts = [part.split() for part in line.split("|")]
    items = source(parts[0])
    stages = parts[1:]
    last = stages[-1] if stages else []
    if last in (["sum"], ["count"]):
        stages = stages[:-1]
    for words in stages:
        items = stage(items, words)
    # Nothing has run yet. Only the line below pulls items through.
    if last == ["sum"]:
        return str(sum(items))
    if last == ["count"]:
        return str(sum(1 for _ in items))
    return " ".join(map(str, items)) or "(empty)"


def main() -> None:
    for line in sys.stdin:
        if line.strip():
            print(run(line))


if __name__ == "__main__":
    main()
