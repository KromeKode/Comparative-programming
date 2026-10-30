def main() -> None:
    line = "up:"
    for i in range(5):  # stops before 5
        line += f" {i}"
    print(line)

    line = "down:"
    for i in range(5, 0, -1):  # stops before 0
        line += f" {i}"
    print(line)

    line = "by threes:"
    for i in range(0, 10, 3):
        line += f" {i}"
    print(line)

    line = "halving:"
    n = 100
    while n > 0:
        line += f" {n}"
        n //= 2
    print(line)

    colors = ["red", "green", "blue"]
    line = "colors:"
    for color in colors:
        line += f" {color}"
    print(line)

    line = "numbered:"
    for i, color in enumerate(colors, start=1):
        line += f" {i}={color}"
    print(line)

    # Python has no do-while: loop forever and break at the end.
    m = 100
    while True:
        m *= 2
        if m >= 50:
            break
    print(f"do-while: {m}")


if __name__ == "__main__":
    main()
