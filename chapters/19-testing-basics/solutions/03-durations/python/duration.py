"""Durations such as 1h30m: parse_duration reads one, and format_duration writes one."""

UNITS = "dhms"  # largest first, which is the order they come in
SECONDS = [86400, 3600, 60, 1]  # in each of the units


def parse_duration(text: str) -> int:
    """The number of seconds that text says, such as 5400 for "1h30m".

    Raises ValueError if text isn't a duration.
    """
    if text == "":
        raise ValueError("a duration can't be empty")
    seconds = 0
    next_unit = 0  # each part's unit comes after the one before it in UNITS
    i = 0
    while i < len(text):
        start = i
        while i < len(text) and text[i] in "0123456789":
            i += 1
        if i == start:
            raise ValueError(f"{text!r} has a part with no number")
        if i == len(text):
            raise ValueError(f"{text!r} ends with a number that has no unit")
        number = int(text[start:i])
        unit = UNITS.find(text[i])
        if unit == -1:
            raise ValueError(f"{text[i]!r} isn't a unit")
        if unit < next_unit:
            raise ValueError(f"{text!r} has its units out of order, or one twice")
        seconds += number * SECONDS[unit]
        next_unit = unit + 1
        i += 1
    return seconds


def format_duration(seconds: int) -> str:
    """seconds as a duration, such as "1h30m" for 5400, leaving out the units that are 0."""
    if seconds == 0:
        return "0s"
    parts: list[str] = []
    for unit, size in zip(UNITS, SECONDS, strict=True):
        amount, seconds = divmod(seconds, size)
        if amount > 0:
            parts.append(f"{amount}{unit}")
    return "".join(parts)
