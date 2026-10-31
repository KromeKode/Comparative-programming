def main() -> None:
    numbers = [3, 1, 4, 1, 5]
    print("numbers:", numbers)  # a list prints as [3, 1, 4, 1, 5]
    print("length:", len(numbers))
    print(f"first: {numbers[0]}, last: {numbers[-1]}")  # -1 counts from the end

    numbers.append(9)
    print("append 9:", numbers)
    numbers.insert(1, 2)
    print("insert 2 at index 1:", numbers)
    removed = numbers.pop(0)
    print(f"remove index 0 (was {removed}):", numbers)

    print("contains 4:", "yes" if 4 in numbers else "no")
    print("index of 1:", numbers.index(1))  # ValueError if it is missing

    numbers.sort()
    print("sorted:", numbers)
    print("slice 1..4:", numbers[1:4])  # a new list, copied

    numbers.reverse()
    print("reversed:", numbers)


if __name__ == "__main__":
    main()
