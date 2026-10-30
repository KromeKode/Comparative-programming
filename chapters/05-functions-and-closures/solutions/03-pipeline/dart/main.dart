import 'dart:io';
import 'dart:math';

int Function(int) makeStep(String op, int n) => switch (op) {
  'add' => (x) => x + n,
  'sub' => (x) => x - n,
  'mul' => (x) => x * n,
  'min' => (x) => min(x, n),
  'max' => (x) => max(x, n),
  _ => throw FormatException('unknown step: $op'),
};

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final words = line.trim().split(RegExp(r'\s+'));
    if (words.first.isEmpty) continue; // a blank line
    final start = int.parse(words.first);
    final steps = [
      for (var i = 1; i + 1 < words.length; i += 2)
        makeStep(words[i], int.parse(words[i + 1])),
    ];
    var value = start;
    for (final step in steps) {
      value = step(value);
    }
    print('$start -> $value');
  }
}
