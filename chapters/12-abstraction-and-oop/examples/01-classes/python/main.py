from functools import total_ordering
from math import gcd


@total_ordering
class Fraction:
    """A fraction kept in lowest terms, with the sign on the numerator."""

    __slots__ = ("_numerator", "_denominator")

    def __init__(self, numerator: int, denominator: int):
        if denominator == 0:
            raise ValueError("denominator is zero")
        if denominator < 0:
            numerator, denominator = -numerator, -denominator
        divisor = gcd(numerator, denominator)
        # A leading underscore means "private" by convention only.
        self._numerator = numerator // divisor
        self._denominator = denominator // divisor

    def _pair(self) -> tuple[int, int]:
        return (self._numerator, self._denominator)

    def __add__(self, other: "Fraction") -> "Fraction":
        a, b = self._pair()
        c, d = other._pair()
        return Fraction(a * d + c * b, b * d)

    # Lowest terms make equal fractions look the same, so == can compare fields.
    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Fraction):
            return NotImplemented
        return self._pair() == other._pair()

    # Objects that are equal must have equal hashes.
    def __hash__(self) -> int:
        return hash(self._pair())

    # total_ordering writes <=, >, and >= from __lt__ and __eq__.
    def __lt__(self, other: "Fraction") -> bool:
        a, b = self._pair()
        c, d = other._pair()
        return a * d < c * b

    def __str__(self) -> str:
        if self._denominator == 1:
            return str(self._numerator)
        return f"{self._numerator}/{self._denominator}"


def yes_no(value: bool) -> str:
    return "yes" if value else "no"


half, third = Fraction(1, 2), Fraction(1, 3)
print(f"1/2 + 1/3 = {half + third}")
print(f"2/-4 is stored as {Fraction(2, -4)}")
print(f"6/3 is stored as {Fraction(6, 3)}")
try:
    Fraction(1, 0)
except ValueError as error:
    print(f"1/0: error: {error}")
print(f"1/2 == 2/4: {yes_no(half == Fraction(2, 4))}")
print(f"1/3 < 1/2: {yes_no(third < half)}")
fractions = [Fraction(3, 4), third, Fraction(-1, 2), Fraction(2, 4)]
print("sorted:", ", ".join(str(f) for f in sorted(fractions)))
