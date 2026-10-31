import sys


def swap(words: list[str], i: int, j: int) -> None:
    # A function cannot reach the caller's variables, only the objects they
    # refer to, so swap takes the list and two indexes.
    words[i], words[j] = words[j], words[i]


def reverse(words: list[str], start: int, end: int) -> int:
    """Reverse words[start..end] and return how many swaps that took."""
    swaps = 0
    while start < end:
        swap(words, start, end)
        swaps += 1
        start += 1
        end -= 1
    return swaps


def parse_position(text: str, count: int) -> int | None:
    if not (text.isascii() and text.isdigit()):
        print(f'error: "{text}" is not a position')
        return None
    value = int(text)
    if not 1 <= value <= count:
        print(f"error: position {text} is out of range (1 to {count})")
        return None
    return value


def main() -> None:
    lines = sys.stdin.read().splitlines()
    words = lines[0].split() if lines else []
    swaps = 0
    for line in lines[1:]:
        parts = line.split()
        if not parts:
            continue
        command = parts[0]
        if command not in ("swap", "reverse"):
            print(f'error: unknown command "{command}"')
            continue
        if len(parts) != 3:
            print(f"error: {command} needs 2 positions, got {len(parts) - 1}")
            continue
        i = parse_position(parts[1], len(words))
        if i is None:
            continue
        j = parse_position(parts[2], len(words))
        if j is None:
            continue
        if command == "swap":
            swap(words, i - 1, j - 1)
            swaps += 1
        else:
            swaps += reverse(words, min(i, j) - 1, max(i, j) - 1)
        print(" ".join(words))
    print(f"swaps: {swaps}")


if __name__ == "__main__":
    main()
