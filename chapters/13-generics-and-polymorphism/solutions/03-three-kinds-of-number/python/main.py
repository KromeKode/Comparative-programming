import math
import re
import sys
from collections.abc import Callable
from dataclasses import dataclass
from typing import Protocol, Self


class CalcError(Exception):
    pass


# What eval needs from a number type. Self means "the same type as the one
# that has the method", so an Int adds only to an Int.
class Number(Protocol):
    def __add__(self, other: Self, /) -> Self: ...

    def __sub__(self, other: Self, /) -> Self: ...

    def __mul__(self, other: Self, /) -> Self: ...

    def __truediv__(self, other: Self, /) -> Self: ...


WHOLE = r"-?[0-9]+"


@dataclass(frozen=True)
class Int:
    value: int

    @classmethod
    def parse(cls, text: str) -> "Int | None":
        return cls(int(text)) if re.fullmatch(WHOLE, text) else None

    def __add__(self, other: "Int") -> "Int":
        return Int(self.value + other.value)

    def __sub__(self, other: "Int") -> "Int":
        return Int(self.value - other.value)

    def __mul__(self, other: "Int") -> "Int":
        return Int(self.value * other.value)

    def __truediv__(self, other: "Int") -> "Int":
        if other.value == 0:
            raise CalcError("division by zero")
        # Round toward zero, as C, Rust, Go, Java, and JavaScript do. Python's //
        # rounds down, so -7 // 2 would be -4.
        quotient = abs(self.value) // abs(other.value)
        return Int(quotient if (self.value < 0) == (other.value < 0) else -quotient)

    def __str__(self) -> str:
        return str(self.value)


@dataclass(frozen=True)
class Frac:
    top: int
    bottom: int

    @staticmethod
    def make(top: int, bottom: int) -> "Frac":
        """Keep every fraction in lowest terms with a positive bottom."""
        if bottom < 0:
            top, bottom = -top, -bottom
        divisor = math.gcd(top, bottom)
        return Frac(top // divisor, bottom // divisor)

    @classmethod
    def parse(cls, text: str) -> "Frac | None":
        match = re.fullmatch(f"({WHOLE})(?:/([0-9]+))?", text)
        if not match:
            return None
        bottom = int(match.group(2) or "1")
        return cls.make(int(match.group(1)), bottom) if bottom != 0 else None

    def __add__(self, other: "Frac") -> "Frac":
        return Frac.make(
            self.top * other.bottom + other.top * self.bottom,
            self.bottom * other.bottom,
        )

    def __sub__(self, other: "Frac") -> "Frac":
        return Frac.make(
            self.top * other.bottom - other.top * self.bottom,
            self.bottom * other.bottom,
        )

    def __mul__(self, other: "Frac") -> "Frac":
        return Frac.make(self.top * other.top, self.bottom * other.bottom)

    def __truediv__(self, other: "Frac") -> "Frac":
        if other.top == 0:
            raise CalcError("division by zero")
        return Frac.make(self.top * other.bottom, self.bottom * other.top)

    def __str__(self) -> str:
        return str(self.top) if self.bottom == 1 else f"{self.top}/{self.bottom}"


@dataclass(frozen=True)
class Mod7:
    value: int  # always 0 to 6

    @classmethod
    def parse(cls, text: str) -> "Mod7 | None":
        return cls(int(text) % 7) if re.fullmatch(WHOLE, text) else None

    def __add__(self, other: "Mod7") -> "Mod7":
        return Mod7((self.value + other.value) % 7)

    def __sub__(self, other: "Mod7") -> "Mod7":
        return Mod7((self.value - other.value) % 7)

    def __mul__(self, other: "Mod7") -> "Mod7":
        return Mod7(self.value * other.value % 7)

    def __truediv__(self, other: "Mod7") -> "Mod7":
        if other.value == 0:
            raise CalcError("division by zero")
        # x to the power 5 is the inverse of x when 7 is prime: x * x**5 = x**6 = 1.
        return self * Mod7(pow(other.value, 5, 7))

    def __str__(self) -> str:
        return str(self.value)


def evaluate[N: Number](parse: Callable[[str], N | None], tokens: list[str]) -> N:
    """Run the tokens on a stack of N. The same code works for every number type."""
    stack: list[N] = []
    operators: dict[str, Callable[[N, N], N]] = {
        "+": lambda a, b: a + b,
        "-": lambda a, b: a - b,
        "*": lambda a, b: a * b,
        "/": lambda a, b: a / b,
    }
    for token in tokens:
        if token in operators:
            if len(stack) < 2:
                raise CalcError("not enough numbers")
            right = stack.pop()
            left = stack.pop()
            stack.append(operators[token](left, right))
        else:
            number = parse(token)
            if number is None:
                raise CalcError(f'bad token "{token}"')
            stack.append(number)
    if len(stack) != 1:
        raise CalcError(f"expected one result, found {len(stack)}")
    return stack[0]


KINDS: dict[str, Callable[[list[str]], str]] = {
    "int": lambda tokens: str(evaluate(Int.parse, tokens)),
    "frac": lambda tokens: str(evaluate(Frac.parse, tokens)),
    "mod7": lambda tokens: str(evaluate(Mod7.parse, tokens)),
}


def main() -> None:
    for line in sys.stdin:
        words = line.split()
        if not words:
            continue
        try:
            if words[0] not in KINDS:
                raise CalcError(f'unknown kind "{words[0]}"')
            print(KINDS[words[0]](words[1:]))
        except CalcError as error:
            print(f"error: {error}")


if __name__ == "__main__":
    main()
