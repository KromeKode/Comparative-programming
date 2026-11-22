import functools
import inspect
import sys
from collections.abc import Callable


class Tracer:
    """Counts calls, and prints each one when tracing is on."""

    def __init__(self) -> None:
        self.on = False
        self.depth = 0
        self.calls = 0

    def reset(self, on: bool) -> None:
        self.on, self.depth, self.calls = on, 0, 0

    def say(self, text: str) -> None:
        if self.on:
            print("  " * self.depth + text)


tracer = Tracer()

# Every traced function, by name, with the number of arguments it takes.
FUNCTIONS: dict[str, tuple[Callable[..., int], int]] = {}


def traced(func: Callable[..., int]) -> Callable[..., int]:
    """Replaces func with a wrapper that traces each call, and registers it."""

    @functools.wraps(func)
    def wrapper(*args: int) -> int:
        tracer.calls += 1
        tracer.say(f"{func.__name__}({', '.join(map(str, args))})")
        tracer.depth += 1
        try:
            result = func(*args)
        finally:
            tracer.depth -= 1
        tracer.say(f"-> {result}")
        return result

    # The number of arguments comes from the function's own signature.
    FUNCTIONS[func.__name__] = (wrapper, len(inspect.signature(func).parameters))
    return wrapper


# The recursive calls below look up the global names fib, gcd, and power as
# they run, and find the wrappers, so they are traced too.
@traced
def fib(n: int) -> int:
    return n if n < 2 else fib(n - 1) + fib(n - 2)


@traced
def gcd(a: int, b: int) -> int:
    return a if b == 0 else gcd(b, a % b)


@traced
def power(base: int, exponent: int) -> int:
    if exponent == 0:
        return 1
    if exponent % 2 == 0:
        half = power(base, exponent // 2)
        return half * half
    return base * power(base, exponent - 1)


def plural(count: int, word: str) -> str:
    return f"{count} {word}" + ("" if count == 1 else "s")


def run(line: str) -> None:
    command, name, *words = line.split()
    if command not in ("trace", "count"):
        print(f"unknown command: {command}")
        return
    if name not in FUNCTIONS:
        print(f"unknown function: {name}")
        return
    func, arity = FUNCTIONS[name]
    if len(words) != arity:
        print(f"{name} takes {plural(arity, 'argument')}, got {len(words)}")
        return
    args = [int(word) for word in words]
    tracer.reset(on=command == "trace")
    result = func(*args)
    call = f"{name}({', '.join(map(str, args))})"
    print(f"{call} = {result} in {plural(tracer.calls, 'call')}")


def main() -> None:
    for line in sys.stdin:
        if line.strip():
            run(line)


if __name__ == "__main__":
    main()
