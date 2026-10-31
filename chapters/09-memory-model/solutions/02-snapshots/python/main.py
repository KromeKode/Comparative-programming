import sys

type Grid = list[list[str]]


def clone(grid: Grid) -> Grid:
    # grid.copy() would copy only the outer list, and the copy would share its
    # rows with the grid. Copy each row too.
    return [row.copy() for row in grid]


def parse_number(text: str) -> int | None:
    if not (text.isascii() and text.isdigit()):
        print(f'error: "{text}" is not a number')
        return None
    return int(text)


def one_character(text: str) -> str | None:
    if len(text) != 1:
        print(f'error: "{text}" is not one character')
        return None
    return text


def takes(command: str, want: int, got: int) -> bool:
    if got != want:
        plural = "" if want == 1 else "s"
        print(f"error: {command} takes {want} value{plural}, got {got}")
    return got == want


def main() -> None:
    lines = sys.stdin.read().splitlines()
    if not lines:
        return
    width, height = map(int, lines[0].split())
    # [["."] * width] * height would put one row in every slot.
    grid = [["."] * width for _ in range(height)]
    saved: Grid | None = None

    for line in lines[1:]:
        parts = line.split()
        if not parts:
            continue
        command, values = parts[0], parts[1:]
        match command:
            case "paint":
                if not takes(command, 3, len(values)):
                    continue
                x = parse_number(values[0])
                if x is None:
                    continue
                y = parse_number(values[1])
                if y is None:
                    continue
                if not (1 <= x <= width and 1 <= y <= height):
                    print(
                        f"error: ({values[0]}, {values[1]}) is outside the "
                        f"{width} by {height} grid"
                    )
                    continue
                c = one_character(values[2])
                if c is not None:
                    grid[y - 1][x - 1] = c
            case "fill":
                if takes(command, 1, len(values)):
                    c = one_character(values[0])
                    if c is not None:
                        grid = [[c] * width for _ in range(height)]
            case "save":
                if takes(command, 0, len(values)):
                    saved = clone(grid)
            case "restore":
                if not takes(command, 0, len(values)):
                    continue
                if saved is None:
                    print("error: nothing saved")
                else:
                    # grid = saved would make the two names share one grid,
                    # and the next paint would change the snapshot as well.
                    grid = clone(saved)
            case "show":
                if takes(command, 0, len(values)):
                    for row in grid:
                        print("".join(row))
                    print()
            case _:
                print(f'error: unknown command "{command}"')


if __name__ == "__main__":
    main()
