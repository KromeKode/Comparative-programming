import 'dart:io';

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final words = line.split(RegExp(r'\s+')).where((word) => word.isNotEmpty);
    if (words.isEmpty) continue;
    // toSet() makes a LinkedHashSet, which keeps first occurrences in order.
    print(words.toSet().join(' '));
  }
}
