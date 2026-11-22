import functools
import traceback
from collections.abc import Callable

TESTS: list[Callable[[], str | None]] = []


# A decorator is a function that takes a function and returns what should
# take its place. @test above a def runs test(that function) once, when the
# def runs, and binds the name to the result.
def test(body: Callable[[], None]) -> Callable[[], str | None]:
    # wraps copies the name and docstring of body onto run.
    @functools.wraps(body)
    def run() -> str | None:
        try:
            body()
        except AssertionError as error:
            # The traceback knows the file and line that failed, and can
            # read that line of source code back.
            line = traceback.extract_tb(error.__traceback__)[-1].line
            return (line or "").removeprefix("assert ")
        return None

    TESTS.append(run)
    return run


@test
def addition() -> None:
    assert 2 + 2 == 4
    assert -3 + 3 == 0


@test
def strings() -> None:
    word = "abc".upper()
    assert word == "ABC"
    assert len(word) == 3


@test
def rounding() -> None:
    assert 0.5 + 0.25 == 0.75
    assert 0.1 + 0.2 == 0.3


@test
def sorting() -> None:
    assert sorted([3, 1, 2]) == [1, 2, 3]


def main() -> None:
    failed = 0
    print(f"running {len(TESTS)} tests")
    for run in TESTS:
        failure = run()
        if failure is None:
            print(f"PASS {run.__name__}")
        else:
            print(f"FAIL {run.__name__}: {failure}")
            failed += 1
    print(f"{len(TESTS) - failed} passed, {failed} failed")
    # a real runner would exit with 1 when a test fails


if __name__ == "__main__":
    main()
