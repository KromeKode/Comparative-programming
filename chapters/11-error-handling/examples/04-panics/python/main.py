from dataclasses import dataclass


@dataclass
class User:
    name: str


numbers = [10, 20, 30]
users = {1: User("ada")}
index, zero = 5, 0


def find_user(user_id: int) -> User | None:
    return users.get(user_id)


def attempt(label, action):
    """Run action and report what it returned or which exception it raised."""
    try:
        print(f"{label}: {action()}")
    except Exception as error:
        print(f"{label}: {type(error).__name__}: {error}")


def check_length():
    assert len(numbers) == 4, "expected 4 numbers"
    return "passed"


# Each of these is a bug, and Python raises an ordinary exception for each.
attempt("numbers[5]", lambda: numbers[index])
attempt("10 // zero", lambda: 10 // zero)
# A type checker such as mypy reports this line: find_user can return None.
attempt("find_user(7).name", lambda: find_user(7).name)
attempt("assert len(numbers) == 4", check_length)
