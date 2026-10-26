from itertools import product

GRID = [[3, 8, 1], [9, 4, 7], [2, 6, 5]]


def main() -> None:
    for target in [7, 10]:
        checked = 0
        # Python has no labeled break, so turn the two loops into one.
        for r, c in product(range(len(GRID)), range(len(GRID[0]))):
            checked += 1
            if GRID[r][c] == target:
                print(f"{target}: row {r}, column {c} (checked {checked} cells)")
                break
        else:  # runs only when the loop ended without a break
            print(f"{target}: not found (checked {checked} cells)")


if __name__ == "__main__":
    main()
