import copy


def add_five(items: list[int]) -> None:
    items.append(5)  # changes the caller's list, because items refers to it


def yes_no(flag: bool) -> str:
    return "yes" if flag else "no"


def main() -> None:
    # Every variable holds a reference. Assignment copies the reference.
    a = [1, 2, 3]
    b = a
    b.append(4)
    print(f"b = a, then b gets 4: a = {a}")

    add_five(a)
    print(f"a function appends 5: a = {a}")

    # Numbers and strings cannot change, so sharing them is never visible.
    n = 1
    m = n
    m += 1  # makes a new int and points m at it
    print(f"m = n, then m += 1: n = {n}, m = {m}")

    # A shallow copy is a new outer list holding the same inner lists.
    grid = [[1, 2], [3, 4]]
    shallow = grid.copy()
    shallow[0][0] = 9
    print(f"shallow copy, then copy[0][0] = 9: grid = {grid}")

    deep = copy.deepcopy(grid)
    deep[0][0] = 7
    print(f"deep copy, then copy[0][0] = 7: grid = {grid}")

    # * repeats the reference, so all three rows are one list.
    rows = [[0, 0]] * 3
    rows[0][0] = 1
    print(f"one row in 3 slots, then rows[0][0] = 1: rows = {rows}")

    # `is` asks "same object?"; == asks "equal items?".
    same = a.copy()
    print(f"a and b are the same list: {yes_no(a is b)}")
    print(f"a and its copy are the same list: {yes_no(a is same)}")
    print(f"a and its copy hold equal items: {yes_no(a == same)}")


if __name__ == "__main__":
    main()
