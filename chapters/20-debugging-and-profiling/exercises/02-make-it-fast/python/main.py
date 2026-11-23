# Simulates a day of payments between bank accounts, and prints a summary. It gives
# the right answers, but slowly: ../README.md says what it prints, and how fast it
# has to be.
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


def find(accounts: list[Account], name: str) -> Account:
    """Gives the account with this name."""
    for account in accounts:
        if account.name == name:
            return account
    raise KeyError(name)


def total(accounts: list[Account]) -> int:
    return sum(account.balance for account in accounts)


def main() -> None:
    count, transfers, seed = map(int, sys.stdin.readline().split())
    accounts = [Account(account_name(i), START) for i in range(count)]
    random = Random(seed)
    refused = 0
    for _ in range(transfers):
        source = find(accounts, account_name(random.next() % count))
        target = find(accounts, account_name(random.next() % count))
        amount = random.next() % 500 + 1
        if source.balance < amount:
            refused += 1
        else:
            source.balance -= amount
            target.balance += amount
        # Money is never made or lost, so the total stays the same.
        if total(accounts) != count * START:
            sys.exit("error: the total has changed")

    print(f"transfers: {transfers}, refused: {refused}")
    print(f"total: {total(accounts)}")
    richest = sorted(accounts, key=lambda account: (-account.balance, account.name))
    for account in richest[:5]:
        print(account.name, account.balance)


if __name__ == "__main__":
    main()
