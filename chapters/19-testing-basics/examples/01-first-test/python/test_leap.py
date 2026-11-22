# pytest runs every function whose name starts with test_, in every file whose
# name starts with test_. A plain assert is enough: when one fails, pytest shows
# the values on each side.
from leap import is_leap_year


def test_divisible_by_4() -> None:
    assert is_leap_year(2024)
    assert not is_leap_year(2023)


def test_centuries() -> None:
    assert not is_leap_year(1900)
    assert is_leap_year(2000)
