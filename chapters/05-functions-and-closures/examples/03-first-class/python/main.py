from collections.abc import Callable


def add_one(x: int) -> int:
    return x + 1


def triple(x: int) -> int:
    return x * 3


def apply_twice(f: Callable[[int], int], x: int) -> int:
    return f(f(x))


def compose(outer: Callable[[int], int], inner: Callable[[int], int]) -> Callable[[int], int]:
    def composed(x: int) -> int:
        return outer(inner(x))

    return composed


def main() -> None:
    print("apply_twice(add_one, 5) =", apply_twice(add_one, 5))
    print("apply_twice(triple, 5) =", apply_twice(triple, 5))
    square = lambda x: x * x  # noqa: E731 (a lambda: one expression, no name)
    print("apply_twice(square, 3) =", apply_twice(square, 3))
    add_then_triple = compose(triple, add_one)
    print("compose(triple, add_one)(5) =", add_then_triple(5))


if __name__ == "__main__":
    main()
