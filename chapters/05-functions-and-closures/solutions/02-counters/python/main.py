import sys
from collections.abc import Callable


def make_counter(start: int, step: int) -> Callable[[], int]:
    next_value = start

    def counter() -> int:
        nonlocal next_value
        value = next_value
        next_value += step
        return value

    return counter


def main() -> None:
    counters: dict[str, Callable[[], int]] = {}
    for line in sys.stdin:
        match line.split():
            case ["new", name, start, step]:
                counters[name] = make_counter(int(start), int(step))
            case ["next", name]:
                if name in counters:
                    print(f"{name}: {counters[name]()}")
                else:
                    print(f"{name}: no such counter")


if __name__ == "__main__":
    main()
