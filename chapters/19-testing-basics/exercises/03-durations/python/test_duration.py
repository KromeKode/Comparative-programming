# Write tests for parse_duration and format_duration here, as test_ functions. One is
# here to start with, and the README lists what they promise.
from duration import parse_duration


def test_parse() -> None:
    assert parse_duration("1h30m") == 5400
