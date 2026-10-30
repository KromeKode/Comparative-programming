def classify(point: tuple[int, int]) -> str:
    match point:
        case (0, 0):
            return "origin"
        case (x, 0):  # x binds the first value
            return f"on the x axis at x = {x}"
        case (0, y):
            return f"on the y axis at y = {y}"
        case (x, y) if x == y:  # a guard
            return f"on the diagonal at {x}"
        case (x, y):
            return f"x = {x}, y = {y}"
    raise AssertionError("unreachable: the last case matches every pair")


def main() -> None:
    for x, y in [(0, 0), (3, 0), (0, -2), (4, 4), (-1, 5)]:
        print(f"({x}, {y}): {classify((x, y))}")


if __name__ == "__main__":
    main()
