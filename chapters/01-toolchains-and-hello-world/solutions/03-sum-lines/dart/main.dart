import 'dart:io';

void main() {
  var total = 0; // int is 64 bits when Dart runs natively
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break; // end of input
    final trimmed = line.trim();
    if (trimmed.isEmpty) continue;
    total += int.parse(trimmed);
  }
  print(total);
}
