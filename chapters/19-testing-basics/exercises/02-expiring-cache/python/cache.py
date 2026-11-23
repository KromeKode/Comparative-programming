"""A cache that keeps each value for a while after it was put, and then forgets it."""

import time
from dataclasses import dataclass
from typing import Protocol


class Clock(Protocol):
    """Something that tells the time, in whole seconds."""

    def now(self) -> int: ...


class SystemClock:
    """The computer's clock, which counts the seconds since 1970."""

    def now(self) -> int:
        return int(time.time())


@dataclass
class _Entry:
    value: str
    expires: int  # the time the entry stops being live

    def is_live(self, now: int) -> bool:
        return now < self.expires


class Cache:
    """Keeps each value for ttl seconds after it was put."""

    def __init__(self, ttl: int, clock: Clock) -> None:
        self._ttl = ttl
        self._clock = clock
        self._entries: dict[str, _Entry] = {}

    def put(self, key: str, value: str) -> None:
        """Stores value under key for the next ttl seconds, replacing any value there."""
        now = self._clock.now()
        self._entries = {k: e for k, e in self._entries.items() if e.is_live(now)}
        self._entries[key] = _Entry(value, now + self._ttl)

    def get(self, key: str) -> str | None:
        """The value under key, or None if there is none or it has expired."""
        entry = self._entries.get(key)
        if entry is None or not entry.is_live(self._clock.now()):
            return None
        return entry.value

    def __len__(self) -> int:
        """How many entries haven't expired."""
        now = self._clock.now()
        return sum(1 for entry in self._entries.values() if entry.is_live(now))
