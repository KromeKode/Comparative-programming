import pytest

from roman import to_roman


# parametrize makes one test of each pair, named after its values, such as
# test_numerals[1994-MCMXCIV], so a failure says which case it was.
@pytest.mark.parametrize(
    ("n", "want"),
    [
        (1, "I"),
        (4, "IV"),
        (9, "IX"),
        (14, "XIV"),
        (40, "XL"),
        (90, "XC"),
        (400, "CD"),
        (1994, "MCMXCIV"),
        (2024, "MMXXIV"),
        (3999, "MMMCMXCIX"),
    ],
)
def test_numerals(n: int, want: str) -> None:
    assert to_roman(n) == want


# The errors are part of what to_roman promises, so they get tests too.
@pytest.mark.parametrize("n", [0, 4000, -1])
def test_out_of_range(n: int) -> None:
    with pytest.raises(ValueError, match="out of range"):
        to_roman(n)
