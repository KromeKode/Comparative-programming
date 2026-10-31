import 'dart:io';

const overdraftLimit = 100;
const overdraftFee = 5;
const savingsWithdrawals = 2;

class AccountError implements Exception {
  final String message;

  AccountError(this.message);

  @override
  String toString() => message;
}

// What every account has. An abstract class cannot be made directly, and each
// kind must fill in the methods that have no body.
abstract class Account {
  final String name;
  int balance = 0;

  Account(this.name);

  void deposit(int amount) {
    balance += amount;
  }

  void withdraw(int amount) {
    checkWithdrawal(amount);
    balance -= amount;
  }

  void checkWithdrawal(int amount);

  String monthEnd();
}

class Checking extends Account {
  Checking(super.name);

  @override
  void checkWithdrawal(int amount) {
    if (balance - amount < -overdraftLimit) {
      throw AccountError('over the overdraft limit of $overdraftLimit');
    }
  }

  @override
  String monthEnd() {
    if (balance < 0) {
      balance -= overdraftFee;
      return 'fee $overdraftFee';
    }
    return 'no fee';
  }
}

class Savings extends Account {
  int _withdrawals = 0;

  Savings(super.name);

  @override
  void checkWithdrawal(int amount) {
    if (_withdrawals == savingsWithdrawals) {
      throw AccountError('only $savingsWithdrawals withdrawals a month');
    }
    if (amount > balance) {
      throw AccountError('insufficient funds');
    }
  }

  @override
  void withdraw(int amount) {
    super.withdraw(amount);
    _withdrawals++;
  }

  @override
  String monthEnd() {
    final interest = balance ~/ 100;
    balance += interest;
    _withdrawals = 0;
    return 'interest $interest';
  }
}

// A constructor can be passed around as a function with ".new".
final Map<String, Account Function(String)> kinds = {
  'checking': Checking.new,
  'savings': Savings.new,
};

Account find(Map<String, Account> accounts, String name) =>
    accounts[name] ?? (throw AccountError('no account named $name'));

void run(Map<String, Account> accounts, List<String> words) {
  switch (words) {
    case ['open', final kind, final name]:
      final make = kinds[kind];
      if (make == null) {
        throw AccountError('unknown account kind: $kind');
      }
      if (accounts.containsKey(name)) {
        throw AccountError('$name already has an account');
      }
      accounts[name] = make(name);
      print('$name: opened a $kind account');
    case ['deposit', final name, final text]:
      final account = find(accounts, name);
      final amount = int.parse(text);
      account.deposit(amount);
      print('$name: deposited $amount, balance ${account.balance}');
    case ['withdraw', final name, final text]:
      final account = find(accounts, name);
      final amount = int.parse(text);
      try {
        account.withdraw(amount);
      } on AccountError catch (error) {
        throw AccountError('$name: $error');
      }
      print('$name: withdrew $amount, balance ${account.balance}');
    case ['month']:
      // A Dart map literal keeps its keys in the order they were added.
      for (final account in accounts.values) {
        final change = account.monthEnd();
        print('${account.name}: $change, balance ${account.balance}');
      }
  }
}

void main() {
  final accounts = <String, Account>{};
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final words = line.trim().split(RegExp(r'\s+'));
    if (words.first.isEmpty) continue;
    try {
      run(accounts, words);
    } on AccountError catch (error) {
      print('error: $error');
    }
  }
}
