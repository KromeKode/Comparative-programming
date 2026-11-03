from typing import NamedTuple


class Score(NamedTuple):
    name: str
    points: int


SCORES = [
    Score("ada", 91),
    Score("bob", 78),
    Score("cy", 85),
    Score("dee", 62),
    Score("eve", 99),
    Score("finn", 54),
]


def yes_no(value: bool) -> str:
    return "yes" if value else "no"


def main() -> None:
    # A list comprehension is a map and a filter in one expression.
    passed = [score.name for score in SCORES if score.points >= 70]
    print("passed:", ", ".join(passed))

    # Without the brackets it is a generator expression, which sum pulls from
    # one item at a time, and no list is built.
    print("total of the passes:", sum(s.points for s in SCORES if s.points >= 70))

    best = max(SCORES, key=lambda score: score.points)
    print(f"best: {best.name} with {best.points}")
    print("everyone passed:", yes_no(all(s.points >= 70 for s in SCORES)))
    print("anyone above 95:", yes_no(any(s.points > 95 for s in SCORES)))

    ranked = sorted(SCORES, key=lambda score: score.points, reverse=True)
    top = [f"{i}. {s.name} {s.points}" for i, s in enumerate(ranked[:3], start=1)]
    print("top three:", ", ".join(top))

    # Two for clauses flatten the names into letters, and {...} keeps one of each.
    letters = sorted({letter for score in SCORES for letter in score.name})
    print("letters used:", " ".join(letters))


if __name__ == "__main__":
    main()
