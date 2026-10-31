def show(items: set[int]) -> str:
    return " ".join(str(n) for n in sorted(items))  # a set has no order


def main() -> None:
    a = {1, 2, 3, 4}
    b = {3, 4, 5}
    print("a:", show(a))
    print("b:", show(b))
    print("union:", show(a | b))
    print("intersection:", show(a & b))
    print("a minus b:", show(a - b))
    print("3 in a:", "yes" if 3 in a else "no")

    print("unique of 3 1 3 2 1, sorted:", show(set([3, 1, 3, 2, 1])))

    # Tuples can be set members and dict keys because they cannot change.
    path = [(0, 0), (1, 0), (0, 0), (1, 1)]
    visited = set(path)
    print(f"visited {len(path)} points, {len(visited)} distinct")
    print("(1, 0) visited:", "yes" if (1, 0) in visited else "no")


if __name__ == "__main__":
    main()
