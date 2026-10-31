import sys


def shift_letter(c: str, shift: int) -> str:
    """Shift an ASCII letter and leave every other character alone."""
    if "a" <= c <= "z":
        return chr((ord(c) - ord("a") + shift) % 26 + ord("a"))
    if "A" <= c <= "Z":
        return chr((ord(c) - ord("A") + shift) % 26 + ord("A"))
    return c


def main() -> None:
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        number, _, text = line.partition(" ")  # split at the first space only
        shift = int(number)  # Python's % is floored, so negative shifts just work
        print("".join(shift_letter(c, shift) for c in text))


if __name__ == "__main__":
    main()
