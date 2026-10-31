import sys


def two_sum(target: int, numbers: list[int]) -> tuple[int, int] | None:
    """Return the pair with the smallest j, and for that j the smallest i."""
    # Each number seen so far, mapped to the first index where it appeared.
    first_index: dict[int, int] = {}
    for j, n in enumerate(numbers):
        i = first_index.get(target - n)
        if i is not None:
            return i, j
        first_index.setdefault(n, j)  # keeps the first index
    return None


def main() -> None:
    for line in sys.stdin:
        target, colon, rest = line.partition(":")
        if not colon:
            continue  # a blank line
        pair = two_sum(int(target), [int(word) for word in rest.split()])
        print(f"{pair[0]} {pair[1]}" if pair else "none")


if __name__ == "__main__":
    main()
