def yes_no(condition: bool) -> str:
    return "yes" if condition else "no"


def main() -> None:
    text = "  The quick  brown fox  "

    trimmed = text.strip()
    print(f"trimmed: [{trimmed}]")
    # split() with no argument splits on runs of whitespace.
    print("words:", "|".join(trimmed.split()))
    print("upper:", trimmed.upper())
    print('contains "quick":', yes_no("quick" in trimmed))
    print('index of "brown":', trimmed.find("brown"))  # -1 if missing; index() raises
    print("replaced:", trimmed.replace("quick", "slow"))
    print('starts with "The":', yes_no(trimmed.startswith("The")))

    fruits = ["banana", "apple", "Cherry"]
    print("sorted:", " ".join(sorted(fruits)))  # compares code points
    print("sorted ignoring case:", " ".join(sorted(fruits, key=str.lower)))


if __name__ == "__main__":
    main()
