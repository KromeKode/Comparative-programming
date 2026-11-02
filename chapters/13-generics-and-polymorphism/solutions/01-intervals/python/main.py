import re
import sys
from collections.abc import Callable
from dataclasses import dataclass
from typing import Any, Protocol


class Ordered(Protocol):
    def __lt__(self, other: Any, /) -> bool: ...


class LineError(Exception):
    pass


# order=True compares the fields in order, which is the right order here:
# major first, then minor, then patch, each as a number.
@dataclass(frozen=True, order=True)
class Version:
    major: int
    minor: int
    patch: int

    def __str__(self) -> str:
        return f"{self.major}.{self.minor}.{self.patch}"


@dataclass(frozen=True)
class Interval[T: Ordered]:
    low: T
    high: T

    def __str__(self) -> str:
        return f"{self.low}..{self.high}"


def merge[T: Ordered](intervals: list[Interval[T]]) -> list[Interval[T]]:
    """Sort by the low ends, then join each interval to the last one if they overlap."""
    merged: list[Interval[T]] = []
    for interval in sorted(intervals, key=lambda each: each.low):
        if merged and not merged[-1].high < interval.low:
            last = merged[-1]
            merged[-1] = Interval(last.low, max(last.high, interval.high))
        else:
            merged.append(interval)
    return merged


def parse_int(text: str) -> int | None:
    return int(text) if re.fullmatch(r"-?[0-9]+", text) else None


def parse_word(text: str) -> str | None:
    return text if re.fullmatch(r"[a-z]+", text) else None


def parse_version(text: str) -> Version | None:
    parts = text.split(".")
    if len(parts) != 3 or not all(re.fullmatch(r"[0-9]+", part) for part in parts):
        return None
    return Version(int(parts[0]), int(parts[1]), int(parts[2]))


def run[T: Ordered](
    kind: str, parse: Callable[[str], T | None], words: list[str]
) -> str:
    """Parse every interval on a line with parse, merge them, and format the result."""
    intervals: list[Interval[T]] = []
    for word in words:
        ends = word.split("..")
        if len(ends) != 2:
            raise LineError(f'bad interval "{word}"')
        low, high = parse(ends[0]), parse(ends[1])
        if low is None:
            raise LineError(f'bad {kind} "{ends[0]}"')
        if high is None:
            raise LineError(f'bad {kind} "{ends[1]}"')
        if high < low:
            raise LineError(f"{word} is backwards")
        intervals.append(Interval(low, high))
    return " ".join([kind] + [str(interval) for interval in merge(intervals)])


# Each entry fixes T for one kind. The lambdas all have the same type, so
# one dict can hold them even though their T's differ.
KINDS: dict[str, Callable[[list[str]], str]] = {
    "int": lambda words: run("int", parse_int, words),
    "word": lambda words: run("word", parse_word, words),
    "version": lambda words: run("version", parse_version, words),
}


def main() -> None:
    for line in sys.stdin:
        words = line.split()
        if not words:
            continue
        try:
            if words[0] not in KINDS:
                raise LineError(f'unknown kind "{words[0]}"')
            print(KINDS[words[0]](words[1:]))
        except LineError as error:
            print(f"error: {error}")


if __name__ == "__main__":
    main()
