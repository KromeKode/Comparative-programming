def main() -> None:
    # // is floor division: it rounds down, toward negative infinity.
    print("7 // 2 =", 7 // 2)
    print("-7 // 2 =", -7 // 2)
    # % matches //, so the remainder takes the sign of the right operand.
    print("7 % 3 =", 7 % 3)
    print("-7 % 3 =", -7 % 3)
    print("7 % -3 =", 7 % -3)
    # / is always true division and always returns a float.
    print("7 / 2 =", 7 / 2)
    # int() truncates a float toward zero, which gives C's answer.
    print("int(-7 / 2) =", int(-7 / 2))


if __name__ == "__main__":
    main()
