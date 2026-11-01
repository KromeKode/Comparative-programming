import sys
from abc import ABC, abstractmethod

OVERDRAFT_LIMIT = 100
OVERDRAFT_FEE = 5
SAVINGS_WITHDRAWALS = 2


class AccountError(Exception):
    pass


class Account(ABC):
    """What every account has. Each kind fills in its own rules."""

    kind = "account"

    def __init__(self, name: str) -> None:
        self.name = name
        self.balance = 0

    def deposit(self, amount: int) -> None:
        self.balance += amount

    def withdraw(self, amount: int) -> None:
        self.check_withdrawal(amount)
        self.balance -= amount

    @abstractmethod
    def check_withdrawal(self, amount: int) -> None: ...

    @abstractmethod
    def month_end(self) -> str: ...


class Checking(Account):
    kind = "checking"

    def check_withdrawal(self, amount: int) -> None:
        if self.balance - amount < -OVERDRAFT_LIMIT:
            raise AccountError(f"over the overdraft limit of {OVERDRAFT_LIMIT}")

    def month_end(self) -> str:
        if self.balance < 0:
            self.balance -= OVERDRAFT_FEE
            return f"fee {OVERDRAFT_FEE}"
        return "no fee"


class Savings(Account):
    kind = "savings"

    def __init__(self, name: str) -> None:
        super().__init__(name)
        self.withdrawals = 0

    def check_withdrawal(self, amount: int) -> None:
        if self.withdrawals == SAVINGS_WITHDRAWALS:
            raise AccountError(f"only {SAVINGS_WITHDRAWALS} withdrawals a month")
        if amount > self.balance:
            raise AccountError("insufficient funds")

    def withdraw(self, amount: int) -> None:
        super().withdraw(amount)
        self.withdrawals += 1

    def month_end(self) -> str:
        interest = self.balance // 100
        self.balance += interest
        self.withdrawals = 0
        return f"interest {interest}"


KINDS: dict[str, type[Account]] = {"checking": Checking, "savings": Savings}


def find(accounts: dict[str, Account], name: str) -> Account:
    if name not in accounts:
        raise AccountError(f"no account named {name}")
    return accounts[name]


def run(accounts: dict[str, Account], words: list[str]) -> None:
    match words:
        case ["open", kind, name]:
            if kind not in KINDS:
                raise AccountError(f"unknown account kind: {kind}")
            if name in accounts:
                raise AccountError(f"{name} already has an account")
            accounts[name] = KINDS[kind](name)
            print(f"{name}: opened a {kind} account")
        case ["deposit", name, amount]:
            account = find(accounts, name)
            account.deposit(int(amount))
            print(f"{name}: deposited {int(amount)}, balance {account.balance}")
        case ["withdraw", name, amount]:
            account = find(accounts, name)
            try:
                account.withdraw(int(amount))
            except AccountError as error:
                raise AccountError(f"{name}: {error}") from None
            print(f"{name}: withdrew {int(amount)}, balance {account.balance}")
        case ["month"]:
            for account in accounts.values():
                change = account.month_end()
                print(f"{account.name}: {change}, balance {account.balance}")


def main() -> None:
    accounts: dict[str, Account] = {}
    for line in sys.stdin:
        words = line.split()
        if not words:
            continue
        try:
            run(accounts, words)
        except AccountError as error:
            print(f"error: {error}")


if __name__ == "__main__":
    main()
