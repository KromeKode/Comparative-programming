// Simulates a day of payments between bank accounts, and prints a summary.
// ../README.md says what it prints, and how fast it has to be.
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
  // Finding an account by its name was the slowest part: a Map finds it at once,
  // where looking through the list took time in proportion to the accounts.
  final byName = {for (final account in accounts) account.name: account};

  final random = Random(seed);
  var refused = 0;
  for (var t = 0; t < transfers; t++) {
    final source = byName[accountName(random.next() % count)]!;
    final target = byName[accountName(random.next() % count)]!;
    final amount = random.next() % 500 + 1;
    if (source.balance < amount) {
      refused++;
    } else {
      source.balance -= amount;
      target.balance += amount;
    }
  }
  // Adding up every account after every transfer was the other slow part. A transfer
  // moves money and can't change the total, so checking it once, at the end, catches
  // the same bugs.
  if (total(accounts) != count * start) {
    stderr.writeln('error: the total has changed');
    exit(1);
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
