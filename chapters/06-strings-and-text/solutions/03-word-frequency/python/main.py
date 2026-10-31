import sys
from collections import Counter


def words(text: str) -> list[str]:
    """Split text into runs of ASCII letters and non-ASCII characters."""
    found = []
    current = []
    for c in text + " ":  # the extra space ends the last word
        if (c.isascii() and c.isalpha()) or not c.isascii():
            current.append(c.lower() if c.isascii() else c)
        elif current:
            found.append("".join(current))
            current = []
    return found


def main() -> None:
    counts = Counter(words(sys.stdin.read()))
    # Most frequent first; ties in code point order, which is how str compares.
    for word, count in sorted(counts.items(), key=lambda item: (-item[1], item[0])):
        print(count, word)


if __name__ == "__main__":
    main()
