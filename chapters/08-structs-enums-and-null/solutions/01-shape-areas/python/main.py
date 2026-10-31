import math
import sys
from dataclasses import dataclass


@dataclass(frozen=True)
class Circle:
    radius: float


@dataclass(frozen=True)
class Rectangle:
    width: float
    height: float


@dataclass(frozen=True)
class Triangle:
    base: float
    height: float


type Shape = Circle | Rectangle | Triangle

# How many sizes each kind of shape needs.
SIZES_NEEDED = {"circle": 1, "rectangle": 2, "triangle": 2}


def area(shape: Shape) -> float:
    match shape:
        case Circle(radius):
            return math.pi * radius * radius
        case Rectangle(width, height):
            return width * height
        case Triangle(base, height):
            return base * height / 2


def parse_size(text: str) -> float:
    try:
        value = float(text)
    except ValueError:
        raise ValueError(f'"{text}" is not a number') from None
    if value < 0:
        raise ValueError(f'"{text}" is negative')
    return value


def parse(name: str, texts: list[str]) -> Shape:
    """Turns a shape name and its sizes into a Shape, or raises ValueError."""
    needed = SIZES_NEEDED.get(name)
    if needed is None:
        raise ValueError(f'unknown shape "{name}"')
    if len(texts) != needed:
        plural = "" if needed == 1 else "s"
        raise ValueError(f"{name} needs {needed} size{plural}, got {len(texts)}")
    sizes = [parse_size(text) for text in texts]
    match name:
        case "circle":
            return Circle(sizes[0])
        case "rectangle":
            return Rectangle(sizes[0], sizes[1])
        case _:
            return Triangle(sizes[0], sizes[1])


def main() -> None:
    total = 0.0
    for line in sys.stdin:
        words = line.split()
        if not words:
            continue
        try:
            shape = parse(words[0], words[1:])
        except ValueError as error:
            print(f"error: {error}")
            continue
        total += area(shape)
        print(f"{words[0]}: {area(shape):.2f}")
    print(f"total: {total:.2f}")


if __name__ == "__main__":
    main()
