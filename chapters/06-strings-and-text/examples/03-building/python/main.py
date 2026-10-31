def main() -> None:
    fruits = ["apple", "banana", "cherry"]
    print("joined:", ", ".join(fruits))

    # Strings are immutable, so build the pieces first and join them once.
    print("built:", "-".join(str(n) for n in range(1, 6)))

    print(f"padded: [{'Ada':<8}] [{90:>8}]")
    print(f"zero-padded: {42:05}")
    print(f"two decimals: {3.14159:.2f}")


if __name__ == "__main__":
    main()
