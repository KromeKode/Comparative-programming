import sys


def is_leap(year: int) -> bool:
    return year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)


def main() -> None:
    for line in sys.stdin:
        text = line.strip()
        if not text:  # an empty string is falsy
            continue
        year = int(text)
        print(f"{year}: {'leap year' if is_leap(year) else 'common year'}")


if __name__ == "__main__":
    main()
