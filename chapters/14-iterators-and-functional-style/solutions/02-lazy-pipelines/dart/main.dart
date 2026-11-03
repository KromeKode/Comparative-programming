import 'dart:io';

bool isPrime(int n) {
  if (n < 2) return false;
  for (var d = 2; d * d <= n; d++) {
    if (n % d == 0) return false;
  }
  return true;
}

// The sources are sync* generators, so the endless ones cost nothing until asked.
Iterable<int> naturals() sync* {
  for (var n = 1; ; n++) {
    yield n;
  }
}

Iterable<int> fibonacci() sync* {
  var (a, b) = (0, 1);
  while (true) {
    yield a;
    (a, b) = (b, a + b);
  }
}

Iterable<int> range(int low, int high) sync* {
  for (var n = low; n <= high; n++) {
    yield n;
  }
}

Iterable<int> repeat(int n) sync* {
  while (true) {
    yield n;
  }
}

// Iterable has no step method, but indexed pairs each item with its position.
Iterable<int> step(Iterable<int> items, int n) =>
    items.indexed.where((pair) => pair.$1 % n == 0).map((pair) => pair.$2);

Iterable<int> source(List<String> words) => switch (words) {
  ['naturals'] => naturals(),
  ['primes'] => naturals().where(isPrime),
  ['fibonacci'] => fibonacci(),
  ['range', final low, final high] => range(int.parse(low), int.parse(high)),
  ['repeat', final n] => repeat(int.parse(n)),
  _ => throw ArgumentError('unknown source $words'),
};

bool Function(int) predicate(List<String> words) => switch (words) {
  ['even'] => (x) => x.isEven,
  ['odd'] => (x) => x.isOdd,
  ['prime'] => isPrime,
  ['above', final n] => (x) => x > int.parse(n),
  ['below', final n] => (x) => x < int.parse(n),
  _ => throw ArgumentError('unknown predicate $words'),
};

int Function(int) function(List<String> words) => switch (words) {
  ['square'] => (x) => x * x,
  ['negate'] => (x) => -x,
  ['add', final n] => (x) => x + int.parse(n),
  ['mul', final n] => (x) => x * int.parse(n),
  _ => throw ArgumentError('unknown function $words'),
};

// Each stage wraps the Iterable it is given in a new lazy one.
Iterable<int> stage(Iterable<int> items, List<String> words) => switch (words) {
  ['map', ...final rest] => items.map(function(rest)),
  ['filter', ...final rest] => items.where(predicate(rest)),
  ['takewhile', ...final rest] => items.takeWhile(predicate(rest)),
  ['dropwhile', ...final rest] => items.skipWhile(predicate(rest)),
  ['take', final n] => items.take(int.parse(n)),
  ['skip', final n] => items.skip(int.parse(n)),
  ['step', final n] => step(items, int.parse(n)),
  _ => throw ArgumentError('unknown stage $words'),
};

String run(String line) {
  final parts = line
      .split('|')
      .map((part) => part.trim().split(RegExp(r'\s+')))
      .toList();
  // A sum or count at the end is not a stage: it says what to do with the items.
  final finish = switch (parts.last) {
    ['sum'] || ['count'] => parts.removeLast().first,
    _ => 'list',
  };
  final items = parts.skip(1).fold(source(parts.first), stage);
  // Nothing has run yet. Only the lines below pull items through. Each use of
  // an Iterable starts it again from the source, so each runs it only once.
  final text = switch (finish) {
    'sum' => '${items.fold(0, (a, b) => a + b)}',
    'count' => '${items.length}',
    _ => items.join(' '),
  };
  return text.isEmpty ? '(empty)' : text;
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    if (line.trim().isNotEmpty) print(run(line));
  }
}
