def main() -> None:
    inputs = ["42", "4x2", "", "99999999999999999999"]

    # int raises an exception when it cannot parse. Python's ints have no
    # size limit, so the last one is fine.
    print("int:")
    for text in inputs:
        try:
            print(f"  {text!r} -> {int(text)}")
        except ValueError as error:
            print(f"  {text!r} -> ValueError: {error}")

    # Asking first is the other style. It needs a second pass over the text,
    # and isdigit misses signs, so Python code usually just tries.
    print("str.isdigit:")
    for text in inputs:
        print(f"  {text!r} -> {text.isdigit()}")


if __name__ == "__main__":
    main()
