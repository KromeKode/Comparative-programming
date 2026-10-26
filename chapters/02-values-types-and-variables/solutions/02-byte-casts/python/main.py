import sys


def main() -> None:
    for line in sys.stdin:
        if not line.strip():
            continue
        x = int(line)
        u8 = x & 0xFF  # keep the lowest 8 bits; works for negatives too
        i8 = u8 - 256 if u8 >= 128 else u8  # read those bits as a signed number
        print(f"{x} -> u8 {u8}, i8 {i8}")


if __name__ == "__main__":
    main()
