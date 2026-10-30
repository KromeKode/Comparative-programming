def min_max(values: list[int]) -> tuple[int, int]:
    return min(values), max(values)  # the comma makes a tuple


def main() -> None:
    lowest, highest = min_max([3, -1, 4, 1, 5])  # unpacking
    print(f"min and max of [3, -1, 4, 1, 5]: {lowest} {highest}")

    quotient, remainder = divmod(17, 5)  # a built-in that returns a pair
    print(f"17 divided by 5: {quotient} remainder {remainder}")

    first, second = "hello", "world"
    first, second = second, first
    print(f"swapped: {first} {second}")


if __name__ == "__main__":
    main()
