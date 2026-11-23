NUMERALS = [
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
    """n in Roman numerals. Raises ValueError if n is outside 1 to 3999."""
    if n < 1 or n > 3999:
        raise ValueError(f"{n} is out of range")
    numeral = ""
    for value, letters in NUMERALS:
        while n >= value:
            numeral += letters
            n -= value
    return numeral
