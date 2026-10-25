import sys

INT64_MIN = -(2**63)
INT64_MAX = 2**63 - 1


def main() -> None:
    for line in sys.stdin:
        parts = line.split()
        if len(parts) != 2:
            continue  # a blank line
        total = int(parts[0]) + int(parts[1])  # exact: Python ints never overflow
        print(total if INT64_MIN <= total <= INT64_MAX else "overflow")


if __name__ == "__main__":
    main()
