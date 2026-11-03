from typing import Self


class Ring:
    """Keeps the last three items pushed into it, oldest first."""

    def __init__(self) -> None:
        self._items = [0, 0, 0]
        self._start = 0
        self._len = 0

    def push(self, item: int) -> None:
        self._items[(self._start + self._len) % 3] = item
        if self._len < 3:
            self._len += 1
        else:
            self._start = (self._start + 1) % 3  # the oldest item is overwritten

    # A for loop calls iter(ring), which calls __iter__ for a new iterator.
    def __iter__(self) -> "RingIterator":
        return RingIterator(self._items, self._start, self._len)


class RingIterator:
    def __init__(self, items: list[int], start: int, length: int) -> None:
        self._items = items
        self._start = start
        self._len = length
        self._index = 0

    # next(it) calls __next__, which raises StopIteration at the end.
    def __next__(self) -> int:
        if self._index == self._len:
            raise StopIteration
        item = self._items[(self._start + self._index) % 3]
        self._index += 1
        return item

    # An iterator is iterable too: it returns itself, so a for loop over it
    # carries on from where it is.
    def __iter__(self) -> Self:
        return self


def main() -> None:
    ring = Ring()
    for i in range(1, 6):
        ring.push(i)

    print("loop:", end="")
    for item in ring:
        print(f" {item}", end="")
    print()
    print("again:", " ".join(str(item) for item in ring))  # a new iterator each time

    it = iter(ring)
    parts = []
    while True:
        try:
            parts.append(str(next(it)))
        except StopIteration:
            break
    print("by hand:", ", then ".join(parts) + ", then nothing")

    it = iter(ring)
    first = next(it)
    print(f"resumed: took {first}, then the loop got", end="")
    for item in it:  # the loop carries on from where next left off
        print(f" {item}", end="")
    print()


if __name__ == "__main__":
    main()
