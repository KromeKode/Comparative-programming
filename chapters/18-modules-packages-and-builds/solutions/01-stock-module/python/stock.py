"""The stock module: how many of each item there are."""


class Stock:
    """Counts of items. Only the methods below are meant for other modules; the
    leading underscore on _counts says that it is private, by agreement."""

    def __init__(self) -> None:
        self._counts: dict[str, int] = {}

    def add(self, item: str, amount: int) -> None:
        self._counts[item] = self._counts.get(item, 0) + amount

    def take(self, item: str, amount: int) -> bool:
        """Takes amount of item away, and returns True, if there are that many."""
        if self._counts.get(item, 0) < amount:
            return False
        self._counts[item] -= amount
        return True

    def count(self, item: str) -> int:
        return self._counts.get(item, 0)

    def items(self) -> list[tuple[str, int]]:
        """The items there are any of, with their counts, in alphabetical order."""
        return sorted((item, count) for item, count in self._counts.items() if count > 0)
