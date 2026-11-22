"""Exercise 18.1: a stock module. This file is given: write stock.py so that it
runs. It reads the commands, and leaves keeping count to the module."""

import re
import sys

from stock import Stock

# An amount is a whole number from 1 to 999,999,999,999,999.
AMOUNT = re.compile(r"0*[1-9][0-9]{0,14}")


def main() -> None:
    stock = Stock()
    for number, raw in enumerate(sys.stdin.read().splitlines(), start=1):
        words = raw.split()
        if not words:
            continue
        match words:
            case ["add", item, amount] if AMOUNT.fullmatch(amount):
                stock.add(item, int(amount))
                print(f"added {int(amount)} {item}, now {stock.count(item)}")
            case ["take", item, amount] if AMOUNT.fullmatch(amount):
                if stock.take(item, int(amount)):
                    print(f"took {int(amount)} {item}, now {stock.count(item)}")
                else:
                    print(f"error: line {number}: only {stock.count(item)} {item} in stock")
            case ["count", item]:
                print(f"{item}: {stock.count(item)}")
            case ["list"]:
                items = [f"{item} {count}" for item, count in stock.items()]
                print("stock: " + (", ".join(items) if items else "empty"))
            case _:
                print(f"error: line {number}: expected add, take, count, or list")


if __name__ == "__main__":
    main()
