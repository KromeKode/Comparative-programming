class WithdrawError(Exception):
    """The base class: catch it to handle every kind of withdraw failure."""


class InvalidAmount(WithdrawError):
    def __init__(self, amount: int):
        super().__init__(f"invalid amount: {amount}")
        self.amount = amount


class InsufficientFunds(WithdrawError):
    def __init__(self, needed: int, available: int):
        super().__init__(f"insufficient funds: need {needed}, have {available}")
        self.needed = needed
        self.available = available


class AccountFrozen(WithdrawError):
    def __init__(self):
        super().__init__("account is frozen")


class Account:
    def __init__(self, balance: int):
        self.balance = balance
        self.frozen = False

    def withdraw(self, amount: int) -> None:
        if amount <= 0:
            raise InvalidAmount(amount)
        if self.frozen:
            raise AccountFrozen()
        if amount > self.balance:
            raise InsufficientFunds(amount, self.balance)
        self.balance -= amount


def attempt(account: Account, amount: int) -> None:
    try:
        account.withdraw(amount)
        print(f"withdraw {amount}: ok, {account.balance} left")
    except InsufficientFunds as error:
        # The exception carries data, so the caller can recover.
        print(f"withdraw {amount}: error: {error}")
        account.withdraw(error.available)
        print(f"  took {error.available} instead, {account.balance} left")
    except AccountFrozen as error:
        print(f"withdraw {amount}: error: {error}")
        print("  call the bank to unfreeze it")
    except WithdrawError as error:
        print(f"withdraw {amount}: error: {error}")


account = Account(100)
print(f"start with {account.balance}")
for amount in [30, -5, 100]:
    attempt(account, amount)
print("freeze the account")
account.frozen = True
attempt(account, 10)
