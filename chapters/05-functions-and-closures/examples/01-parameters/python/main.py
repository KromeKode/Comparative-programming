def greet(name: str, greeting: str = "Hello", punctuation: str = "!") -> str:
    return f"{greeting}, {name}{punctuation}"


# *values collects any number of positional arguments into a tuple.
def total(*values: int) -> int:
    return sum(values)


def main() -> None:
    print(greet("Ada"))
    print(greet("Grace", "Hi"))
    print(greet("Linus", punctuation="?"))  # a keyword argument skips greeting

    print("total() =", total())
    print("total(1, 2, 3) =", total(1, 2, 3))
    numbers = [4, 3, 2, 1]
    print("total of [4, 3, 2, 1] =", total(*numbers))  # * spreads a list


if __name__ == "__main__":
    main()
