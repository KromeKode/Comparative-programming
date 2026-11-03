import sys
from collections import defaultdict
from collections.abc import Callable, Iterable
from typing import NamedTuple


class Grade(NamedTuple):
    name: str
    subject: str
    score: int


# itertools.groupby only groups items that sit next to each other, so a
# dictionary of lists is the usual way to group by a key.
def group_by(
    grades: Iterable[Grade], key: Callable[[Grade], str]
) -> dict[str, list[Grade]]:
    groups: defaultdict[str, list[Grade]] = defaultdict(list)
    for grade in grades:
        groups[key(grade)].append(grade)
    return groups


def report(grades: list[Grade]) -> list[str]:
    lines = ["subjects:"]
    for subject, group in sorted(group_by(grades, lambda g: g.subject).items()):
        scores = [g.score for g in group]
        best = max(scores)
        names = sorted(g.name for g in group if g.score == best)
        average = sum(scores) // len(scores)
        lines.append(
            f"  {subject}: average {average}, best {best} ({', '.join(names)})"
        )

    students = group_by(grades, lambda g: g.name)
    totals = {name: sum(g.score for g in group) for name, group in students.items()}
    ranked = sorted(totals.items(), key=lambda entry: (-entry[1], entry[0]))
    lines.append("ranking:")
    lines += [
        f"  {i}. {name} {total}" for i, (name, total) in enumerate(ranked, start=1)
    ]

    passed = sorted(
        name for name, group in students.items() if all(g.score >= 50 for g in group)
    )
    lines.append(f"passed everything: {', '.join(passed) or 'nobody'}")
    return lines


def main() -> None:
    grades = []
    for line in sys.stdin:
        words = line.split()
        if words:
            grades.append(Grade(words[0], words[1], int(words[2])))
    print("\n".join(report(grades)))


if __name__ == "__main__":
    main()
