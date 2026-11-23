# Counts the letters in its input, and draws a bar for each one. It expects lowercase
# text, and doesn't check: section 20.3 gives it capitals, to see what happens.
import sys


def count_letters(text: str) -> list[int]:
    counts = [0] * 26
    for c in text:
        if c.isalpha():
            counts[ord(c) - ord("a")] += 1
    return counts


def main() -> None:
    counts = count_letters(sys.stdin.read())
    for i, n in enumerate(counts):
        if n > 0:
            print(chr(ord("a") + i), n, "#" * n)


if __name__ == "__main__":
    main()
