from collections.abc import Sequence
from dataclasses import dataclass
from typing import Any, Protocol


# A generic class (Python 3.12 syntax). T is a type parameter, checked by mypy
# and pyright. At run time it is ignored: a Stack holds anything.
class Stack[T]:
    def __init__(self) -> None:
        self._items: list[T] = []

    def push(self, item: T) -> None:
        self._items.append(item)

    def pop(self) -> T | None:
        return self._items.pop() if self._items else None

    def peek(self) -> T | None:
        return self._items[-1] if self._items else None

    def __len__(self) -> int:
        return len(self._items)


# A bound: T must have __lt__, so max() and < work on it.
class Ordered(Protocol):
    def __lt__(self, other: Any, /) -> bool: ...


def largest[T: Ordered](items: Sequence[T]) -> T | None:
    return max(items) if items else None


# Two type parameters. swap returns a Pair with them the other way round.
@dataclass(frozen=True)
class Pair[A, B]:
    first: A
    second: B

    def swap(self) -> "Pair[B, A]":
        return Pair(self.second, self.first)

    def __str__(self) -> str:
        return f"({self.first}, {self.second})"


def show[T](value: T | None) -> str:
    return "none" if value is None else str(value)


def report[T](name: str, stack: Stack[T]) -> None:
    popped = show(stack.pop())
    top = show(stack.peek())
    print(f"{name}: popped {popped}, top is {top}, {len(stack)} left")


def main() -> None:
    numbers = Stack[int]()
    numbers.push(3)
    numbers.push(1)
    numbers.push(4)
    report("numbers", numbers)

    words = Stack[str]()
    words.push("pear")
    words.push("fig")
    report("words", words)

    empty = Stack[float]()
    report("empty", empty)

    print(f"largest number: {show(largest([3, 1, 4, 1, 5]))}")
    print(f"largest word: {show(largest(['pear', 'fig', 'apple']))}")
    nothing: list[int] = []
    print(f"largest of nothing: {show(largest(nothing))}")

    pair = Pair(7, "seven")  # Pair[int, str], inferred
    print(f"{pair} swapped is {pair.swap()}")


if __name__ == "__main__":
    main()
