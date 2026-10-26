import math


def yes_no(condition: bool) -> str:
    return "yes" if condition else "no"


def main() -> None:
    a = "hello"
    b = "".join(["hel", "lo"])  # built separately, same characters
    print("strings with ==:", yes_no(a == b))
    # `is` asks a different question: are these the same object?
    print("strings with is:", yes_no(a is b))

    xs = [1, 2, 3]
    ys = [1, 2, 3]
    print("lists with ==:", yes_no(xs == ys))

    nan = math.nan
    print("NaN == NaN:", yes_no(nan == nan))


if __name__ == "__main__":
    main()
