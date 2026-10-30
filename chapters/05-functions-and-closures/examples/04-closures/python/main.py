from collections.abc import Callable


def make_adder(n: int) -> Callable[[int], int]:
    def add(x: int) -> int:
        return x + n

    return add


def make_accumulator() -> Callable[[int], int]:
    total = 0

    def accumulate(x: int) -> int:
        nonlocal total  # without this, total += x would create a new local variable
        total += x
        return total

    return accumulate


def main() -> None:
    add5 = make_adder(5)
    add10 = make_adder(10)
    print("add5(10) =", add5(10))
    print("add10(10) =", add10(10))

    running = make_accumulator()
    print("running total:", running(3), running(5), running(2))

    made = []
    for i in range(3):
        made.append(lambda: i)  # captures the variable i, not its value now
    print("made in a loop:", *(f() for f in made))

    fixed = []
    for i in range(3):
        fixed.append(lambda i=i: i)  # a default value is evaluated now, once
    print("made in a loop, with i=i:", *(f() for f in fixed))


if __name__ == "__main__":
    main()
