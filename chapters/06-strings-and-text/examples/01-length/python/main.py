def main() -> None:
    # A Python str is a sequence of code points; encode() turns it into bytes.
    s = "Zo\u00eb \U0001f44d\U0001f3fd"

    print("text:", s)
    print(f"built-in length: {len(s)} (code points)")
    print("UTF-8 bytes:", len(s.encode("utf-8")))
    print("UTF-16 code units:", len(s.encode("utf-16-le")) // 2)  # 2 bytes per unit
    print("code points:", len(s))


if __name__ == "__main__":
    main()
