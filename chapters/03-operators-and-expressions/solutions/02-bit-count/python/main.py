import sys


def main() -> None:
    for line in sys.stdin:
        text = line.strip()
        if not text:
            continue
        n = int(text)
        print(f"{n}: {n:b} (ones {n.bit_count()}, width {n.bit_length()})")


if __name__ == "__main__":
    main()
