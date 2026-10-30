import sys
from collections.abc import Callable


def make_step(op: str, n: int) -> Callable[[int], int]:
    match op:
        case "add":
            return lambda x: x + n
        case "sub":
            return lambda x: x - n
        case "mul":
            return lambda x: x * n
        case "min":
            return lambda x: min(x, n)
        case "max":
            return lambda x: max(x, n)
        case _:
            raise ValueError(f"unknown step: {op}")


def main() -> None:
    for line in sys.stdin:
        words = line.split()
        if not words:
            continue  # a blank line
        start = int(words[0])
        # words[1::2] are the operations and words[2::2] the numbers.
        steps = [make_step(op, int(n)) for op, n in zip(words[1::2], words[2::2])]
        value = start
        for step in steps:
            value = step(value)
        print(f"{start} -> {value}")


if __name__ == "__main__":
    main()
