# Copies stdin to stdout, with each date in it written as YYYY-MM-DD.
import re
import sys

MONTHS = [
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December",
]  # fmt: skip
# Each month's name, or its first three letters: "Jan(?:uary)?", and so on.
MONTH = "|".join(f"{name[:3]}(?:{name[3:]})?" if name[3:] else name for name in MONTHS)
D, M, Y = "([0-9]{1,2})", "([0-9]{1,2})", "([0-9]{4})"
# re.ASCII makes \b look at ASCII letters, digits, and _ alone. Without it, \b would
# take "é" for a letter, and [0-9] is there because \d would take "٣" for a digit.
DATE = re.compile(
    rf"\b(?:{D}/{M}/{Y}|({MONTH}) {D}, {Y}|{D} ({MONTH}) {Y})\b",
    re.ASCII,
)


def days_in(month: int, year: int) -> int:
    if month == 2:
        leap = year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)
        return 29 if leap else 28
    return 30 if month in (4, 6, 9, 11) else 31


def rewrite(match: re.Match[str]) -> str:
    # Only one of the three forms matched, so the groups of the other two are None.
    g = match.groups()
    if g[0] is not None:
        day, month, year = int(g[0]), int(g[1]), int(g[2])
    else:
        name, day_text, year_text = (g[3], g[4], g[5]) if g[3] else (g[7], g[6], g[8])
        day, year = int(day_text), int(year_text)
        month = next(i for i, m in enumerate(MONTHS, 1) if m.startswith(name))
    if not (1 <= month <= 12 and 1 <= day <= days_in(month, year)):
        return match[0]  # not a real date, so left as it was
    return f"{year:04}-{month:02}-{day:02}"


def main() -> None:
    # Bytes, decoded and encoded by hand, so that nothing but the dates changes, whatever
    # the platform does with line ends in text mode.
    text = sys.stdin.buffer.read().decode("utf-8")
    sys.stdout.buffer.write(DATE.sub(rewrite, text).encode("utf-8"))


if __name__ == "__main__":
    main()
