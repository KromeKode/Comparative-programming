import sys


def main() -> None:
    # Python ints never overflow, so no size to choose.
    print(sum(int(line) for line in sys.stdin if line.strip()))


if __name__ == "__main__":
    main()
