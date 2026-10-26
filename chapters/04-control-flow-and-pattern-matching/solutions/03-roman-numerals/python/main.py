import sys

TABLE = [
    (1000, "M"),
    (900, "CM"),
    (500, "D"),
    (400, "CD"),
    (100, "C"),
    (90, "XC"),
    (50, "L"),
    (40, "XL"),
    (10, "X"),
    (9, "IX"),
    (5, "V"),
    (4, "IV"),
    (1, "I"),
]


def to_roman(n: int) -> str:
    out = ""
    for value, symbol in TABLE:
        while n >= value:  # take the biggest value that still fits
            out += symbol
            n -= value
    return out


def letter_value(letter: str) -> int:
    match letter:
        case "I":
            return 1
        case "V":
            return 5
        case "X":
            return 10
        case "L":
            return 50
        case "C":
            return 100
        case "D":
            return 500
        case "M":
            return 1000
        case _:
            raise ValueError(f"not a Roman numeral letter: {letter}")


def from_roman(text: str) -> int:
    values = [letter_value(letter) for letter in text]
    total = 0
    for i, value in enumerate(values):
        # A smaller letter before a bigger one is subtracted: IV is 5 - 1.
        if i + 1 < len(values) and value < values[i + 1]:
            total -= value
        else:
            total += value
    return total


def main() -> None:
    for line in sys.stdin:
        text = line.strip()
        if not text:
            continue
        if text.isdigit():
            print(f"{text} -> {to_roman(int(text))}")
        else:
            print(f"{text} -> {from_roman(text)}")


if __name__ == "__main__":
    main()
