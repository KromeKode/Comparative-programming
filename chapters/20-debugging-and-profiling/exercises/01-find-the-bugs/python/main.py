# Prints a report of each student's scores. ../README.md says what it should print.
import sys
from pathlib import Path


def rounded_average(total: int, count: int) -> int:
    """Gives total / count, rounded to the nearest whole number, with halves up."""
    return (2 * total + count) // (2 * count)


def main() -> None:
    # Each student's scores, in the order the students first appear.
    scores: dict[str, list[int]] = {}
    # The input is the file named on the command line, if there is one, so that a
    # debugger can keep the terminal to itself.
    text = Path(sys.argv[1]).read_text() if len(sys.argv) > 1 else sys.stdin.read()
    for line in text.splitlines():
        fields = line.split()
        if fields:
            scores[fields[0]] = [int(field) for field in fields[1:]]

    best_name, best_average = "-", -1
    class_total, class_count = 0, 0
    total = 0
    for name, marks in scores.items():
        for mark in marks:
            total += mark
        class_total += total
        class_count += len(marks)
        average = rounded_average(total, len(marks))
        print(name, average)
        if average >= best_average:
            best_name, best_average = name, average

    if class_count == 0:
        print("class average: -")
    else:
        print("class average:", rounded_average(class_total, class_count))
    print("top:", best_name)


if __name__ == "__main__":
    main()
