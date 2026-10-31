from dataclasses import dataclass


@dataclass
class User:
    name: str
    nickname: str | None = None  # None when the user has no nickname


def find(users: list[User], name: str) -> User | None:
    return next((user for user in users if user.name == name), None)


def main() -> None:
    users = [User("ada", "Countess"), User("alan"), User("grace", "Amazing Grace")]

    # Python has no ?. or ?? operator, so the checks for None are spelled out.
    for user in users:
        shown = user.nickname if user.nickname is not None else user.name
        print(f"{user.name} is shown as {shown}")

    lengths = [len(user.nickname) if user.nickname is not None else 0 for user in users]
    print("nickname lengths:", *lengths)

    for name in ["alan", "linus"]:
        found = find(users, name)
        print(f'find "{name}": {found.name if found is not None else "nobody"}')

    for name in ["ada", "alan", "linus"]:
        found = find(users, name)
        nickname = found.nickname if found is not None else None
        print(f'nickname of "{name}": {nickname if nickname is not None else "none"}')


if __name__ == "__main__":
    main()
