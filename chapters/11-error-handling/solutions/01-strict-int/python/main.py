import sys

MAX = 2**31 - 1
MIN = -(2**31)


class ParseError(Exception):
    """The base class for every way parse_int can fail."""


class Empty(ParseError):
    def __init__(self) -> None:
        super().__init__("empty input")


class NoDigits(ParseError):
    def __init__(self) -> None:
        super().__init__("no digits after the sign")


class InvalidCharacter(ParseError):
    def __init__(self, character: str, position: int) -> None:
        super().__init__(f"invalid character '{character}' at position {position}")
        self.character = character
        self.position = position


class TooLarge(ParseError):
    def __init__(self) -> None:
        super().__init__(f"too large, the maximum is {MAX}")


class TooSmall(ParseError):
    def __init__(self) -> None:
        super().__init__(f"too small, the minimum is {MIN}")


def parse_int(text: str) -> int:
    """Parse a 32-bit signed integer: an optional sign, then digits 0 to 9."""
    if text == "":
        raise Empty()
    negative = text[0] == "-"
    start = 1 if text[0] in "+-" else 0
    if start == len(text):
        raise NoDigits()
    # Check every character first, so a bad one is reported even when the
    # digits before it are already too large.
    for index in range(start, len(text)):
        if text[index] not in "0123456789":
            raise InvalidCharacter(text[index], index + 1)
    # Python's integers never overflow, so the limits are checked at the end.
    value = 0
    for character in text[start:]:
        value = value * 10 + (ord(character) - ord("0"))
    if negative:
        value = -value
    if value > MAX:
        raise TooLarge()
    if value < MIN:
        raise TooSmall()
    return value


def main() -> None:
    for line in sys.stdin.read().splitlines():
        try:
            print(f'"{line}" -> {parse_int(line)}')
        except ParseError as error:
            print(f'"{line}" -> error: {error}')


if __name__ == "__main__":
    main()
