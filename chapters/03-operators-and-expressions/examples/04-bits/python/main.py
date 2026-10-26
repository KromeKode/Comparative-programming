READ = 0b001
WRITE = 0b010
EXEC = 0b100


def main() -> None:
    perms = READ | WRITE  # turn bits on with |
    print(f"read | write: {perms:08b}")
    # & binds tighter than == in Python, so no parentheses are needed.
    print("has write?", "yes" if perms & WRITE != 0 else "no")
    print("has exec?", "yes" if perms & EXEC != 0 else "no")
    perms &= ~WRITE  # turn a bit off: AND with every bit except WRITE
    print(f"clear write: {perms:08b}")
    perms ^= EXEC  # flip a bit with ^
    print(f"toggle exec: {perms:08b}")
    print("5 << 2 =", 5 << 2)
    print("-16 >> 2 =", -16 >> 2)
    print("set bits in 10110:", (0b10110).bit_count())  # Python 3.10 and later


if __name__ == "__main__":
    main()
