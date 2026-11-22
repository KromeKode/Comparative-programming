import pytest

from duration import format_duration, parse_duration


@pytest.mark.parametrize(
    ("text", "seconds"),
    [
        ("45s", 45),
        ("1h30m", 5400),
        ("2d", 172800),
        ("1d2h3m4s", 93784),
        # A number can be more than the next unit up holds, and it can be 0.
        ("90m", 5400),
        ("0s", 0),
        ("1h0m", 3600),
    ],
)
def test_parse(text: str, seconds: int) -> None:
    assert parse_duration(text) == seconds


@pytest.mark.parametrize(
    "text",
    [
        "",  # no parts
        "90",  # a number with no unit
        "1h30",  # the same, after a part
        "h",  # a unit with no number
        "1x",  # not a unit
        "1h1h",  # a unit twice
        "30m1h",  # units out of order
        "1h 30m",  # anything else
        "-1h",
    ],
)
def test_parse_rejects(text: str) -> None:
    with pytest.raises(ValueError):
        parse_duration(text)


@pytest.mark.parametrize(
    ("seconds", "text"),
    [
        (5400, "1h30m"),
        (93784, "1d2h3m4s"),
        (59, "59s"),
        # Units that are 0 are left out, wherever they are, unless everything is.
        (3600, "1h"),
        (86401, "1d1s"),
        (0, "0s"),
    ],
)
def test_format(seconds: int, text: str) -> None:
    assert format_duration(seconds) == text


def test_round_trip() -> None:
    for seconds in range(100_000):
        assert parse_duration(format_duration(seconds)) == seconds
