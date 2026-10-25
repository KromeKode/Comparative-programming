import sys


def main() -> None:
    name = sys.stdin.readline().strip()
    if name:
        print(f"Hello, {name}!")
    else:
        print("Hello, stranger!")


if __name__ == "__main__":
    main()
