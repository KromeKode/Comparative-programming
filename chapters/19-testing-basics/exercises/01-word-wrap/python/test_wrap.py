# Write tests for wrap here, as test_ functions. One is here to start with, and
# the README lists what wrap promises.
from wrap import wrap


def test_one_line() -> None:
    assert wrap("one two", 10) == ["one two"]
