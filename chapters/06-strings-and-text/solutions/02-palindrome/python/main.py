import sys


def is_palindrome(text: str) -> bool:
    # A str is a sequence of code points, so reversing it keeps characters whole.
    kept = [
        c.lower() if c.isascii() else c for c in text if not c.isascii() or c.isalnum()
    ]
    return kept == kept[::-1]


def main() -> None:
    for line in sys.stdin:
        text = line.strip()
        if text:
            verdict = "palindrome" if is_palindrome(text) else "not a palindrome"
            print(f"{text}: {verdict}")


if __name__ == "__main__":
    main()
