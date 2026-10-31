def set_first(items: list[int]) -> None:
    items[0] = -1  # the function gets the same list object, not a copy


def main() -> None:
    a = [1, 2, 3]

    b = a  # a second name for the same list
    b[0] = 99
    print("b = a, then b[0] = 99: a is", a)

    c = a.copy()  # or list(a), or a[:]
    c[0] = 0
    print("c = copy of a, then c[0] = 0: a is", a)

    set_first(a)
    print("set_first(a) sets a[0] = -1: a is", a)

    grid = [[0, 0], [0, 0]]
    g = grid.copy()  # a new outer list holding the same two rows
    g[0][0] = 1
    print("g = copy of grid, then g[0][0] = 1: grid is", grid)


if __name__ == "__main__":
    main()
