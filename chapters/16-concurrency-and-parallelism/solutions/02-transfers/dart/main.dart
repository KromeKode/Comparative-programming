import 'dart:async';
import 'dart:io';

typedef Transfer = ({int source, int target, int amount});

String plural(int count, String noun) => '$count $noun${count == 1 ? '' : 's'}';

// The whole numbers on a line, or null if a word isn't one.
List<int>? numbers(String line) {
  final words = line.split(RegExp(r'\s+')).where((word) => word.isNotEmpty);
  final number = RegExp(r'^[0-9]+$');
  return words.every(number.hasMatch) ? words.map(int.parse).toList() : null;
}

// An async lock. acquire completes once every earlier holder has called
// release, and gives back the function that releases it.
class Lock {
  Future<void> _last = Future.value();

  Future<void Function()> acquire() {
    final released = Completer<void>();
    final acquired = _last.then((_) => released.complete);
    _last = released.future;
    return acquired;
  }
}

// Lets the other tasks run, as a real bank would while it waits on its
// database. Future(() {}) completes on a later turn of the event loop.
Future<void> pause() => Future(() {});

class Bank {
  final List<int> balances;
  // One lock for each account, so that transfers between different
  // accounts can wait on the database at the same time.
  final List<Lock> locks;

  Bank(this.balances) : locks = [for (final _ in balances) Lock()];

  Future<void> transfer(Transfer t) async {
    if (t.source == t.target) return; // a task would wait for itself forever
    // Every task takes the two locks in the same order, lower account
    // first, so no two tasks can each hold the lock the other wants.
    final releaseFirst = await locks[t.source < t.target ? t.source : t.target]
        .acquire();
    final releaseSecond = await locks[t.source < t.target ? t.target : t.source]
        .acquire();
    try {
      final sourceBalance = balances[t.source];
      final targetBalance = balances[t.target];
      await pause();
      balances[t.source] = sourceBalance - t.amount;
      balances[t.target] = targetBalance + t.amount;
    } finally {
      releaseSecond();
      releaseFirst();
    }
  }
}

(int, List<int>, List<Transfer>) parse(List<String> lines) {
  final first = numbers(lines.isEmpty ? '' : lines[0]);
  if (first == null || first.length != 1 || first[0] < 1 || first[0] > 16) {
    throw FormatException('line 1 must be the number of workers, from 1 to 16');
  }
  final balances = numbers(lines.length < 2 ? '' : lines[1]);
  if (balances == null || balances.isEmpty) {
    throw FormatException('line 2 must be the starting balances');
  }
  final transfers = <Transfer>[];
  for (var i = 2; i < lines.length; i++) {
    switch (numbers(lines[i])) {
      case []:
        break;
      case [final source, final target, final amount]:
        final missing = [source, target].where((a) => a >= balances.length);
        if (missing.isNotEmpty) {
          print('error: line ${i + 1}: no account ${missing.first}');
        } else {
          transfers.add((source: source, target: target, amount: amount));
        }
      default:
        print(
          'error: line ${i + 1}: a transfer is three numbers: from, to, and amount',
        );
    }
  }
  return (first[0], balances, transfers);
}

Future<void> main() async {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    lines.add(line!);
  }
  final int workers;
  final List<int> balances;
  final List<Transfer> transfers;
  try {
    (workers, balances, transfers) = parse(lines);
  } on FormatException catch (error) {
    print('error: ${error.message}');
    return;
  }
  final bank = Bank(balances);

  // Worker i applies transfers i, i + workers, i + 2 * workers, and so on.
  // The workers are async functions on this one isolate, and each one stops
  // at every await, so the others run while it waits.
  final shares = [
    for (var i = 0; i < workers; i++)
      [for (var j = i; j < transfers.length; j += workers) transfers[j]],
  ];
  await Future.wait([
    for (final share in shares)
      () async {
        for (final t in share) {
          await bank.transfer(t);
        }
      }(),
  ]);

  print(
    '${plural(workers, 'worker')}, ${plural(balances.length, 'account')}, '
    '${plural(transfers.length, 'transfer')}',
  );
  for (final (i, share) in shares.indexed) {
    final moved = share.fold(0, (sum, t) => sum + t.amount);
    print('worker $i: ${plural(share.length, 'transfer')}, $moved moved');
  }
  for (final (i, balance) in bank.balances.indexed) {
    print('account $i: $balance');
  }
  print('total: ${bank.balances.fold(0, (a, b) => a + b)}');
}
