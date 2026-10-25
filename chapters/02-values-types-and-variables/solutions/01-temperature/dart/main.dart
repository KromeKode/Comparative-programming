import 'dart:io';

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final parts = line.trim().split(RegExp(r'\s+'));
    if (parts.length != 2) continue; // a blank line
    final value = double.parse(parts[0]);
    if (parts[1] == 'C') {
      final f = value * 9 / 5 + 32;
      print('${value.toStringAsFixed(1)} C = ${f.toStringAsFixed(1)} F');
    } else {
      final c = (value - 32) * 5 / 9;
      print('${value.toStringAsFixed(1)} F = ${c.toStringAsFixed(1)} C');
    }
  }
}
