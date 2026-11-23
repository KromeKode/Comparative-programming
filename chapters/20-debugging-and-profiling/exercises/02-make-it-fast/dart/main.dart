// Simulates a day of payments between bank accounts, and prints a summary. It gives
// the right answers, but slowly: ../README.md says what it prints, and how fast it
// has to be.
import 'dart:io';

const start = 1000; // what each account starts with

/// The Park-Miller generator, which every language's version uses, so that they all
/// make up the same transfers from the same seed.
class Random {
  Random(this.state);

  int state;

  int next() => state = state * 48271 % 2147483647;
}

class Account {
  Account(this.name, this.balance);

  final String name;
  int balance;
}

String accountName(int i) => 'acct${i.toString().padLeft(6, '0')}';

/// Gives the account with this name.
Account find(List<Account> accounts, String name) {
  for (final account in accounts) {
    if (account.name == name) return account;
  }
  throw StateError('no account called $name');
}

int total(List<Account> accounts) {
  var sum = 0;
  for (final account in accounts) {
    sum += account.balance;
  }
  return sum;
}

void main() {
  final numbers = stdin
      .readLineSync()!
      .trim()
      .split(RegExp(r'\s+'))
      .map(int.parse)
      .toList();
  final [count, transfers, seed] = numbers;
  final accounts = [
    for (var i = 0; i < count; i++) Account(accountName(i), start),
  ];

  final random = Random(seed);
  var refused = 0;
  for (var t = 0; t < transfers; t++) {
    final source = find(accounts, accountName(random.next() % count));
    final target = find(accounts, accountName(random.next() % count));
    final amount = random.next() % 500 + 1;
    if (source.balance < amount) {
      refused++;
    } else {
      source.balance -= amount;
      target.balance += amount;
    }
    // Money is never made or lost, so the total stays the same.
    if (total(accounts) != count * start) {
      stderr.writeln('error: the total has changed');
      exit(1);
    }
  }

  print('transfers: $transfers, refused: $refused');
  print('total: ${total(accounts)}');
  accounts.sort(
    (a, b) => a.balance != b.balance
        ? b.balance - a.balance
        : a.name.compareTo(b.name),
  );
  for (final account in accounts.take(5)) {
    print('${account.name} ${account.balance}');
  }
}
