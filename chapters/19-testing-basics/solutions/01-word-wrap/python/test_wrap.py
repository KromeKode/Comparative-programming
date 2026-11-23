import pytest

from wrap import wrap


@pytest.mark.parametrize(
    ("text", "width", "want"),
    [
        # Lines as long as they can be, and a last line that is shorter.
        ("the quick brown fox jumps", 10, ["the quick", "brown fox", "jumps"]),
        # A line can be exactly the width, and no longer: "ab cd" is 5.
        ("the quick brown", 9, ["the quick", "brown"]),
        ("ab cd", 4, ["ab", "cd"]),
        ("ab cd", 5, ["ab cd"]),
        # One space between words, whatever the text had.
        ("  the   quick  ", 20, ["the quick"]),
        # No words, no lines.
        ("", 10, []),
        ("   ", 10, []),
        # A word longer than the width gets a line of its own, wherever it is.
        ("extraordinary", 5, ["extraordinary"]),
        ("an extraordinary day", 5, ["an", "extraordinary", "day"]),
        ("a b", 1, ["a", "b"]),
    ],
)
def test_wrap(text: str, width: int, want: list[str]) -> None:
    assert wrap(text, width) == want
