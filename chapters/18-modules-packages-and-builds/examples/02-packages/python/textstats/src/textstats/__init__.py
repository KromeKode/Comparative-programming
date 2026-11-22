"""The textstats package. A folder with an __init__.py is a package, and this
file is what importing it runs. A leading underscore marks a name as private,
but only by agreement: Python doesn't stop anyone using it."""

import re
from collections.abc import Iterator

VERSION = "1.0"


def _words(text: str) -> Iterator[str]:
    """The words in text: runs of ASCII letters."""
    return (match.group() for match in re.finditer(r"[A-Za-z]+", text))


def word_count(text: str) -> int:
    """The number of words in text."""
    return sum(1 for _ in _words(text))


def longest_word(text: str) -> str:
    """The first of the longest words in text."""
    return max(_words(text), key=len, default="")
