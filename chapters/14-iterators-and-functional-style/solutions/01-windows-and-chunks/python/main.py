import sys
from collections import deque
from collections.abc import Iterable, Iterator


# Each adapter takes any iterable and pulls one item at a time, keeping only
# what it needs: at most k items for windows and chunks, one run for runs.
def windows(items: Iterable[int], k: int) -> Iterator[list[int]]:
    window: deque[int] = deque(maxlen=k)  # a full deque drops its oldest item
    for item in items:
        window.append(item)
        if len(window) == k:
            yield list(window)


def chunks(items: Iterable[int], k: int) -> Iterator[list[int]]:
    chunk: list[int] = []
    for item in items:
        chunk.append(item)
        if len(chunk) == k:
            yield chunk
            chunk = []
    if chunk:
        yield chunk


def runs(items: Iterable[int]) -> Iterator[list[int]]:
    run: list[int] = []
    for item in items:
        if run and item != run[-1]:
            yield run
            run = []
        run.append(item)
    if run:
        yield run


def show(groups: Iterable[list[int]]) -> str:
    return " ".join("[" + " ".join(map(str, group)) + "]" for group in groups)


def answer(words: list[str]) -> str:
    command = words[0]
    if command == "runs":
        text = show(runs(int(word) for word in words[1:]))
    else:
        k = int(words[1])
        numbers = (int(word) for word in words[2:])
        if command == "windows":
            text = show(windows(numbers, k))
        elif command == "chunks":
            text = show(chunks(numbers, k))
        else:  # sums: windows, then a sum of each
            text = " ".join(str(sum(window)) for window in windows(numbers, k))
    return text or "(none)"


def main() -> None:
    for line in sys.stdin:
        words = line.split()
        if words:
            print(answer(words))


if __name__ == "__main__":
    main()
