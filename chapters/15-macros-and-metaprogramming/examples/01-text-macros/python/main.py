import inspect
from typing import Final

# Python has no macros. A constant is an ordinary name, and Final asks the
# type checker to refuse any later assignment to it.
LIMIT: Final = 3


def square(x: int) -> int:
    return x * x


clicks = 0


def bump_clicks() -> int:
    global clicks
    clicks += 1
    return clicks


def log(*parts: object) -> None:
    # A running program can look at its own call stack. stack()[1] is the
    # caller's frame, which knows the line it is on.
    caller = inspect.stack()[1]
    print(f"line {caller.lineno}: log:", *parts)


def main() -> None:
    # An f-string with = prints the expression's text as well as its value.
    print(f"{LIMIT * 2 = }")
    print(f"{square(1 + 2) = }")

    # square is a function, so its argument runs once.
    print(f"{square(bump_clicks()) = }")
    print(f"{clicks = }")
    log(clicks, "clicks")

    # __debug__ is Python's one compile-time switch. python3 -O sets it to
    # False, and the compiler then drops this if statement and every assert.
    if __debug__:
        print("__debug__ is on, so assert statements run")
    else:
        print("running under -O, so assert statements were dropped")


if __name__ == "__main__":
    main()
