import sys


def main() -> None:
    # A dict keeps its keys in the order they were added, which is the order the
    # groups must be printed in.
    groups: dict[str, list[str]] = {}
    for word in sys.stdin.read().split():
        word = word.lower()
        key = "".join(sorted(word))  # anagrams have the same letters once sorted
        groups.setdefault(key, []).append(word)

    for group in groups.values():
        print(" ".join(group))


if __name__ == "__main__":
    main()
