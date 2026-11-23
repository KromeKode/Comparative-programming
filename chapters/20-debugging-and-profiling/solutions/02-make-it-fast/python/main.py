# Simulates a day of payments between bank accounts, and prints a summary.
# ../README.md says what it prints, and how fast it has to be.
import sys
from dataclasses import dataclass

START = 1000  # what each account starts with


class Random:
    """The Park-Miller generator, which every language's version uses, so that they
    all make up the same transfers from the same seed."""

    def __init__(self, seed: int) -> None:
        self.state = seed

    def next(self) -> int:
        self.state = self.state * 48271 % 2147483647
        return self.state


@dataclass
class Account:
    name: str
    balance: int


def account_name(i: int) -> str:
    return f"acct{i:06d}"


def total(accounts: list[Account]) -> int:
    return sum(account.balance for account in accounts)


def main() -> None:
    count, transfers, seed = map(int, sys.stdin.readline().split())
    accounts = [Account(account_name(i), START) for i in range(count)]
    # Finding an account by its name was the slowest part: a dict finds it at once,
    # where looking through the list took time in proportion to the accounts.
    by_name = {account.name: account for account in accounts}
    random = Random(seed)
    refused = 0
    for _ in range(transfers):
        source = by_name[account_name(random.next() % count)]
        target = by_name[account_name(random.next() % count)]
        amount = random.next() % 500 + 1
        if source.balance < amount:
            refused += 1
        else:
            source.balance -= amount
            target.balance += amount
    # Adding up every account after every transfer was the other slow part. A
    # transfer moves money and can't change the total, so checking it once, at the
    # end, catches the same bugs.
    if total(accounts) != count * START:
        sys.exit("error: the total has changed")

    print(f"transfers: {transfers}, refused: {refused}")
    print(f"total: {total(accounts)}")
    richest = sorted(accounts, key=lambda account: (-account.balance, account.name))
    for account in richest[:5]:
        print(account.name, account.balance)


if __name__ == "__main__":
    main()
