def main() -> None:
    s = "Zo\u00eb \U0001f44d\U0001f3fd"

    print("UTF-8 bytes:", s.encode("utf-8").hex(" ").upper())
    print("code points:", " ".join(f"U+{ord(c):04X}" for c in s))
    print("first 3 code points:", s[:3])  # indexes count code points


if __name__ == "__main__":
    main()
