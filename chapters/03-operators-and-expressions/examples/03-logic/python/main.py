calls = 0  # counts how many times the right side ran


def big_ratio(x: int) -> bool:
    global calls
    calls += 1
    return 10 // x > 1  # dividing by zero here would raise ZeroDivisionError


def display_name(name: str) -> str:
    # `or` returns its left operand if that is truthy, and its right operand otherwise.
    return name or "stranger"


def main() -> None:
    for x in [0, 5]:
        # `and` stops as soon as the left side is false, so big_ratio never sees 0.
        ok = x != 0 and big_ratio(x)
        print(f"x = {x}: {'yes' if ok else 'no'}")
    print(f"right side ran {calls} of 2 times")
    print(f'name "": {display_name("")}')
    print(f'name "Ada": {display_name("Ada")}')


if __name__ == "__main__":
    main()
