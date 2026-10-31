import dataclasses
from dataclasses import dataclass


# @dataclass writes __init__, __repr__, and __eq__ from the list of fields.
@dataclass
class Point:
    x: int
    y: int

    def steps_from_origin(self) -> int:
        return abs(self.x) + abs(self.y)

    def __str__(self) -> str:
        return f"({self.x}, {self.y})"


def main() -> None:
    p = Point(1, 2)
    print(f"p = {p}")
    print(f"p.x + p.y = {p.x + p.y}")

    # replace() makes a new Point with some fields changed.
    q = dataclasses.replace(p, x=5)
    print(f"q = p with x = 5: q = {q}, p = {p}")

    # Not a copy: r and p name the same object.
    r = p
    r.y = 9
    print(f"r = p, then r.y = 9: r = {r}, p = {p}")

    a = Point(3, -4)
    b = Point(3, -4)
    print(f"a = {a}, b = {b}, a == b: {'yes' if a == b else 'no'}")
    print(f"steps from (0, 0) to a: {a.steps_from_origin()}")


if __name__ == "__main__":
    main()
