from itertools import count, islice


# Both functions say when they run, so the output shows the order of the work.
def square(n: int) -> int:
    print(f"  square {n}")
    return n * n


def even(x: int) -> bool:
    print(f"  even? {x}")
    return x % 2 == 0


def main() -> None:
    numbers = [1, 2, 3, 4, 5]

    # Eager: a list comprehension builds the whole list before the next line.
    print("eager:")
    squares = [square(n) for n in numbers]
    evens = [x for x in squares if even(x)]
    print("  first two:", *evens[:2])

    # Lazy: generator expressions, in parentheses, describe the work. islice
    # pulls each number through both of them, and stops after two.
    print("lazy:")
    lazy_squares = (square(n) for n in numbers)
    lazy_evens = (x for x in lazy_squares if even(x))
    print("  first two:", *islice(lazy_evens, 2))

    # count(1) counts up forever, so only a lazy pipeline can use it.
    big_evens = (x for x in (n * n for n in count(1)) if x % 2 == 0 and x > 50)
    print("endless:", *islice(big_evens, 3))


if __name__ == "__main__":
    main()
