// A sealed class: every subclass is in this file, so a switch over a
// WithdrawError can be checked for missing cases.
sealed class WithdrawError implements Exception {
  final String message;
  WithdrawError(this.message);
  @override
  String toString() => message;
}

class InvalidAmount extends WithdrawError {
  final int amount;
  InvalidAmount(this.amount) : super('invalid amount: $amount');
}

class InsufficientFunds extends WithdrawError {
  final int needed;
  final int available;
  InsufficientFunds(this.needed, this.available)
    : super('insufficient funds: need $needed, have $available');
}

class AccountFrozen extends WithdrawError {
  AccountFrozen() : super('account is frozen');
}

class Account {
  int balance;
  bool frozen = false;
  Account(this.balance);

  void withdraw(int amount) {
    if (amount <= 0) throw InvalidAmount(amount);
    if (frozen) throw AccountFrozen();
    if (amount > balance) throw InsufficientFunds(amount, balance);
    balance -= amount;
  }
}

void attempt(Account account, int amount) {
  try {
    account.withdraw(amount);
    print('withdraw $amount: ok, ${account.balance} left');
  } on WithdrawError catch (error) {
    print('withdraw $amount: error: $error');
    // Leave out a case and dart analyze reports it.
    switch (error) {
      case InsufficientFunds(:final available):
        account.withdraw(available);
        print('  took $available instead, ${account.balance} left');
      case AccountFrozen():
        print('  call the bank to unfreeze it');
      case InvalidAmount():
        break;
    }
  }
}

void main() {
  final account = Account(100);
  print('start with ${account.balance}');
  for (final amount in [30, -5, 100]) {
    attempt(account, amount);
  }
  print('freeze the account');
  account.frozen = true;
  attempt(account, 10);
}
