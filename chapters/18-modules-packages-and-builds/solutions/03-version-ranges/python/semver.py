"""Versions and ranges as npm writes them: 1.2.3-beta.1, ^1.2, ~1.2.3, >=1 <2, 1.x || 3.

Everything starting with _ is private to this module, by agreement: main.py uses
only parse_version, parse_range, Version, Range, and highest.
"""

import re
from dataclasses import dataclass

_NUMBER = r"0|[1-9][0-9]{0,14}"
_IDENTIFIER = r"(?:0|[1-9][0-9]*|[0-9]*[A-Za-z-][0-9A-Za-z-]*)"
_PRE = rf"{_IDENTIFIER}(?:\.{_IDENTIFIER})*"
_VERSION = re.compile(rf"({_NUMBER})\.({_NUMBER})\.({_NUMBER})(?:-({_PRE}))?")
_PART = rf"(?:{_NUMBER}|[xX*])"
# A partial version, 1 or 1.2 or 1.2.3, with x, X, or * for any part. A pre-release
# can follow only three numbers, which _expand checks.
_PARTIAL = re.compile(rf"({_PART})(?:\.({_PART})(?:\.({_PART})(?:-({_PRE}))?)?)?")
_COMPARATOR = re.compile(r"(<=|>=|<|>|=|\^|~)?(.*)")
_WILD = ("x", "X", "*")


@dataclass(frozen=True)
class Version:
    major: int
    minor: int
    patch: int
    pre: tuple[str, ...] = ()

    def core(self) -> tuple[int, int, int]:
        return (self.major, self.minor, self.patch)

    def __str__(self) -> str:
        text = f"{self.major}.{self.minor}.{self.patch}"
        return text + ("-" + ".".join(self.pre) if self.pre else "")


def parse_version(text: str) -> Version | None:
    """The version that text is, or None if it isn't one."""
    match = _VERSION.fullmatch(text)
    if not match:
        return None
    pre = tuple(match[4].split(".")) if match[4] else ()
    return Version(int(match[1]), int(match[2]), int(match[3]), pre)


def _compare_identifiers(a: str, b: str) -> int:
    if a.isdigit() and b.isdigit():
        return (int(a) > int(b)) - (int(a) < int(b))
    if a.isdigit() != b.isdigit():
        return -1 if a.isdigit() else 1
    return (a > b) - (a < b)


def compare(a: Version, b: Version) -> int:
    """Negative if a comes before b, 0 if they are equal, and positive if after."""
    if a.core() != b.core():
        return -1 if a.core() < b.core() else 1
    if not a.pre or not b.pre:
        # A version without a pre-release comes after the same version with one.
        return (not a.pre) - (not b.pre)
    for x, y in zip(a.pre, b.pre):
        if (order := _compare_identifiers(x, y)) != 0:
            return order
    return (len(a.pre) > len(b.pre)) - (len(a.pre) < len(b.pre))


# A simple comparator: an operator, =, <, <=, >, or >=, and a full version.
_Simple = tuple[str, Version]
# Before every version, so < it matches nothing.
_FIRST = Version(0, 0, 0, ("0",))


def _expand(token: str) -> list[_Simple] | None:
    """The simple comparators that a comparator stands for, or None if it isn't one."""
    match = _COMPARATOR.fullmatch(token)
    assert match is not None  # (.*) matches anything
    op, rest = match.groups()
    partial = _PARTIAL.fullmatch(rest)
    if not partial:
        return None
    parts = [partial[1], partial[2], partial[3]]
    # How many parts are numbers. A wildcard can't come before a number.
    known = 0
    while known < 3 and parts[known] is not None and parts[known] not in _WILD:
        known += 1
    if any(part is not None and part not in _WILD for part in parts[known:]):
        return None
    if partial[4] and known < 3:
        return None
    pre = tuple(partial[4].split(".")) if partial[4] else ()
    nums = [int(part) for part in parts[:known]] + [0] * (3 - known)
    low = Version(nums[0], nums[1], nums[2], pre)

    def after(level: int) -> Version:
        """The first version after all those whose parts up to level match: the
        -0 pre-release of the next major, minor, or patch."""
        if level == 0:
            return Version(nums[0] + 1, 0, 0, ("0",))
        if level == 1:
            return Version(nums[0], nums[1] + 1, 0, ("0",))
        return Version(nums[0], nums[1], nums[2] + 1, ("0",))

    if known == 0:
        # Any version: nothing is outside it, and nothing is above or below it.
        return [("<", _FIRST)] if op in ("<", ">") else []
    if op in (None, "="):
        return [("=", low)] if known == 3 else [(">=", low), ("<", after(known - 1))]
    if op == ">":
        return [(">", low)] if known == 3 else [(">=", Version(*after(known - 1).core()))]
    if op == ">=":
        return [(">=", low)]
    if op == "<":
        return [("<", low)] if known == 3 else [("<", Version(*low.core(), ("0",)))]
    if op == "<=":
        return [("<=", low)] if known == 3 else [("<", after(known - 1))]
    if op == "~":
        return [(">=", low), ("<", after(0 if known == 1 else 1))]
    # ^ allows changes that don't change the first part that isn't 0.
    first = next((i for i in range(known) if nums[i] != 0), known - 1)
    return [(">=", low), ("<", after(first))]


def _holds(version: Version, simple: _Simple) -> bool:
    op, bound = simple
    order = compare(version, bound)
    return {"=": order == 0, "<": order < 0, "<=": order <= 0, ">": order > 0, ">=": order >= 0}[op]


class Range:
    """A range: sets of simple comparators joined by ||. A version is in the range
    if it passes every comparator of some set."""

    def __init__(self, sets: list[list[_Simple]]) -> None:
        self._sets = sets

    def allows(self, version: Version) -> bool:
        for simples in self._sets:
            if not all(_holds(version, simple) for simple in simples):
                continue
            # A pre-release is in only if a comparator of the set names a pre-release
            # of the same major, minor, and patch: >=1.0.0-beta allows 1.0.0-rc.1,
            # but not 1.1.0-rc.1, and >=1.0.0 allows neither.
            if version.pre and not any(
                bound.pre and bound.core() == version.core() for _, bound in simples
            ):
                continue
            return True
        return False


def parse_range(text: str) -> Range:
    """The range that text is. Raises ValueError, saying why, if it isn't one."""
    sets = []
    for side in text.split("||"):
        tokens = side.split()
        if not tokens:
            raise ValueError("each side of || needs a comparator")
        simples: list[_Simple] = []
        for token in tokens:
            expanded = _expand(token)
            if expanded is None:
                raise ValueError(f"{token} isn't a comparator")
            simples += expanded
        sets.append(simples)
    return Range(sets)


def highest(versions: list[Version], allowed: Range) -> Version | None:
    """The highest of versions that the range allows, or None if it allows none."""
    best = None
    for version in versions:
        if allowed.allows(version) and (best is None or compare(version, best) > 0):
            best = version
    return best
