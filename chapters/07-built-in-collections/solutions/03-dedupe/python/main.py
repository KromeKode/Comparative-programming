import sys


def main() -> None:
    for line in sys.stdin:
        words = line.split()
        if words:
            # A dict keeps the first occurrence of each key, in order, so its keys are
            # the answer. A set would lose the order.
            print(" ".join(dict.fromkeys(words)))


if __name__ == "__main__":
    main()
