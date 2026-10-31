class Bag:
    """A bag of items. add_all is written in terms of add."""

    def __init__(self) -> None:
        self._items: list[str] = []

    def add(self, item: str) -> None:
        self._items.append(item)

    def add_all(self, items: list[str]) -> None:
        for item in items:
            self.add(item)  # calls the subclass's add, if it has one

    def size(self) -> int:
        return len(self._items)


# Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag(Bag):
    def __init__(self) -> None:
        super().__init__()
        self.count = 0

    def add(self, item: str) -> None:
        self.count += 1
        super().add(item)

    def add_all(self, items: list[str]) -> None:
        self.count += len(items)
        super().add_all(items)  # which calls self.add, which counts again


# Composition: a CountingBag has a Bag, and passes the work on to it.
class ComposedCountingBag:
    def __init__(self) -> None:
        self._bag = Bag()
        self.count = 0

    def add(self, item: str) -> None:
        self.count += 1
        self._bag.add(item)

    def add_all(self, items: list[str]) -> None:
        self.count += len(items)
        self._bag.add_all(items)  # the inner Bag calls its own add

    def size(self) -> int:
        return self._bag.size()


bags: list[tuple[str, InheritedCountingBag | ComposedCountingBag]] = [
    ("inheritance", InheritedCountingBag()),
    ("composition", ComposedCountingBag()),
]
for label, bag in bags:
    bag.add_all(["apple", "pear", "plum"])
    bag.add("fig")
    print(f"{label}: the bag holds {bag.size()} items, the count says {bag.count}")
