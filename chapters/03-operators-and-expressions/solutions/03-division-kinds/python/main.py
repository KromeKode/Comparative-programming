import sys


def main() -> None:
    for line in sys.stdin:
        parts = line.split()
        if len(parts) != 2:
            continue  # a blank line
        a, b = int(parts[0]), int(parts[1])
        # Python's // and % floor: they round toward negative infinity.
        fq, fm = divmod(a, b)
        # To truncate, divide the sizes and then put the sign back.
        # int(a / b) also works here, but goes through a float and breaks past 2^53.
        q = abs(a) // abs(b)
        if (a < 0) != (b < 0):
            q = -q
        r = a - b * q
        print(f"{a} {b} -> truncated {q} {r}, floored {fq} {fm}")


if __name__ == "__main__":
    main()
