import sys


def main() -> None:
    for line in sys.stdin:
        parts = line.split()
        if len(parts) != 2:
            continue  # a blank line
        value = float(parts[0])
        if parts[1] == "C":
            print(f"{value:.1f} C = {value * 9 / 5 + 32:.1f} F")
        else:
            print(f"{value:.1f} F = {(value - 32) * 5 / 9:.1f} C")


if __name__ == "__main__":
    main()
