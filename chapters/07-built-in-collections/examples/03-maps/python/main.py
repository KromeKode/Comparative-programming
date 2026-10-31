def show(stock: dict[str, int]) -> str:
    # A dict remembers the order its keys were added in.
    return " ".join(f"{key}={value}" for key, value in stock.items())


def main() -> None:
    stock = {"apple": 3, "banana": 5}
    print("stock:", show(stock))

    stock["cherry"] = 7
    print("add cherry:", show(stock))

    print("banana:", stock["banana"])  # KeyError if the key is missing
    if "durian" not in stock:
        print(f"durian: {stock.get('durian', 0)} (missing)")

    stock["apple"] += 2
    print("apple += 2:", show(stock))
    del stock["banana"]
    print("remove banana:", show(stock))

    print("has cherry:", "yes" if "cherry" in stock else "no")
    print("size:", len(stock))


if __name__ == "__main__":
    main()
