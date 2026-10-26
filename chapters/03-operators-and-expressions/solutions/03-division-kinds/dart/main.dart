import 'dart:io';

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final parts = line.trim().split(RegExp(r'\s+'));
    if (parts.length != 2) continue; // a blank line
    final a = int.parse(parts[0]);
    final b = int.parse(parts[1]);
    final q = a ~/ b; // ~/ truncates toward zero
    final r = a.remainder(b); // remainder() truncates too
    // Dart's % is Euclidean, not floored, so compute the floored pair directly.
    final fq = (a / b).floor();
    final fm = a - b * fq;
    print('$a $b -> truncated $q $r, floored $fq $fm');
  }
}
